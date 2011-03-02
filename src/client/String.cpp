#include <cstdio>
#include <cstring>

#include "String.h"
#include "Exceptions.h"

unsigned int AbstractConstString::toUInt() const
{
  unsigned int v;
  int retval = sscanf(constData(), "%u", &v);
  if (retval==EOF || retval<1)
    throw StringFormatException("toUInt");
  return v;
}

bool AbstractConstString::operator==(const AbstractConstString &str) const
{
  return 0==strcmp(constData(), str.constData());
}

char AbstractConstString::operator[](size_t index) const
{
  return constData()[index];
}
