#include <stdio.h>
#include <stdlib.h>
#include <math.h>
#include <algorithm>
using namespace std;
#include <xmmintrin.h>

 class GenState {
 public:
   float ph;
  float x[2];
  float y[2];
  float x2[2];
  float y2[2];
  float x3[2];
  float y3[2];
  GenState();
 };


GenState :: GenState()
{
  memset(this,0,13*sizeof(float));
}

class Gen {
public:
  Gen();
  virtual ~Gen() { }
  float w;
  float m;
  virtual void gen(float dw, float dm, float *in, float *out, int n)=0;
  GenState state;
};

Gen :: Gen()
{
  m = 0.0;
}

class Bandpass : public Gen
{
public:
  Bandpass();
  virtual ~Bandpass() {}
  void gen(float dw, float dm, float *in, float *out, int n);
  float Q;
  int stages;
};

Bandpass :: Bandpass()
{
  Q = 3016.0;
  stages = 2;
}

void Bandpass :: gen(float dw, float dm, float *in, float *out, int n)
{
  float y0;

  float a,a2,aoQ,d;
  if(!n) return;

  a = 1.0/(2.0*tan(0.5*w));
  a2 = a*a;
  aoQ = a/Q;
  d = (4*a2+2*aoQ+1);

  float A1, A2, B0;
  float A11, A21, B01;

  A1 = -(8*a2-2) / d;
  A2 = (4*a2 - 2*aoQ + 1) / d;
  B0 = 2*aoQ/d;

  int nout = n;
  float w1 = w+n*dw;
  
  if(w1 > M_PI) {
    if(w < M_PI) {
      nout = min(n,(int)lrintf((M_PI - w) / dw));
      w1 = M_PI;
      a = 0.0;
    } else {
      nout = 0;
    }
  } else {
    a = 1.0/(2.0*tan(0.5*w1));
  }

  a2 = a*a;
  aoQ = a/Q;
  d = (4*a2+2*aoQ+1);

  A11 = -(8*a2-2) / d;
  A21 = (4*a2 - 2*aoQ + 1) / d;
  B01 = 2*aoQ/d;

  float dA1, dA2, dB0;
  if(nout != 0) {
    dA1 = (A11 - A1) / nout;
    dA2 = (A21 - A2) / nout;
    dB0 = (B01 - B0) / nout;
  }

  float *end = out + nout;
  while(out != end) {
    float x0;
    if(stages >= 3) {
      y0 = (B0 * (*in - state.x3[1]) - A1 * state.y3[0] - A2 * state.y3[1]);
      state.y3[1] = state.y3[0];
      state.y3[0] = y0;
      state.x3[1] = state.x3[0];
      state.x3[0] = *in;
      x0 = y0;
    } else {
      x0 = *in;
    }
    if(stages >= 2) {
      y0 = (B0 * (x0 - state.x2[1]) - A1 * state.y2[0] - A2 * state.y2[1]);
      state.y2[1] = state.y2[0];
      state.y2[0] = y0;
      state.x2[1] = state.x2[0];
      state.x2[0] = x0;
      x0 = y0;
    } else {
      x0 = *in;
    }

    y0 = (B0 * (x0 - state.x[1]) - A1 * state.y[0] - A2 * state.y[1]);
    state.y[1] = state.y[0];
    state.y[0] = y0;
    state.x[1] = state.x[0];
    state.x[0] = *in;
    
    *out += m * y0;
    in++;
    out++;
    m += dm;
    if(m<0) { m = 0.0f; break; }
    A1 += dA1;
    A2 += dA2;
    B0 += dB0;
  }

  w = w1;
}


int main(int c, char **v) {

  printf("%x\n",-12 & 31U);
  float d = 1;
  printf("%d\n",(int)lrintf(d-0.5f));
  int N = atoi(v[1]);
  float Q = atof(v[2]);

  int n = 256;
  float *in = new float[n];
  float *out = new float[n];
  memset(in,0,n*sizeof(float));
  memset(out,0,n*sizeof(float));
  //  _mm_setcsr( _mm_getcsr() | 0x8040 );

  in[0] = 1;

  Bandpass gen;
  gen.w = 1.0;
  gen.m = .2;
  gen.Q = Q;

  for(int i=0;i<N;i++) {
    for(int j=0;j<N;j++) {
      gen.gen(0,0,in,out,n);
      in[0] = 0;
      //printf("%g\n",out[0]);
    }
  }
}
