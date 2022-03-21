#pragma once

#include <internal/dlist.h>
#include <internal/pal.h>
#include <rocket_fiber.h>
#include <rocket_types.h>

typedef enum {
  NONE = 0,
  RUNNABLE = 1,
  BLOCKED = 2,
  COMPLETED = 3,
} rocket_fiber_state_t;

typedef struct rocket_fiber {
  dlist_node_t list_node;

  // State of the fiber.
  rocket_fiber_state_t state;
  // Executor that's current running the fiber.
  rocket_executor_t* executor;
  // Function running in the fiber.
  rocket_task_func_t task_func;
  // Context used in the function.
  void* context;

  pal_stack_t stack;
  void* stk_ptr;
} rocket_fiber_t;

rocket_fiber_t* rocket_fiber_create(
    rocket_executor_t* executor,
    rocket_task_func_t func,
    void* context);
rocket_fiber_t* get_current_fiber();
void set_current_fiber(void* fiber);
void rocket_fiber_destroy(rocket_fiber_t* fiber);
