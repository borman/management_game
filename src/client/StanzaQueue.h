#ifndef STANZAQUEUE_H
#define STANZAQUEUE_H

#include "Stanza.h"

class StanzaQueue
{
  public:
    StanzaQueue();
    ~StanzaQueue();

    StanzaQueue &operator <<(Stanza *cmd);
    StanzaQueue &operator >>(Stanza *&cmd);

    bool isEmpty() const { return root == NULL; }
  private:
    struct Node
    {
      Stanza *cmd;
      Node *next;
      Node *prev;
    };

    StanzaQueue(const StanzaQueue &) {}

    Node *root;
};

#endif // STANZAQUEUE_H

