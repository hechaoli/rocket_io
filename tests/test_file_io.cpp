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

#include <rocket/rocket_engine.h>
#include <rocket/rocket_executor.h>
#include <rocket/rocket_fiber.h>

#include <gtest/gtest.h>

static const size_t queue_depth = 10;

// Worker function to create and unlink a number of files.
static void* open_close_unlink_files_worker(void* context) {
  const char* name = (const char*)context;
  static const size_t FILE_COUNT = 32;
  char filename[128];

  for (int i = 0; i < FILE_COUNT; i++) {
    snprintf(filename, sizeof(filename), "%s%d", name, i);
    int fd = openat_await(AT_FDCWD, filename, O_CREAT, 0644);
    EXPECT_GT(fd, 0);
    EXPECT_EQ(close_await(fd), 0);
  }

  for (int i = 0; i < FILE_COUNT; i++) {
    snprintf(filename, sizeof(filename), "%s%d", name, i);
    EXPECT_EQ(unlink(filename), 0);
  }

  return nullptr;
}

// Test case to verify basic file creation.
static void test_file_open_and_unlink(void) {
  rocket_engine_t* engine = rocket_engine_create(queue_depth);
  EXPECT_NE(engine, nullptr);

  rocket_executor_t* executor = rocket_executor_create(engine);
  EXPECT_NE(executor, nullptr);

  rocket_executor_submit_task(
    executor, open_close_unlink_files_worker, (void*)"file");
  rocket_executor_submit_task(
    executor, open_close_unlink_files_worker, (void*)"another_file");
  rocket_executor_submit_task(
    executor, open_close_unlink_files_worker, (void*)"yet_another_file");
  rocket_executor_execute(executor);

  rocket_executor_destroy(executor);
  rocket_engine_destroy(engine);
}

TEST(Fibers, FileOpenAndUnlink) {
  test_file_open_and_unlink();
}

static void* file_open_write_read_close_worker(void* context) {
  const char* filename = (const char*)context;
  int fd = openat_await(AT_FDCWD, filename, O_CREAT | O_RDWR, 0644);
  EXPECT_GT(fd, 0);

  const char write_buf[] = "pi ... ka ... pika pika ... pikachu!";
  const int write_buf_size = sizeof(write_buf);

  ssize_t nbytes = writeat_await(fd, write_buf, write_buf_size, /*offset=*/0);
  EXPECT_EQ(nbytes, write_buf_size);

  char read_buf[write_buf_size];
  nbytes = readat_await(fd, read_buf, write_buf_size, /*offset=*/0);
  EXPECT_EQ(nbytes, write_buf_size);
  EXPECT_EQ(memcmp(read_buf, write_buf, write_buf_size), 0);
  EXPECT_EQ(close_await(fd), 0);
  EXPECT_EQ(unlink(filename), 0);
  return nullptr;
}

TEST(FileIO, FileOpenWriteReadClose) {
  rocket_engine_t* engine = rocket_engine_create(queue_depth);
  EXPECT_NE(engine, nullptr);

  rocket_executor_t* executor = rocket_executor_create(engine);
  EXPECT_NE(executor, nullptr);

  rocket_executor_submit_task(
    executor, file_open_write_read_close_worker, (void*)"file");
  rocket_executor_submit_task(
    executor, file_open_write_read_close_worker, (void*)"another_file");
  rocket_executor_submit_task(
    executor, file_open_write_read_close_worker, (void*)"yet_another_file");
  rocket_executor_execute(executor);

  rocket_executor_destroy(executor);
  rocket_engine_destroy(engine);
}
