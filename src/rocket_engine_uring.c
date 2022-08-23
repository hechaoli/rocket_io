/*
 * MIT License
 *
 * Copyright (c) 2022 Andrew Rogers <andrurogerz@gmail.com>, Hechao Li
 * <hechaol@outlook.com>
 *
 * Permission is hereby granted, free of charge, to any person obtaining a copy
 * of this software and associated documentation files (the "Software"), to deal
 * in the Software without restriction, including without limitation the rights
 * to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
 * copies of the Software, and to permit persons to whom the Software is
 * furnished to do so, subject to the following conditions:
 *
 * The above copyright notice and this permission notice shall be included in all
 * copies or substantial portions of the Software.
 *
 * THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
 * IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
 * FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
 * AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
 * LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
 * OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE
 * SOFTWARE.
 */

#include <liburing.h>
#include <stdio.h>

#include <rocket/rocket_engine.h>
#include <rocket/rocket_fiber.h>

#include "rocket_executor.h"
#include "rocket_future.h"
#include "switch.h"

typedef struct {
  int dirfd;
  const char* pathname;
  int oflag;
  int pmode;
} openat_context_t;

// The io_uring implementation of rocket engine.
struct rocket_engine {
  struct io_uring uring;
};

typedef void (*io_uring_prepare_t)(struct io_uring_sqe* sqe, void* context);

rocket_engine_t* rocket_engine_create(size_t queue_depth) {
  rocket_engine_t* engine = malloc(sizeof(rocket_engine_t));
  if (engine == NULL) {
    return NULL;
  }

  if (io_uring_queue_init(queue_depth, &engine->uring, 0) < 0) {
    perror("io_uring_queue_init");
    free(engine);
    return NULL;
  }

  return engine;
}

void rocket_engine_destroy(rocket_engine_t* engine) {
  io_uring_queue_exit(&engine->uring);
  free(engine);
}

rocket_future_t* rocket_engine_await_next(rocket_engine_t* engine) {
  struct io_uring_cqe* cqe = NULL;
  if (io_uring_wait_cqe(&engine->uring, &cqe) < 0) {
    perror("io_uring_wait_cqe");
    return NULL;
  }

  // Complete the future object associated with the request.
  rocket_future_t* future = io_uring_cqe_get_data(cqe);
  future->completed = true;
  future->error = 0;
  future->result = cqe->res;
  io_uring_cqe_seen(&engine->uring, cqe);

  return future;
}

static int io_uring_submit_await(
    io_uring_prepare_t prepare_func,
    void* context) {
  rocket_fiber_t* fiber = get_current_fiber();
  rocket_engine_t* engine = rocket_executor_get_engine(fiber->executor);

  rocket_future_t future = {
      .completed = false,
      .error = -1,
      .result = -1,
      .fiber = fiber,
  };

  struct io_uring_sqe* sqe = io_uring_get_sqe(&engine->uring);
  if (sqe == NULL) {
    perror("io_uring_get_sqe");
    return -1;
  }

  // Prepare the request using caller arguments.
  prepare_func(sqe, context);

  // Stash the future as user data associated with the request.
  io_uring_sqe_set_data(sqe, &future);

  // Signal request submission.
  if (io_uring_submit(&engine->uring) < 0) {
    perror("io_uring_submit");
    return -1;
  }

  // Wait for completion.
  if (rocket_future_await(&future) < 0) {
    perror("rocket_future_await");
    return -1;
  }

  return future.result;
}

static void prepare_openat(struct io_uring_sqe* sqe, void* context) {
  openat_context_t* openat_context = (openat_context_t*)context;
  // Prepare the open request using caller arguments.
  io_uring_prep_openat(
      sqe,
      openat_context->dirfd,
      openat_context->pathname,
      openat_context->oflag,
      openat_context->pmode);
}

