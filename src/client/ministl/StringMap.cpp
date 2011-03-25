#include "StringMap.h"
    
size_t StringMap::at(const String &str) const
{
  size_t pos;
  if (find(str, pos))
    return m_data[pos].val;
  else throw "StringMap: bad index";
}

size_t &StringMap::operator [](const String &str)
{
  size_t pos;
  if (!find(str, pos))
    pos = insert(str, 0);
  return m_data[pos].val;
}

bool StringMap::find(const String &str, size_t &where) const
{
  for (size_t i=0; i<m_data.size(); i++)
    if (str == m_data[i].str)
    {
      where = i;
      return true;
    }
  return false;
}

size_t StringMap::insert(const String &str, size_t val)
{
  m_data.push_back(Binding(str, val));
  return m_data.size()-1;
}

