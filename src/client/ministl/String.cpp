#include <cstring>
#include "String.h"

String::String(const char *str)
  : m_length(strlen(str)), m_data(strlen(str)+1)
{
  strcpy(m_data.data(), str);
}

String::String(const String &str)
  : m_length(str.length()), m_data(str.length()+1)
{
  strcpy(m_data.data(), str.m_data.data());
}


String &String::operator =(const String &str)
{
  m_length = str.length();
  m_data.reserve(m_length+1);
  strcpy(m_data.data(), str.m_data.data());
  return *this;
}

String &String::operator +=(char c)
{
  m_data.reserve(m_length+2);
  m_data[m_length++] = c;
  m_data[m_length] = '\0';
  return *this;
}

String &String::operator +=(const String &str)
{
  m_data.reserve(m_length+str.length()+1);
  strcpy(m_data.data()+m_length, str.c_str());
  m_length += str.length();
  return *this;
}

bool String::operator ==(const char *str) const
{
  return 0 == strcmp(m_data.data(), str);
}
 
