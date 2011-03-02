#include <cassert>

#include "Queue.h"

void AbstractQueue::add_node(AbstractQueue::Node *node)
{
  if (root == NULL)
  {
    node->next = node;
    node->prev = node;
    root = node;
  }
  else
  {
    node->next = root;
    node->prev = root->prev;
    root->prev->next = node;
    root->prev = node;
  }
}

AbstractQueue::Node *AbstractQueue::take_node()
{
  assert(!isEmpty());

  Node *old_root = root;
  root = root->next;
  if (root == old_root)
    root = NULL;
  else
  {
    old_root->next->prev = old_root->prev;
    old_root->prev->next = old_root->next;
  }

  return old_root;
}

