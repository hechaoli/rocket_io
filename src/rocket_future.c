#include <internal/rocket_executor.h>
#include <internal/rocket_fiber.h>
#include <internal/rocket_future.h>

// TODO: Add timeout
int rocket_future_await(rocket_future_t* future) {
  rocket_fiber_t* fiber = get_current_fiber();
  assert(!dlist_node_in_list(&fiber->list_node));
  if (future->completed) {
    return future->error;
  }

  future->fiber = fiber;
  fiber->state = BLOCKED;
  rocket_executor_t* executor = fiber->executor;

  dlist_push_tail(&executor->blocked, &future->list_node);
  rocket_fiber_yield();

  return future->error;
}

