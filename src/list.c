/*
 * Copyright 2010, Mikhail "borman" Borisov <borisov.mikhail@gmail.com>
 *
 * This file is part of borman's model management game server.
 *
 * This program is free software: you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation, either version 3 of the License, or
 * (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program.  If not, see <http://www.gnu.org/licenses/>.
 */


#include <stdlib.h>
#include <assert.h>
#include <string.h>

#include "list.h"
#include "debug.h"

static List list_node_alloc(void *default_data);
static void list_node_free(List node);

List list_push_typed(List list, const char *type, void *data)
{
  List new_head = list_node_alloc(data);
  new_head->type = type;
  new_head->next = list;
  return new_head;
}

List list_push_back_typed(List list, const char *type, void *data)
{
  List element = list_push_typed(NULL, type, data);
  if (list==NULL)
    return element;
  else
  {
    List l = list;
    while (l->next != NULL)
      l = l->next;
    l->next = element;
    return list;
  }
}

List list_pop(List list)
{
  List head;

  assert(list != NULL);

  head = list;
  list = list->next;
  list_node_free(head);
  return list;
}

List list_reverse(List list)
{
  List prev = NULL;
  while (list != NULL)
  {
    List next = list->next;
    list->next = prev;
    prev = list;
    list = next;
  }
  return prev;
}


void *list_head_typed(List list, const char *type)
{
  assert(list != NULL);
#ifdef USE_LIST_TYPEINFO
  if (strcmp(type, list->type) != 0)
    fatal("List: type incompatibility: requested <%s>, have <%s>", type, list->type);
#endif

  return list->data;
}


void list_delete(List list)
{
  while (list != NULL)
  {
    List head = list;
    list = list->next;
    list_node_free(head);
  }
}

size_t list_size(List list)
{
  size_t size = 0;
  while (list != NULL)
  {
    size++;
    list = list->next;
  }
  return size;
}


/**
 * Node allocation
 */

static List list_node_alloc(void *default_data)
{
  List node = (List) malloc(sizeof(ListNode));

  node->data = default_data;
  node->next = NULL;

  return node;
}

static void list_node_free(List node)
{
  free(node);
}


