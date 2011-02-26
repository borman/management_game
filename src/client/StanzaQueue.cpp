#include <cassert>

#include "StanzaQueue.h"

StanzaQueue::StanzaQueue()
  : root(NULL) 
{
}

StanzaQueue::~StanzaQueue()
{
  while (!isEmpty())
  {
    Stanza *cmd;
    (*this) >> cmd;
    delete cmd;
  }
}

StanzaQueue &StanzaQueue::operator <<(Stanza *cmd)
{
  Node *node = new Node;
  node->cmd = cmd;

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

  return *this;
}

StanzaQueue &StanzaQueue::operator >>(Stanza *&cmd)
{
  assert(!isEmpty());

  cmd = root->cmd;

  Node *old_root = root;
  root = root->next;
  if (root == old_root)
    root = NULL;
  else
  {
    old_root->next->prev = old_root->prev;
    old_root->prev->next = old_root->next;
  }
  delete old_root;

  return *this;
}

