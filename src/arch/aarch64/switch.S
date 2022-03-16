.global switch_run_context
.global init_run_context

/*
 * Initialize stack for an execution context.
 *
 * x0: stk_ptr [in, out] Pointer to an uninitialized stack for the new
 * execution context.
 * x1: entry_point [in] Function pointer where execution begins.
 * x2: entry_point_context [in] Argument passed to entry point.
 */
init_run_context:
  // Push stack frame, set frame pointer.
  stp x29, x30, [sp, #-16]!
  mov x29, sp

  // Load a dummy value that will be pushed on the stack in place of valid
  // register values.
  ldr x9, =0xBADC0FFEEDEADDAD

  // Get the input stack pointer argument.
  ldr x10, [x0] // x0 = &stk_ptr

  // First, push the entry point address to the stack and a dummy frame pointer
  // value. The entry point address will be "returned" to the first time
  // switch_run_context is called on the new execution context.
  stp x9, x1, [x10, #-16]! // x29, x30 (fp, lr)

  // Next, put dummy values on the stack for all callee saved registers. These
  // values will be popped (and ignored) the first time switch_run_context is
  // called for the new execution context.
  stp x9, x9, [x10, #-16]! // x27, x28
  stp x9, x9, [x10, #-16]! // x25, x26
  stp x9, x9, [x10, #-16]! // x23, x24
  stp x9, x9, [x10, #-16]! // x21, x22
  stp x9, x9, [x10, #-16]! // x19, x20
  stp x9, x9, [x10, #-16]! // d14, d15
  stp x9, x9, [x10, #-16]! // d12, d13
  stp x9, x9, [x10, #-16]! // d10, d11
  stp x9, x9, [x10, #-16]! // d8, d9

  // Finally, put the context value for the entry point being initialized at the
  // top of the stack. This will be restored by switch_run_context and passed
  // as the first argument to the entry point function the first time it runs.
  str x2, [x10, #-16]!

  // Return the updated stack pointer at its new location.
  str x10, [x0]  // *stk_ptr = x10

  // Restore stack frame and return to caller.
  ldp x29, x30, [sp], #16
  ret

/*
 * Switch from one execution context to another.
 *
 * x0: src_stk_ptr     [in, out] Current (source) stack pointer.
 * x1: dst_stk_ptr     [in] Pointer to the stack to switch to.
 * x2: switch_context  [in] The context passed to switch_callback.
 * x3: switch_callback [in] Callback invoked after switching.
 */
switch_run_context:
  // Push stack frame and set frame pointer.
  stp x29, x30, [sp, #-16]!
  mov x29, sp

  // Push all callee-saved registers to the stack.
  stp x27, x28, [sp, #-16]!
  stp x25, x26, [sp, #-16]!
  stp x23, x24, [sp, #-16]!
  stp x21, x22, [sp, #-16]!
  stp x19, x20, [sp, #-16]!
  stp d14, d15, [sp, #-16]!
  stp d12, d13, [sp, #-16]!
  stp d10, d11, [sp, #-16]!
  stp d8, d9, [sp, #-16]!

  // Push NULL as the context argument. It is not needed except on the first
  // invocation, which was set up properly by init_run_context.
  ldr x9, =0
  str x9, [sp, #-16]!

  // Return new stack pointer to caller, *src_stk_ptr.
  mov x10, sp // x10 = sp
  str x10, [x0] // *src_stk_ptr = x10

  // Switch to the desination stack pointer.
  mov sp, x1

  // Call the switch callback, passing the context as the first argument.
  mov x0, x2 // x0 = switch_context
  blr x3

  // Pop the previous context value into the first argument register.
  ldr x0, [sp], #16

  // Pop callee-saved registers from the switched stack.
  ldp d8, d9, [sp], #16
  ldp d10, d11, [sp], #16
  ldp d12, d13, [sp], #16
  ldp d14, d15, [sp], #16
  ldp x19, x20, [sp], #16
  ldp x21, x22, [sp], #16
  ldp x23, x24, [sp], #16
  ldp x25, x26, [sp], #16
  ldp x27, x28, [sp], #16

  // Restore stack frame and return to switched function.
  ldp x29, x30, [sp], #16
  ret
