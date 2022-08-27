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

# A demo/test program to exercise the library
add_executable(rocket_io_demo main.c)
target_link_libraries(rocket_io_demo PRIVATE rocket_io pthread)
add_test(NAME rocket_io_demo COMMAND rocket_io_demo)

# Benchmark test program
set(
  BENCHMARK_SRC
  benchmark/benchmark_common.c
  benchmark/benchmark_file_io.c
)
add_executable(benchmark_file_io ${BENCHMARK_SRC} ${PUBLIC_HEADERS})
target_link_libraries(benchmark_file_io PRIVATE rocket_io pthread)
add_test(NAME benchmark_file_io COMMAND benchmark_file_io -n 10 -c 10 -s 4096)

# Echo server program
set(
  ECHO_SERVER_SRC
  benchmark/echo_server.c
)
add_executable(echo_server ${ECHO_SERVER_SRC} ${PUBLIC_HEADERS})
target_link_libraries(echo_server PRIVATE rocket_io pthread)
