#ifndef ABSTRACTQUEUE_H
#define ABSTRACTQUEUE_H

#include <cstddef>

class AbstractQueue
{
  public:
    AbstractQueue(): root(NULL) {}

    bool isEmpty() const { return root == NULL; }

  protected:
    struct Node
    {
      Node(void *v): value(v) {}
      void *value;
      Node *next;
      Node *prev;
    };

    void add_node(Node *node);
    Node *take_node();

  private:
    AbstractQueue(const AbstractQueue &) {}

    Node *root;
};

template<class T>
class Queue: public AbstractQueue
{
  public:
    ~Queue()
    {
      while (!isEmpty())
        delete pop();
    }

    void push(T *v)
    {
      add_node(new Node(reinterpret_cast<void *>(v)));
    }
    T *pop()
    {
      Node *node = take_node();
      T *ret = reinterpret_cast<T *>(node->value);
      delete node;
      return ret;
    }

    AbstractQueue &operator <<(T *v) { push(v); return *this; }
    AbstractQueue &operator >>(T *&v) { v = pop(); return *this; }
};

#endif // ABSTRACTQUEUE_H

