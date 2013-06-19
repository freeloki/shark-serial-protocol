/*
  Ring buffer Library.
  Created by Pirmin Kalberer.
  Released into the public domain.
*/

#ifndef RingBuffer_h
#define RingBuffer_h

class RingBuffer
{
private:
  int size;
  unsigned char* buffer;
  int start;
  int end;
  int cnt;

public:
  RingBuffer(int size);
  ~RingBuffer();
  bool push(unsigned char value);
  int pop(unsigned char& value);
  unsigned char peek();
  unsigned int count();
  void reset();
};

#endif
