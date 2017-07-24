#include "sbsms.h"
#include <stdlib.h>
#include <stdio.h>
#include <assert.h>
#include "sincCoeffs.h"
#include "real.h"
#include "utils.h"
#include <algorithm>
#include "buffer.h"
using namespace std;

namespace _sbsms_ {

#define SBSMS_RESAMPLE_CHUNK_SIZE 8192L

  
class ResamplerImp {
 public:
  ResamplerImp(SBSMSResampleCB func, void *data, SlideType slideType = SlideConstant);
  ResamplerImp(SampleBuf *in, float ratio, SlideType slideType = SlideConstant);
  ~ResamplerImp();
  inline long read(audio *audioOut, long frames);
  inline void writingComplete();
  inline void reset();
  inline void init();
  inline long samplesInOutput();

 protected:
  SBSMSFrame frame;
  countType startAbs;
  countType midAbs;
  float midAbsf;
  countType endAbs;
  countType writePosAbs;
  bool bInput;
  SampleBuf *in;
  SampleBuf *out;
  SBSMSResampleCB cb;
  void *data;
  bool bPull;
  long inOffset;
  SlideType slideType;
  Slide *slide;
  bool bWritingComplete;
};

Resampler :: Resampler(SBSMSResampleCB cb, void *data, SlideType slideType)
{
  imp = new ResamplerImp(cb,data,slideType);
}

ResamplerImp :: ResamplerImp(SBSMSResampleCB cb, void *data, SlideType slideType)
{
  init();
  this->cb = cb;
  this->data = data;
  bPull = true;
  bInput = true;
  this->slideType = slideType;
  frame.size = 0;
}


Resampler :: Resampler(SampleBuf *in, float ratio, SlideType slideType)
{
  imp = new ResamplerImp(in,ratio,slideType);
}

ResamplerImp :: ResamplerImp(SampleBuf *in, float ratio, SlideType slideType)
{
  init();
  bPull = false;
  this->in = in;
  bInput = true;
  this->slideType = slideType;
  frame.ratio0 = ratio;
  frame.ratio1 = ratio;
}

void ResamplerImp :: init()
{
  inOffset = 0;
  startAbs = 0;
  midAbs = 0;
  endAbs = 0;
  writePosAbs = 0;
  midAbsf = 0.0f;
  out = new SampleBuf(0);
  slide = NULL;
  bWritingComplete = false;
}

void Resampler :: reset() {  imp->reset(); }
void ResamplerImp :: reset()
{
  if(slide) delete slide;
  delete out;
  init();
  frame.size = 0;
  bInput = true;
}

Resampler :: ~Resampler() { delete imp; }
ResamplerImp :: ~ResamplerImp()
{
  if(slide) delete slide;
  delete out;
}

void updateSlide(Slide *slide, float *f, float *scale, int *maxDist, float *ratio)
{
  float stretch = slide->getStretch();
  slide->step();
  if(stretch <= 1.0f) {
    *f = SBSMS_SINC_RES;
    *scale = stretch;
    *maxDist = lrintf(SBSMS_SINC_SAMPLES);
  } else {
    *f = SBSMS_SINC_RES / stretch;
    *scale = 1.0f;
    *maxDist = lrintf(SBSMS_SINC_SAMPLES * stretch);
  }
  *ratio = stretch;
}

long Resampler :: read(audio *audioOut, long samples) { return imp->read(audioOut,samples); }
long ResamplerImp :: read(audio *audioOut, long samples)
{
  if(!bPull) {
    frame.buf = in->getReadBuf();
    frame.size = in->nReadable();
    if(frame.size) {
      bInput = true;
      if(slide) delete slide;
      slide = new Slide(slideType,1.0f/frame.ratio0,1.0f/frame.ratio1,frame.size);
    }
  }
  
  long nRead = out->nReadable();
  while(nRead < samples && bInput) {
    if(bInput && inOffset == frame.size) {
      if(!bPull) {
        in->advance(frame.size);
        bInput = false;
      } else {
        cb(data,&frame);
        if(frame.size) {
          if(slide) delete slide;
          slide = new Slide(slideType,1.0f/frame.ratio0,1.0f/frame.ratio1,frame.size);
        } else {
          bWritingComplete = true;
        }
      }
      if(bWritingComplete) {
        bInput = false;
        long n = (long)(midAbs - writePosAbs);
        out->grow(n);
        out->writePos += n;
      }
      inOffset = 0;
    }
    if(frame.size) {
      if(slideType == SlideIdentity) {
        out->write(frame.buf,frame.size);
        inOffset = frame.size;
      } else {
        bool bNoSinc = false;
        if(fabs(frame.ratio0-1.0f) < 1e-6f && 
           fabs((float)(frame.ratio1 - frame.ratio0)/(float)frame.size) < 1e-9f) {
          bNoSinc = true;
        }
        float f;
        float scale;
        int maxDist;
        float ratio;
        updateSlide(slide,&f,&scale,&maxDist,&ratio);
        int fi = lrintf(f);
        float ff = f - fi;
        if(ff<0.0f) {
          ff += 1.0f;
          fi--;
        }
        // absolute start position
        startAbs = max((countType)0,midAbs-maxDist);
        // samples to advance
        long advance = max(0L,(long)(startAbs - maxDist - writePosAbs));
        writePosAbs += advance;
        // absolute end position
        endAbs = midAbs + maxDist;
        // starting position in output
        long start = (long)(startAbs - writePosAbs);
        assert(start>=0);
        // zero point in output
        long mid = (long)(midAbs - writePosAbs);
        // ending position in output
        long end = (long)(endAbs - writePosAbs);
        // advance
        out->writePos += advance;
        
        if(bNoSinc) {
          // how far ahead to write
          int nAhead = mid+frame.size;
          out->N = nAhead;
          out->grow(nAhead);
          long nWrite = min(SBSMS_RESAMPLE_CHUNK_SIZE,frame.size-inOffset);
          for(int j=0;j<nWrite;j++) {
            out->buf[out->writePos+mid+j][0] += frame.buf[j+inOffset][0];
            out->buf[out->writePos+mid+j][1] += frame.buf[j+inOffset][1];
          }
          inOffset += nWrite;
          midAbsf += nWrite;
          int nWritten = lrintf(midAbsf);
          midAbsf -= nWritten;
          midAbs += nWritten;
        } else {
          long nWrite = min(SBSMS_RESAMPLE_CHUNK_SIZE,frame.size-inOffset);
          audio *i = &(frame.buf[inOffset]);
          for(int j=0;j<nWrite;j++) {
            // how far ahead to write
            int nAhead = end;
            out->N = nAhead;
            out->grow(nAhead);
            audio *o = &(out->buf[out->writePos+start]);
            float d = (start-mid-midAbsf)*f;
            int di = lrintf(d);
            float df = d-di;
            if(df<0.0f) {
              df += 1.0f;
              di--;
            }
            float i0 = (*i)[0];
            float i1 = (*i)[1];
            for(int k=start;k<end;k++) {
              int k0 = (di<0)?-di:di; 
              int k1 = (di<0)?k0-1:k0+1;
              float sinc;
              if(k1>=SBSMS_SINC_SIZE) {
                if(k0>=SBSMS_SINC_SIZE) {
                  sinc = 0.0f;
                } else {
                  sinc = scale*sincTable[k0];
                }
              } else if(k0>=SBSMS_SINC_SIZE) {
                sinc = scale*sincTable[k1];
              } else {
                sinc = scale*((1.0f-df)*sincTable[k0] + df*sincTable[k1]);
              }
              (*o)[0] += i0 * sinc;
              (*o)[1] += i1 * sinc;
              di += fi;
              df += ff;
              if(!(df<1.0f)) {
                df -= 1.0f;
                di++;
              }
              o++;
            }
            i++;

            updateSlide(slide,&f,&scale,&maxDist,&ratio);
            fi = lrintf(f);
            ff = f - fi;
            if(ff<0.0f) {
              ff += 1.0f;
              fi--;
            }
            midAbsf += ratio;          
            int nWritten = lrintf(midAbsf);
            midAbsf -= nWritten;
            midAbs += nWritten;
            startAbs = max((countType)0,midAbs-maxDist);
            endAbs = midAbs + maxDist;
            start = (long)(startAbs - writePosAbs);
            mid = (long)(midAbs - writePosAbs);
            end = (long)(endAbs - writePosAbs);
          }
          inOffset += nWrite;
        }
      }
      nRead = out->nReadable();
    }
  }

  nRead = out->read(audioOut,samples);

  return nRead;
}


long Resampler :: samplesInOutput() { return imp->samplesInOutput(); }
long ResamplerImp :: samplesInOutput()
{
  long samplesFromBuffer = lrintf(0.5f*(frame.ratio0+frame.ratio1)*(frame.size-inOffset));
  return (long)(out->writePos + (midAbs - writePosAbs) - out->readPos + samplesFromBuffer);
}

void ResamplerImp :: writingComplete()
{
  bWritingComplete = true;
}

}
