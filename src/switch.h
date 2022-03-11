
#pragma once

void init_run_context(
    void** stk_ptr,
    void* entry_point,
    void* entry_point_context);
// src_stk_ptr will be updated to current stack pointer.
// Then current stack pointer will be updated to dst_stk_ptr.
void switch_run_context(void** src_stk_ptr, void* dst_stk_ptr, void* dst_fiber);

void* get_current_fiber();

void execute_start(void* stk_ptr);
