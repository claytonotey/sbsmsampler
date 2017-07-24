#ifndef VSTPROGRAM_H
#define VSTPROGRAM_H

#include "sample.h"
#include "vstsbsms.h"
#include "audioeffectx.h"
#include "synth.h"

#include <stack>
#include <list>
#include <string>

using namespace std;

class SBSMSampler;
class SBSMSamplerGUI;

class SBSMSProgram {
 public:
  SBSMSProgram(SBSMSampler *sampler);
  ~SBSMSProgram();

  int getNumSamples();
  void setParameter(int index, float value);
  float getParameter(int index);	
  void getParameterLabel(int index, char *label);
	void getParameterDisplay(int index, char *text);
	void getParameterName(int index, char *text);
  bool string2parameter(int index, char *text);
  void setName(const string &name);
  void getName(string &name);
  void getSampleData(int index, void *buf);
  void setSampleData(int index, void *buf);
  void getFileName(int index, string &str);
  bool isFileName(int index, const string &str);
  void getSampleName(int index, string &str);
  void setSampleFile(int index, const string &sbsmsname, const string &name);
  void setSample(int index, SBSMS *sbsms, countType samplesToProcess);
  bool isChannel(int index, int channel);
  bool isOpen(int index);
  void close(int index);
  void update(int index);
  void lockIndexList(list<int> &indexList);
  void unlockIndexList();
  SBSMSample *newSample();
  void removeSample(int index);
  void setSampleRate(float sampleRate);
  void setBlockSize(int blockSize);
  void triggerOn(int note, int velocity, int channel, SBSMSVoice **newVoices, int latency);
  void triggerOff(int note, int channel, int latency);
  void setPitchbend(float value, int channel);
  void setModWheel(float value, int channel);
  void setChannelAfterTouch(float value, int channel);
  void setPolyphonicAfterTouch(int note, float value, int channel);
  void setTime(VstTimeInfo *time);
  void resume();

protected:
  int spectN;
  string name;
  pthread_mutex_t sMutex;
  pthread_mutex_t indexMutex;
  stack<int> indexStack;
  list<int> indexList;
  ProgramSynthesizer programSynth;
  SBSMSampler *sampler;
  SBSMSample *samples[NUM_SAMPLES];
};

#endif
