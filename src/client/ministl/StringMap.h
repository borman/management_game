#ifndef STRINGMAP_H
#define STRINGMAP_H

#include <cstddef>
#include "Vector.h"
#include "String.h"

class StringMap
{
  public:
    size_t at(const String &str) const;
    size_t operator [](const String &str) const { return at(str); }
    size_t &operator [](const String &str);
  private:
    bool find(const String &str, size_t &where) const;
    size_t insert(const String &str, size_t val);
    struct Binding
    {
      Binding(const String &s=String(), size_t v=0): str(s), val(v) {}
      String str;
      size_t val;
    };
    Vector<Binding> m_data;
};

#endif // STRINGMAP_H
