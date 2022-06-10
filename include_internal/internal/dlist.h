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

#pragma once

#include <assert.h>
#include <stddef.h>
#include <stdlib.h>
#include <stdbool.h>

#ifndef container_of
#define container_of(ptr, type, member) ({                      \
        const typeof( ((type *)0)->member ) *__mptr = (ptr);    \
        (type *)( (char *)__mptr - offsetof(type,member) );})
#endif

typedef struct dlist_node {
  struct dlist_node* prev;
  struct dlist_node* next;
} dlist_node_t;

static inline void dlist_init(dlist_node_t* list)
{
  assert(list != NULL);
  *list = (dlist_node_t){ list, list };
}

static inline bool dlist_is_empty(dlist_node_t* list)
{
  assert(list != NULL);
  return (list == list->next);
}

static inline bool dlist_node_in_list(dlist_node_t* node)
{
  return (node->next != NULL) && (node->prev != NULL);
}

static inline void dlist_link_nodes(dlist_node_t* prev, dlist_node_t* next)
{
  assert(prev != NULL);
  assert(next != NULL);
  prev->next = next;
  next->prev = prev;
}

static inline void dlist_clear_node(dlist_node_t* node)
{
  assert(node != NULL);
  *node = (dlist_node_t){ NULL, NULL };
}

static inline void dlist_remove_node(dlist_node_t* node)
{
  assert(!dlist_is_empty(node));
  assert(dlist_node_in_list(node));

  dlist_node_t* prev = node->prev;
  dlist_node_t* next = node->next;
  dlist_link_nodes(prev, next);
  dlist_clear_node(node);
}

static inline dlist_node_t* dlist_pop_head(dlist_node_t* list)
{
  assert(list != NULL);
  assert(dlist_node_in_list(list));
  if (dlist_is_empty(list)) {
    return NULL;
  }

  dlist_node_t* head = list->next;
  dlist_remove_node(head);
  return head;
}

static inline void dlist_push_tail(dlist_node_t* list, dlist_node_t* node)
{
  assert(list != NULL);
  assert(node != NULL);
  assert(dlist_node_in_list(list));

  dlist_node_t* tail = list->prev;
  dlist_link_nodes(tail, node);
  dlist_link_nodes(node, list);
}
