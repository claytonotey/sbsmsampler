#ifndef GENCTRL_H
#define GENCTRL_H

#include "vstgui.h"
#include "tempoctrl.h"
#include "rightclickcontrols.h"
#include "statictext.h"
#include "vstcontrols.h"

class ProgramView;

class GenCtrl : public CViewContainer
{
 public:
  GenCtrl(CControlListener *listener, CRect size, CFrame *frame, CBitmap *bitmap);
  virtual ~GenCtrl();
  
 protected:
  CControlListener *listener;
	CBitmap *bmKnob;
  CBitmap *bmOnOff;
};


class OscCtrl : public GenCtrl 
{
 public:
  OscCtrl(CControlListener *listener, CRect size, CFrame *frame, CBitmap *bitmap);
  RightClickAnimKnob *knobDistortion0;
  RightClickAnimKnob *knobDistortion1;
  COnOffButton *onoffDistortion1Mode;
};

class CombCtrl : public GenCtrl
{
 public:
  CombCtrl(CControlListener *listener, CRect size, CFrame *frame, CBitmap *bitmap);
  RightClickAnimKnob *knobCombFB0;
  RightClickAnimKnob *knobCombFB1;
  COnOffButton *onoffCombFB1Mode;
};

class BandpassCtrl : public GenCtrl
{
 public:
  BandpassCtrl(CControlListener *listener, CRect size, CFrame *frame, CBitmap *bitmap);
  RightClickAnimKnob *knobFilterQ0;
  RightClickAnimKnob *knobFilterQ1;
  COnOffButton *onoffFilterQ1Mode;
};

class DecimateCtrl : public GenCtrl
{
 public:
  DecimateCtrl(CControlListener *listener, CRect size, CFrame *frame, CBitmap *bitmap);
  RightClickAnimKnob *knobDecBits0;
  RightClickAnimKnob *knobDecBits1;
  COnOffButton *onoffDecBits1Mode;
};

class GranCtrl : public GenCtrl
{
 public:
  GranCtrl(CControlListener *listener, CRect size, CFrame *frame, CBitmap *bitmap);
  COptionMenu *optionGranMode;
  RightClickAnimKnob *knobGranRate0;
  RightClickAnimKnob *knobGranRate1;
  COnOffButton *onoffGranRate1Mode;
  RightClickAnimKnob *knobGranSmooth0;
  RightClickAnimKnob *knobGranSmooth1;
  COnOffButton *onoffGranSmooth1Mode;

};

class DWGSCtrl : public GenCtrl
{
 public:
  DWGSCtrl(CControlListener *listener, CRect size, CFrame *frame, CBitmap *bitmap);
  RightClickAnimKnob *knobDWGSDecay0;
  RightClickAnimKnob *knobDWGSDecay1;
  COnOffButton *onoffDWGSDecay1Mode;
  RightClickAnimKnob *knobDWGSLopass0;
  RightClickAnimKnob *knobDWGSLopass1;
  COnOffButton *onoffDWGSLopass1Mode;
  RightClickAnimKnob *knobDWGSStringPos0;
  RightClickAnimKnob *knobDWGSStringPos1;
  COnOffButton *onoffDWGSStringPos1Mode;

};


#endif
