
#pragma once

#include "dlist.h"

typedef struct rocket_executor rocket_executor_t;
typedef struct rocket_fiber rocket_fiber_t;
typedef struct rocket_future rocket_future_t;

// Function running in the fiber.
typedef void (*rocket_task_func_t)(void* context);
