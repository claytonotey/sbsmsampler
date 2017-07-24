#ifndef TRACKPOINT_H
#define TRACKPOINT_H

#include "sbsms.h"

namespace _sbsms_ {

class TrackPoint {
 public:
  float f;
  float y;
  float ph;
  countType time;
};

class TrackPoint1 : public TrackPoint {
 public:
  char flags;
  int index;
  int precursorIndex;
  int descendantIndex;
};

template<class T>
class TrackImp;

 class TrackPoint2 : public TrackPoint {
 public:
  TrackPoint2();
  TrackPoint2(audio *gx, float x, float y, int N, int M, countType time);
  ~TrackPoint2();

  int M;
  TrackImp<TrackPoint2> *owner;
  float contF;
  bool bConnect;
  bool bConnected;
  bool bDelete;
  TrackPoint2 *dupcont;
  TrackPoint2 *cont;
  TrackPoint2 *dup[3];

 protected:
  void init();
};

}

#endif
