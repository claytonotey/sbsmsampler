#ifndef GEN_H
#define GEN_H

#include "filter.h"
#include "dwgs.h"

namespace _sbsms_ {

#define PhShift 5
#define WShift 21
#define Ph1 65535
#define WPI 536870912
#define W2PI 1073741824
#define W2PIMask 1073741823
#define WScale 1.708913188941079e8f
#define MScale 4.656683928435187e-10f

class GenParams {
public:
  long *tab1;
  long *tab2;
  float Q;
  float fb;
  float bits;
  int granMode;
  float granRate;
  float granSmooth;
  float c1;
  float c3;
  float inpos;
};

class GenState {
 public:
  float ph;
  union {
    float x0;
    float cursor;
  };
  union {
    float x1;
    float start;
  };
  union {
    float y0;
    float nextStart;
  };
  union {
    float y1;
    float crossover;
  };
  float length;

  GenState();
};

class Gen {
public:
  Gen(int c);
  virtual ~Gen() { }
  int c;
  float w;
  float m;
  float x;
  float v;
  float pAM;
  float pFM;
  virtual void setParams(GenParams *params) {};
  virtual void gen(float dw, float dm, float *in, float *out, int n)=0;
  GenState state;
};

class Osc : public Gen
{
public:
  Osc(int c);
  virtual ~Osc() {}
  void setParams(GenParams *params);
  void gen(float dw, float dm, float *in, float *out, int n);
  long *tab1;
  long *tab2;
};

class Bandpass : public Gen
{
public:
  Bandpass(int c);
  virtual ~Bandpass() {}
  void setParams(GenParams *params);
  void gen(float dw, float dm, float *in, float *out, int n);
  float Q;
  int stages;
};


class Delay : public Gen {
 public:
  Delay(int c);
  virtual ~Delay();
  void setFreq(float f0);
  void gen(float dw, float dm, float *in, float *out, int n);  
  void setParams(GenParams *params);
  fbdelay *del;
  Thiran *fracdelay;
};

class Decimator : public Gen 
{
public:
  Decimator(int c);
  void gen(float dw, float dm, float *in, float *out, int n);  
  void setParams(GenParams *params);
  float cursor;
  float hold;
  float multiplier;
  float samplesToHold;
};


enum {
  GranulateKeyLengthTrackLengthPBLength = 0,
  GranulateKeyLengthTrackLengthPBStep,
  GranulateKeyLengthTrackStepPBLength,
  GranulateKeyLengthTrackStepPBStep,
  GranulateKeyStepTrackLengthPBLength,
  GranulateKeyStepTrackLengthPBStep
};

class Granugrain {
 public:
  Granugrain(int channels, int maxSize);
  ~Granugrain();
  void reset(int maxSize);
  void setBaseFreq(float pitch, float f0);
  void setPitchbend(float pb);
  void writeInputs(float **inputs, int n, int offset);

  float *inputs[2];
  int channels;
  int maxSize;
  int size;
  int time;
  float pitch;
  float f0;
  float pb;
};

class Granulator : public Gen {
 public:
  Granulator(int c, Granugrain *g);
  void gen(float dw, float dm, float *in, float *out, int n);  
  void setParams(GenParams *params);
  void setFreq(float f);
  Granugrain *g;

  float step;
  //float length;
  float nextLength;
  //float cursor;
  //float start;
  //float nextStart;
  //float crossover;
  float nextCrossover;

  int granMode;  
  float rate;
  float smooth;
};

class DWGS : public Gen {
 public:
  DWGS(int c);
  ~DWGS();
  void gen(float dw, float dm, float *in, float *out, int n);  
  void setFreq(float f);
  void setParams(GenParams *params);
 
  float inpos;
  float zc;
  float c1;
  float c3;
  float Z;
  float Zb;
  dwgs *d;
};


}

#endif
