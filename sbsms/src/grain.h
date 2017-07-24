#ifndef GRAIN_H
#define GRAIN_H

#include "fft.h"
#include "sbsms.h"

namespace _sbsms_ {

enum windowType {
  hann,
  hannpoisson
};

class GrainAllocator;

class grain {
 public:
  t_fft *x;
  int type;
  int N;
  int h;
  float synthScale;
  audio *peak;
  
  void analyze();
  void synthesize();
  void downsample(grain *gdown);

  friend class GrainAllocator;

 protected:
  grain(int N, int N2);
  int refCount;
  float *ww;
  fftplan *fftPlan, *ifftPlan;
};


class GrainAllocator {
public:
  GrainAllocator(int N, int N2, int type);
  ~GrainAllocator();

  grain *create();
  void reference(grain *g);
  void forget(grain *g);

  friend class grain;

protected:
  void destroy(grain *g);
  int N;
  int N2;
  int type;
  float *ww;
  audio *peak;
  fftplan *fftPlan;
  fftplan *ifftPlan;
};

}

#endif
