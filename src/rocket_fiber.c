#include <stdint.h>

#include <internal/dlist.h>
#include <internal/rocket_executor.h>
#include <internal/rocket_fiber.h>
#include <internal/switch.h>

static __thread rocket_fiber_t *current_fiber;

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

rocket_fiber_t* get_current_fiber() {
  return current_fiber;
}

void set_current_fiber(void* fiber) {
  current_fiber = fiber;
}

void rocket_fiber_yield() {
  rocket_fiber_t* from_fiber = get_current_fiber();
  switch_run_context(&from_fiber->stk_ptr,
                     from_fiber->executor->execute_loop_stk_ptr,
                     /*switch_context=*/NULL, set_current_fiber);
}

void rocket_fiber_destroy(rocket_fiber_t* fiber) {
  stack_destroy(&fiber->stack);
  free(fiber);
}
