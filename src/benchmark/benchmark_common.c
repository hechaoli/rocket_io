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
