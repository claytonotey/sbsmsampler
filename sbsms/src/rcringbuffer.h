#ifndef RCRINGBUFFER_H
#define RCRINGBUFFER_H
#include <stdlib.h>
#include <assert.h>

template<class T>
class ReferenceCountedBuffer
{
 public:
  ReferenceCountedBuffer(int n);
  ~ReferenceCountedBuffer();
  inline void remember();
  inline void forget();
  inline operator T*() const { return buf; }
 protected:
  T *buf;
  int count;
};

template<class T>
ReferenceCountedBuffer<T> :: ReferenceCountedBuffer(int n)
{
  buf = (T*)calloc(n,sizeof(T));
  count = 0;
}

template<class T>
ReferenceCountedBuffer<T> :: ~ReferenceCountedBuffer()
{
  free(buf);
}

template<class T>
void ReferenceCountedBuffer<T> :: remember()
{
  count++;
}

template<class T>
void ReferenceCountedBuffer<T> :: forget()
{
  if(--count <= 0) delete this;
}

template<class T>
class ReferenceCountedRingBuffer
{
 public:
  ReferenceCountedRingBuffer(int length, int N);
  ~ReferenceCountedRingBuffer();

  void write(T *buf, long n);
  void advance(long n);
  void getReadBuf(ReferenceCountedBuffer<T> **buf, long *offset);

 protected:
  long readPos, writePos;
  int N;
  long length;
  ReferenceCountedBuffer<T> *buf;  
};

template<class T>
ReferenceCountedRingBuffer<T> :: ReferenceCountedRingBuffer(int length, int N) 
{
  this->length = length;
  this->N = N;
  this->buf = new ReferenceCountedBuffer<T>(length<<1);
  buf->remember();
  this->readPos = 0;
  this->writePos = 0;
}

template<class T>
ReferenceCountedRingBuffer<T> :: ~ReferenceCountedRingBuffer() 
{
  buf->forget();
}

template<class T>
void ReferenceCountedRingBuffer<T> :: write(T *in, long n)
{
  if(in) {
    memcpy(buf+writePos,in,n*sizeof(T));
  } else {
    memset(buf,0,n*sizeof(T));
  }
  writePos += n;
}

template<class T>
void ReferenceCountedRingBuffer<T> :: advance(long n) {
  assert(readPos+n <= writePos);
  assert(writePos+N <= (length<<1));
  readPos += n;
  if(readPos >= length) {
    long endPos;
    endPos = writePos+N;
    ReferenceCountedBuffer<T> *newBuf = new ReferenceCountedBuffer<T>(length<<1);
    memcpy(newBuf,buf+readPos,(endPos-readPos)*sizeof(T));
    buf->forget();
    buf = newBuf;
    buf->remember();
    writePos -= readPos;
    readPos = 0;
  }
}

template<class T>
void ReferenceCountedRingBuffer<T> :: getReadBuf(ReferenceCountedBuffer<T> **buf, long *offset) 
{
  *buf = this->buf;
  *offset = readPos;
}

#endif
