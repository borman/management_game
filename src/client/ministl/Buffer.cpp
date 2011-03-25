#include <cstdlib>
#include <new>

#include "Buffer.h"

BufferBackend::BufferBackend(size_t init_size)
  : mem_size(0), mem_data(NULL)
{
  rawReserve(init_size);
}

BufferBackend::~BufferBackend()
{
  if (mem_data != NULL)
    free(mem_data);
}

void BufferBackend::rawReserve(size_t size)
{
  if (size <= mem_size) 
    return;
  mem_size = ((size + mem_block_size-1) / mem_block_size) * mem_block_size;

  if (mem_data == 0)
    mem_data = malloc(mem_size);
  else
    mem_data = realloc(mem_data, mem_size);

  if (mem_data == NULL)
    throw std::bad_alloc();
}

