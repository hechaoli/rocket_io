# Rocket I/O

## Overview
Rocket I/O is an async runtime with the support of various asynchronous I/O
backend such as [`io_uring`](https://kernel.dk/io_uring.pdf). The goal of
Rocket I/O is to make programming with asynchronous I/O simpler and more
intuitive.

As we know, input and output (I/O) operations on a computer can be extremely
slow compared to running CPU instructions. A simple approach of performing an
I/O operation is to block the current execution until the I/O is complete
(i.e. synchronous I/O). However, this can be a waste of system resources
because the CPU does nothing useful while waiting for I/O completion. A better
approach would be asynchronous I/O, which means after issuing an I/O request,
instead of waiting for it to complete, the CPU can spend time processing
instructions not depending on the I/O result.

There are many options to implement asynchronous I/O on Linux such as
[`select`](https://man7.org/linux/man-pages/man2/select.2.html),
[`poll`](https://man7.org/linux/man-pages/man2/poll.2.html),
[`epoll`](https://man7.org/linux/man-pages/man7/epoll.7.html),
[`aio`](https://man7.org/linux/man-pages/man7/aio.7.html),
[`io_uring`](https://kernel.dk/io_uring.pdf), etc. However, each option has an
distinct way of supporting asynchronous I/O which is usually not that intuitive
and straightforward. A developer needs to read the documentation thoroughly to
figure out the correct usage.

For an application programmer, asynchronous I/O simply means, once I make an
I/O request (e.g. read a file by calling `read` system call), something else
not depending on this I/O result can run before the I/O is complete. And the
execution depending on the I/O result will resume after the I/O is complete.
Rocket I/O's async runtime provides primitives to use asynchronous I/O in such
an intuitive way. See [Example](#example) for a concrete example.

## Concepts

### Fiber
Like thread, a fiber an execution unit for concurrent tasks. The difference is
that threads use preemptive multitasking while fibers use cooperative
multitasking. That means, a thread can be preempted involuntarily by kernel at
any time in favor of another thread while a fiber may yield the execution
voluntarily to another fiber when it's idle or blocked by an I/O operation.

In Rocket I/O library, each task runs in a fiber. When blocked by I/O, it
yields the execution.

Fibers are scheduled by an executor.

### Executor
An executor schedules the execution of fibers. When one fiber yields the
execution, the executor schedules the next runnable fiber to run. When an I/O
is complete, it resumes the execution of the blocking fiber.

All fibers in an executor run within a single thread.

### Rocket I/O Engine
A Rocket I/O engine is an asynchronous I/O backend such as `epoll`, `aio`,
`io_uring`, etc. Each executor has one Rocket I/O engine. For now, only
`io_uring` is supported but other engines will be added later. 

## Example

They following code is an example of running two tasks, both of which involve
some I/O operations, on a Rocket I/O executor.

```c

#include <rocket/rocket_engine.h>
#include <rocket/rocket_executor.h>

static void* open_close_file(void* context) {
  const char* filename = context;
  printf("Open and close file %s\n", filename);
  int fd = openat_await(AT_FDCWD, filename, O_CREAT, 0644);
  printf("fd = %d\n", fd);
  close_await(fd);
  return NULL;
}

int main() {
  rocket_engine_t* engine = rocket_engine_create(/*queue_depth=*/10);
  rocket_executor_t* executor = rocket_executor_create(engine);

  rocket_executor_submit_task(executor, open_close_file, "foo");
  rocket_executor_submit_task(executor, open_close_file, "bar");
  rocket_executor_execute(executor);

  rocket_executor_destroy(executor);
  rocket_engine_destroy(engine);
}
```

Each task simply opens a file and creates it if it doesn't exist already and
then closes it. Both open and close are I/O operations. The execution order is:
* Task 1 tries to open file "foo". It is now blocked by I/O so it will yield
  the execution.
* Task 2 tries to open file "bar". It is now blocked by I/O so it will yield
  the execution.
* Task 1's open I/O is done and the execution resumes. fd of file "foo" is
  printed. Then it tries to close file "foo" and again blocked by IO.
  Therefore, it yields again.
* Task 2's open I/O is done and the execution resumes. fd of file "bar" is
  printed. Then it tries to close file "bar" and again blocked by IO.
  Therefore, it yields again.
* Task 1's close I/O is done and the execution resumes. It has nothing else to
  do and returns.
* Task 2's close I/O is done and the execution resumes. It has nothing else to
  do and returns.

Note that all the "yield when blocked by I/O" and "resume when I/O is complete"
magics are done by Rocket I/O library. The programmer only needs to use
asynchronous file APIs such as `open_await` and `close_await` to indicate that
this is an asynchronous I/O, which is more intuitive than using, say, [`aio`
APIs](https://man7.org/linux/man-pages/man7/aio.7.html) directly.

If instead of using the two asynchronous I/O API `openat_await` and
`close_await`, the synchronous I/O API `open` and `close` are used, then the
execution order is:
* Task 1 tries to open file "foo". It waits until the I/O is done.
* Task 1 then tries to close file "foo". It waits until the I/O is done.
* Task 1 has nothing else to do and returns.
* Task 2 tries to open file "bar". It waits until the I/O is done.
* Task 2 then tries to close file "bar". It waits until the I/O is done.
* Task 2 has nothing else to do and returns.

In this case, the CPU resource is wasted while task 1 or task 2 is waiting for
I/O completion.

## How to Use the Library
### Prerequisite

Rocket I/O is currently only supported on Linux on the following architectures:
* x86_64
* aarch64

The only asynchronous I/O engine supported by Rocket I/O library for now is
[`io_uring`](https://kernel.dk/io_uring.pdf). Therefore, to use it, the Linux
kernel must have `io_uring` support. It is available since kernel 5.1 but
support for it could be compiled out. The safest way to check for support is to
check whether the `io_uring` system calls are available.
```
$ grep io_uring_setup /proc/kallsyms
```

The Rocket I/O library also depends on [`liburing`
library](https://github.com/axboe/liburing/tree/master), which you can either
build from source or install using:

```
$ sudo dnf install liburing liburing-devel
```

Or

```
$ sudo apt install liburing liburing-dev
```

### Configure
```
$ cmake -S . -B build
```

### Build
To build the project:
```
$ cmake --build build
```

### Executing
To run the demo app:
```
$ ./build/tests/rocket_io_demo
```

To run a file I/O benchmark binary for `rocket_io` library:
```
$ ./build/tests/benchmark_file_io
```

To run an echo server using `rocket_io` library:
```
$ ./build/tests/echo_server
```

### Install
To install `rocket_io` library:

```
$ sudo cmake --install build
```

By default, the library is installed to `/usr/local/`. To change the install
location, run:
```
$ cmake --install build --prefix=<path_to_install>
```

### Uninstall
To uninstall the previously installed files:
```
$ sudo xargs rm < build/install_manifest.txt
```

## Limitations and Future Work

The Rocket I/O library currently has the following limitations, which will be
solved by future work. 

* The rocket executor is single-threaded. In other words, all fibers run on a
  single thread. As a result, not all CPU cores are utilized and the executor
  is not thread-safe.
* Each task submitted to the executor could have a return value. But currently
  there is no way to retrieve the value yet.
* The only supported asynchronous I/O engine for now is `io_uring`. Other
  engines like `epoll`, `aio`, etc, could be added later for systems without
  `io_uring` support.
* The library only works on Linux for now. Support for other OSes could be
  added later.
* Supported APIs for now:
  * File-related APIs
    * `openat`
    * `read`
    * `write`
    * `close`
  * Socket-related APIs
    * `accept`
    * `send`
    * `recv`
* Automation tests and detailed documentation are yet to be added.

## Benchmark
See [benchmark](src/benchmark/README.md).
