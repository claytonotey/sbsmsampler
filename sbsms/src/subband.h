#ifndef SUBBAND_H
#define SUBBAND_H

#include "real.h"
#include "buffer.h"
#include "sms.h"
#include <stdio.h>
#include "config.h"
#ifdef MULTITHREADED
#include "pthread.h"
#endif

namespace _sbsms_ {

class SubBand {
 public:
  SubBand(SubBand *parent, int M, int channels, SBSMSQuality *quality, long prepad, 
          bool bPreAnalyze, bool bSynthesize, 
          TrackAllocator *ta, PeakAllocator **pa, SubBand *source = NULL);
  ~SubBand();

  long write(audio *buf, long n, float stretch, float pitch);
  long writeFromFile(FILE *fp, float stretch, float pitch);
  long writeFromMemory(float stretch, float pitch);
  long read(audio *buf, long n);
  void particlePopulate(int c, float pitch, SBSMSynthesizer *synthMod);
  void particles(float pitch, SBSMSynthesizer *synthMod);
  long synthFromMemory(audio *buf, long n, float stretch, float pitch, SBSMSynthesizer *synthMod);
  long synthFromFile(audio *buf, long n, float stretch, float pitch, FILE *fp);
  void writingComplete();
  void setFramesInFile(long frames);
  long getFramesInFile();
  void reset(bool flushInput = false);
  void seek(long framePos, countType samplePos);
  long getOutputSamplesQueued();
  long getInputSamplesQueued();
  long getInputFrameSize();
  long getFramePos();
  void writeFooter(FILE *fp);
  countType getSamplePosFromMemory();
  void writeInputs(float **inputs, int samples, int offset);
  long readInputs(ArrayRingBuffer<float> *buf[2], int samples, bool bAdvance);
  void prepadInputs(int samples);
  void setLeftPos(countType pos);
  void setRightPos(countType pos);

  long getDrop(float stretch);
  bool writeInit();
  long analyzeInit(int,bool,long n=0);
  long extractInit(int,bool);
  long markInit(int,bool);
  long assignInit(int,bool);
  long renderInit(int,bool);
  long writeFromFileInit(int,bool);
  long readInit();

  bool isDone();

  void analyze(int);
  void extract(int);
  void mark(int);
  void assign(int);
  void start(int);
  void advance(int);
  void render(int);

  void addRenderer(SMSRenderer *);
  void removeRenderer(SMSRenderer *);
  long renderSynchronous();
  void process(bool);

  void stepAnalyzeFrame(int);
  void stepExtractFrame(int);
  void stepMarkFrame(int);
  void stepAssignFrame(int);
  void stepRenderFrame(int);
  void stepReadFrame();

  long preAnalyze(audio *buf, long n, float stretch);
  void preAnalyzeComplete();

#ifdef MULTITHREADED
  pthread_mutex_t dataMutex;
  pthread_mutex_t grainMutex[3];
#endif
  friend class SBSMSImp;

 protected:
  void init();
  long synthFromMemoryInit(int c, long n, float *stretch);
  long synthFromFileInit(long n, float *stretch, float pitch, FILE *fp);
  long synth(int c, long n, float stretch, float pitch, SBSMSynthesizer *synthMod);
  void stepSynthGrain(int c, int grainCompleted);

  long getFramesAtFront(int);
  long writeFromFile(FILE *fp, int c);
  long writeFromMemory(int c);
  void readSubSamples();
  void setStretch(float stretch);
  void setStretchMod(float stretch);
  void setPitch(float pitch);
  void calculateStretchMod(long kstart, long kend);
  float calculateOnset(grain *g1, grain *g2);
  float getOnset(long k);

  bool bWritingComplete;
  int nRenderLatencyOriginal;
  int nRenderLatency;
  int writeSlack;
  int extractSlack;
  int analyzeSlack;
  int markSlack;
  int assignSlack;
  int renderSlack;
  countType leftPos;
  countType rightPos;
  long nToDrop;
  long nFramesInFile;

  SubBand *source;
  list<SMSRenderer*> renderers;
  RingBuffer<float> stretchPreAnalysis;
  RingBuffer<float> *stretchMod;
  RingBuffer<float> stretchRender[2];
  RingBuffer<float> pitchRender[2];
  int inputFrameSize;
  RingBuffer<int> outputFrameSize;
  RingBuffer<float> onset;
  grain *gPrev;

  long inputSamplesQueued;
  float outputSamplesQueued;
  float lastStretchMod;
  float totalSizef;
  bool bPreAnalyze;
  SBSMSQuality *quality;
  int channels;
  int N;
  int N0,N1,N2,Nlo;
  int M_MAX;
  int h;
  int hsub;
  int M;
  long prepad;
  countType samplePos;
  int s;
  long nReadFromOutputFrame;
  long nToWriteForGrain;
  int res;
  int nGrainsPerFrame;
  int resTotal;
  long nGrainsToDrop;

  bool bBackwards[2];
  bool bSynthStarted[2];
  long synthFramePos[2];
  long bSynthGrainStart[2];
  long nGrainsSynthed[2];

  long nGrainsToAnalyze[3];
  long nGrainsToExtract[2];
  long nGrainsToMark[2];
  long nGrainsToAssign[2];
  long nGrainsToAdvance[2];
  long nGrainsToRender[2];
  long nGrainsWritten;
  long nGrainsMarked[2];
  long nGrainsAssigned[2];
  long nGrainsStarted[2];
  long nGrainsAdvanced[2];
  long nGrainsRendered[2];
  long nGrainsRead;

  long nFramesAnalyzed[3];
  long nFramesExtracted[2];
  long nFramesMarked[2];
  long nFramesAssigned[2];
  long nFramesRendered[2];
  long nFramesRead;

  SubBand *parent;
  SubBand *sub;
  SampleBufBase *outMixer;
  SMSSynthRenderer *synthRenderer;
  SMS *sms;
  SMSFile *smsFile;
  SMSMemory *smsMemory;
  SMSAnalyze *smsAnalyze;
  SampleBuf *samplesSubIn;
  SampleBuf *samplesSubOut;
  GrainBuf *grains[3];
  GrainBuf *analyzedGrains[3][2];
  GrainBuf *grainsIn;
  GrainBuf *grainsPre;
  
  GrainAllocator *downSampledGrainAllocator;
  audio *x1[2];
  audio *x2[2];

};

}

#endif
