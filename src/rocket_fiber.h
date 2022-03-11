
#pragma once

#include "dlist.h"
#include "pal.h"
#include "rocket_common.h"

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
void rocket_fiber_init(void* ret_addr);

// from_fiber should be current fiber. Needed before we have get_current_fiber()
void rocket_fiber_yield();
// from_fiber should be current fiber. Needed before we have get_current_fiber()
void rocket_fiber_switch(rocket_fiber_t* from_fiber, rocket_fiber_t* to_fiber);
void rocket_fiber_exit(rocket_fiber_t* fiber);
void rocket_fiber_destroy(rocket_fiber_t* fiber);
