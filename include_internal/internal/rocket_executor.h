#pragma once

#include <internal/dlist.h>
#include <rocket_executor.h>

struct rocket_executor {
  rocket_engine_t* engine;

  // Runnable fibers.
  dlist_node_t runnable;
  // Blocked fibers.
  dlist_node_t blocked;

  void* execute_loop_stk_ptr;
};

rocket_engine_t* rocket_executor_get_engine(rocket_executor_t* executor);
