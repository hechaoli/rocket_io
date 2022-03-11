
#pragma once

#include <stddef.h>

typedef struct {
  void* stack_mem;
  size_t stack_size;
} pal_stack_t;

// Allocate a new stack. Returns 0 on success, -1 on failure.
int stack_create(size_t min_size, pal_stack_t* stack, void** stk_ptr);

// Deallocate a stack. Returns 0 on success, -1 on failure.
int stack_destroy(pal_stack_t* stack);
