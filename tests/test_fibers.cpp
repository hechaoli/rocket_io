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

#include <alloca.h>
#include <pthread.h>

#include <rocket/rocket_engine.h>
#include <rocket/rocket_executor.h>
#include <rocket/rocket_fiber.h>

#include <gtest/gtest.h>

static const size_t queue_depth = 10;

typedef struct {
  size_t count;
  size_t max;
  size_t workers;
} count_context_t;

// Worker function used by the round-robin scheduling test case.
static void* count_yield_worker(void* context) {
  count_context_t* count_context = (count_context_t*)context;
  while (count_context->count < count_context->max) {
    // Increment the counter and snapshot the previous value.
    const size_t last = ++count_context->count;

    // Yield to the next fiber to run.
    rocket_fiber_yield();

    /* Other workers should have incremented the counter since we yielded, but
     * never beyond the maximum.
     */
    const size_t expected_count =
        std::min(last + count_context->workers - 1, count_context->max);
    EXPECT_EQ(expected_count, count_context->count);
  }
  return nullptr;
}

/* Test case to verify that work items are scheduled round-robin when they
 * manually yield and are not blocked on IO.
 */
static void test_round_robin_scheduling(void) {
  rocket_engine_t* engine = rocket_engine_create(queue_depth);
  ASSERT_NE(engine, nullptr);

  rocket_executor_t* executor = rocket_executor_create(engine);
  ASSERT_NE(executor, nullptr);

  count_context_t count_context = {
    .count = 0,
    .max = 50,
    .workers = 5,
  };

  rocket_executor_submit_task(executor, count_yield_worker, &count_context);
  rocket_executor_submit_task(executor, count_yield_worker, &count_context);
  rocket_executor_submit_task(executor, count_yield_worker, &count_context);
  rocket_executor_submit_task(executor, count_yield_worker, &count_context);
  rocket_executor_submit_task(executor, count_yield_worker, &count_context);
  rocket_executor_execute(executor);

  EXPECT_EQ(count_context.count, count_context.max);

  rocket_executor_destroy(executor);
  rocket_engine_destroy(engine);
}

TEST(Fibers, RoundRobinScheduling) {
  test_round_robin_scheduling();
}

// Recursive helper used by the stack allocation test.
static void large_stack_alloc_worker_helper(int count) {
  char* buffer = (char*)alloca(getpagesize());
  if (count > 0) {
    large_stack_alloc_worker_helper(count - 1);
  }
}

// Worker function used by the stack allocation test.
static void* large_stack_alloc_worker(void* context) {
  /* Fibers are given a max stack of 64KiB. This recursive call attempts to
   * access as much of it as possible. Hold out one full page for whatever
   * the compiler implicitly puts on the stack that we can't control.
   */
  const size_t depth = 65536 / getpagesize() - 1;
  large_stack_alloc_worker_helper(depth - 1);
  return nullptr;
}

// Test case to verify fiber stacks can accessed up to their max size.
static void test_max_stack_growth(void) {
  rocket_engine_t* engine = rocket_engine_create(queue_depth);
  ASSERT_NE(engine, nullptr);

  rocket_executor_t* executor = rocket_executor_create(engine);
  ASSERT_NE(executor, nullptr);

  rocket_executor_submit_task(executor, large_stack_alloc_worker, nullptr);
  rocket_executor_submit_task(executor, large_stack_alloc_worker, nullptr);
  rocket_executor_execute(executor);

  /* There are no assertions in this test. If the fiber is unable to allocate
   * stack it will result in a SIGSEGV triggering test failure.
   */

  rocket_executor_destroy(executor);
  rocket_engine_destroy(engine);
}

TEST(Fibers, MaxStackGrowth) {
  test_max_stack_growth();
}

static void* worker_thread(void* context) {
  void (*test_func)(void) = (void (*)(void))context;
  test_func();
  return nullptr;
}

/* Test case to verify that different threads can run their own independent
 * executors and fibers without stomping on one another. Reuses work items
 * from other tests cases.
 */
TEST(Fibers, MultiThread) {
  void(*test_funcs[])(void) = {
    test_round_robin_scheduling,
    test_round_robin_scheduling,
    test_round_robin_scheduling,
    test_max_stack_growth,
  };

  const size_t thread_count = sizeof(test_funcs)/sizeof(test_funcs[0]);
  pthread_t threads[thread_count];

  for (size_t i = 0; i < thread_count; i++) {
    ASSERT_EQ(0, pthread_create(
      &threads[i], /*attr=*/nullptr, worker_thread, (void*)test_funcs[i]));
  }

  for (size_t i = 0; i < thread_count; i++) {
    pthread_join(threads[i], nullptr);
  }
}
