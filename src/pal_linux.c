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

#include <errno.h>
#include <stdint.h>
#include <stdio.h>
#include <sys/mman.h>
#include <unistd.h>

#include "pal.h"

#define STACK_ALIGN __alignof__(long double)
#define ROUND_UP(a, b) ((a) % (b) ? ((a) + (b)) - ((a) % (b)) : (a))

int stack_create(size_t min_size, pal_stack_t* stack, void** stk_ptr) {
  const size_t page_size = getpagesize();
  const size_t guard_size = page_size;
  const size_t stack_size = ROUND_UP(min_size, page_size) + (2 * guard_size);
  const int prot = PROT_READ | PROT_WRITE;
  const int flags = MAP_PRIVATE | MAP_ANONYMOUS | MAP_STACK;
  void* stack_mem = mmap(NULL, stack_size, prot, flags, -1, 0);
  if (stack_mem == MAP_FAILED) {
    perror("mmap");
    return -1;
  }

  // Add a guard page at start of stack.
  void* begin_guard = (char*)stack_mem + stack_size - guard_size;
  if (mprotect(begin_guard, guard_size, PROT_NONE) < 0) {
    perror("mprotect");
    goto error;
  }

  // Add a guard page at end of stack.
  void* end_guard = stack_mem;
  if (mprotect(end_guard, guard_size, PROT_NONE) < 0) {
    perror("mprotect");
    goto error;
  }

  stack->stack_mem = stack_mem;
  stack->stack_size = stack_size;
  *stk_ptr = begin_guard;
  return 0;

error:
  if (stack_mem != NULL && munmap(stack->stack_mem, stack->stack_size) < 0) {
    perror("munmap");
  }
  return -1;
}

int stack_destroy(pal_stack_t* stack) {
  if (munmap(stack->stack_mem, stack->stack_size) < 0) {
    perror("munmap");
    return -1;
  }

  return 0;
}
