#ifndef TEXTBUFFER_H
#define TEXTBUFFER_H

#include <cstddef>

class TextBuffer
{
  public:
    TextBuffer(size_t base_size = 0);
    ~TextBuffer();

    TextBuffer &operator<<(const char *str);
    TextBuffer &operator<<(char c);

    void clear() { size = 0; }

    const char *c_str() const { return data; }

  private:
    void provideSize(size_t new_size);

    char *data;
    size_t mem_size;
    size_t size;
};

#endif // TEXTBUFFER_H
