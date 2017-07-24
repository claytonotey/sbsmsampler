#include "sbsmsampler.h"
#include "sbsmsamplergui.h"
#include "programview.h"

ProgramView :: ProgramView(SBSMSamplerGUI *gui, SBSMSProgram *p) : CViewContainer(gui->getRect(), gui->getFrame())
{
  setMode (kOnlyDirtyUpdate);

  this->sampler = gui->sampler;
  this->p = p;

  for(int i=0; i<NUM_SAMPLES; i++) {
    sampleViews[i] = NULL;
  }

  CPoint point;
  CRect size;

  CBitmap *bmBack = new CBitmap(kBack);
  setBackground(bmBack);

  // add
  bmAdd = new CBitmap(kAdd);
  point(0, 0);
  size(6, 6, 6+bmAdd->getWidth(), 6+bmAdd->getHeight()/2);
  addButton = new CKickButton(size, this, tagAdd, bmAdd, point);
  addButton->setTransparency(false);
  addView(addButton);

  // remove
  bmRemove = new CBitmap(kRemove);
  point(0, 0);
  size(31, 6, 31+bmRemove->getWidth(), 6+bmRemove->getHeight()/2);
  removeButton = new CKickButton(size, this, tagRemove, bmRemove, point);
  removeButton->setTransparency(false);
  addView(removeButton);

  // tabs
  bmSampleBack = new CBitmap(kSampleBack);
  CBitmap *bmTab = new CBitmap(kTabButton);
  sampleViewSize(0, 24, bmSampleBack->getWidth(), bmBack->getHeight());
  tabView = new CTabView(sampleViewSize, gui->getFrame(), bmTab);
  tabView->setTransparency (true);
  addView(tabView);

  list<int> indexList;
  p->lockIndexList(indexList);
  for(list<int>::iterator i = indexList.begin(); i != indexList.end(); ++i) {
    int k = *i;
    newSampleView(k);
  }
  p->unlockIndexList();
}

ProgramView :: ~ProgramView()
{
  debug("~pv\n");
  if(bmSampleBack) bmSampleBack->forget();
  if(bmAdd) bmAdd->forget();
  if(bmRemove) bmRemove->forget();
  debug("~pv 1\n");
}

SBSMSDrawRenderer *ProgramView :: getWaveRenderer(int index)
{
  SampleView *sv = sampleViews[index];
  if(sv) return sv->getWaveRenderer();
  else return NULL;
}

void ProgramView :: setWaveRenderer(int index, SBSMSDrawRenderer *r)
{
  SampleView *sv = sampleViews[index];
  if(sv) sv->setWaveRenderer(r);
}
 
void ProgramView :: updateRender(int index)
{
  SampleView *sv = sampleViews[index];
  char str[1024];
  sprintf(str,"pv update render %p\n",sv);
  debug(str);
  if(sv) sv->updateRender();
}

SBSMSDrawRenderer *ProgramView :: getNewWaveRenderer(int index, countType samplesToProcess)
{
  SampleView *sv = sampleViews[index];
  if(sv) return sv->getNewWaveRenderer(samplesToProcess);
  else return NULL;
}

void ProgramView :: newSampleView(int index)
{
  SampleView *sv = new SampleView(this,sampleViewSize,getFrame(),bmSampleBack);
  sampleViews[index] = sv;
  sv->index = index;
  tabView->addTab(sv, "-");
  tabView->alignTabs();
  tabView->selectTab(sv);
}

void ProgramView :: setSampleViewIndex(int index)
{
  tabView->selectTab(sampleViews[index]);
}

int ProgramView :: getCurrSampleViewIndex()
{
  SampleView *sv = (SampleView*)tabView->getCurrentChild();
  if(sv) return sv->index;
  else return -1;
}

void ProgramView :: removeSampleView(int index)
{
  list<int> indexList;
  p->lockIndexList(indexList);
  for(list<int>::iterator i = indexList.begin(); i != indexList.end(); ++i) {
    int k = *i;   
    SampleView *sv = sampleViews[k];
    sv->remember();
  }
  tabView->removeAllTabs();
  for(list<int>::iterator i = indexList.begin(); i != indexList.end(); ++i) {
    int k = *i;   
    SampleView *sv = sampleViews[k];
    if(sv->index == index) sv->forget();
    else tabView->addTab(sv,"-");
  }
  p->unlockIndexList();
  sampleViews[index] = NULL;
  tabView->alignTabs();
}
  
void ProgramView :: setSampleFile(int index, const string &sbsmsname, const string &name)
{
  SampleView *sv = sampleViews[index];
  if(sv) {
    tabView->setTabName(sv,name.c_str());
    sv->setSampleFile(sbsmsname, name);
  }
}

void ProgramView :: setCurrPos(int index, const set<float> &pos)
{
  SampleView *sv = sampleViews[index];
  if(sv) sv->setCurrPos(pos);
}

void ProgramView :: setParameter(int index, float value)
{
  int sample = index/NUM_PARAMS;
  int param = index%NUM_PARAMS;
  SampleView *sv = sampleViews[sample];
  if(sv) sv->setParameter(param, value);
}


void ProgramView :: setChannelParameter(int channel, int param, float value)
{
  list<int> indexList;
  p->lockIndexList(indexList);
  for(list<int>::iterator i = indexList.begin(); i != indexList.end(); ++i) {
    int k = *i;   
    SampleView *sv = sampleViews[k];
    if(p->isChannel(k,channel) && sv) {
      sv->setParameter(param, value);
    }
  }
  p->unlockIndexList();
}

void ProgramView :: valueChanged(CDrawContext* context, CControl* control)
{
	long tag = control->getTag();
  switch(tag) {
  case tagAdd:
    if(control->getValue() == 1.0) {
      sampler->newSample();
    }
    break;
  case tagRemove:
    if(control->getValue() == 1.0) {
      int index = getCurrSampleViewIndex();
      sampler->removeSample(index);
    }
    break;
  }
}
