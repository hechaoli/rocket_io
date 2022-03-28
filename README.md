# Rocket IO

## Overview
TODO

## Configure
```
$ ./configure
```

Options:
* `--prefix=<install_prefix>`: Installation prefix of `rocket_io` library.
* `--enable-debug`: Include debug symbols. [Default]
* `--disable-debug`: Don't include debug symbols.

## Build
To build a binary with examples of using `rocket_io` library:

```
$ make
$ ./rocket_io_demo
```

To build a file I/O benchmark binary for `rocket_io` library:
```
$ make benchmark_file_io
$ ./benchmark_file_io
```

To build an echo server using `rocket_io` library:
```
$ make echo_server
$ ./echo_server
```

## Install
To install `rocket_io` library:

```
$ ./configure
$ sudo make install
```

By default, the library is installed to `/usr/local/lib`. To change the install
location, run:
```
$ ./configure --prefix=<path_to_install>
$ make install
```

## Usage
Example usage of the library after installation:
```
$ clang src/main.c -o rocket_io_demo -lrocket_io -lpthread -luring
$ ./rocket_io_demo

$ clang src/benchmark/echo_server.c -o echo_server -lrocket_io -lpthread -luring
$ ./echo_server
```

## Uninstall
```
$ sudo make uninstall
```
