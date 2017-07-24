#ifndef SAMPLEVIEW_H
#define SAMPLEVIEW_H

#include "tempoctrl.h"
#include "wavedisplaysliders.h"
#include "cfileselector.h"
#include "rightclickcontrols.h"
#include "tempoctrl.h"
#include "statictext.h"
#include "sample.h"
#include "sbsms.h"
#include "genctrl.h"

class ProgramView;

class SampleView : public CViewContainer, CControlListener
{
 public:
  SampleView(ProgramView *pv, CRect size, CFrame *frame, CBitmap *bitmap);
  virtual ~SampleView();
	void valueChanged(CDrawContext* context, CControl* control);
  void setParameter(int index, float value);
  void setCurrPos(const set<float> &pos);
  void setWaveRenderer(SBSMSDrawRenderer *r);
  SBSMSDrawRenderer *getWaveRenderer();
  void updateRender();
  SBSMSDrawRenderer *getNewWaveRenderer(countType samplesToProcess);
  void setSampleFile(const string &sbsmsname, const string &name);
  void selectFile();
  int index;
  void setGenCtrl(int mode);
  
 protected:
  CKickButton *buttonOpen;
  CStaticText *textFile;
  CWaveDisplaySliders *waveDisplay;
  
  COptionMenu *optionChannel;
  COptionMenu *optionKeyBase;
  COptionMenu *optionOctaveBase;
  COptionMenu *optionKeyLo;
  COptionMenu *optionOctaveLo;
  COptionMenu *optionKeyHi;
  COptionMenu *optionOctaveHi;  
  COptionMenu *optionPitchbendRange;
  COptionMenu *optionLoop;
  COptionMenu *optionSynthMode;
  COnOffButton *onoffFollowMode;
  RightClickVSlider *sliderVolume;  
  RightClickAnimKnob *knobAttack;
  RightClickAnimKnob *knobRelease;
  TempoCtrl *knobRate;
  TempoCtrl *knobAMFreq0;
  RightClickAnimKnob *knobAMFreq1; 
  COnOffButton *onoffAMFreq1Mode;
  RightClickAnimKnob *knobAMDepth0;
  RightClickAnimKnob *knobAMDepth1;
  COnOffButton *onoffAMDepth1Mode;
  TempoCtrl *knobFMFreq0;
  RightClickAnimKnob *knobFMFreq1; 
  COnOffButton *onoffFMFreq1Mode;
  RightClickAnimKnob *knobFMDepth0;
  RightClickAnimKnob *knobFMDepth1;
  COnOffButton *onoffFMDepth1Mode;
  RightClickAnimKnob *knobSidebandMod;
  RightClickAnimKnob *knobSidebandBW;
  RightClickAnimKnob *knobSidebandBWScale;
  RightClickAnimKnob *knobSidebandRelease;
  RightClickAnimKnob *knobSidebandEnv;
  RightClickAnimKnob *knobModPivot;
  RightClickAnimKnob *knobThresh;
  RightClickAnimKnob *knobForce;
  RightClickAnimKnob *knobDrag;
  RightClickAnimKnob *knobSpring;

  GenCtrl *currGenCtrl;
  GenCtrl *genCtrl[6];

  OscCtrl *oscCtrl;
  BandpassCtrl *bandpassCtrl;
  CombCtrl *combCtrl;
  DecimateCtrl *decimateCtrl;
  GranCtrl *granCtrl;
  DWGSCtrl *dwgsCtrl;

  ProgramView *pv;
  SBSMSampler *sampler;

  CBitmap *bmOscCtrl;
  CBitmap *bmBandpassCtrl;
  CBitmap *bmCombCtrl;
  CBitmap *bmDecimateCtrl;
  CBitmap *bmGranCtrl;
  CBitmap *bmDWGSCtrl;

  CBitmap *bmTempo;
  CBitmap *bmPos;
	CBitmap *bmBrowse;
	CBitmap *bmKnob;
	CBitmap *bmSlider;
  CBitmap *bmOnOff;
};

#endif
