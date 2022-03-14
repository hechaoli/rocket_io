#include <pthread.h>
#include <stdio.h>
#include <string.h>
#include <unistd.h>

#include "dlist.h"
#include "rocket_engine.h"
#include "rocket_executor.h"
#include "rocket_fiber.h"
#include "rocket_future.h"
#include "switch.h"

#define MAX_NUM 10

int num = 0;
static const size_t queue_depth = 10;

static void func_odd(void* context) {
  while (num < MAX_NUM) {
    if (num % 2 == 0) {
      rocket_fiber_yield();
    } else {
      fprintf(stdout, "%d\n", num);
      num++;
    }
  }
}

static void func_even(void* context) {
  while (num < MAX_NUM) {
    if (num % 2 == 1) {
      rocket_fiber_yield();
    } else {
      fprintf(stdout, "%d\n", num);
      num++;
    }
  }
}

static void big_stack_helper(int count) {
  char* buffer = alloca(getpagesize());
  if (count > 0) {
    big_stack_helper(count - 1);
  }
}

static void big_stack(void* context) {
  const size_t depth = 65536 / getpagesize() - 1;
  big_stack_helper(depth);
}

static void openfiles(void* context) {
  const char* name = context;
  fprintf(stdout, "hello from %s\n", name);
  for (int i = 0; i < 32; i++) {
    char filename[128];
    snprintf(filename, sizeof(filename), "%s%d", name, i);
    int fd = openat_await(AT_FDCWD, filename, O_CREAT, 0644);
    fprintf(stdout, "fd for %s is %d\n", filename, fd);
    int ret = close_await(fd);
    unlink(filename);
  }
}

static void open_write_read_close(void* context) {
  const char* filename = context;
  fprintf(stdout, "========== BEGIN open_write_read_close ==========\n");
  int fd = openat_await(AT_FDCWD, filename, O_CREAT | O_RDWR, 0644);
  fprintf(stdout, "========== fd for %s is %d ==========\n", filename, fd);
  const char write_buf[] = "pi ... ka ... pika pika ... pikachu!";
  int write_buf_size = sizeof(write_buf);
  int nbytes = writeat_await(fd, write_buf, write_buf_size, /*offset=*/0);
  fprintf(
      stdout,
      "========== Wrote %d bytes to file %s (fd %d) ==========\n",
      nbytes,
      filename,
      fd);
  if (nbytes != write_buf_size) {
    fprintf(
        stderr,
        "========== ERROR: Expected write %d bytes. Actual write %d bytes ==========\n",
        write_buf_size,
        nbytes);
  }

  char read_buf[write_buf_size];
  nbytes = readat_await(fd, read_buf, write_buf_size, /*offset=*/0);
  fprintf(
      stdout,
      "========== Read %d bytes from file %s (fd %d) ==========\n",
      nbytes,
      filename,
      fd);
  if (nbytes != write_buf_size) {
    fprintf(
        stderr,
        "========== ERROR: Expected read %d bytes. Actual read %d bytes ==========\n",
        write_buf_size,
        nbytes);
  }
  if (memcmp(read_buf, write_buf, write_buf_size) != 0) {
    fprintf(
        stderr,
        "========== ERROR: Expected read %s. Actual read %s ==========\n",
        write_buf,
        read_buf);
  }

  int ret = close_await(fd);
  if (ret != 0) {
    fprintf(stderr, "========== ERROR: Failed to close fd %d ==========\n", fd);
  }
  fprintf(stdout, "========== END open_write_read_close ==========\n");
  unlink(filename);
}

static void* thread_func1(void* context) {
  fprintf(stdout, "START thread 1\n");
  rocket_engine_t* engine = rocket_engine_create(queue_depth);
  rocket_executor_t* executor = rocket_executor_create(engine);

  rocket_executor_submit_task(executor, open_write_read_close, "pikachu");
  rocket_executor_submit_task(executor, func_odd, NULL);
  rocket_executor_submit_task(executor, func_even, NULL);
  rocket_executor_execute(executor);

  rocket_executor_destroy(executor);
  rocket_engine_destroy(engine);
  fprintf(stdout, "END thread 1\n");
  return NULL;
}

static void* thread_func2(void* context) {
  fprintf(stdout, "START thread 2\n");
  rocket_engine_t* engine = rocket_engine_create(queue_depth);
  rocket_executor_t* executor = rocket_executor_create(engine);

  rocket_executor_submit_task(executor, openfiles, "foo");
  rocket_executor_submit_task(executor, openfiles, "bar");
  rocket_executor_submit_task(executor, big_stack, NULL);
  rocket_executor_execute(executor);

  rocket_executor_destroy(executor);
  rocket_engine_destroy(engine);
  fprintf(stdout, "END thread 2\n");
  return NULL;
}

int main(int argc, const char** argv) {
  pthread_t thread1, thread2;
  int ret1 =
      pthread_create(&thread1, /*attr=*/NULL, thread_func1, /*context=*/NULL);
  if (ret1 != 0) {
    fprintf(stderr, "Failed to create thread1: %s", strerror(ret1));
    return -1;
  }

  int ret2 =
      pthread_create(&thread2, /*attr=*/NULL, thread_func2, /*context=*/NULL);
  if (ret2 != 0) {
    fprintf(stderr, "Failed to create thread2: %s", strerror(ret2));
    return -1;
  }

  pthread_join(thread1, NULL);
  pthread_join(thread2, NULL);

  fprintf(stdout, "completed\n");

  return 0;
}
