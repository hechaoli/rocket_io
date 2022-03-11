#pragma once

#include "dlist.h"
#include "rocket_common.h"
#include "rocket_engine.h"

rocket_executor_t* rocket_executor_create(rocket_engine_t* engine);
rocket_fiber_t* rocket_executor_submit_task(
    rocket_executor_t* executor,
    rocket_task_func_t func,
    void* context);

// Start executing the fibers in the executor.
// Returns only after all existing fibers finish running.
void rocket_executor_execute(rocket_executor_t* executor);
void rocket_executor_destroy(rocket_executor_t* executor);
rocket_engine_t* rocket_executor_get_engine(rocket_executor_t* executor);
