#include "vstprogram.h"
#include "sbsmsampler.h"

char paramName[NUM_PARAMS][kVstMaxNameLen+1] = 
{
"attack",
"release",
"loop",
"follow mode",
"rate",
"AM frequency",
"AM frequency mod",
"AM depth",
"AM depth mod",
"FM frequency",
"FM frequency mod",
"FM depth",
"FM depth mod",
"distortion",
"distortion mod",
"filter Q",
"Q mod",
"sideband mod",
"sideband bw",
"sideband bw scale",
"sideband release",
"mod pivot freq",
"volume",
"key base",
"octave base",
"key lo",
"octave lo",
"key hi",
"octave hi",
"channel",
"pitchbend range",
"left pos",
"right pos",
"start pos",
"synth mode",
"comb fb",
"dec bits",
"gran mode",
"gran smooth",
"dwgs decay",
"dwgs lopass",
"dwgs string pos"
};

char paramLabel[NUM_PARAMS][kVstMaxNameLen+1] = 
{
"s",
"s",
"",
"",
"",
"Hz",
"",
"",
"",
"Hz",
"",
"",
"",
"",
"",
"",
"",
"",
"",
"",
"",
"",
"",
"",
"",
"",
"",
"",
"",
"",
"",
"",
"",
"",
"",
"",
};

SBSMSProgram ::SBSMSProgram(SBSMSampler *sampler)
{
  pthread_mutex_init(&sMutex,NULL);
  pthread_mutex_init(&indexMutex,NULL);
  this->sampler = sampler;
  for(int k=k<NUM_SAMPLES-1;k>=0;k--) {
    samples[k] = NULL;
    indexStack.push(k);
  }
}

SBSMSProgram :: ~SBSMSProgram()
{
  pthread_mutex_lock(&sMutex);
  for(int k=0;k<NUM_SAMPLES;k++) {
    SBSMSample *s = samples[k];
    if(s) delete s;
  }
  pthread_mutex_unlock(&sMutex);
}

int SBSMSProgram :: getNumSamples()
{
  int ret;
  pthread_mutex_lock(&sMutex);
  ret = indexList.size();
  pthread_mutex_unlock(&sMutex);
  return ret;
}

void SBSMSProgram :: lockIndexList(list<int> &indexList)
{
  pthread_mutex_lock(&indexMutex);
  indexList.assign(this->indexList.begin(),this->indexList.end());
}

void SBSMSProgram :: unlockIndexList()
{
  pthread_mutex_unlock(&indexMutex);
}

SBSMSample *SBSMSProgram :: newSample()
{  
  pthread_mutex_lock(&sMutex);
  pthread_mutex_lock(&indexMutex);
  if(indexStack.empty()) {
    pthread_mutex_unlock(&indexMutex);
    pthread_mutex_unlock(&sMutex);
    return NULL;
  }
  int index = indexStack.top();
  indexStack.pop();
  SBSMSample *s = new SBSMSample(sampler,index,&programSynth);
  programSynth.sampleSynths.push_back(&s->synth);
  samples[index] = s;
  indexList.push_back(index);   
  pthread_mutex_unlock(&indexMutex);
  pthread_mutex_unlock(&sMutex);
  return s;
}

void SBSMSProgram :: removeSample(int index)
{
  pthread_mutex_lock(&sMutex);
  pthread_mutex_lock(&indexMutex);
  SBSMSample *s = samples[index];
  if(s) programSynth.sampleSynths.remove(&s->synth);
  if(s) delete s;
  samples[index] = NULL;
  indexStack.push(index);
  indexList.remove(index);
  pthread_mutex_unlock(&indexMutex);
  pthread_mutex_unlock(&sMutex);
}

void SBSMSProgram :: setName(const string &name)
{
  pthread_mutex_lock(&sMutex);
  this->name.assign(name);
  pthread_mutex_unlock(&sMutex);
}

void SBSMSProgram :: setSampleFile(int index, const string &sbsmsname, const string &name)
{
  pthread_mutex_lock(&sMutex);
  SBSMSample *s = samples[index];
  if(s) s->setFileName(sbsmsname);
  if(s) s->setName(name);
  pthread_mutex_unlock(&sMutex);
}

void SBSMSProgram :: setSample(int index, SBSMS *sbsms, countType samplesToProcess)
{
  pthread_mutex_lock(&sMutex);
  SBSMSample *s = samples[index];
  if(s) s->setSample(sbsms, samplesToProcess);
  pthread_mutex_unlock(&sMutex);
}

bool SBSMSProgram :: isOpen(int index)
{  
  bool ret;
  pthread_mutex_lock(&sMutex);
  SBSMSample *s = samples[index];
  if(s) ret = s->isOpen();
  else ret = false;
  pthread_mutex_unlock(&sMutex);
  return ret;
}

