#ifndef SMS_H
#define SMS_H

#include "track.h"
#include "grain.h"
#include "buffer.h"
#include "peak.h"
#include "sbsms.h"
#include "config.h"
#ifdef MULTITHREADED
#include "pthread.h"
#endif
#include <list>
using namespace std;

namespace _sbsms_ {

class SMSRenderer {
 public:
  virtual ~SMSRenderer() {}
  virtual void startFrame(float stretchMod) {}
  virtual void startTime(int c, countType samplePos, countType time, int n, float h2, float pos) {}
  virtual void renderTrack(int c, Track *t, float f0, float f1, SBSMSynthesizer *synth) {}
  virtual void endTime(int c) {}
  virtual void endFrame() {}
  virtual void end() {}
};

class SMSSynthRenderer : public SMSRenderer, public SampleBufBase {
 public:
  SMSSynthRenderer(int channels, int M);
  ~SMSSynthRenderer();
  void startTime(int c, countType samplePos, countType time, int n, float h2, float pos);
  void renderTrack(int c, Track *t, float f0, float f1, SBSMSynthesizer *synth);
  void endTime(int c);
  long read(audio *out, long n);
  void reset(bool flushInput = false);
  void writeInputs(float **inputs, int samples, int offset);
  long readInputs(ArrayRingBuffer<float> *buf[2], int samples, bool bAdvance);
  void prepadInputs(int samples);
  
 protected:
  int channels;
  int M;
  float **in;
  float *synthBuf[2];
  float *inBuf[2];
  ArrayRingBuffer<float> *ins[2];
  SampleBuf *sines[2];
  countType samplePos[2];
  countType time[2];
  int n[2];
  float h2[2];
  float pos[2];
  int inputPad[2];
  int nPad[2];
#ifdef MULTITHREADED
  pthread_mutex_t bufferMutex;
#endif
};

 class SMSFileRenderer : public SMSRenderer {
 public:
  SMSFileRenderer(int channels, FILE *fp, TrackAllocator *ta);
  void startFrame(float stretchMod);
  void startTime(int c, countType samplePos, countType time, int n, float h2, float pos);
  void renderTrack(int c, Track *t, float f0, float f1, SBSMSynthesizer *synth);
  void endTime(int c);
  void endFrame();
  void end();
 protected:
  int channels;
  int nTracks[2];
  FILE *fp;
  TrackAllocator *ta;
  int frameBytes;
  RingBuffer<int> frameBytesBuf;
  fpos_t start[2];
  countType time;
};

class SMS {
public:
  virtual ~SMS() {}
  virtual void reset()=0;
  virtual void seek(long time)=0;
  virtual void particlePopulate(int c, float pitch, SBSMSynthesizer *synth)=0;
  virtual void renderTracks(int c, list<SMSRenderer*> &renderers, float a, float f0, float f1)=0;
  virtual long synthInit(int c, int n, bool bBackwards, float stretch)=0;
  virtual int synthTracks(int c, int n, SMSRenderer *renderer, bool bBackwards, float stretch, float pitch, SBSMSynthesizer *synthMod)=0;
  virtual void pruneTracks(int c)=0;
  virtual void stepSynth(int c, int n)=0;
  virtual bool isPastLeft(int c)=0;
  virtual bool isPastRight(int c)=0;
  virtual void setLeftPos(countType pos)=0;
  virtual void setRightPos(countType pos)=0;
  virtual countType getSamplePos()=0;
};

template<class T>
class SMSBase : public SMS {
public:  
  SMSBase(int N, int M, int M_MAX, int h, int res, int N0, int N1, int N2, int channels, TrackAllocatorImp<T> *ta, PeakAllocator **pa);
  virtual ~SMSBase();
  void reset();
  void renderTracks(int c, list<SMSRenderer*> &renderers, float a, float f0, float f1);
  void seek(long time);
  void assignTrackPoint(T *tp, char flags, int index, int precursorIndex, int c, bool bBackwards);
  void particlePopulate(int c, float pitch, SBSMSynthesizer *synth);
  long synthInit(int c, int n, bool bBackwards, float stretch);
  int synthTracks(int c, int n, SMSRenderer *renderer, bool bBackwards, float stretch, float pitch, SBSMSynthesizer *synth);
  void pruneTracks(int c);
  void stepSynth(int c, int n);
  bool isPastLeft(int c);
  bool isPastRight(int c);
  countType getSamplePos();
  void setInputs(float **inputs);
  void setLeftPos(countType pos);
  void setRightPos(countType pos);
 protected:
  countType leftPos;
  countType rightPos;
  void addTrack(int c, TrackImp<T> *t);  
#ifdef MULTITHREADED
  pthread_mutex_t tplbMutex[2];
  pthread_mutex_t trackMutex[2];
#endif
  TrackAllocatorImp<T> *ta;
  PeakAllocator **pa;
  int channels;
  float pad0, pad2;
  int peakWidth0;
  int peakWidth1;
  float peakThresh;
  float maxMerit2;
  float maxDF2;
  float dMCoeff;
  float maxMerit2Match;
  float maxDF2Match;
  float dMCoeffMatch;
  int minNpts;
  float maxFMatch;
  float minFMatch;
  int kStart;
  int kEnd;
  float mNorm;
  float minPeakTopRatio;
  float localFavorRatio;
  float maxDecRatio;

