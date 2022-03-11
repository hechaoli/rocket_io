.global switch_run_context
.global init_run_context
.global execute_start
.global get_current_fiber

# Initialize stack for an execution context.
#
# rdi: stk_ptr [in, out] Pointer to an uninitialized stack for the new context.
# rsi: entry_point [in] Function pointer where execution begins.
#
init_run_context:
  pushq %rbp
  movq  %rsp, %rbp

  # Get the stack pointer
  movq (%rdi), %rsp # rdi = &stk_ptr
  # Push the entry point address to the stack.
  pushq %rsi # rsi = entry_point

  # Push dummy values to the stack for all callee saved register positions.
  # These will be popped in switch_run_context.
  movq $0xDEADBEEFC0FFEE41, %r10
  pushq %r10 # rbp
  pushq %r10 # rbx
  pushq %r10 # r12
  pushq %r10 # r13
  pushq %r10 # r14
  pushq %r10 # r15
  pushq %rdx # rdx = entry_point_context

  movq %rsp, (%rdi) # Update stk_ptr

  movq  %rbp, %rsp
  popq  %rbp
  retq

# Switch from one execution context to another.
#
# rdi: src_stk_ptr [in, out] Current (source) stack pointer.
# rsi: dst_stk_ptr [in] Pointer to the stack to switch to.
# rdx: dst_fiber   [in] Pointer to the fiber to switch to.
#
switch_run_context:
  # Push all callee-saved registers to the current stack.
  pushq %rbp
  pushq %rbx
  pushq %r12
  pushq %r13
  pushq %r14
  pushq %r15
  pushq %rdi # HACK: for the entry_point_context

  # Save the old stack pointer to src_stk_ptr
  movq %rsp, (%rdi)

  # Update the stack pointer to dst_stk_ptr
  movq %rsi, %rsp

  movq %rdx, %fs:0 # Set current fiber

  popq %rdi # HACK: for the entry_point_context
  # Pop all callee-saved registers from the new stack.
  popq %r15
  popq %r14
  popq %r13
  popq %r12
  popq %rbx
  popq %rbp

  # This will return to the dst fiber
  retq

# Get the pointer to current fiber
get_current_fiber:
  movq %fs:0, %rax
  retq
