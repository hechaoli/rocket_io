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

#pragma once

#include <rocket/rocket_fiber.h>
#include <rocket/rocket_types.h>

#include "dlist.h"
#include "pal.h"

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
