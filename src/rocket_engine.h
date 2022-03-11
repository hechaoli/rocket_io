#pragma once

#include <fcntl.h>

#include "rocket_common.h"

typedef struct rocket_engine rocket_engine_t;

rocket_engine_t* rocket_engine_create(size_t queue_depth);
rocket_future_t* rocket_engine_await_next(rocket_engine_t* engine);
void rocket_engine_destroy(rocket_engine_t* engine);

int openat_await(int dirfd, const char* pathname, int oflag, int pmode);
int readat_await(int fd, void* buf, size_t nbyts, off_t offset);
int writeat_await(int fd, const void* buf, size_t nbyts, off_t offset);
int close_await(int fd);
