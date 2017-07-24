#ifndef PEAK_H
#define PEAK_H

/* 
Semantics are such that if destroy(p) or destroyAll() is called then all peaks create()'d after p are invalid.  It is OK to create() a peak, destroy it before calling create() again, then create() another, or to create() a set of peaks, then destroy them ALL in any order, and repeat.
 */

#include "sbsms.h"

namespace _sbsms_ {

class peak {
 public:  
  peak *tp;
  peak *tn;
  peak *tp2;
  peak *tn2;
  float x;
  float y;
  int k;
  float m;
};

class PeakAllocator {
 public:
  PeakAllocator();
  ~PeakAllocator();
  peak *create();
  void destroyAll();

 protected:
  long n;
  long size;
  peak *peaks;

};
}

#endif
