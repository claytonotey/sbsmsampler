#include <math.h>
#include <cstdlib>
#include <cstring>
#include "grain.h"
#include "sbsms.h"
#include "real.h"
#include "utils.h"

#include <map>
using namespace std;

namespace _sbsms_ {

GrainAllocator :: GrainAllocator(int N, int N2, int type)
{
  this->N = N;
  this->N2 = N2;
  this->type = type;

  fftPlan = planFFT(N);
  ifftPlan = planIFFT(N);

  int k1 = (N-N2)/2;

  ww = (float*)calloc(N,sizeof(float));
  for(int k=0;k<N2;k++) {
    if(type == hann) {
      ww[k+k1] = 0.5f*(1.0f - cos((float)k/(float)N2*TWOPI));
    } else if(type == hannpoisson) {
      ww[k+k1] = 0.5f*(1.0f - cos((float)k/(float)N2*TWOPI)) * exp(-2.0f*fabs((float)(k-N2/2))/(float)N2);
    }
  }

  peak = (audio*) calloc(N,sizeof(audio));
  for(int k=0;k<N;k++) {
    peak[k][0] = ww[k]/(float)(N2/2);
  }
  FFT(fftPlan,peak);
}

GrainAllocator :: ~GrainAllocator()
{
  destroy_fftplan(fftPlan);
  destroy_fftplan(ifftPlan);
  free(ww);
  free(peak);
}

grain* GrainAllocator :: create()
{
  grain *g = new grain(N,N2);
  g->refCount = 0;
  g->fftPlan = fftPlan;
  g->ifftPlan = ifftPlan;
  g->ww = ww;
  g->peak = peak;
  return g;
}

void GrainAllocator :: reference(grain *g)
{
  g->refCount++;
}

void GrainAllocator :: forget(grain *g)
{
  g->refCount--;
  if(g->refCount <= 0) {
    destroy(g);
  }
}

void GrainAllocator :: destroy(grain *g)
{
  free_audio_buf(g->x);
  delete g;
}
  

grain :: grain(int N, int N2)
{ 
  this->N = N;
  this->synthScale = 1.0f / (float)N2;
  x = (audio*) malloc(N*sizeof(audio));
}

void grain :: analyze() 
{
  for(int k=0;k<N;k++) {
    for(int c=0;c<2;c++) {
      x[k][c] *= ww[k];
    }
  }
  FFT(fftPlan,x);
}

void grain :: synthesize() 
{
  IFFT(ifftPlan,x);
  for(int k=0;k<N;k++) {
    for(int c=0;c<2;c++) {
      x[k][c] *= ww[k] * synthScale;
    }
  }
}

void grain :: downsample(grain *g2)
{
  grain *g = this;

  int N2 = N/2;
  int N4 = N/4;
  for(int c=0;c<2;c++) {
    for(int k=0;k<N4;k++)
      g2->x[k][c] = g->x[k][c];
    
    g2->x[N4][c] = 0.5f*(g->x[N4][c] + g->x[N-N4][c] );
    
    for(int k=N4+1;k<N2;k++)
      g2->x[k][c] = g->x[k+N2][c];
  }
}

}
