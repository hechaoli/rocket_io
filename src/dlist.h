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
