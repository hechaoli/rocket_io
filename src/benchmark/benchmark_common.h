#pragma once

typedef int(*benchmark_func_t)(const void* params);
typedef void(*print_params_t)(const void* params);

/**
 * Benchmark a function.
 *
 * @param[in] name Name of the function to benchmark.
 * @param[in] func The function to benchmark.
 * @param[in] params Params used in the benchmark that will be passed to func.
 * @param[in] print_params The function used to print params.
 */
void benchmark(const char *name, benchmark_func_t func, const void *params,
               print_params_t print_params);
