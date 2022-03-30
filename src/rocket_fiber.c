/*
 * MIT License
 *
 * Copyright (c) 2022 Andrew Rogers <andrurogerz@gmail.com>, Hechao Li
 * <hechaol@outlook.com>
 *
 * Permission is hereby granted, free of charge, to any person obtaining a copy
 * of this software and associated documentation files (the "Software"), to deal
 * in the Software without restriction, including without limitation the rights
 * to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
 * copies of the Software, and to permit persons to whom the Software is
 * furnished to do so, subject to the following conditions:
 *
 * The above copyright notice and this permission notice shall be included in all
 * copies or substantial portions of the Software.
 *
 * THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
 * IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
 * FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
 * AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
 * LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
 * OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE
 * SOFTWARE.
 */

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
