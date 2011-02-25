#include <cstring>
#include "TextBuffer.h"

TextBuffer::TextBuffer(size_t base_size = 0)
{
  if (base_size == 0)
    mem_size = 256;
  else
    mem_size = base_size;

  data = (char *) malloc(mem_size);
  size = 0;
}

TextBuffer::~TextBuffer()
{
  free(data);
}

TextBuffer &TextBuffer::operator<<(const char *str)
{
  const size_t length = strlen(str);
  provideSize(size+length+1);
  strcpy(data+size, str);
}

void TextBuffer::provideSize(size_t new_size)
{
  if (new_size <= mem_size)
   return; 
  else
  {
    while (new_size > mem_size)
      mem_size *= 2;
    data = (char *) realloc(data, mem_size);
  }
}
