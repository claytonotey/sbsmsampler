#include "render.h"
#include "sbsms.h"
#include "real.h"
#include "vstsbsms.h"

SBSMSDrawRenderer :: SBSMSDrawRenderer(countType samplesToProcess, CFrame *frame, const CRect &rect, int leftOffset, int rightOffset)
{  
  x0 = leftOffset;
  context = new COffscreenContext(frame,rect.getWidth(),rect.getHeight());
  float Fs = 44100.0f;
  minF = 6.28318530717959f*(40.0f/Fs);
  maxF = 6.28318530717959f*(20000.0f/Fs);
  for(int k=0;k<256;k++) {
    color[0][k](k,0,0,255);
    color[1][k](0,k,0,255);
  }
  this->rect = rect;
  context->setFillColor(kBlackCColor);
  context->setFrameColor(kBlackCColor);
  context->fillRect(rect);
  width = rect.getWidth();
  height = rect.getHeight();
  xScale = (double)(width - leftOffset - rightOffset) / (double)samplesToProcess;
  yScale = (double)height / log(maxF / minF);
  context->setDrawMode(kOrMode);
  pthread_mutex_init(&drawMutex,NULL);
}

SBSMSDrawRenderer :: ~SBSMSDrawRenderer()
{
  delete context;
}

CCoord SBSMSDrawRenderer :: getX(countType samplePos)
{
  CCoord x = x0 + (CCoord)(xScale * (double)samplePos);
  if(x >= width) x = width - 1;
  else if(x < 0) x = 0;
  return x;
}

CCoord SBSMSDrawRenderer :: getY(float f)
{
  CCoord y = height - (f==0.0f?0:(long)(yScale * log(f / minF)));
  if(y >= height) y = height - 1;
  else if(y < 0) y = 0;
  return y;
}

CColor SBSMSDrawRenderer :: getColor(int c, float m)
{
  int k = (m==0.0f?0:lrintf((0.1f*log(m)+1.0f)*255.0f));
  if(k<0) k = 0;
  if(k>255) k = 255;
  return color[c][k];
}

void SBSMSDrawRenderer :: render(int c, countType samplePos, long nSamples, float f0, float f1, float y0, float y1)
{
  if(y0 > 0.0f && y1 > 0.0f && f0 > minF && f0 < maxF && f1 > minF && f1 < maxF) {
    CPoint p0(getX(samplePos),getY(f0));
    CPoint p1(getX(samplePos+nSamples),getY(f1));
    CColor col = getColor(c,0.5f*(y0 + y1));
    pthread_mutex_lock(&drawMutex);
    context->moveTo(p0);        
    context->setFrameColor(col);
    context->lineTo(p1);    
    pthread_mutex_unlock(&drawMutex);
  }
}

void SBSMSDrawRenderer :: copyFrom(COffscreenContext *c, const CRect &r)
{
  pthread_mutex_lock(&drawMutex);
  context->copyFrom(c,r);
  pthread_mutex_unlock(&drawMutex);
}

void SBSMSDrawRenderer :: cancel()
{
  pthread_mutex_lock(&drawMutex);
  context->setFillColor(kBlackCColor);
  context->setFrameColor(kBlackCColor);
  context->fillRect(rect);
  pthread_mutex_unlock(&drawMutex);
}