int openat_await(int dirfd, const char* pathname, int oflag, mode_t pmode) {
  openat_context_t context;
  context.dirfd = dirfd;
  context.pathname = pathname;
  context.oflag = oflag;
  context.pmode = pmode;
  return io_uring_submit_await(prepare_openat, &context);
}

typedef struct {
  int fd;
  void* buf;
  size_t nbytes;
  off_t offset;
} readat_context_t;

static void prepare_readat(struct io_uring_sqe* sqe, void* context) {
  readat_context_t* readat_context = (readat_context_t*)context;
  io_uring_prep_read(
      sqe,
      readat_context->fd,
      readat_context->buf,
      readat_context->nbytes,
      readat_context->offset);
}

ssize_t readat_await(int fd, void* buf, size_t nbytes, off_t offset) {
  readat_context_t context;
  context.fd = fd;
  context.buf = buf;
  context.nbytes = nbytes;
  context.offset = offset;
  return io_uring_submit_await(prepare_readat, &context);
}

typedef struct {
  int fd;
  const void* buf;
  size_t nbytes;
  off_t offset;
} writeat_context_t;

static void prepare_writeat(struct io_uring_sqe* sqe, void* context) {
  writeat_context_t* writeat_context = (writeat_context_t*)context;
  io_uring_prep_write(
      sqe,
      writeat_context->fd,
      writeat_context->buf,
      writeat_context->nbytes,
      writeat_context->offset);
}

ssize_t writeat_await(int fd, const void* buf, size_t nbytes, off_t offset) {
  writeat_context_t context;
  context.fd = fd;
  context.buf = buf;
  context.nbytes = nbytes;
  context.offset = offset;
  return io_uring_submit_await(prepare_writeat, &context);
}

static void prepare_close(struct io_uring_sqe* sqe, void* context) {
  int fd = *(int*)context;
  io_uring_prep_close(sqe, fd);
}

int close_await(int fd) {
  return io_uring_submit_await(prepare_close, &fd);
}

typedef struct {
  int sockfd;
  struct sockaddr *addr;
  socklen_t *addrlen;
  int flags;
} accept_context_t;

static void prepare_accept(struct io_uring_sqe* sqe, void* context) {
  accept_context_t* accept_context = context;
  io_uring_prep_accept(sqe, accept_context->sockfd, accept_context->addr,
                       accept_context->addrlen, accept_context->flags);
}

int accept_await(int sockfd, struct sockaddr *addr, socklen_t *addrlen,
                 int flags) {
  accept_context_t context;
  context.sockfd = sockfd;
  context.addr = addr;
  context.addrlen = addrlen;
  context.flags = flags;
  return io_uring_submit_await(prepare_accept, &context);
}

typedef struct {
  int sockfd;
  const void* buf;
  size_t len;
  int flags;
} send_context_t;

static void prepare_send(struct io_uring_sqe* sqe, void* context) {
  send_context_t* send_context = context;
  io_uring_prep_send(sqe, send_context->sockfd, send_context->buf,
                     send_context->len, send_context->flags);
}

ssize_t send_await(int sockfd, const void *buf, size_t len, int flags) {
  send_context_t context;
  context.sockfd = sockfd;
  context.buf = buf;
  context.len = len;
  context.flags = flags;
  return io_uring_submit_await(prepare_send, &context);
}

typedef struct {
  int sockfd;
  void* buf;
  size_t len;
  int flags;
} recv_context_t;

static void prepare_recv(struct io_uring_sqe* sqe, void* context) {
  recv_context_t* recv_context = context;
  io_uring_prep_recv(sqe, recv_context->sockfd, recv_context->buf,
                     recv_context->len, recv_context->flags);
}

ssize_t recv_await(int sockfd, void *buf, size_t len, int flags) {
  recv_context_t context;
  context.sockfd = sockfd;
  context.buf = buf;
  context.len = len;
  context.flags = flags;
  return io_uring_submit_await(prepare_recv, &context);
}
