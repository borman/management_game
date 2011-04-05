#ifndef MAP_H
#define MAP_H

#include <cstddef>
#include "Vector.h"

template<class K, class V>
class Map
{
  public:
    // Exception
    class BadIndex {};

    const V &at(const K &key) const
    {
      size_t pos;
      if (find(key, pos))
        return m_data[pos].val;
      else throw BadIndex();
    }
    
    const V &operator [](const K &key) const { return at(key); }
    
    V &operator [](const K &key)
    {
      size_t pos;
      if (!find(key, pos))
        pos = insert(key, 0);
      return m_data[pos].val;
    }

    size_t count(const K &key) const
    {
      size_t pos;
      if (find(key, pos))
        return 1;
      else
        return 0;
    }

  private:
    struct Binding
    {
      Binding(const K &k=K(), const V &v=V()): key(k), val(v) {}
      K key;
      V val;
    };

    bool find(const K &key, size_t &where) const
    {
      for (size_t i=0; i<m_data.size(); i++)
        if (key == m_data[i].key)
        {
          where = i;
          return true;
        }
      return false;
    }

    size_t insert(const K &key, const V &val)
    {
      m_data.push_back(Binding(key, val));
      return m_data.size()-1;
    }

    Vector<Binding> m_data;
};

#endif // MAP_H
