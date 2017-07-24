#ifndef PROGRAMVIEW_H
#define PROGRAMVIEW_H

#include "vstgui.h"
#include "vstcontrols.h"
#include "vstsbsms.h"
#include "ctabviewx.h"

class SBSMSamplerGUI;
class SBSMSProgram;

class ProgramView : public CViewContainer, public CControlListener 
{
 public:
  ProgramView(SBSMSamplerGUI *gui, SBSMSProgram *p);
  virtual ~ProgramView();

  void valueChanged(CDrawContext* context, CControl* control);
  void setCurrPos(int index, const set<float> &pos);
  void setParameter(int index, float value);
  void setChannelParameter(int channel, int param, float value);
  void newSampleView(int index);
  void setSampleFile(int index, const string &sbsmsname, const string &name);
  void removeSampleView(int index);

  SBSMSDrawRenderer *getWaveRenderer(int index);
  void setWaveRenderer(int index, SBSMSDrawRenderer *r);
  void updateRender(int index);
  SBSMSDrawRenderer *getNewWaveRenderer(int index, countType samplesToProcess);
  int getCurrSampleViewIndex();
  void setSampleViewIndex(int index);
  
  SBSMSampler *sampler;

 protected:
  SBSMSProgram *p;
  CRect sampleViewSize;
  CBitmap *bmSampleBack;
  CBitmap *bmAdd;
  CBitmap *bmRemove;
  CKickButton *addButton;
  CKickButton *removeButton;
  CTabView *tabView;
  CStaticText *textProgress;
  SampleView* sampleViews[NUM_SAMPLES];
};

#endif
