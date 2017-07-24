#include "sbsmsampler.h"
#include "sbsmsamplergui.h"

SBSMSamplerGUI :: SBSMSamplerGUI(SBSMSampler *sampler) : AEffGUIEditor(sampler)
{
  this->sampler = sampler;
  pv = NULL;
  p = NULL;
  frame = NULL;
  bmBack = NULL;
  sampleIndex = -1;

  bmBack = new CBitmap(kBack);

  pthread_mutex_init(&pvMutex,NULL);
	rect.left   = 0;
	rect.top    = 0;
	rect.right  = (short)bmBack->getWidth();
	rect.bottom = (short)bmBack->getHeight();  
}

SBSMSamplerGUI :: ~SBSMSamplerGUI()
{  
  pthread_mutex_unlock(&pvMutex);
  close();
  for(map<string,SBSMSDrawRenderer*>::iterator i=rMap.begin(); i != rMap.end(); ++i) {
    i->second->forget();
  }
  if(bmBack) bmBack->forget();
}

void SBSMSamplerGUI :: close()
{

  debug("gui close\n");
  pthread_mutex_lock(&pvMutex);
  debug("gui close 0\n");
  if(pv) sampleIndex = pv->getCurrSampleViewIndex();
  if(frame) frame->forget();
  frame = NULL;
  pv = NULL;
  debug("gui close 1\n");
  pthread_mutex_unlock(&pvMutex);  

  AEffGUIEditor::close();  
  debug("gui close 2\n");
}

bool SBSMSamplerGUI :: open(void *ptr)
{
  debug("gui open\n");
	AEffGUIEditor::open(ptr);
  
  CRect size;
  size(rect.left, rect.top, rect.right, rect.bottom);
  
	frame = new CFrame(size, ptr, this);
	frame->setBackground(bmBack);
  
  debug("gui open 0\n");
  if(p) {
    
    debug("gui open 1\n");
    pthread_mutex_lock(&pvMutex);  
    
    debug("gui open 2\n");
    pv = new ProgramView(this, p);
    if(sampleIndex >= 0) pv->setSampleViewIndex(sampleIndex);

  debug("gui open 3\n");
    updateParameters();
    
  debug("gui open 4\n");
    frame->addView(pv);
    
  debug("gui open 5\n");
    pthread_mutex_unlock(&pvMutex);
    
    list<int> indexList;
    p->lockIndexList(indexList);
    for(list<int>::iterator i = indexList.begin(); i != indexList.end(); ++i) {
      int index = *i;
      sampler->drawSampleFile(index);
    }
    p->unlockIndexList();
  }
  return true;
}

void SBSMSamplerGUI :: clearWaveRenderer(int index)
{
  pthread_mutex_lock(&pvMutex);
  if(p) {
    string sf;
    p->getFileName(index,sf);
    if(rMap.find(sf) != rMap.end()) {
      rMap[sf]->cancel();
      rMap.erase(sf);
    }
  }
  pthread_mutex_unlock(&pvMutex);
}

SBSMSDrawRenderer *SBSMSamplerGUI :: setWaveRenderer(int index)
{
  SBSMSDrawRenderer *r = NULL;
  pthread_mutex_lock(&pvMutex);
  if(p && pv) {
    string sf;
    p->getFileName(index,sf);
    if(rMap.find(sf) != rMap.end()) {
      r = rMap[sf];
      pv->setWaveRenderer(index,r);
    }
  }
  pthread_mutex_unlock(&pvMutex);
  return r;
}

SBSMSDrawRenderer *SBSMSamplerGUI :: putWaveRenderer(int index)
{
  SBSMSDrawRenderer *r;
  pthread_mutex_lock(&pvMutex);
  if(p && pv) {    
    r = pv->getWaveRenderer(index);
    string sf;
    p->getFileName(index,sf);
    if(r) rMap[sf] = r;
  }
  pthread_mutex_unlock(&pvMutex);
  return r;
}

