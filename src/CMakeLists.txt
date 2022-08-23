# MIT License
#
# Copyright (c) 2022 Andrew Rogers <andrurogerz@gmail.com>, Hechao Li
# <hechaol@outlook.com>
#
# Permission is hereby granted, free of charge, to any person obtaining a copy
# of this software and associated documentation files (the "Software"), to deal
# in the Software without restriction, including without limitation the rights
# to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
# copies of the Software, and to permit persons to whom the Software is
# furnished to do so, subject to the following conditions:
#
# The above copyright notice and this permission notice shall be included in all
# copies or substantial portions of the Software.
#
# THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
# IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
# FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
# AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
# LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
# OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE
# SOFTWARE.

# Public header files
file(
  GLOB PUBLIC_HEADERS CONFIGURE_DEPENDS
  "${PUBLIC_HEADERS_DIR}/rocket/*.h"
)
install(FILES ${PUBLIC_HEADERS} DESTINATION include/rocket)

# Rocket I/O library target
set(
  LIB_SRC
  dlist.h
  pal_linux.c
  pal.h
  rocket_executor.c
  rocket_executor.h
  rocket_engine_uring.c
  rocket_engine.h
  rocket_fiber.c
  rocket_fiber.h
  rocket_future.c
  rocket_future.h
  arch/${CMAKE_HOST_SYSTEM_PROCESSOR}/switch.S
)
add_library(rocket_io ${LIB_SRC} ${PUBLIC_HEADERS})
target_include_directories(rocket_io PUBLIC ${PUBLIC_HEADERS_DIR})
target_link_libraries(rocket_io PRIVATE uring)
install(TARGETS rocket_io DESTINATION lib)

# A demo/test program to exercise the library
add_executable(rocket_io_demo main.c)
target_link_libraries(rocket_io_demo PRIVATE rocket_io pthread)

# Benchmark test program
set(
  BENCHMARK_SRC
  benchmark/benchmark_common.c
  benchmark/benchmark_file_io.c
)
add_executable(benchmark_file_io ${BENCHMARK_SRC} ${PUBLIC_HEADERS})
target_link_libraries(benchmark_file_io PRIVATE rocket_io pthread)

# Echo server program
set(
  ECHO_SERVER_SRC
  benchmark/echo_server.c
)
add_executable(echo_server ${ECHO_SERVER_SRC} ${PUBLIC_HEADERS})
target_link_libraries(echo_server PRIVATE rocket_io pthread)
