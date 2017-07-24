#ifndef SAMPLE_H
#define SAMPLE_H

#include <queue>
#include <list>
#include <string>
using namespace std;


#include "sbsms.h"
#include "synth.h"
#include "vstsbsms.h"
#include "voice.h"

enum {
  kFileNameLength = 4096,
  kNameLength = 1024
};


class SBSMSampler;
class SBSMSProgram;

struct SBSMSampleData {
  char filename[kFileNameLength+1];
  char name[kNameLength+1];
  float params[NUM_PARAMS];
};

class SBSMSample 
{
public:
  SBSMSample(SBSMSampler *sampler, int index, ProgramSynthesizer *programSynth);
  ~SBSMSample();

  void getName(string &str);
  void setName(const string &str);
  bool isFileName(const string &str);
  void getFileName(string &str);
  void setFileName(const string &str);
  void getData(void *buf);
  void setData(void *buf);
  void setParameter(int param, float value);
  float getParameter(int param);
	void getParameterDisplay(int index, char *text);
  bool string2parameter(int index, char *text);

  void update();
  void updateAttack();
  void updateRelease();
  void updateVolume();
  void updateChannel();
  void updateRate();  
  void updateRateMode();  
  void updateAMFreq0();
  void updateAMFreq0Mode();
  void updateAMFreq1();
  void updateAMFreq1Mode();
  void updateAMDepth0();
  void updateAMDepth1();
  void updateAMDepth1Mode();
  void updateFMFreq0();
  void updateFMFreq0Mode();
  void updateFMFreq1();
  void updateFMFreq1Mode();
  void updateFMDepth0();
  void updateFMDepth1();
  void updateFMDepth1Mode();
  void updateDistortion0();
  void updateDistortion1();
  void updateDistortion1Mode();
  void updateFilterQ0();
  void updateFilterQ1();
  void updateFilterQ1Mode();
  void updateSidebandMod();
  void updateSidebandBW();
  void updateSidebandBWScale();
  void updateSidebandRelease();
  void updateSidebandEnv();
  void updateModPivot();
  void updateNoteBase();
  void updateKeyBase();
  void updateOctaveBase();
  void updateNoteLo();
  void updateKeyLo();
  void updateOctaveLo();
  void updateNoteHi();
  void updateKeyHi();
  void updateOctaveHi();
  void updatePitchbendRange();
  void updateLeftPos();  
  void updateRightPos();
  void updateStartPos();
  void updateLoop();
  void updateFollowMode();
  void updateSynthMode();
  void updateCombFB0();
  void updateCombFB1();
  void updateCombFB1Mode();
  void updateDecBits0();
  void updateDecBits1();
  void updateDecBits1Mode();
  void updateGranMode();
  void updateGranRate0();
  void updateGranRate1();
  void updateGranRate1Mode();
  void updateGranSmooth0();
  void updateGranSmooth1();
  void updateGranSmooth1Mode();
  void updateDWGSDecay0();
  void updateDWGSDecay1();
  void updateDWGSDecay1Mode();
  void updateDWGSLopass0();
  void updateDWGSLopass1();
  void updateDWGSLopass1Mode();
  void updateDWGSStringPos0();
  void updateDWGSStringPos1();
  void updateDWGSStringPos1Mode();
  void updateThresh();
  void updateForce();
  void updateDrag();
  void updateSpring();

  bool setAttack(float value);
  bool setRelease(float value);
  bool setVolume(float value);
  bool setChannel(int value);
  bool setRate(float value);  
  bool setRateMode(float value);  
  bool setAMFreq0(float value);
  bool setAMFreq0Mode(float value);
  bool setAMFreq1(float value);
  bool setAMFreq1Mode(float value);
  bool setAMDepth0(float value);
  bool setAMDepth1(float value);
  bool setAMDepth1Mode(float value);
  bool setFMFreq0(float value);
  bool setFMFreq0Mode(float value);
  bool setFMFreq1(float value);
  bool setFMFreq1Mode(float value);
  bool setFMDepth0(float value);
  bool setFMDepth1(float value);
  bool setFMDepth1Mode(float value);
  bool setDistortion0(float value);
  bool setDistortion1(float value);
  bool setDistortion1Mode(float value);
  bool setFilterQ0(float value);
  bool setFilterQ1(float value);
  bool setFilterQ1Mode(float value);
  bool setSidebandMod(float value);
  bool setSidebandBW(float value);
  bool setSidebandBWScale(float value);
  bool setSidebandRelease(float value);
  bool setSidebandEnv(float value);
  bool setModPivot(float value);
  bool setKeyBase(int value);
  bool setOctaveBase(int value);
  bool setKeyLo(int value);
  bool setOctaveLo(int value);
  bool setKeyHi(int value);
  bool setOctaveHi(int value);
  bool setPitchbendRange(int value);
  bool setLeftPos(float value);  
  bool setRightPos(float value);
  bool setStartPos(float value);
  bool setSynthMode(float value);
  bool setGranMode(float value);
  bool setLoop(int value);
  bool setFollowMode(int value);
  bool setCombFB0(float value);
  bool setCombFB1(float value);
  bool setCombFB1Mode(float value);
  bool setDecBits0(float value);
  bool setDecBits1(float value);
  bool setDecBits1Mode(float value);
  bool setGranRate0(float value);
  bool setGranRate1(float value);
  bool setGranRate1Mode(float value);
  bool setGranSmooth0(float value);
  bool setGranSmooth1(float value);
  bool setGranSmooth1Mode(float value);
  bool setDWGSDecay0(float value);
  bool setDWGSDecay1(float value);
  bool setDWGSDecay1Mode(float value);
  bool setDWGSLopass0(float value);
  bool setDWGSLopass1(float value);
  bool setDWGSLopass1Mode(float value);
  bool setDWGSStringPos0(float value);
  bool setDWGSStringPos1(float value);
  bool setDWGSStringPos1Mode(float value);
  bool setThresh(float value);
  bool setForce(float value);
  bool setDrag(float value);
  bool setSpring(float value);

  void setSample(SBSMS *sbsms, countType samplesToProcess);
  void setBlockSize(int value);
  void setSampleRate(float value);
  void setPitchbend(float value);
  void setChannelAfterTouch(float value);
  void setPolyphonicAfterTouch(int note, float value);
  void setTime(VstTimeInfo *time);

  bool isOpen();
  void close();
  int getIndex();
  int getNoteBase();
  int getNoteLo();
  int getNoteHi();
  int getChannel();
  void resume();


  SBSMSVoice *triggerOn(int note, int velocity, int latency);
  void triggerOff(int note, int latency);
  void returnVoice(SBSMSVoice *v);
  SBSMSampler *sampler;
  SampleSynthesizer synth;
  float dattack;
  float drelease;
  float volume;  

protected:
  SBSMSVoice *getAvailableVoice();
  SBSMSVoice *lastVoice;
  countType samplesToProcess;
  int numVoices;
  int index;
  int maxGrainSize;
  SBSMSampleData data;
  int pitchbendRange;
  float leftPos;
  float rightPos;
  float startPos;
  int loop;
  int followMode;
  float Fs;
  float attack;
  float release;
  float rate;
  int rateMode;
  int fAM0Mode;
  int fFM0Mode;
  int keyBase, octaveBase, noteBase;
  int keyLo, octaveLo, noteLo;
  int keyHi, octaveHi, noteHi;
  int channel;
  bool bOpen;
  SBSMSVoice *notes[NUM_NOTES];
  SBSMSVoice *voices[NUM_VOICES];
  queue<int> indexQueue;

};

#endif
