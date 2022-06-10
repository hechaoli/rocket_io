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

#include "benchmark_common.h"

#include <assert.h>
#include <stdio.h>
#include <time.h>

#define BILLION  1000000000.0

/**
 * Benchmark a function.
 *
 * @param[in] name Name of the function to benchmark.
 * @param[in] func The function to benchmark.
 * @param[in] params Params used in the benchmark that will be passed to func.
 * @param[in] print_params The function used to print params.
 */
void benchmark(const char *name, benchmark_func_t func, const void *params,
               print_params_t print_params)
{
  fprintf(stdout, "------------------------------------------------------\n");
  fprintf(stdout, "Benchmarking %s with params:\n", name);
  print_params(params);

  struct timespec start_wall_time, end_wall_time;
  clock_t start_cpu_time = clock();
  clock_gettime(CLOCK_MONOTONIC, &start_wall_time);

  assert(0 == func(params));

  double cpu_time = (double)(clock() - start_cpu_time) / CLOCKS_PER_SEC;
  clock_gettime(CLOCK_MONOTONIC, &end_wall_time);
  double wall_time =
      (end_wall_time.tv_sec - start_wall_time.tv_sec) +
      (end_wall_time.tv_nsec - start_wall_time.tv_nsec) / BILLION;

  fprintf(stdout,
          "\nFinished %s in %f seconds CPU time, %f seconds real time\n", name,
          cpu_time, wall_time);
  fprintf(stdout, "------------------------------------------------------\n");
}
