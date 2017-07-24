#include <math.h>
#include "real.h"
#include "utils.h"
#include "trackpoint.h"

namespace _sbsms_ {

TrackPoint2 :: TrackPoint2(audio *gx, float x, float y, int N, int M, countType time)
{
  init();
  float k = x;
  int ki = lrintf(k);
  float kf = ki<k?k-ki:ki-k;

  float norm0 = square(gx[ki][0]) + square(gx[ki][1]); 
  float ph0;
  if(norm0 > 0.0f) {
    ph0 = atan2(gx[ki][1],gx[ki][0]);
  } else {
    ph0 = 0.0f;
  }

  int ki1 = ki<k?ki+1:ki-1;
  float norm1, ph1;
  if(ki == N-1) {
    norm1 = norm0;
    ph1 = ph0;
  } else {
    norm1 = square(gx[ki1][0]) + square(gx[ki1][1]);
    if(norm1 > 0.0f) {
      ph1 = atan2(gx[ki1][1],gx[ki1][0]);
    } else { 
      ph1 = 0.0f;
    }
  }

  float ifreq = TWOPI*k/(float)N;
  ph0 = ph0 + (float)ki*PI;
  ph1 = ph1 + (float)ki1*PI;
  if(kf < 0.5) {
    float dp = canon(ph1 - ph0);
    ph1 = ph0 + dp;
  } else {
    float dp = canon(ph0 - ph1);
    ph0 = ph1 + dp;
  }
  float ph = ((1.0f-kf)*ph0 + kf*ph1);
  
  this->f = ifreq;  
  this->y = y;
  this->ph = canon(ph);  
  this->M = M;
  this->time = time;
}

TrackPoint2 :: TrackPoint2()
{
  init();
}

void TrackPoint2 :: init()
{
  for(int d=0;d<3;d++) 
    dup[d] = NULL; 
  contF = 1e9;
  cont = NULL;
  dupcont = NULL;
  bConnect = false;
  bConnected = false;
  bDelete = false;
  owner = NULL;
}

TrackPoint2 :: ~TrackPoint2()
{
  for(int d=0;d<3;d++) {
    if(dup[d]) {
      dup[d]->dup[2-d] = NULL;
    }
  }
}

}
