#ifndef VSTSBSMS_H
#define VSTSBSMS_H

#include <utility>
#include <vector>
#include <string>
#include <algorithm>
#include <string.h>
#include <pthread.h>
#include <math.h>

using namespace std;

#define NUM_PARAMS 71
#define NUM_PROGRAMS 1
#define NUM_SAMPLES 8
#define NUM_VOICES 12
#define NUM_NOTES 128

extern char keyName[12][3];

enum {
  PITCHBEND_MAX_RANGE = 24
};

enum {
  pAttack = 0,
  pRelease,
  pLoop,
  pFollowMode,
  pRate,
  pRateMode,
  pAMFreq0,
  pAMFreq0Mode,
  pAMFreq1,
  pAMFreq1Mode,
  pAMDepth0,
  pAMDepth1,
  pAMDepth1Mode,
  pFMFreq0,
  pFMFreq0Mode,
  pFMFreq1,
  pFMFreq1Mode,
  pFMDepth0,
  pFMDepth1,
  pFMDepth1Mode,
  pDistortion0,
  pDistortion1,
  pDistortion1Mode,
  pFilterQ0,
  pFilterQ1,
  pFilterQ1Mode,
  pSidebandMod,
  pSidebandBW,
  pSidebandBWScale,
  pSidebandRelease,
  pSidebandEnv,
  pModPivot,
  pVolume,
  pKeyBase,
  pOctaveBase,
  pKeyLo,
  pOctaveLo,
  pKeyHi,
  pOctaveHi,
  pChannel,
  pPitchbendRange,
  pLeftPos,
  pRightPos,
  pStartPos,
  pSynthMode,
  pCombFB0,
  pCombFB1,
  pCombFB1Mode,
  pDecBits0,
  pDecBits1,
  pDecBits1Mode,
  pGranMode,
  pGranRate0,
  pGranRate1,
  pGranRate1Mode,
  pGranSmooth0,
  pGranSmooth1,
  pGranSmooth1Mode,
  pDWGSDecay0,
  pDWGSDecay1,
  pDWGSDecay1Mode,
  pDWGSLopass0,
  pDWGSLopass1,
  pDWGSLopass1Mode,
  pDWGSStringPos0,
  pDWGSStringPos1,
  pDWGSStringPos1Mode,
  pForce,
  pDrag,
  pSpring,
  pThresh
};

enum {
  loopForward = 0,
  loopBackward,
  loopForwardLoop,
  loopBackwardLoop,
  loopStill,
  loopStillLoop,
};


struct cmp_str {
  bool operator()(char const *a, char const *b) {
    return strcmp(a, b) < 0;
  }
};

void debug(const char *str, bool close = false);

class Tempos {
public:
  vector<pair<float,string> > s;
  Tempos() {
    s.push_back(pair<float,string>(1.0/64.0,string("1/64")));
    s.push_back(pair<float,string>(1.0/48.0,string("1/48")));
    s.push_back(pair<float,string>(1.0/32.0,string("1/32")));
    s.push_back(pair<float,string>(1.0/24.0,string("1/24")));
    s.push_back(pair<float,string>(1.0/16.0,string("1/16")));
    s.push_back(pair<float,string>(1.0/12.0,string("1/12")));
    s.push_back(pair<float,string>(1.0/8.0,string("1/8")));
    s.push_back(pair<float,string>(1.0/6.0,string("1/6")));
    s.push_back(pair<float,string>(3.0/16.0,string("3/16")));
    s.push_back(pair<float,string>(1.0/4.0,string("1/4")));
    s.push_back(pair<float,string>(1.0/3.0,string("1/3")));
    s.push_back(pair<float,string>(3.0/8.0,string("3/8")));
    s.push_back(pair<float,string>(1.0/2.0,string("1/2")));
    s.push_back(pair<float,string>(3.0/4.0,string("3/4")));
    s.push_back(pair<float,string>(1.0,string("1/1")));
    s.push_back(pair<float,string>(3.0/2.0,string("3/2")));
    s.push_back(pair<float,string>(2.0,string("2/1")));
    s.push_back(pair<float,string>(3.0,string("3/1")));
    s.push_back(pair<float,string>(4.0,string("4/1")));
    s.push_back(pair<float,string>(8.0,string("8/1")));
    s.push_back(pair<float,string>(16.0,string("16/1")));
  }
};

extern Tempos tempo;
 
inline bool tempoCmp(const pair<float,string> &a, const pair<float,string> &b) {
  return a.first < b.first;
}

inline float closestTempo(float f)
{
  if(f < tempo.s[0].first) {
    return tempo.s[0].first;
  } else if(f > tempo.s.back().first) {
    return tempo.s.back().first;
  }
  float dmin = 1e6;
  float vmin = 0.0;
  for(vector< pair<float,string> >::iterator k=tempo.s.begin(); k != tempo.s.end(); ++k) {
    float d = fabsf(k->first - f);
    if(d < dmin) {
      dmin = d;
      vmin = k->first;
    }
  }
  return vmin;
}

#endif
