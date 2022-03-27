.global switch_run_context
.global init_run_context

# Initialize stack for an execution context.
#
# rdi: stk_ptr [in, out] Pointer to an uninitialized stack for the new context.
# rsi: entry_point [in] Function pointer where execution begins.
# rdx: entry_point_context [in] Argument passed to entry point.
#
init_run_context:
  pushq %rbp
  movq  %rsp, %rbp

  # Get the stack pointer
  movq (%rdi), %rsp # rdi = &stk_ptr

  # Load a dummy value that will be pushed on the stack in place of valid
  # register values.
  movq $0xBADC0FFEEDEADDAD, %r10

  // First, push the entry point address to the stack and a dummy frame pointer
  // value. The entry point address will be "returned" to the first time
  // switch_run_context is called on the new execution context.
  pushq %r10
  pushq %rsi # rsi = entry_point

  # Push dummy values to the stack for all callee saved register positions.
  # These will be popped in switch_run_context.
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
# rdi: src_stk_ptr     [in, out] Current (source) stack pointer.
# rsi: dst_stk_ptr     [in] Pointer to the stack to switch to.
# rdx: switch_context  [in] The context passed to switch_callback.
# rcx: switch_callback [in] Callback invoked after switching.
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

  # Call switch_callback(switch_context)
  movq %rdx, %rdi
  callq *%rcx

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
