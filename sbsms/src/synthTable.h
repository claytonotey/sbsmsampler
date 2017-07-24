// -*- mode: c++ -*-
#ifndef SYNTHTABLE_H
#define SYNTHTABLE_H
#include "utils.h"

namespace _sbsms_ {

class SynthTable {
public:
  SynthTable();
  long *getDistSynthTable1(int j) { return distSynthTable1 + 512 * j;}
  long *getDistSynthTable2(int j) { return distSynthTable2 + 512 * j;}
  long distSynthTable1[65664];
  long distSynthTable2[65664];
};

extern SynthTable distSynthTable;

}

#endif
