#include "dwgs.h"
#include "utils.h"
#include <algorithm>
#include <math.h>
#include <stdio.h>

using namespace std;

namespace _sbsms_ {


dwgs :: dwgs(float Z, float Zb, float Zh)
{
	loss = new Loss();
  fracdelay = new Thiran(8);
  d0 = new delay<4096>();
  d1 = new delay<4096>();
  d2 = new delay<4096>();
  d3 = new delay<4096>();

  a0 = 0.0f;
  a0_2 = 0.0f;
  a0_3 = 0.0f;
  a0_4 = 0.0f;
  a1 = 0.0f;
  a1_2 = 0.0f;
  a1_3 = 0.0f;
  a1_4 = 0.0f;
  alpha = 2.0f * Z / (Z +  Zb);
  alpha = 2.0f * alpha - 1.0f;
}

void dwgs :: set(float f, float inpos) 
{
  this->f = f;
  float deltot = TWOPI/f;
  del1 = lrintf(inpos*0.5*deltot);
  if(del1 < 2)
    del1 = 1;
  float lowpassdelay = .002;
  del2 = lrintf(0.5*(deltot-2.0*del1));
  del3 = lrintf(0.5*(deltot-2.0*del1)-lowpassdelay-6.0);
  if(del2 < 2)
    del2 = 1;
  if(del3 < 2)
    del3 = 1;
  
  float D = (deltot-(float)(del1+del1+del2+del3)-lowpassdelay);
  int N = (int)D;
  if(N < 1) {
    N = 0;
  }

  fracdelay->create(D,N);

  del1 = min(del1,4095);
  del2 = min(del2,4095);
  del3 = min(del3,4095);

  d0->setDelay(del1-1);
  d1->setDelay(del1-1);
  d2->setDelay(del2-1);
  d3->setDelay(del3-1);
}

void dwgs :: damper(float c1, float c3)
{
	loss->create(f,c1,c3);
}

float dwgs :: go(float load) {

  a0 = d0->goDelay(a0_2);
  a1_2 = d1->goDelay(a1);
  a0_3 = d2->goDelay(a0_4);
  a1_4 = d3->goDelay(a1_3);
  
  a1 = -a0;
  a0_2 = a0_3 + load;
  a1_3 = a1_2 + load;
  float a = fracdelay->filter(alpha * a1_4);
  a0_4 = loss->filter(a);
  
  return a0_4;
}

dwgs :: ~dwgs()
{
  delete d0;
  delete d1;
  delete d2;
  delete d3;
  delete loss;
  delete fracdelay;
}

}

