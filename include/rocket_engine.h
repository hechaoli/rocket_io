#pragma once

#include <fcntl.h>
#include <rocket_types.h>

rocket_engine_t* rocket_engine_create(size_t queue_depth);
void rocket_engine_destroy(rocket_engine_t* engine);

int openat_await(int dirfd, const char* pathname, int oflag, mode_t pmode);
ssize_t readat_await(int fd, void* buf, size_t nbyts, off_t offset);
ssize_t writeat_await(int fd, const void* buf, size_t nbyts, off_t offset);
int close_await(int fd);
