/*
 * Copyright 2010, Mikhail "borman" Borisov <borisov.mikhail@gmail.com>
 *
 * This file is part of borman's management game server.
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


#ifndef LIST_H
#define LIST_H

#include <stddef.h>
#include <stdint.h>

/**
 * Lists module.
 *
 * Main entity is List type. 
 * No memory management is exposed.
 * All subroutines work listwise.
 *
 * User is not expected to modify list nodes' contents in any way
 * except by using this module's API.
 *
 * TODO: Document list typeinfo management
 */

/* This type is supposed to be able to store data of any scalar type */
typedef size_t ListItem;

typedef struct ListNode
{
  ListItem data;
  const char *type;
  struct ListNode *next;
} ListNode;
typedef ListNode *List;

typedef int (*ListItemPredicate)(ListItem item);
typedef void (*ListItemDestructor)(ListItem item);

#if defined(USE_LIST_TYPEINFO) 
# define L_TYPE(type) #type
#else
# define L_TYPE(type) NULL
#endif

#define list_push(list, type, data) (list_push_typed(list, L_TYPE(type), (ListItem)(data)))
#define list_push_back(list, type, data) (list_push_back_typed(list, L_TYPE(type), (ListItem)(data)))
#define list_head(list, type)       ((type)list_head_typed(list, L_TYPE(type)))
#define list_filter(list, type, pred, destr) (list_filter_typed(list, L_TYPE(type), pred, destr))

#define FOREACH(type, var, list) \
do \
{ \
  List _list = list; \
  while (_list != NULL) \
  { \
    type var = list_head(_list, type); \
    do

#define FOREACH_END \
    while (0); \
    _list = _list->next; \
  } \
} while (0)


/**
 * Insert a new element before list head.
 * Returns a new list.
 */
List list_push_typed(List list, const char *type, ListItem data);
/**
 * Insert a new element after its last element.
 * Returns a new list.
 */
List list_push_back_typed(List list, const char *type, ListItem data);
/**
 * Remove list head.
 * Returns a new list.
 */
List list_pop(List list);
/**
 * Reverse list.
 * Returns a new pointer to this list, old pointer becomes invalid.
 */
List list_reverse(List list);

/** 
 * Extract list's head.
 */
ListItem list_head_typed(List list, const char *type);

/**
 * Delete items for which predicate(item) evaluates to true.
 * If destr is not NULL, it is called for each item deleted
 */
List list_filter_typed(List list, const char *type, 
    ListItemPredicate pred, ListItemDestructor destr);

/** 
 * Free memory used by this list.
 */
void list_delete(List list);

/**
 * Calculate list size
 */
size_t list_size(List list);

#endif /* LIST_H */

