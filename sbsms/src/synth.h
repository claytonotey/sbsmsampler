// -*- mode: c++ -*-
#ifndef SYNTHMOD_H
#define SYNTHMOD_H

#include "sbsms.h"
#include "grain.h"
using namespace std;

#include <map>
#include <queue>
#include <list>
#include <pthread.h>
#include "gen.h"

class SBSMSVoice;

namespace _sbsms_ {

enum {
  SynthModeOsc = 0,
  SynthModeFilter,
  SynthModeDelay,
  SynthModeDecimate,
  SynthModeGranulate,
  SynthModeDWGS,
  DistMax = 127
};


/*struct CompareParticle
{
  bool operator()(const pair<float,int> &p1, const pair<float,int> &p2) const
  {return (p1.first>p2.first);}        
}

typedef set< pair<float, int>  CompareParticle> pset;
*/

class Particles {
public:
  Particles();
  float force(float w, float m);
  void zero();
  void addParticle(float w, float m);
  vector<float> f;
  vector<float> m;
  //pset big;
  int n;
};

class SampleSynthesizer;
class ProgramSynthesizer;

class VoiceSynthesizer : public SBSMSynthesizer {
public:
  VoiceSynthesizer(SampleSynthesizer *sampleSynth, SBSMSVoice *voice, int spectN);
  ~VoiceSynthesizer();

  bool synth(int c,
             float *in,
             float *out,
             countType synthtime,
             float h2,
             float offset,
             int n,
             float fScale0,
             float fScale1,
             Track *t);

  void setGrain(Granugrain *g);
  void setSideGrain(grain *sideGrain, int samples);

  void particleStep(int c, int n, Track *t);
  float particleForce(int c, float w, float m);
  void particleInit(int c);
  void particlePopulate(int c, float offset, float pitch, countType synthtime, Track *t);
  bool isPopulateRequired();
  
  float getSidebandMod(float w, int c);
  float getSidebandEnv(float w, int c);
  float getPivotMod(float w);

  Particles particles[2];

  friend class Track;
  friend class SBSMSVoice;
  int spectN;
  int spectWidth;
  audio *spectX;
  audio *spectX2;
  int synthMode;
  float spectScale;
  Granugrain *granuGrain;
  SampleSynthesizer *sampleSynth;
  SBSMSVoice *voice;
};

class SampleSynthesizer {
 public:
  SampleSynthesizer();

  float fFM0;
  float fFM1;
  int fFM1Mode;
  float mFM0;
  float mFM1;
  int mFM1Mode;
  float fAM0;
  float fAM1;
  int fAM1Mode;
  float mAM0;
  float mAM1;
  int mAM1Mode;
  float dist0;
  float dist1;
  int dist1Mode;
  int synthMode;
  float sidebandMod;
  float sidebandBW;
  float sidebandBWScale;
  float sidebandRelease;
  float sidebandEnv;
  float Q0;
  float Q1;
  int Q1Mode;
  float modPivot;
  float combFB0;
  float combFB1;
  int combFB1Mode;
  float decBits0;
  float decBits1;
  int decBits1Mode;
  int granMode;
  float granRate0;
  float granRate1;
  int granRate1Mode;
  float granSmooth0;
  float granSmooth1;
  int granSmooth1Mode;
  float dwgsDecay0;
  float dwgsDecay1;
  int dwgsDecay1Mode;
  float dwgsLopass0;
  float dwgsLopass1;
  int dwgsLopass1Mode;
  float dwgsStringPos0;
  float dwgsStringPos1;
  int dwgsStringPos1Mode;
  float mForce;
  float mSpring;
  float mDrag;
  float mThresh;

  bool isPopulateRequired();

  void setSynthMode(int mode);

  friend class VoiceSynthesizer;
  ProgramSynthesizer *programSynth;
  list<VoiceSynthesizer*> voiceSynths;

protected:
  pthread_mutex_t synthMutex;
};

class ProgramSynthesizer {
 public:

  list<SampleSynthesizer*> sampleSynths;

};

}

#endif
