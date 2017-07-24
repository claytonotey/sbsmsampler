#ifndef SBSMSAMPLERGUI_H_
#define SBSMSAMPLERGUI_H_

#include "vstgui.h"
#include "statictext.h"
#include "aeffguieditor.h"
#include "sampleview.h"
#include "vstprogram.h"
#include "programview.h"
#include <map>
#include <string>
using namespace std;

enum {
  tagAdd = 1000,
  tagRemove,
  tagOpen,
  tagKeyBase,
  tagOctaveBase,
  tagKeyLo,
  tagOctaveLo,
  tagKeyHi,
  tagOctaveHi,
  tagChannel,
  tagVolume,
  tagAttack,
  tagRelease,
  tagRate, 
  tagRateMode, 
  tagAMFreq0,
  tagAMFreq0Mode,
  tagAMFreq1,
  tagAMFreq1Mode,
  tagAMDepth0,
  tagAMDepth1,
  tagAMDepth1Mode,
  tagFMFreq0,
  tagFMFreq0Mode,
  tagFMFreq1,
  tagFMFreq1Mode,
  tagFMDepth0,
  tagFMDepth1,
  tagFMDepth1Mode,
  tagDistortion0,
  tagDistortion1,
  tagDistortion1Mode,
  tagFilterQ0,
  tagFilterQ1,
  tagFilterQ1Mode,
  tagSidebandMod,
  tagSidebandBW,
  tagSidebandBWScale,
  tagSidebandRelease,
  tagSidebandEnv,
  tagModPivot,
  tagParticleForce,
  tagParticleDrag,
  tagParticleSpring,
  tagPitchbendRange,
  tagWaveDisplay,
  tagLeftPos,
  tagRightPos,
  tagStartPos,
  tagLoop,
  tagFollowMode,
  tagSynthMode,
  tagCombFB0,
  tagCombFB1,
  tagCombFB1Mode,
  tagDecBits0,
  tagDecBits1,
  tagDecBits1Mode,
  tagGranMode,
  tagGranRate0,
  tagGranRate1,
  tagGranRate1Mode,
  tagGranSmooth0,
  tagGranSmooth1,
  tagGranSmooth1Mode,
  tagDWGSDecay0,
  tagDWGSDecay1,
  tagDWGSDecay1Mode,
  tagDWGSLopass0,
  tagDWGSLopass1,
  tagDWGSLopass1Mode,
  tagDWGSStringPos0,
  tagDWGSStringPos1,
  tagDWGSStringPos1Mode,
  tagThresh,
  tagForce,
  tagDrag,
  tagSpring
};

enum {
  kSampleBack = 128,
  kAdd,
  kRemove,
  kSlider,
  kKnob,
  kBack,
  kBrowse,
  kTabButton,
  kWaveSlider,
  kOnOff,
  kTempo,
  kOscCtrl,
  kBandpassCtrl,
  kCombCtrl,
  kDecimateCtrl,
  kGranCtrl,
  kDWGSCtrl
};

class SBSMSampler;

class SBSMSamplerGUI : public AEffGUIEditor
{
 public:
  SBSMSamplerGUI(SBSMSampler *sampler);
  ~SBSMSamplerGUI();

  void setProgram(SBSMSProgram *p);
  void newSample(int index);
  void removeSample(int index);
  void setParameter(int index, float value);
  void setChannelParameter(int channel, int param, float value);
  bool open(void *ptr);
  void close();
  void idle();
  
  CRect getRect();
  SBSMSampler *sampler;
  void clearWaveRenderer(int index);
  SBSMSDrawRenderer *setWaveRenderer(int index);
  SBSMSDrawRenderer *putWaveRenderer(int index);
  SBSMSDrawRenderer *getWaveRenderer(int index);
  void updateRender(int index);
  SBSMSDrawRenderer *getNewWaveRenderer(int index, countType samplesToProcess);
  void setSampleFile(int index);
  void setCurrPos(int index, const set<float> &pos);

 protected:
  int sampleIndex;
  map<string, SBSMSDrawRenderer*> rMap;
  pthread_mutex_t pvMutex;
  void updateParameters(int index);
  void updateParameters();
  SBSMSProgram *p;
  ProgramView *pv;
  CBitmap *bmBack;
};

#endif
