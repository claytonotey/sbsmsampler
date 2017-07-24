#include <stdlib.h>
#include <stdio.h>

#include "pcm.h"
#include <algorithm>
using namespace std;

#define PCM_READ_BUF_SIZE 4096L
#define PCM_WRITE_BUF_SIZE 4096L

long PcmReader :: read(float *buf, long block_size) {
  if(info.channels == 1) {
    float srcbuf[2*PCM_READ_BUF_SIZE];
    long nread = -1;
    long nreadTotal = 0;
    while(nreadTotal < block_size && nread) {
      long ntoread = min(PCM_READ_BUF_SIZE,block_size-nreadTotal);
      nread = (long)sf_readf_float(in, srcbuf, ntoread);
      for(int i=0;i<nread;i++) {
		int i2 = (nreadTotal+i)<<1;
		buf[i2] = buf[i2+1] = srcbuf[i];
      }
      nreadTotal += nread;
    }
    if(nreadTotal == 0)
      bDone = true;
    return nreadTotal;
  } else if(info.channels == 2) {
    long nread = (long)sf_readf_float(in, buf, block_size);
    if(nread == 0)
      bDone = true;
    return nread;
  } else {
    abort();
	return 0;
  }
}

bool PcmReader :: done()
{
  return bDone;
}

PcmReader :: PcmReader(const char *filename) 
{
  total = 0;
  bDone = false;
  info.format = 0;
  in = sf_open(filename, SFM_READ, &info);
  
  bError = false;
  if (!in) {
    perror("cannot open file for reading");
    bError = true;
  }
}

bool PcmReader :: isError()
{
  return bError;
}

int PcmReader :: getSampleRate()
{
  return info.samplerate;
}

countType PcmReader :: getFrames()
{
  return info.frames;
}

int PcmReader :: getChannels()
{
  return info.channels;
}

PcmReader :: ~PcmReader() 
{  
  sf_close(in);
}

PcmWriter :: PcmWriter(const char *filename, sf_count_t size, int samplerate, int channels) 
{
  total = 0;
  info.format = SF_FORMAT_WAV | SF_FORMAT_FLOAT;
  info.frames = size;
  info.samplerate = samplerate;
  info.channels = channels;
  info.sections = 1;
  info.seekable = 0;

  if (!sf_format_check(&info))
    info.format = (info.format & SF_FORMAT_TYPEMASK);

  out = sf_open(filename, SFM_WRITE, &info);

  bError = false;
  if (!sf_format_check(&info)) {
    bError = true;
    perror("bad format for writing pcm");
  }
  if (!out) {
    perror("cannot open file for writing");
    bError = true;
  }
}

bool PcmWriter :: isError()
{
  return bError;
}

long PcmWriter :: write(float *data, long n)
{
  return (long)sf_writef_float(out, (float *)data, n);
}
 
PcmWriter :: ~PcmWriter()
{
}

void PcmWriter :: close() 
{
  if(out) sf_close(out);
}
