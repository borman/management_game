#ifndef QUEUE_H
#define QUEUE_H

template<class T>
class Queue
{
  public:
    Queue()
      : m_front(NULL), m_back(NULL) {}

    T &front() { return m_front->data; }
    void pop() 
    {
      Node *t = m_front;
      if (m_front == m_back)
        m_front = m_back = NULL;
      else
        m_front = m_front->next;
      delete t;
    }

    void push(const T &v)
    {
      Node *node = new Node(v);
      if (m_front==NULL)
        m_front = m_back = node;
      else
      {
        m_back->next = node;
        m_back = node;
      }
    }

    bool empty() const { return m_front == NULL; }
  private:
    struct Node
    {
      Node(const T &d): next(NULL), data(d) {}
      Node *next;
      T data;
    };
    Node *m_front;
    Node *m_back;
};

#endif // QUEUE_H