SBSMSDrawRenderer *SBSMSamplerGUI :: getWaveRenderer(int index)
{
  SBSMSDrawRenderer *r;
  pthread_mutex_lock(&pvMutex);
  string sf;
  p->getFileName(index,sf);
  if(rMap.find(sf) != rMap.end()) {
    r = rMap[sf];
  } else {
    r = NULL;
  }
  pthread_mutex_unlock(&pvMutex);
  return r;
}


void SBSMSamplerGUI :: updateRender(int index)
{
  pthread_mutex_lock(&pvMutex);
  if(pv) pv->updateRender(index);
  pthread_mutex_unlock(&pvMutex);
}

SBSMSDrawRenderer *SBSMSamplerGUI :: getNewWaveRenderer(int index, countType samplesToProcess)
{
  SBSMSDrawRenderer *r;
  pthread_mutex_lock(&pvMutex);
  if(pv) r = pv->getNewWaveRenderer(index,samplesToProcess);
  else r = NULL;
  pthread_mutex_unlock(&pvMutex);
  return r;
}

void SBSMSamplerGUI :: idle()
{
  AEffGUIEditor::idle();
}

void SBSMSamplerGUI :: setProgram(SBSMSProgram *p)
{
  pthread_mutex_lock(&pvMutex);
  this->p = p;
  if(frame) {
    if(pv) {
      frame->removeView(pv);
    }
    pv = new ProgramView(this, p);
    frame->addView(pv);
  }
  updateParameters();
  pthread_mutex_unlock(&pvMutex);
}

CRect SBSMSamplerGUI :: getRect()
{
  CRect size(rect.left , rect.top, rect.right, rect.bottom);
  return size;
}

void SBSMSamplerGUI :: setCurrPos(int index, const set<float> &pos)
{
  pthread_mutex_lock(&pvMutex);
  if(pv) pv->setCurrPos(index, pos);
  pthread_mutex_unlock(&pvMutex);
}

void SBSMSamplerGUI :: setParameter(int index, float value)
{
  pthread_mutex_lock(&pvMutex);
  if(pv) pv->setParameter(index, value);
  pthread_mutex_unlock(&pvMutex);
}

void SBSMSamplerGUI :: setChannelParameter(int channel, int param, float value)
{
  pthread_mutex_lock(&pvMutex);
  if(pv) pv->setChannelParameter(channel, param, value);
  pthread_mutex_unlock(&pvMutex);
}

void SBSMSamplerGUI :: setSampleFile(int index)
{
  pthread_mutex_lock(&pvMutex); 
  if(p && pv) {
    string sf;
    p->getFileName(index,sf);
    string ss;
    p->getSampleName(index,ss);
    pv->setSampleFile(index, sf, ss);
  }
  pthread_mutex_unlock(&pvMutex);
}

void SBSMSamplerGUI :: newSample(int index)
{
  pthread_mutex_lock(&pvMutex);
  if(pv) pv->newSampleView(index);
  updateParameters(index);
  pthread_mutex_unlock(&pvMutex);
}

void SBSMSamplerGUI :: removeSample(int index)
{
  pthread_mutex_lock(&pvMutex);
  if(pv) pv->removeSampleView(index);
  updateParameters();
  pthread_mutex_unlock(&pvMutex);
}

void SBSMSamplerGUI :: updateParameters()
{
  list<int> indexList;
  p->lockIndexList(indexList);
  for(list<int>::iterator i = indexList.begin(); i != indexList.end(); ++i) {
    int index = *i;
    updateParameters(index);
  }
  p->unlockIndexList();
}

void SBSMSamplerGUI :: updateParameters(int index)
{
  if(p && pv) {
    string sf;
    p->getFileName(index,sf);
    string ss;
    p->getSampleName(index,ss);
    if(sf.length()) {
      if(rMap.find(sf) != rMap.end()) {
        pv->setSampleFile(index,sf,ss);
        pv->setWaveRenderer(index,rMap[sf]);
      }
    }
    for(int j=0;j<NUM_PARAMS;j++) {
      int param = index*NUM_PARAMS + j;
      pv->setParameter(param,sampler->getParameter(param));
    }
  }
}