  double synthOffset[2];
  float grainPos[2];
  double grainLength[2];
  list< TrackImp<T>* > trax[2];
  countType addtime[2];
  countType assigntime[2];
  countType marktime[2];
  countType synthtime[2];
  countType samplePos[2];
  double h2cum[2];
  double samplePosCum[2];
  int h2i[2];
  int res;
  int N;
  int h;
  double h1;
  int M;
  int Nover2;
  PointerBuffer< list<T*> > *trackPointListBuffer[2];
  audio* x0[2];
  audio* x1[2];
  audio* x2[2];
  float* dec[2];
  float* mag0[2];
  float* mag1[2];
  float* mag2[2];
  float *peak0;
  float *peak1;
  bool bPeakSet;
  float magmax;
};

class SMSAnalyze {
public:
  virtual ~SMSAnalyze() {}
  virtual long addTrackPoints(grain *g0, grain *g1, grain *g2, int c)=0;
  virtual long assignTrackPoints(long offset, SMSAnalyze *hi, SMSAnalyze *lo, int c)=0;
  virtual long startNewTracks(long offset, int c)=0;
  virtual void markDuplicates(long offset, SMSAnalyze *hi, SMSAnalyze *lo, int c)=0;
  virtual void advanceTrackPoints(int c)=0;
};

class SMSFile {
public:  
  virtual ~SMSFile() {}
  virtual void assignTrackPointsFromFile(FILE *fp, int c)=0;
};

class SMSMemory {
public:  
  virtual ~SMSMemory() {}  
  virtual bool assignTrackPointsFromMemory(int c)=0;
  virtual bool assignTrackPointsFromMemory(int c, bool bBackwards, int offset)=0;
};

class SMS0 : public SMSBase<TrackPoint>, public SMSFile {
public:
  SMS0(int N, int M, int M_MAX, int h, int res, int N0, int N1, int N2, int channels, TrackAllocatorImp<TrackPoint> *ta, PeakAllocator **pa);
  void assignTrackPointsFromFile(FILE *fp, int c);
};

class SMS1 : public SMSBase<TrackPoint1>, public SMSFile, public SMSMemory {
public:
  SMS1(int N, int M, int M_MAX, int h, int res, int N0, int N1, int N2, int channels, TrackAllocatorImp<TrackPoint1> *ta, PeakAllocator **pa, SMS1 *source);
  ~SMS1();
  void assignTrackPointsFromFile(FILE *fp, int c);
  bool assignTrackPointsFromMemory(int c);
  bool assignTrackPointsFromMemory(int c, bool bBackwards, int offset);
 protected:
  SMS1 *source;
  vector<TrackPoint1*> *trackPointCache[2];
  vector<long> *trackPointIndexAtTime[2];
  vector<int> *nTracksAtTime[2];
};

class SMS2 : public SMSBase<TrackPoint2>, public SMSAnalyze {
 public:
  SMS2(int N, int M, int M_MAX, int h, int res, int N0, int N1, int N2, int channels, TrackAllocatorImp<TrackPoint2> *ta, PeakAllocator **pa);
  long addTrackPoints(grain *g0, grain *g1, grain *g2, int c);
  long assignTrackPoints(long offset, SMSAnalyze *hi, SMSAnalyze *lo, int c);
  long startNewTracks(long offset, int c);
  void markDuplicates(long offset, SMSAnalyze *hi, SMSAnalyze *lo, int c);
  void advanceTrackPoints(int c);

 protected:
  long assignTrackPoints_(long offset, SMS2 *hi, SMS2 *lo, float dtlo, long offsetlo, int c);
  bool connectTrackPoints(TrackPoint2 *tp0, TrackPoint2 *tp1, SMS2 *hi, SMS2 *lo, float dtlo, int c);
  bool adoptTrack(TrackImp<TrackPoint2> *precursor, SMS2 *lender, TrackPoint2 *tp, float m, float dt, int c);
  void markDuplicatesLo(long offset, SMS2 *lo, long offsetlo, int c);
  void adjustPeaks(list<peak*> &peaks,
                   float *mag0,
                   float *mag1,
                   float *mag2,
                   float *dec);
  long assignTrackPoints(long offset, SMS2 *hi, SMS2 *lo, float dtlo, long offsetlo, int c);
  bool contTrack(TrackImp<TrackPoint2> *t, TrackPoint2 *tp, int c);
  void extractTrackPoints(audio *x, float *mag0, float *mag1, float *mag2, countType time, PointerBuffer< list<TrackPoint2*> > *tplb, int c);
  bool nearestTrackPoint(list<TrackPoint2*> *tpl, TrackPoint2 *tp0, float m0, float m1, float dt, list<TrackPoint2*>::iterator *minpos, float *minFx, float maxMerit2, float maxDF2);
  float merit(TrackPoint2 *tp0, TrackPoint2 *tp1, float m0, float m1, float dt, float dMCoeff, float *df, float maxDF2);
  void calcmags(float *mag, audio *x, float q);
  peak *makePeak(float *mag, float *mag2, int k, int c);
};

}

#endif
