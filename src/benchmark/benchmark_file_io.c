#include <errno.h>
#include <fcntl.h>
#include <pthread.h>
#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>

#include <rocket_executor.h>
#include <rocket_engine.h>
#include "benchmark_common.h"

typedef struct {
  int (*openat)(int dirfd, const char *pathname, int oflag, mode_t mode);
  ssize_t (*readat)(int fd, void *buf, size_t nbyts, off_t offset);
  ssize_t (*writeat)(int fd, const void *buf, size_t nbyts, off_t offset);
  int (*close)(int fd);
} file_io_dispatch_table_t;

typedef struct {
  int num_threads;
  int num_cycles_per_thread;
  int num_bytes_per_io;
} params_t;

typedef struct {
  const char *filename_prefix;
  int thread_num;
  int cycles;
  size_t io_bytes;
  file_io_dispatch_table_t io_dispatch_table;
} file_io_context_t;

static file_io_dispatch_table_t sync_io_dispatch_table = {
    .openat = &openat,
    .readat = &pread,
    .writeat = &pwrite,
    .close = &close,
};

static file_io_dispatch_table_t async_io_dispatch_table = {
    .openat = &openat_await,
    .readat = &readat_await,
    .writeat = &writeat_await,
    .close = &close_await,
};

static void print_params(const void *params_in) {
  const params_t *params = params_in;
  fprintf(stdout, "[%d threads, %d IO cycles per thread, %d bytes per IO]\n",
          params->num_threads, params->num_cycles_per_thread,
          params->num_bytes_per_io);
}

static void *file_io_func(void *context_in) {
  file_io_context_t *context = context_in;
  char filename[128] = {0};
  snprintf(filename, sizeof(filename), "%s%d", context->filename_prefix,
           context->thread_num);

  ssize_t ret = 0;
  uint8_t *write_buf = malloc(context->io_bytes);
  if (write_buf == NULL) {
    fprintf(stderr, "Failed to allocate write buffer\n");
    return (void *)-1;
  }
  uint8_t *read_buf = malloc(context->io_bytes);
  if (read_buf == NULL) {
    free(write_buf);
    fprintf(stderr, "Failed to allocate read buffer\n");
    return (void *)-1;
  }

  for (int i = 0; i < context->cycles; i++) {
    // Open
    int fd = context->io_dispatch_table.openat(AT_FDCWD, filename,
                                               O_CREAT | O_RDWR, 0644);
    if (fd < 0) {
      fprintf(stderr, "Failed to open file %s: %s\n", filename,
              strerror(errno));
      ret = -1;
      goto cleanup;
    }
    // Write
    ssize_t bytes =
        context->io_dispatch_table.writeat(fd, write_buf, context->io_bytes, 0);
    if (bytes < 0) {
      fprintf(stderr, "Failed to write %zu bytes to file %s (fd: %d): %s\n",
              context->io_bytes, filename, fd, strerror(errno));
      close(fd);
      ret = -1;
      goto cleanup;
    } else if (bytes != context->io_bytes) {
      fprintf(stderr,
              "Failed to write %zu bytes to file %s (fd: %d). Only %zu bytes "
              "written\n",
              context->io_bytes, filename, fd, bytes);
      close(fd);
      ret = -1;
      goto cleanup;
    }
    // Read
    bytes =
        context->io_dispatch_table.readat(fd, read_buf, context->io_bytes, 0);
    if (bytes < 0) {
      fprintf(stderr, "Failed to read %zu bytes to file %s (fd: %d): %s\n",
              context->io_bytes, filename, fd, strerror(errno));
      close(fd);
      ret = -1;
      goto cleanup;
    } else if (bytes != context->io_bytes) {
      fprintf(stderr,
              "Failed to read %zu bytes to file %s (fd: %d). Only %zu bytes "
              "read\n",
              context->io_bytes, filename, fd, bytes);
      close(fd);
      ret = -1;
      goto cleanup;
    }
    // Verify content
    if (memcmp(read_buf, write_buf, context->io_bytes) != 0) {
      fprintf(stderr, "Read buf doesn't match write buf!\n");
      close(fd);
      ret = -1;
      goto cleanup;
    }
    // Close
    if (context->io_dispatch_table.close(fd) != 0) {
      fprintf(stderr, "Failed to close file %s (fd: %d): %s\n", filename, fd,
              strerror(errno));
      ret = -1;
      goto cleanup;
    }
  }
cleanup:
  unlink(filename);
  free(write_buf);
  free(read_buf);
  return (void *)ret;
}

