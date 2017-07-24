#include "synthTable.h"
#include "synth.h"

namespace _sbsms_ {

SynthTable :: SynthTable()
{
  enum { M = 128, N = 512 };

  for(int j=0; j<M; j++) {
      long *tab1j = distSynthTable1 + j * N;
      long *tab2j = distSynthTable2 + j * N;
      float od;
      if(j > 0) {
        float x = 1.5f * (float)j / (float)(M-1);
        od = exp(x*x*x);
      }
      float c0 = 1.0f;
      for(int k=0;k<N;k++) {
        float c1 = cos((float)(k+1)/(float)N*TWOPI);
        if(j == 0) {
          tab1j[k] = lrintf(c0 / MScale);
          tab2j[k] = lrintf((c1-c0) / (65536.0 * MScale));
        } else {
          float d0 = max(-1.0f,min(1.0f, c0 * od));
          float d1 = max(-1.0f,min(1.0f, c1 * od));
          tab1j[k] = lrintf(d0/MScale);
          tab2j[k] = lrintf((d1-d0) / (65536.0 * MScale));
        }
        c0 = c1;
      }
  }
    
}

SynthTable distSynthTable;

}

