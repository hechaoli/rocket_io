
#include <stdint.h>

#include "dlist.h"
#include "pal.h"
#include "rocket_fiber.h"

rocket_fiber_t* rocket_fiber_create(
    rocket_executor_t* executor,
    rocket_task_func_t func,
    void* context) {
  // Freed in rocket_fiber_destroy.
  rocket_fiber_t* fiber = malloc(sizeof(rocket_fiber_t));
  fiber->state = RUNNABLE;
  fiber->executor = executor;
  fiber->task_func = func;
  fiber->context = context;
  if (stack_create(65536, &fiber->stack, &fiber->stk_ptr) < 0) {
    free(fiber);
    return NULL;
  }
  assert(fiber->stk_ptr != NULL);
  return fiber;
}

// from_fiber should be current fiber. Needed before we have get_current_fiber()
void rocket_fiber_switch(rocket_fiber_t* from_fiber, rocket_fiber_t* to_fiber) {
}
void rocket_fiber_exit(rocket_fiber_t* fiber) {}

void rocket_fiber_destroy(rocket_fiber_t* fiber) {
  stack_destroy(&fiber->stack);
  free(fiber);
}
