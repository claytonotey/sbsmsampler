#ifndef SBSMSRENDER_H
#define SBSMSRENDER_H

#include "sbsms.h"
#include "vstsbsms.h"
#include "vstgui.h"

using namespace _sbsms_;
 
class SBSMSDrawRenderer : public SBSMSRenderer, public CReferenceCounter {
 public:
  SBSMSDrawRenderer(countType samplesToProcess, CFrame *frame, const CRect &rec, int leftOffset, int rightOffset);
  virtual ~SBSMSDrawRenderer();
  void render(int c, countType samplePos, long nSamples, float f0, float f1, float y0, float y1);
  void cancel();
  void copyFrom(COffscreenContext *c, const CRect &r);

 protected:
  CCoord getX(countType samplePos);
  CCoord getY(float f);
  CColor getColor(int c, float m);

  COffscreenContext *context;
  CColor color[2][256];
  countType time;
  CRect rect;
  CCoord width;
  CCoord height;
  int x0;
  double xScale;
  double yScale;
  float minF;
  float maxF;
  pthread_mutex_t drawMutex;
};

#endif
