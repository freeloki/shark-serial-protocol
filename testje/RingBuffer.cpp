/*
  Ring buffer Library.
  Created by Pirmin Kalberer.
  Released into the public domain.
*/

#include "RingBuffer.h"

// AVR LibC Includes
#include <stdlib.h>

RingBuffer::RingBuffer(int size)
: size(size), start(0), end(0), cnt(0)
{
  buffer = (unsigned char*)calloc(size, sizeof(unsigned char));
}

RingBuffer::~RingBuffer()
{
  free(buffer);
}

bool RingBuffer::push(unsigned char value)
{
  buffer[end] = value;
  if (++end > size) end = 0;
  if (cnt == size) {
    if (++start > size) start = 0;
    return false;
  } else {
    ++cnt;
    return true;
  }
}

int RingBuffer::pop(unsigned char& value)
{
  value = buffer[start];
  if (cnt > 0)
  {
    --cnt;
    if (++start > size) start = 0;
    return cnt;
  }else
  {
      return -1;
  }
}

unsigned char RingBuffer::peek()
{
  return buffer[start];
}

unsigned int RingBuffer::count()
{
  return cnt;
}

void RingBuffer::reset()
{
    start=cnt=end=0;
}
