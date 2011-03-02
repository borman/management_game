#ifndef STRING_H
#define STRING_H

#include <cstddef>

// Interfaces

class AbstractConstString
{
  public:
    virtual const char *constData() const = 0;

    unsigned int toUInt() const;
    char operator[](size_t index) const;
    bool operator==(const AbstractConstString &str) const;
};

class AbstractString: public AbstractConstString
{
  public:
    virtual char *data() = 0;
};

// Implementations

class ConstString: public AbstractConstString
{
  public:
    ConstString(const char *str): data_ptr(str) {}

    virtual const char *constData() { return data_ptr; }
  private:
    const char *data_ptr;
};

/* Requires: Vector<char>

class String: public AbstractString
{
  public:
    explicit String(const char *str = "");
  private:
};

*/

#endif // STRING_H