void SBSMSProgram :: close(int index)
{
  pthread_mutex_lock(&sMutex);
  SBSMSample *s = samples[index];
  if(s) s->close();
  pthread_mutex_unlock(&sMutex);
}

bool SBSMSProgram :: isFileName(int index, const string &str)
{
  bool ret = false;
  pthread_mutex_lock(&sMutex);
  SBSMSample *s = samples[index];
  if(s) ret = s->isFileName(str);
  pthread_mutex_unlock(&sMutex);
  return ret;
}

void SBSMSProgram :: getFileName(int index, string &str)
{
  pthread_mutex_lock(&sMutex);
  SBSMSample *s = samples[index];
  if(s) s->getFileName(str);
  pthread_mutex_unlock(&sMutex);
}

void SBSMSProgram :: getSampleName(int index, string &str)
{
  pthread_mutex_lock(&sMutex);
  SBSMSample *s = samples[index];
  if(s) s->getName(str);
  pthread_mutex_unlock(&sMutex);
}

void SBSMSProgram :: getSampleData(int index, void *buf)
{
  pthread_mutex_lock(&sMutex);
  SBSMSample *s = samples[index];
  if(s) s->getData(buf);
  pthread_mutex_unlock(&sMutex);
}

void SBSMSProgram :: setSampleData(int index, void *buf)
{
  pthread_mutex_lock(&sMutex);
  SBSMSample *s = samples[index];
  if(s) s->setData(buf);
  pthread_mutex_unlock(&sMutex);
}

void SBSMSProgram :: update(int index)
{
  pthread_mutex_lock(&sMutex);
  SBSMSample *s = samples[index];
  if(s) s->update();
  pthread_mutex_unlock(&sMutex);
}

void SBSMSProgram :: resume()
{
  pthread_mutex_lock(&sMutex);
  pthread_mutex_lock(&indexMutex);
  for(list<int>::iterator i = indexList.begin(); i != indexList.end(); ++i) {
    int k = *i;
    SBSMSample *s = samples[k];
    s->resume();
  }
  pthread_mutex_unlock(&indexMutex);
  pthread_mutex_unlock(&sMutex);
}

void SBSMSProgram :: getName(string &name)
{
  pthread_mutex_lock(&sMutex);
  name.assign(this->name);
  pthread_mutex_unlock(&sMutex);
}

void SBSMSProgram :: triggerOn(int note, int velocity, int channel, SBSMSVoice **newVoices, int latency)
{
  pthread_mutex_lock(&sMutex);
  pthread_mutex_lock(&indexMutex);
  int j = 0;
  for(list<int>::iterator i = indexList.begin(); i != indexList.end(); ++i) {
    int k = *i;
    SBSMSample *s = samples[k];
    if(s->isOpen() && channel == s->getChannel() && note >= s->getNoteLo() && note <= s->getNoteHi()) {
      newVoices[j++] = s->triggerOn(note, velocity, latency);
    }
  }
  newVoices[j++] = NULL;
  pthread_mutex_unlock(&indexMutex);
  pthread_mutex_unlock(&sMutex);
}

void SBSMSProgram :: triggerOff(int note, int channel, int latency)
{
  pthread_mutex_lock(&sMutex);
  pthread_mutex_lock(&indexMutex);
  for(list<int>::iterator i = indexList.begin(); i != indexList.end(); ++i) {
    int k = *i;
    SBSMSample *s = samples[k];
    if(channel == s->getChannel())
      s->triggerOff(note,latency);
  }
  pthread_mutex_unlock(&indexMutex);
  pthread_mutex_unlock(&sMutex);
}

void SBSMSProgram :: setSampleRate(float sampleRate)
{
  pthread_mutex_lock(&sMutex);
  pthread_mutex_lock(&indexMutex);
  for(list<int>::iterator i = indexList.begin(); i != indexList.end(); ++i) {
    int k = *i;
    SBSMSample *s = samples[k];
    s->setSampleRate(sampleRate);
  }
  pthread_mutex_unlock(&indexMutex);
  pthread_mutex_unlock(&sMutex);
}

void SBSMSProgram :: setBlockSize(int blockSize)
{
  pthread_mutex_lock(&sMutex);
  pthread_mutex_lock(&indexMutex);
  for(list<int>::iterator i = indexList.begin(); i != indexList.end(); ++i) {
    int k = *i;
    SBSMSample *s = samples[k];
    s->setBlockSize(blockSize);
  }
  pthread_mutex_unlock(&indexMutex);
  pthread_mutex_unlock(&sMutex);
}

void SBSMSProgram :: setParameter(int index, float value)
{
  pthread_mutex_lock(&sMutex);
  int sample = index/NUM_PARAMS;
  SBSMSample *s = samples[sample];
  if(!s) {
    pthread_mutex_unlock(&sMutex);
    return;
  }
  int param = index%NUM_PARAMS;
  s->setParameter(param,value);
  pthread_mutex_unlock(&sMutex);
}

