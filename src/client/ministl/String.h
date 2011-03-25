#ifndef STRING_H
#define STRING_H

#include <cstddef>
#include "Buffer.h"

class String
{
  public:
    String(const char *str = "");
    String(const String &str);

    size_t length() const { return m_length; }
    const char *c_str() const { return m_data.data(); }
    bool empty() const { return length()==0; }
    void clear() { *this = ""; }

    String &operator =(const String &str);
    String &operator +=(char c);
    String &operator +=(const String &str);

    bool operator ==(const char *str) const; 
    bool operator ==(const String &str) const 
      { return *this == str.m_data.data(); }

    bool operator !=(const char *str) const
      { return !(*this == str); }
    bool operator !=(const String &str) const 
      { return !(*this == str.m_data.data()); }

  private:
    size_t m_length;
    Buffer<char> m_data;
};

#endif // STRING_H
