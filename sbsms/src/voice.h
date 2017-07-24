#ifndef SBSMSVOICE_H
#define SBSMSVOICE_H

#include "audiobuffer.h"
#include "sbsms.h"
#include "vstsbsms.h"
#include "synth.h"
#include "buffer.h"
#include "audioeffectx.h"

using namespace _sbsms_;

class SBSMSample;

class SBSMSVoice 
{
 public:
  SBSMSVoice(SBSMSample *sample, int blockSize, float Fs, SampleSynthesizer *synth, int spectN);
  ~SBSMSVoice();

  bool isActive();
  bool isReady();
  void preprocess(int samples);
  void process(float **inputs, float *outBuf, int samples, grain *sideGrain);
  void wait();
  bool play();
  bool stop();
  void close();
  void resume();
  int getSampleIndex();
  bool open(SBSMS *sbsms, countType samplesToProcess);
  void setBlockSize(int value);
  void setSampleRate(float value);
  void setLeftPos(float pos);  
  void setRightPos(float pos);
  void setStartPos(float pos);
  float getCurrPos();
  void setStartSample(countType samplePos);
  countType getCurrSample();
  void setLoop(int loop);
  void setVolume(float vol);
  void setRate(float rate);
  void setTime(VstTimeInfo *time);

  /*
  void setAMFreq0(float value);
  void setAMFreq1(float value);
  void setAMDepth0(float value);
  void setAMDepth1(float value);
  void setFMFreq0(float value);
  void setFMFreq1(float value);
  void setFMDepth0(float value);
  void setFMDepth1(float value);
  void setDistortion0(float value);
  void setDistortion1(float value);
  */

  void setAttack(float attack);
  void setRelease(float release);
  void setPitchbend(float pbend);

  bool isPlaying();
  long read(SBSMSFrame *frame);
  void write();
  void writingComplete();
  friend class SBSMSampler;
  friend class SBSMSample;
  friend class VoiceSynthesizer;
protected:
  SBSMSVoice *prev;
  SBSMSVoice *next;
  SBSMSample *sample;
  void seekStart();
  int index;
  void triggerOn(float notePitch, float noteVol, float baseFreq, int latency);
  void triggerOff(int latency);
  void forceOff();
  void setupTransition();

  Granugrain *granuGrain;
  int maxGrainSize;
  AudioBuffer *rb;
  SBSMS *sbsms;
  SBSMS *sbsmsTransition;
  SBSMSInterfaceVariableRate *iface;
  float Fs;
  ArrayRingBuffer<float> *transitionBuf[2];

  pthread_cond_t playCond;
  pthread_mutex_t playCondMutex;

  pthread_cond_t inputCond;
  pthread_mutex_t inputCondMutex;

  VoiceSynthesizer *voiceSynth;
  VoiceSynthesizer *voiceSynthTransition;
  long nTransition;
  long nDoneTransition;
  long nDoneTransition1;
  long nDoneTransition2;
  float pbendPitch;
  float notePitch;
  float noteVol;
  float volUp;
  float volDown;
  float env;
  float volume;
  float rate;
  float currPos;
  float leftPos;
  float rightPos;
  float startPos;  
  countType currSample;
  countType leftSample;
  countType rightSample;
  countType startSample;
  countType nextStartSample;
  int loop;
  bool bLoop;
  bool bBackwards;
  bool bGoingBackwards;
  bool bStill;
  countType samplesToProcess;
  bool bOpen;
  bool bPlaying;
  bool bReady;
  bool bAttack;
  bool bRelease;
  bool bActive;
  int channels;
  pthread_t writeThread;
  pthread_mutex_t playMutex;
  pthread_mutex_t sbsmsMutex;
  long blockSize;
  long blockPos;
  float *outBuf;
  float *fbuf;
  audio *abuf;
  long nInputSamples;
  long nInputSamples2;
  float transition1;
  float transition2;
  audio bufTransition1[257];
  audio bufTransition2[257];
  int latencyOn;
  int latencyOff;
  bool bTransition;
};

#endif
