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

#pragma once

#include <fcntl.h>
#include <rocket/rocket_types.h>
#include <sys/socket.h>

rocket_engine_t* rocket_engine_create(size_t queue_depth);
void rocket_engine_destroy(rocket_engine_t* engine);

int openat_await(int dirfd, const char* pathname, int oflag, mode_t pmode);
ssize_t readat_await(int fd, void* buf, size_t nbyts, off_t offset);
ssize_t writeat_await(int fd, const void* buf, size_t nbyts, off_t offset);
int close_await(int fd);

int accept_await(int sockfd, struct sockaddr *addr, socklen_t *addrlen,
                 int flags);
ssize_t send_await(int sockfd, const void *buf, size_t len, int flags);
ssize_t recv_await(int sockfd, void *buf, size_t len, int flags);
