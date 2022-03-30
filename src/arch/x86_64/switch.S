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

  # Push 8 byte padding to the stack for proper 16-byte stack alignment
  pushq $0

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
