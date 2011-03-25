#ifndef BUFFER_H
#define BUFFER_H

#include <cstddef>

class BufferBackend
{
  public:
    BufferBackend(size_t init_size);
    ~BufferBackend();

    void *rawData() { return mem_data; }
    const void *rawData() const { return mem_data; }
    void rawReserve(size_t size);

  private:
    static const size_t mem_block_size = 256;
    size_t mem_size;
    void *mem_data;
};

template<class T>
class Buffer: private BufferBackend
{
  public:
    Buffer(size_t init_count = 0)
      : BufferBackend(init_count * sizeof(T)) {}

    T *data() { return static_cast<T *>(rawData()); }
    const T *data() const { return static_cast<const T *>(rawData()); }

    T &operator [](size_t index) { return data()[index]; }
    const T &operator [](size_t index) const { return data()[index]; }

    void reserve(size_t count) { rawReserve(count * sizeof(T)); }
};

#endif // BUFFER_H
