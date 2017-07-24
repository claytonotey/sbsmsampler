#include <stdlib.h>
#include <string.h>
#include <assert.h>
#include "buffer.h"
#include "sbsms.h"
#include "utils.h"
#include <algorithm>
using namespace std;

namespace _sbsms_ {

#define INIT_SAMPLEBUF_LENGTH 8192
#define INIT_GRAINBUF_LENGTH 128

/****************

 SampleBuf

****************/

SampleBuf :: SampleBuf(int N) 
{
  init(N,0);
}

SampleBuf :: SampleBuf(int N, long delay) 
{
  init(N,delay);
}

void SampleBuf :: init(int N, long delay)
{
  this->delay = delay;
  this->N = N;
  this->length = INIT_SAMPLEBUF_LENGTH;
  this->buf = make_audio_buf(2*length);
  this->readPos = 0;
  this->writePos = 0;
}

SampleBuf :: ~SampleBuf() 
{
  free_audio_buf(buf);
}

long SampleBuf :: write(audio *in, long n)
{
  if(n==0) {
    return 0;
  }
  grow(n);
  memcpy(buf+writePos,in,n*sizeof(audio));
  writePos += n;
  return n;
}

void SampleBuf :: grow(long n)
{
  long pos = writePos+n;
  while(pos >= 2*length) {
    length *= 2;
    audio *newBuf = make_audio_buf(2*length);
    memcpy(newBuf,buf+readPos,(length-readPos)*sizeof(audio));
    free_audio_buf(buf);
    buf = newBuf;
    writePos -= readPos;
    pos -= readPos;
    readPos = 0;
  }
}

long SampleBuf :: write(grain *g, int h)
{
  grow(N);
  g->synthesize();
  float f = 2.6666666666666666666666666f/(float)(N/h);
  
  for(int c=0;c<2;c++) {
    int j = 0;
    for(int k=writePos; k<writePos+N; k++) {
      buf[k][c] += g->x[j++][c] * f;
    }
  }
  writePos += h;
  return h;
}

long SampleBuf :: read(audio *outBuf, long n)
{
  if(n==0) return 0;
  n = max(0L,min(n,nReadable()));
  memcpy(outBuf,buf+readPos,n*sizeof(audio));
  advance(n);
  return n;
}

long SampleBuf :: nReadable()
{
  return max(0L,writePos-delay-readPos);
}

void SampleBuf :: advance(long n) {
  assert(readPos+n <= writePos);
  assert(writePos+N <= (length<<1));
  memset(buf+readPos,0,n*sizeof(audio));
  readPos += n;
  if(readPos >= length) {
    long endPos;
    endPos = writePos+N;
    memcpy(buf,buf+readPos,(endPos-readPos)*sizeof(audio));
    memset(buf+readPos,0,((length<<1)-readPos)*sizeof(audio));
    writePos -= readPos;
    readPos = 0;
  }
}

audio *SampleBuf :: getReadBuf() 
{
  return (buf+readPos);
}

void SampleBuf :: clear()
{
  advance(writePos-readPos);
}

/****************

 GrainBuf

****************/

GrainBuf :: GrainBuf(int N, int h, int N2, int type)
{
  this->length = INIT_GRAINBUF_LENGTH;
  this->buf = (grain**) calloc(2*length,sizeof(grain*));
  this->iBuf = (audio*) calloc(N,sizeof(audio));
  this->grainAllocator = new GrainAllocator(N,N2,type);
  this->N = N;
  this->h = h;
  this->iBufWritePos = 0;
  this->readPos = 0;
  this->writePos = 0;
}

GrainBuf :: ~GrainBuf() 
{
  clear();
  free(buf);
  free_audio_buf(iBuf);
  delete grainAllocator;
}

long GrainBuf :: write(audio *buf2, long n)
{
  if(n==0) {
    return 0;
  }
  long ng = 0;
  int overlap = N - h;
  long bufReadPos = 0;

  while(bufReadPos<n) {
    long nToCopy = min((n-bufReadPos),N-iBufWritePos);
    if(nToCopy+iBufWritePos == N) {
      memcpy(iBuf+iBufWritePos, buf2+bufReadPos, nToCopy*sizeof(audio));
      grain *g = grainAllocator->create();
      memcpy(g->x,iBuf,N*sizeof(audio));
      g->h = h;
      write(g);
      ng++;
      memcpy(iBuf,iBuf+h,overlap*sizeof(audio));
      iBufWritePos = overlap;
      bufReadPos += nToCopy;
    } else break;
  }

  // copy the remainder to the iBuf
  long nToCopy = min((n-bufReadPos),N-iBufWritePos);
  memcpy(iBuf+iBufWritePos, buf2+bufReadPos, nToCopy*sizeof(audio));
  iBufWritePos += nToCopy;
    
  return ng;
}

void GrainBuf :: advance(long n)
{
  assert(readPos+n <= writePos);
  for(int k=readPos;k<readPos+n;k++) {
    grainAllocator->forget(buf[k]);
  }
  readPos += n;
  if(readPos >= length) {
    memcpy(buf,buf+readPos,(writePos-readPos)*sizeof(grain*));
    writePos = writePos - readPos;
    readPos = 0;
  }
}

grain* GrainBuf :: read(long k) 
{
  return buf[k];
}

long GrainBuf :: nReadable()
{
  return writePos - readPos;
}

void GrainBuf :: write(grain *g) 
{
  if(writePos >= 2*length) {
    length *= 2;
    grain **newBuf = (grain**)calloc(2*length,sizeof(grain*));
    memcpy(newBuf,buf+readPos,(writePos-readPos)*sizeof(grain*));
    free(buf);
    buf = newBuf;
    writePos -= readPos;
    readPos = 0;
  }

  grainAllocator->reference(g);
  buf[writePos++] = g;
}

void GrainBuf :: reference(grain *g)
{
  grainAllocator->reference(g);
}

void GrainBuf :: forget(grain *g)
{
  grainAllocator->forget(g);
}

void GrainBuf :: clear()
{
  memset(iBuf,0,N*sizeof(audio));
  iBufWritePos = 0;
  advance(nReadable());
}

/****************

 Mixer

****************/

Mixer :: Mixer(SampleBufBase *b1, SampleBuf *b2)
{
  this->b1 = b1;
  this->b2 = b2;
}

long Mixer :: read(audio *outBuf, long n)
{
  if(n==0) return 0;
  n = min(n,b2->nReadable());
  n = b1->read(outBuf,n);
  audio *buf2 = b2->getReadBuf();
  for(int k=0;k<n;k++) {
    for(int c=0;c<2;c++) 
      outBuf[k][c] += buf2[k][c];
  }
  b2->advance(n);
  return n;
}

}
