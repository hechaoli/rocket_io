#pragma once

#include <internal/dlist.h>
#include <internal/rocket_fiber.h>

typedef struct rocket_future rocket_future_t;

struct rocket_future {
  dlist_node_t list_node;

  // Fiber that is waiting on the future (at most one for now).
  rocket_fiber_t* fiber;
  // True if this future has completed.
  bool completed;
  // Negative value if the future finishes with an error.
  // 0 otherwise.
  int error;
  // Only support integer result for now.
  int result;
};

// TODO: Add timeout
int rocket_future_await(rocket_future_t* future);
