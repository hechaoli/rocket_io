
#include <errno.h>
#include <stdint.h>
#include <stdio.h>
#include <sys/mman.h>
#include <unistd.h>

#include <internal/pal.h>

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
