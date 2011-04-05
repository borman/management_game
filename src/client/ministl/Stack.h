#ifndef STACK_H
#define STACK_H

#include <cstddef>
#include "Vector.h"

template<class T>
class Stack
{
  public:
    T &top() { return m_data[m_data.size()-1]; }

    void pop() { m_data.pop_back(); }
    void push(const T &v) { m_data.push_back(v); }
    bool empty() const { return m_data.empty(); }

    // Introspection
    size_t size() const { return m_data.size(); }
    const T &operator [](size_t i) const { return m_data[i]; }

  private:
    Vector<T> m_data;
};

#endif // STACK_H
