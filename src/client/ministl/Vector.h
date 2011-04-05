#ifndef VECTOR_H
#define VECTOR_H

#include "Buffer.h"

template<class T>
class Vector
{
  public:
    Vector(size_t size = 0): m_size(0) { resize(size); }
    ~Vector() { clear(); }

    Vector(const Vector<T> &other)
      : m_size(other.size()), m_data(other.size())
    {
      for (size_t i=0; i<m_size; i++)
        m_data[i] = new T(*other.m_data[i]);
    }

    Vector<T> &operator=(const Vector<T> &other)
    {
      resize(other.size());
      for (size_t i=0; i<m_size; i++)
        *m_data[i] = T(*other.m_data[i]);
      return *this;
    }

    size_t size() const { return m_size; }
    bool empty() const { return m_size==0; }
  
    void clear()
    {
      resize(0);
    }

    void resize(size_t new_size)
    {
      if (new_size>m_size)
      {
        m_data.reserve(new_size);
        for (size_t i=m_size; i<new_size; i++)
          m_data[i] = new T();
      }
      else if (new_size < m_size)
      {
        for (size_t i=new_size; i<m_size; i++)
        {
          delete m_data[i];
          m_data[i] = NULL;
        }
      }
      m_size = new_size;
    }

    T &operator [](size_t index) { return *m_data[index]; }
    const T &operator [](size_t index) const { return *m_data[index]; }

    void push_back(const T &v)
    {
      m_data.reserve(m_size+1);
      m_data[m_size++] = new T(v);
    }
    void pop_back()
    {
      resize(m_size-1);
    }
  private:
    size_t m_size;
    Buffer<T *> m_data;
};

#endif // VECTOR_H
