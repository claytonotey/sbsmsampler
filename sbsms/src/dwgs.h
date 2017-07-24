#ifndef DWGS_H
#define DWGS_H

#include "filter.h"

namespace _sbsms_ {

class dwgs {
 public:
  dwgs(float Z, float Zb, float Zh); 
  ~dwgs();
  void set(float f, float inpos);
  float go(float load);
  void damper(float c1, float c3);
  float f;
  Loss *loss;
  Thiran *fracdelay;
  float alpha;
  float a0, a0_2, a0_3, a0_4, a1, a1_2, a1_3, a1_4;
  int del1, del2, del3;
  delay<4096> *d0;
  delay<4096> *d1;
  delay<4096> *d2;
  delay<4096> *d3;
};

}

#endif
