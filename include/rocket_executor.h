#pragma once

#include <rocket_types.h>

rocket_executor_t* rocket_executor_create(rocket_engine_t* engine);
void rocket_executor_submit_task(rocket_executor_t *executor,
                                 rocket_task_func_t func, void *context);
// Start executing the fibers in the executor.
// Returns only after all existing fibers finish running.
void rocket_executor_execute(rocket_executor_t* executor);
void rocket_executor_destroy(rocket_executor_t* executor);
