#ifndef COMMANDQUEUE_H
#define COMMANDQUEUE_H

#include "Command.h"

class CommandQueue
{
  public:
    CommandQueue();
    ~CommandQueue();

    CommandQueue &operator <<(Command *cmd);
    CommandQueue &operator >>(Command *&cmd);

    bool isEmpty() const { return root == NULL; }
  private:
    struct Node
    {
      Command *cmd;
      Node *next;
      Node *prev;
    };

    CommandQueue(const CommandQueue &) {}

    Node *root;
};

#endif // COMMANDQUEUE_H

