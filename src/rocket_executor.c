#include <stdio.h>

#include <internal/rocket_engine.h>
#include <internal/rocket_executor.h>
#include <internal/rocket_fiber.h>
#include <internal/rocket_future.h>
#include <internal/switch.h>

typedef struct {
  rocket_fiber_t* fiber;
  void* task_func_context;
} wrapper_context_t;

rocket_executor_t* rocket_executor_create(rocket_engine_t* engine) {
  rocket_executor_t* executor = malloc(sizeof(rocket_executor_t));
  if (executor == NULL) {
    return NULL;
  }

  dlist_init(&executor->runnable);
  dlist_init(&executor->blocked);

  executor->engine = engine;
  executor->execute_loop_stk_ptr = NULL;

  return executor;
}

static void rocket_task_func_wrapper(void* context) {
  rocket_fiber_t* fiber = (rocket_fiber_t*)context;
  // TODO: Have a way retrieve the return value.
  fiber->task_func(fiber->context);
  fiber->state = COMPLETED;
  switch_run_context(&fiber->stk_ptr, fiber->executor->execute_loop_stk_ptr,
                     /*switch_context=*/NULL, set_current_fiber);
}

// 1) Create a fiber using the task.
// 2) Append the fiber to runnable list.
void rocket_executor_submit_task(
    rocket_executor_t* executor,
    rocket_task_func_t func,
    void* context) {
  // Destroyed in rocket_executor_execute.
  rocket_fiber_t* fiber = rocket_fiber_create(executor, func, context);
  init_run_context(
      &fiber->stk_ptr, rocket_task_func_wrapper, /*entry_point_context=*/fiber);
  dlist_push_tail(&executor->runnable, &fiber->list_node);
}

// Start executing the fibers in the executor.
// Returns only after all existing fibers finish running.
void rocket_executor_execute(rocket_executor_t* executor) {
  while (true) {
    if (!dlist_is_empty(&executor->runnable)) {
      dlist_node_t* node = dlist_pop_head(&executor->runnable);
      rocket_fiber_t* fiber = container_of(node, rocket_fiber_t, list_node);
      switch_run_context(&executor->execute_loop_stk_ptr, fiber->stk_ptr, fiber,
                         set_current_fiber);
      switch (fiber->state) {
        case COMPLETED:
          rocket_fiber_destroy(fiber);
          break;
        case RUNNABLE:
          dlist_push_tail(&executor->runnable, &fiber->list_node);
          break;
        case BLOCKED:
          // If a fiber is blocked, the future must have already been added to
          // blocked.
          break;
        default:
          fprintf(stderr, "[BUG] fiber state can't be NONE\n");
          break;
      }

    } else if (!dlist_is_empty(&executor->blocked)) {
      rocket_future_t* future = rocket_engine_await_next(executor->engine);
      if (future == NULL) {
        return;
      }

      // Remove the future from the blocked list and mark the fiber runnable.
      dlist_remove_node(&future->list_node);
      future->fiber->state = RUNNABLE;
      dlist_push_tail(&executor->runnable, &future->fiber->list_node);
    } else {
      return;
    }
  }
}

void rocket_executor_destroy(rocket_executor_t* executor) {
  // TODO: Free all fibers in the lists if any.
  free(executor);
}

rocket_engine_t* rocket_executor_get_engine(rocket_executor_t* executor) {
  return executor->engine;
}