static int read_write_with_pthreads(const void *params_in) {
  int ret = 0;
  const params_t *params = params_in;
  pthread_t *threads = malloc(sizeof(pthread_t) * params->num_threads);
  if (threads == NULL) {
    fprintf(stderr, "Failed to allocate threads\n");
    return -1;
  }
  file_io_context_t *contexts =
      malloc(sizeof(file_io_context_t) * params->num_threads);
  if (contexts == NULL) {
    free(threads);
    fprintf(stderr, "Failed to allocate io contexts\n");
    return -1;
  }
  for (int i = 0; i < params->num_threads; i++) {
    contexts[i].filename_prefix = __FUNCTION__;
    contexts[i].thread_num = i;
    contexts[i].cycles = params->num_cycles_per_thread;
    contexts[i].io_bytes = params->num_bytes_per_io;
    contexts[i].io_dispatch_table = sync_io_dispatch_table;
    int err =
        pthread_create(&threads[i], /*attr=*/NULL, file_io_func, &contexts[i]);
    if (err != 0) {
      fprintf(stderr, "Failed to create thread %d: %s", i, strerror(err));
      ret = -1;
      goto cleanup;
    }
  }
  for (int i = 0; i < params->num_threads; i++) {
    ssize_t retval = -1;
    int err = pthread_join(threads[i], (void **)&retval);
    if (err != 0) {
      fprintf(stderr, "Failed to join thread %d: %s\n", i, strerror(err));
      return -1;
    }
    if (retval != 0) {
      fprintf(stderr, "Thread function failed\n");
      return -1;
    }
  }
cleanup:
  free(threads);
  free(contexts);
  return ret;
}

static int read_write_with_fibers(const void *params_in) {
  const params_t *params = params_in;
  rocket_engine_t *engine =
      rocket_engine_create(/*queue_depth=*/params->num_threads);
  rocket_executor_t *executor = rocket_executor_create(engine);

  file_io_context_t *contexts =
      malloc(sizeof(file_io_context_t) * params->num_threads);
  if (contexts == NULL) {
    rocket_engine_destroy(engine);
    fprintf(stderr, "Failed to allocate io contexts\n");
    return -1;
  }
  for (int i = 0; i < params->num_threads; i++) {
    contexts[i].filename_prefix = __FUNCTION__;
    contexts[i].thread_num = i;
    contexts[i].cycles = params->num_cycles_per_thread;
    contexts[i].io_bytes = params->num_bytes_per_io;
    contexts[i].io_dispatch_table = async_io_dispatch_table;
    rocket_executor_submit_task(executor, file_io_func, &contexts[i]);
  }
  rocket_executor_execute(executor);

  rocket_executor_destroy(executor);
  rocket_engine_destroy(engine);
  free(contexts);
  return 0;
}

void usage(const char *program) {
  fprintf(stdout,
          "Usage: %s -n <# of threads> -c <# of cycles per thread> -s "
          "<IO size in bytes>\n",
          program);
}

// Benchmark pthread + sync IO v.s fiber + async IO
int main(int argc, char **argv) {
  if (argc < 7) {
    usage(argv[0]);
    return -1;
  }
  int opt;
  params_t params;
  while ((opt = getopt(argc, argv, "n:c:s:")) != -1) {
    switch (opt) {
    case 'n':
      params.num_threads = atoi(optarg);
      break;
    case 'c':
      params.num_cycles_per_thread = atoi(optarg);
      break;
    case 's':
      params.num_bytes_per_io = atoi(optarg);
      break;
    default:
      usage(argv[0]);
      return -1;
      break;
    }
  }
  // Pthread + sync IO
  benchmark("read write files with pthreads", read_write_with_pthreads, &params,
            print_params);
  // Fiber + async IO
  benchmark("read write files with fibers", read_write_with_fibers, &params,
            print_params);
  return 0;
}
