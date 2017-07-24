#include "filter.h"
#include <math.h>
#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <algorithm>
using namespace std;

#include "config.h"

namespace _sbsms_ {

Filter :: Filter(int nmax)
{
  this->nmax = nmax;
  b = (float*)malloc(2*(nmax+1)*sizeof(float));	
  x = (float*)malloc(2*(nmax+1)*sizeof(float));
  memset(x,0,2*(nmax+1)*sizeof(float));
  xc = x;
  xend = x+2*nmax;
}

void Filter :: init()
{
  bend = b + (n<<1);
}

Filter :: ~Filter()
{
  free(b);
  free(x);
}

long choose(long n, long k) {
  long divisor = 1;
  long multiplier = n;
  long answer = 1;
  k = min(k,n-k);
  while(divisor <= k) {
    answer = (answer * multiplier) / divisor;
    multiplier--;
    divisor++;
  }
  return answer;
}

Thiran :: Thiran(int nmax) : Filter(nmax)
{
}

void Thiran :: create(float D, int N)
{
  if(N < 1) {
    n = 0;
    b[0] = 1;
    b[1] = 0;
    init();
    return;
  }

  if(D-N > 0.99999) {
    N++;
  }
  if(D-N < 1e-6) {
    b[1] = 1.0;
    b[N<<1] = 1.0;
    for(int k=1;k<=N;k++) {
      b[(k<<1)+1] = 0;
      b[(N-k)<<1] = 0;
    }
  } else {
    int choose = 1;
    for(int k=0;k<=N;k++) {
      float ak = choose;
      for(int n=0;n<=N;n++) {
        ak *= ((float)D-(float)(N-n));
        ak /= ((float)D-(float)(N-k-n));
      }
      b[(k<<1)+1] = ak;
      b[(N-k)<<1] = ak;
      choose = (-choose * (N-k)) / (k+1); 
    }  
  }
  this->n = N;  
  init();
}


void Filter :: clear() 
{
  memset(x,0,2*(nmax+1)*sizeof(float));
}

Loss :: Loss() : Filter(1)
{
  n = 1;  
  init();
}

void Loss :: create(float f0, float c1, float c3)
{
  f0 = f0 * 7019.0;
  float g = 1.0 - c1/f0; 
  float b1 = 4.0*c3+f0;
  float a1 = (-b1+sqrt(b1*b1-16.0*c3*c3))/(4.0*c3);
  b[0] = g*(1+a1);
  b[2] = 0;
  b[1] = 1;
  b[3] = a1;
}

fbdelay :: fbdelay(int n)
{
  init(n);
	fb = 0.0f;
}

fbdelay :: ~fbdelay()
{
  free(x);
  free(y);
}

void fbdelay :: init(int di)
{
  // turn size into a mask for quick modding
  size = 2*di;
  int p = 0;
  while(size) {
    size /= 2;
    p++;
  }
  size = 1;
  while(p) {
    size *= 2;
    p--;
  }
  mask = size - 1;

  x = (float*) malloc(size*sizeof(float*));
  y = (float*) malloc(size*sizeof(float*));
  memset(x,0,size*sizeof(float));
  memset(y,0,size*sizeof(float));

  cursor = 0;
}

void fbdelay :: setFB(float fb)
{
  this->fb = fb;
}

void fbdelay :: setDelay(int di)
{
  this->di = di;
  d1 = (cursor+size-di)&(mask);
}

float fbdelay :: goDelay(float in)
{
  float y0;
  x[cursor] = in;
  y0 = x[d1] + fb*y[d1];
  d1 = (d1 + 1) & mask;
  y[cursor] = y0;
  cursor = (cursor + 1) & mask;
  return y0;
}

}
