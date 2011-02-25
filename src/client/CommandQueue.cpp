#include <cassert>

#include "CommandQueue.h"

CommandQueue::CommandQueue()
  : root(NULL) 
{
}

CommandQueue::~CommandQueue()
{
  while (!isEmpty())
  {
    Command *cmd;
    (*this) >> cmd;
    delete cmd;
  }
}

CommandQueue &CommandQueue::operator <<(Command *cmd)
{
  Node *node = new Node;
  node->cmd = cmd;

  node->next = root;
  node->prev = root->prev;
  root->prev->next = node;
  root->prev = node;

  return *this;
}

CommandQueue &CommandQueue::operator >>(Command *&cmd)
{
  assert(!isEmpty());

  cmd = root->cmd;

  Node *new_root = root->next;
  root->next->prev = root->prev;
  root->prev->next = root->next;
  delete root;
  root = new_root;

  return *this;
}