float SBSMSProgram :: getParameter(int index)
{
  float ret;
  pthread_mutex_lock(&sMutex);
  int sample = index/NUM_PARAMS;
  SBSMSample *s = samples[sample];
  if(!s) {
    pthread_mutex_unlock(&sMutex);
    return 0.0f;
  }
  int param = index%NUM_PARAMS;
  ret = s->getParameter(param);
  pthread_mutex_unlock(&sMutex);
  return ret;
}

void SBSMSProgram :: getParameterLabel(int index, char *label)
{
  int param = index%NUM_PARAMS;
  vst_strncpy(label,paramLabel[param],kVstMaxLabelLen);
}

void SBSMSProgram :: getParameterDisplay(int index, char *text)
{
  pthread_mutex_lock(&sMutex);
  int sample = index/NUM_PARAMS;
  SBSMSample *s = samples[sample];
  if(!s) {
    pthread_mutex_unlock(&sMutex);
  }
  int param = index%NUM_PARAMS;
  s->getParameterDisplay(param,text);
  pthread_mutex_unlock(&sMutex);
}

void SBSMSProgram :: getParameterName(int index, char *text)
{
  int sample = index/NUM_PARAMS;
  int param = index%NUM_PARAMS;
  sprintf(text,"%s %d\n",paramName[param],sample);
}

bool SBSMSProgram :: string2parameter(int index, char *text)
{
  bool ret;
  pthread_mutex_lock(&sMutex);
  int sample = index/NUM_PARAMS;
  SBSMSample *s = samples[sample];
  if(!s) {
    pthread_mutex_unlock(&sMutex);
    return false;
  }
  int param = index%NUM_PARAMS;
  ret = s->string2parameter(param, text);
  pthread_mutex_unlock(&sMutex);
  return ret;
}

bool SBSMSProgram :: isChannel(int index, int channel)
{
  bool ret;
  pthread_mutex_lock(&sMutex);
  SBSMSample *s = samples[index];
  ret = (s && s->getChannel() == channel);
  pthread_mutex_unlock(&sMutex);
  return ret;
}

void SBSMSProgram :: setTime(VstTimeInfo *time)
{
  pthread_mutex_lock(&sMutex);
  pthread_mutex_lock(&indexMutex);
  for(list<int>::iterator i = indexList.begin(); i != indexList.end(); ++i) {
    int k = *i;
    SBSMSample *s = samples[k];
    s->setTime(time);
  }
  pthread_mutex_unlock(&indexMutex);
  pthread_mutex_unlock(&sMutex);
}

void SBSMSProgram :: setPitchbend(float value, int channel)
{
  pthread_mutex_lock(&sMutex);
  pthread_mutex_lock(&indexMutex);
  for(list<int>::iterator i = indexList.begin(); i != indexList.end(); ++i) {
    int k = *i;
    SBSMSample *s = samples[k];
    if(channel == s->getChannel())
      s->setPitchbend(value);
  }
  pthread_mutex_unlock(&indexMutex);
  pthread_mutex_unlock(&sMutex);
}

void SBSMSProgram :: setModWheel(float value, int channel)
{
  pthread_mutex_lock(&sMutex);
  pthread_mutex_lock(&indexMutex);
  for(list<int>::iterator i = indexList.begin(); i != indexList.end(); ++i) {
    int k = *i;
    SBSMSample *s = samples[k];
    if(channel == s->getChannel()) s->setParameter(pRate,value);
  }
  pthread_mutex_unlock(&indexMutex);
  pthread_mutex_unlock(&sMutex);
}

void SBSMSProgram :: setChannelAfterTouch(float value, int channel)
{
  pthread_mutex_lock(&sMutex);
  pthread_mutex_lock(&indexMutex);
  for(list<int>::iterator i = indexList.begin(); i != indexList.end(); ++i) {
    int k = *i;
    SBSMSample *s = samples[k];
    if(channel == s->getChannel()) s->setChannelAfterTouch(value);
  }
  pthread_mutex_unlock(&indexMutex);
  pthread_mutex_unlock(&sMutex);
}

void SBSMSProgram :: setPolyphonicAfterTouch(int note, float value, int channel)
{
  pthread_mutex_lock(&sMutex);
  pthread_mutex_lock(&indexMutex);
  for(list<int>::iterator i = indexList.begin(); i != indexList.end(); ++i) {
    int k = *i;
    SBSMSample *s = samples[k];
    if(channel == s->getChannel()) s->setPolyphonicAfterTouch(note,value);
  }
  pthread_mutex_unlock(&indexMutex);
  pthread_mutex_unlock(&sMutex);
}
