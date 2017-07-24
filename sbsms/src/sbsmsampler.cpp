#include "sbsmsampler.h"
#include "real.h"
#include "convert.h"
#include "grain.h"
#include "utils.h"
#include "synthTable.h"

SBSMSampler :: SBSMSampler(audioMasterCallback audioMaster) : AudioEffectX(audioMaster, NUM_PROGRAMS, NUM_SAMPLES*NUM_PARAMS)
{
#ifdef WIN32
	  int status = pthread_win32_process_attach_np();
#endif
  voiceList = NULL;
  numBlockEvents = 0;
  gui = new SBSMSamplerGUI(this);
  setEditor(gui);

  for(int k=0;k<NUM_PROGRAMS;k++) {
    programs[k] = NULL;
  }
  setProgram(0);
  blockEvents = (BlockEvents*)calloc(EVENTS_QUEUE_MAX,sizeof(BlockEvents));

  outBuf = (float*) calloc(getBlockSize()*2,sizeof(float));
  setSampleRate(getSampleRate());
  setBlockSize(getBlockSize());
  
  pthread_mutex_init(&masterRenderMutex,NULL);
  for(int i=0; i<NUM_SAMPLES; i++) {
    pthread_mutex_init(&(renderMutex[i]),NULL);
    renderer[i] = NULL;
    bRenderThread[i] = false;
    drawer[i] = NULL;
    pthread_mutex_init(&(drawMutex[i]),NULL);
    bDrawThread[i] = false;
    tIndexQueue.push(i);
  }

  setNumInputs(4);
  setNumOutputs(2);
  setUniqueID('SbSm');
  canProcessReplacing();
  programsAreChunks(true);
  VstTimeInfo *time = getTimeInfo(kVstTempoValid|kVstTimeSigValid|kVstClockValid);
  beatPeriod = 60.0 / time->tempo;
  initialLatency = 512;
  latency = initialLatency;
  nfft = 2 * latency;
  stride = nfft/4;
  nstrided = 0;
  inputBuf[0] = new ArrayRingBuffer<float>(initialLatency);
  inputBuf[1] = new ArrayRingBuffer<float>(initialLatency);
  inputBuf[0]->grow(initialLatency);
  inputBuf[1]->grow(initialLatency);
  sideGrain = NULL;
  sideBuf = new audio[latency];
  memset(sideBuf,0,latency*sizeof(audio));
  grainBuf = new GrainBuf(nfft,stride,nfft,hann);
  grainBuf->write(sideBuf,latency);
  x0 = new audio[nfft/2+1];
  x1 = new audio[nfft/2+1];
  
  newSample();
}

SBSMSampler :: ~SBSMSampler() 
{
  delete grainBuf;
  free(blockEvents);
  free(outBuf);
  delete sideBuf;
  delete inputBuf[0];
  delete inputBuf[1];
  delete x0;
  delete x1;
  for(int i=0; i<NUM_SAMPLES; i++) {
     stopRender(i);
  }
  set<SBSMS*> sbsmsSet;
  for(map<string,SBSMS*>::iterator i=fileSBSMSMap.begin(); i != fileSBSMSMap.end(); ++i) {
    printf("deleting %s\n",i->first.c_str());
    sbsmsSet.insert(i->second);
  }
  for(set<SBSMS*>::iterator i = sbsmsSet.begin(); i != sbsmsSet.end(); ++i) {
    delete *i;
  }
}		

/* VST formalities */

char keyName[12][3] = {"C","C#","D","D#","E","F","F#","G","G#","A","A#","B"};

bool SBSMSampler :: bInit = false;

AudioEffect* createEffectInstance (audioMasterCallback audioMaster)
{
  return new SBSMSampler(audioMaster);
}

int SBSMSampler :: getChunk(void** data, bool isPreset)
{
  int size = 0;
  int n = p->getNumSamples();
  size += sizeof(int);
  size += n * sizeof(SBSMSampleData);
  char *chunkStart = (char*)malloc(size);
  char *chunk = chunkStart;
  memcpy(chunk,&n,sizeof(int)); 
  chunk += sizeof(int);
  list<int> indexList;
  p->lockIndexList(indexList);
  for(list<int>::iterator i = indexList.begin(); i != indexList.end(); ++i) {
    int index = *i;
    p->getSampleData(index,chunk); chunk += sizeof(SBSMSampleData);
  }
  p->unlockIndexList();
  *data = chunkStart;
	return size;
}

int SBSMSampler :: setChunk(void* data, int size, bool isPreset)
{
  char *chunk = (char*)data;
  int numSamples;
  memcpy(&numSamples,chunk,sizeof(int)); chunk += sizeof(int);
  for(int k=0;k<numSamples;k++) {
    SBSMSample *s = newSample();
    p->setSampleData(k,chunk); chunk += sizeof(SBSMSampleData);
  }
  list<int> indexList;
  p->lockIndexList(indexList);
  for(list<int>::iterator i = indexList.begin(); i != indexList.end(); ++i) {
    int index = *i;
    string sf;
    string sn;
    p->getFileName(index,sf);
    p->getSampleName(index,sn);
    renderSampleFile(index,sf,sn);
    p->update(index);
  }
  p->unlockIndexList();
	return 1;
}

void SBSMSampler :: resume()
{
  printf("resume\n");
  /*  initialLatency = 512;
  latency = initialLatency;
  setInitialDelay(initialLatency);
  if(sideGrain) {
    grainBuf->forget(sideGrain);
    sideGrain = NULL;
    grainBuf->clear();
    p->resume();
  }
  */
}

void SBSMSampler :: setProgram(int index)
{
  char str[1024];
  sprintf(str,"set program %d\n",index);
  debug(str);
  p = programs[index];
  if(!p) {
    p = new SBSMSProgram(this);
    char name[32];
    sprintf(name,"Prog %d", index);
    p->setName(name);
    programs[index] = p;
  }
  curProgram = index;
  gui->setProgram(p);
}

int SBSMSampler :: getSidebandSpectrumSize()
{
  return nfft;
}

void SBSMSampler :: setProgramName(char *name)
{
  debug("set program name\n");
  p->setName(string(name));
}

void SBSMSampler :: getProgramName(char *name)
{
  debug("get program name\n");
  string str;
  p->getName(str);
  vst_strncpy(name,str.c_str(),kVstMaxProgNameLen);
}

bool SBSMSampler :: getProgramNameIndexed(int category, int index, char* text)
{
  debug("get program name i\n");
  if(index < NUM_PROGRAMS)
    if(programs[index]) {
      string str;
      programs[index]->getName(str);
      vst_strncpy(text, str.c_str(), kVstMaxParamStrLen);
    }
  return true;
}

bool SBSMSampler :: copyProgram(int destination)
{
  return false;
}

void SBSMSampler :: setParameter(int index, float value)
{
  p->setParameter(index,value);
  gui->setParameter(index, value);
}

float SBSMSampler :: getParameter(VstInt32 index)
{
  return p->getParameter(index);
}   
 
bool SBSMSampler :: string2parameter(VstInt32 index, char* text)
{
  return p->string2parameter(index, text);
}

void SBSMSampler :: getParameterName(VstInt32 index, char* text)
{
  p->getParameterName(index, text);
}

void SBSMSampler :: getParameterDisplay(VstInt32 index, char* text)
{
  p->getParameterDisplay(index, text);
}

void SBSMSampler :: getParameterLabel(VstInt32 index, char* text)
{
  p->getParameterLabel(index, text);
}

bool SBSMSampler::getEffectName (char* name)
{
  vst_strncpy (name, "SBSMSampler", kVstMaxEffectNameLen);
  return true;
}

bool SBSMSampler::getProductString (char* text)
{
  vst_strncpy (text, "SBSMSampler", kVstMaxProductStrLen);
  return true;
}

bool SBSMSampler::getVendorString (char* text)
{
  vst_strncpy (text, "Mune", kVstMaxVendorStrLen);
  return true;
}

int SBSMSampler::getVendorVersion ()
{ 
  return 108000; 
}

int SBSMSampler::canDo(char* text)
{
	if (strcmp(text, "receiveVstEvents") == 0)
		return 1;
	if (strcmp(text, "receiveVstMidiEvent") == 0)
		return 1;
	return -1;
}

/* End VST formalities */

int SBSMSampler :: getBlockSize()
{
  return updateBlockSize(); 
}

float SBSMSampler :: getSampleRate() 
{
  return (float)updateSampleRate(); 
}

int SBSMSampler :: getNumMidiInputChannels()  
{
  return 16; 
}

int SBSMSampler :: getNumMidiOutputChannels() 
{ 
  return 16;
}

void SBSMSampler :: clearMaps(const string &sbsmsname)
{
  map<string,SBSMS*>::iterator iSBSMS = fileSBSMSMap.find(sbsmsname);
  if(iSBSMS != fileSBSMSMap.end()) {
    delete iSBSMS->second;
    fileSBSMSMap.erase(iSBSMS);
  }  
  map<string,countType>::iterator iSamples = fileSamplesMap.find(sbsmsname);
  if(iSamples != fileSamplesMap.end()) {
    fileSamplesMap.erase(iSamples);
  }
}

class Renderer {
public:
  Renderer(bool bInMemory, int sIndex, int tIndex, const string &filename, const string &sbsmsname, const string &name, SBSMSampler *sampler, SBSMS *sbsms, SBSMSInterface *iface, SBSMSDrawRenderer *renderer,
           SBSMSQuality *quality = NULL, Slide *rateSlide = NULL, Slide *pitchSlide = NULL, AudioDecoder *decoder = NULL) 
           : filename(filename), sbsmsname(sbsmsname), name(name) {
    this->bInMemory = bInMemory;
    this->sIndex = sIndex;
    this->tIndex = tIndex;
    this->sampler = sampler;
    this->sbsms = sbsms;
    this->iface = iface;
    this->renderer = renderer;
    this->quality = quality;
    this->rateSlide = rateSlide;
    this->pitchSlide = pitchSlide;
    this->decoder = decoder;
  }

  void run() {
    int i = 0;
    long n = 0;
    countType tot = 0;
    countType lastTot;
    countType todo = iface->getSamplesToOutput();
    bActive = true;
    long update = max(10000L,(long)(todo/200));
    do {
      pthread_mutex_lock(&(sampler->renderMutex[tIndex]));
      n = sbsms->renderFrame(iface);
      pthread_mutex_unlock(&(sampler->renderMutex[tIndex]));
      i += n;
      lastTot = tot;
      tot += n;
      if(i > update) {
        i -= update;
        if(renderer) sampler->gui->updateRender(sIndex);
      }
    } while(n && lastTot < todo && bActive);

    if(renderer) sampler->gui->updateRender(sIndex);

    char str[1024];
    sprintf(str,"render done %d %d\n",sIndex,tIndex);
    debug(str);

    debug("render 0\n");
    pthread_mutex_lock(&sampler->masterRenderMutex);
    pthread_mutex_lock(&(sampler->renderMutex[tIndex]));

    if(renderer) sbsms->removeRenderer(renderer);
    if(!bActive) {
      if(renderer) sampler->gui->clearWaveRenderer(sIndex);
      pthread_mutex_unlock(&(sampler->renderMutex[tIndex]));
      pthread_mutex_unlock(&sampler->masterRenderMutex);
      return;
    }

    
    
    debug("render 1\n");
    if(sbsms->isWriteOpen()) sbsms->closeWrite(iface);
    if(sbsms->isReadOpen()) sbsms->closeRead();
    SBSMS *sbsms2;
    if(bInMemory) {
      sbsms2 = sbsms;
    } else {
      sbsms2 = new SBSMS(sbsmsname.c_str(),NULL,false,true);
      while(bActive && sbsms2->renderFrame(iface));
      sbsms2->closeRead();
      if(!bActive) delete sbsms2;
    }
    if(!bActive) {
      if(renderer) sampler->gui->clearWaveRenderer(sIndex);
      pthread_mutex_unlock(&(sampler->renderMutex[tIndex]));
      pthread_mutex_unlock(&sampler->masterRenderMutex);
      return;
    }
    struct stat filestat;
    if(filename.length()) {      
      stat(filename.c_str(),&filestat);
      time_t t = filestat.st_mtime;     
      sampler->fileMTimeMap[filename] = t;
      sampler->fileSamplesMap[filename] = iface->getSamplesToInput();
      sampler->fileSBSMSMap[filename] = sbsms2;
    }    
    stat(sbsmsname.c_str(),&filestat);
    time_t t = filestat.st_mtime;
    sampler->fileMTimeMap[sbsmsname] = t;
    sampler->fileSamplesMap[sbsmsname] = iface->getSamplesToInput();
    sampler->fileSBSMSMap[sbsmsname] = sbsms2;
    sbsms2->reset(iface);
    sbsms2->seek(iface,0);
    
    debug("render 2\n");

    
    debug("render 3\n");

    pthread_mutex_unlock(&(sampler->renderMutex[tIndex]));
    pthread_mutex_unlock(&sampler->masterRenderMutex);

    set<int> &indexSet = sampler->fileSampleIndexMap[sbsmsname];
    for(set<int>::iterator i = indexSet.begin(); i != indexSet.end(); ++i) {
      int k = *i;
      if(sampler->p->isFileName(k,sbsmsname)) {
        if(!sampler->p->isOpen(k)) {
          sampler->p->setSample(k, sbsms2, iface->getSamplesToInput());
        }
        sampler->gui->setSampleFile(k);
      }
    }
    
    debug("render 4\n");

  }

  ~Renderer() {
    if(sbsms && (!bInMemory || !bActive)) delete sbsms;
    if(iface) delete iface;
    if(quality) delete quality;
    if(rateSlide) delete rateSlide;
    if(pitchSlide) delete pitchSlide;
    if(decoder) delete decoder;
  }

  bool bActive;
  bool bInMemory;
  int sIndex;
  int tIndex;
  string filename;
  string sbsmsname;
  string name;
  SBSMSampler *sampler;
  SBSMS *sbsms;
  SBSMSInterface *iface;
  SBSMSDrawRenderer *renderer;
  SBSMSQuality *quality;
  Slide *rateSlide;
  Slide *pitchSlide;
  AudioDecoder *decoder;
};

void SBSMSampler :: stopRender(int index)
{  
  debug("stop render\n");
  pthread_mutex_lock(&(renderMutex[index]));
  if(renderer[index]) {
    renderer[index]->bActive = false;        
    pthread_mutex_unlock(&(renderMutex[index]));
    pthread_join(renderThread[index],NULL);
  } else {
    pthread_mutex_unlock(&(renderMutex[index]));
  }
}

void SBSMSampler :: destroyRenderer(int tIndex)
{
  pthread_mutex_lock(&masterRenderMutex);
  pthread_mutex_lock(&(renderMutex[tIndex]));
  if(renderer[tIndex]) {
    fileRenderingSet.erase(renderer[tIndex]->sbsmsname);
    tIndexQueue.push(tIndex);
    delete renderer[tIndex];
    renderer[tIndex] = NULL;
  }
  pthread_mutex_unlock(&(renderMutex[tIndex]));
  pthread_mutex_unlock(&masterRenderMutex);
  debug("destroy renderer\n");
}

void renderCleanup(void *data)
{
  Renderer *renderer = (Renderer*)data;
  renderer->sampler->destroyRenderer(renderer->tIndex);
}

void *renderThreadCB(void *data) 
{
  pthread_cleanup_push(renderCleanup,data);
  Renderer *renderer = (Renderer*)data;

  renderer->run();

  pthread_cleanup_pop(1);

  pthread_exit(NULL);
  return NULL;
}

void SBSMSampler :: convertAndRenderSampleFile(int sIndex, const string &filename, const string &sbsmsname, const string &name)
{
 
  pthread_mutex_lock(&masterRenderMutex);  
  string ss;
  p->getFileName(sIndex,ss);
  fileSampleIndexMap[ss].erase(sIndex);
  p->setSampleFile(sIndex,sbsmsname,name);
  fileSampleIndexMap[sbsmsname].insert(sIndex);
  if(fileSampleIndexMap[ss].empty()) {
    if(fileRenderingSet.find(ss) != fileRenderingSet.end() && fileTIndexMap.find(ss) != fileTIndexMap.end()) {
      int tIndex = fileTIndexMap[ss];
      stopRender(tIndex);
    }
  }

  bool bRendering = (fileRenderingSet.find(sbsmsname) != fileRenderingSet.end());
  if(bRendering) {
    p->close(sIndex);
    gui->setWaveRenderer(sIndex);
    pthread_mutex_unlock(&masterRenderMutex);
    return;
  }

  if(fileTIndexMap.find(sbsmsname) != fileTIndexMap.end()) {
    int tIndex = fileTIndexMap[sbsmsname];
    pthread_mutex_lock(&(renderMutex[tIndex]));
    if(bRenderThread[tIndex]) {
      pthread_mutex_unlock(&(renderMutex[tIndex]));
      pthread_mutex_unlock(&masterRenderMutex);
      pthread_join(renderThread[tIndex],NULL);
      pthread_mutex_lock(&masterRenderMutex);
    } else {
      pthread_mutex_unlock(&(renderMutex[tIndex]));
    }
  }

  int tIndex = tIndexQueue.front();
  tIndexQueue.pop();
  fileTIndexMap[sbsmsname] = tIndex;

  pthread_mutex_lock(&(renderMutex[tIndex]));  
  struct stat filestat;
  if(!stat(filename.c_str(),&filestat)) {
    time_t t = filestat.st_mtime;    
    if(fileSBSMSMap.find(filename) == fileSBSMSMap.end() ||
      fileMTimeMap.find(filename) == fileMTimeMap.end() || 
      fileMTimeMap[filename] < t) {

      set<int> &indexSet = fileSampleIndexMap[sbsmsname];
      for(set<int>::iterator i = indexSet.begin(); i != indexSet.end(); ++i) {
        int k = *i;
        if(p->isFileName(k,sbsmsname)) {
          p->close(k);
          gui->clearWaveRenderer(k);
        }
      }
      clearMaps(filename);
      clearMaps(sbsmsname);

      AudioDecoder *decoder = import(filename.c_str());
      if(!decoder) {
        printf("File: %s cannot be opened\n",filename.c_str());
        delete decoder;
        pthread_mutex_unlock(&(renderMutex[tIndex]));
        pthread_mutex_unlock(&masterRenderMutex);
        return;
      }
      float srIn = (float) decoder->getSampleRate();
      int channels = decoder->getChannels();
      countType samplesToInput = decoder->getFrames();
      SBSMSQuality *quality = new SBSMSQuality(&SBSMSQualityStandard);
      SBSMS *sbsms = new SBSMS(channels, quality, false, false);
      char str[144];
      sprintf(str,"open %s ...\n",sbsmsname.c_str());
      debug(str);
      sbsms->openWrite(sbsmsname.c_str());
      if(sbsms->getError())  {
        printf("File: %s cannot be opened\n",sbsmsname.c_str());
        delete decoder;
        delete sbsms;
        pthread_mutex_unlock(&(renderMutex[tIndex]));
        pthread_mutex_unlock(&masterRenderMutex);
        return;
      }
      Slide *rateSlide = new Slide(SlideIdentity);
      Slide *pitchSlide = new Slide(SlideIdentity);
      float pitch = (srIn == 44100.0f?1.0f:44100.0f / srIn);
      SBSMSInterface *iface = new SBSMSInterfaceDecoder(rateSlide,pitchSlide,false,
                                                        samplesToInput,0,quality,decoder,pitch);
      SBSMSDrawRenderer *r = gui->getNewWaveRenderer(sIndex,samplesToInput);
      if(r) sbsms->addRenderer(r);
      if(r) gui->putWaveRenderer(sIndex);
      fileSamplesMap[filename] = samplesToInput;
      fileSamplesMap[sbsmsname] = samplesToInput;
      renderer[tIndex] = new Renderer(false,sIndex,tIndex,filename,sbsmsname,name,this,sbsms,iface,r,quality,rateSlide,pitchSlide,decoder);
      int rc = pthread_create(&(renderThread[tIndex]), NULL, renderThreadCB, (void*)renderer[tIndex]);
      if(rc) abort();
      fileRenderingSet.insert(sbsmsname);
      fileTIndexMap[sbsmsname] = tIndex;
      bRenderThread[tIndex] = true;
    } else {
      SBSMS *sbsms = fileSBSMSMap[filename];
      countType samplesToProcess = fileSamplesMap[filename];
      p->setSample(sIndex,sbsms,samplesToProcess);     
      gui->setSampleFile(sIndex);
      gui->setWaveRenderer(sIndex);
    }
  }
  pthread_mutex_unlock(&(renderMutex[tIndex]));
  pthread_mutex_unlock(&masterRenderMutex);
}

void SBSMSampler :: renderSampleFile(int sIndex, const string &sbsmsname, const string &name)
{  
  char str[10240];
  sprintf(str,"set sample file start %s %s\n",sbsmsname.c_str(),name.c_str());
  debug(str);

  pthread_mutex_lock(&masterRenderMutex);  
  string ss;
  p->getFileName(sIndex,ss);
  fileSampleIndexMap[ss].erase(sIndex);
  p->setSampleFile(sIndex,sbsmsname,name);
  fileSampleIndexMap[sbsmsname].insert(sIndex);
  if(fileSampleIndexMap[ss].empty()) {
    if(fileRenderingSet.find(ss) != fileRenderingSet.end() && fileTIndexMap.find(ss) != fileTIndexMap.end()) {
      int tIndex = fileTIndexMap[ss];
      stopRender(tIndex);
    }
  }

  bool bRendering = (fileRenderingSet.find(sbsmsname) != fileRenderingSet.end());
  if(bRendering) {
    p->close(sIndex);
    gui->setWaveRenderer(sIndex);
    pthread_mutex_unlock(&masterRenderMutex);
    return;
  }

  if(fileTIndexMap.find(sbsmsname) != fileTIndexMap.end()) {
    int tIndex = fileTIndexMap[sbsmsname];
    pthread_mutex_lock(&(renderMutex[tIndex]));
    if(bRenderThread[tIndex]) {
      pthread_mutex_unlock(&(renderMutex[tIndex]));
      pthread_mutex_unlock(&masterRenderMutex);
      pthread_join(renderThread[tIndex],NULL);
      pthread_mutex_lock(&masterRenderMutex);
    } else {
      pthread_mutex_unlock(&(renderMutex[tIndex]));
    }
  }
  
  int tIndex = tIndexQueue.front();
  tIndexQueue.pop();
  fileTIndexMap[sbsmsname] = tIndex;

  pthread_mutex_lock(&(renderMutex[tIndex]));
  struct stat filestat;
  if(!stat(sbsmsname.c_str(),&filestat)) {
    time_t t = filestat.st_mtime;  
    if(fileSBSMSMap.find(sbsmsname) == fileSBSMSMap.end() ||
      fileMTimeMap.find(sbsmsname) == fileMTimeMap.end() || 
      fileMTimeMap[sbsmsname] < t) {
      
      set<int> &indexSet = fileSampleIndexMap[sbsmsname];
      for(set<int>::iterator i = indexSet.begin(); i != indexSet.end(); ++i) {
        int k = *i;
        if(p->isFileName(k,sbsmsname)) {
          p->close(k);
          gui->clearWaveRenderer(k);
        }
      }
      clearMaps(sbsmsname);

      countType samplesToInput;
      SBSMS *sbsms = new SBSMS(sbsmsname.c_str(),&samplesToInput,false,true);
      if(sbsms->getError()) {
        fprintf(stderr,"Cannot open file %s\n",sbsmsname.c_str()); 
        delete sbsms;
        pthread_mutex_unlock(&(renderMutex[tIndex]));
        pthread_mutex_unlock(&masterRenderMutex);
        return;
      } 
      SBSMSInterfaceVariableRate *iface = new SBSMSInterfaceVariableRate(samplesToInput);
    
      debug("0\n");
      SBSMSDrawRenderer *r = gui->getNewWaveRenderer(sIndex,samplesToInput);
    
      char str[1024];
      sprintf(str,"1 %p\n",r);
      debug(str);
      if(r) sbsms->addRenderer(r);
      if(r) gui->putWaveRenderer(sIndex);

      debug("2\n");
      renderer[tIndex] = new Renderer(true,sIndex,tIndex,"",sbsmsname,name,this,sbsms,iface,r);
      fileSamplesMap[sbsmsname] = samplesToInput;
      debug("3\n");

      int rc = pthread_create(&(renderThread[tIndex]), NULL, renderThreadCB, (void*)renderer[tIndex]);
      if(rc) abort();
      fileRenderingSet.insert(sbsmsname);
      fileTIndexMap[sbsmsname] = tIndex;
      bRenderThread[tIndex] = true;
    } else {
      SBSMS *sbsms = fileSBSMSMap[sbsmsname];
      countType samplesToProcess = fileSamplesMap[sbsmsname];
      p->setSample(sIndex,sbsms,samplesToProcess);
      gui->setSampleFile(sIndex);
      gui->setWaveRenderer(sIndex);
    }
  } 
  pthread_mutex_unlock(&(renderMutex[tIndex]));
  pthread_mutex_unlock(&masterRenderMutex);
}

class Drawer {
public:
  Drawer(int index, SBSMSampler *sampler, const string &sbsmsname, const string &name, SBSMSDrawRenderer *r) 
    : sbsmsname(sbsmsname), name(name) {
    this->index = index;
    this->sampler = sampler;
    this->r = r;
  }

  void run() {
    sampler->drawSampleFile(this);
  }

  int index;
  SBSMSampler *sampler;
  SBSMSDrawRenderer *r;
  string sbsmsname;
  string name;
};

void SBSMSampler :: destroyDrawer(int index)
{
 pthread_mutex_lock(&(drawMutex[index]));
  if(drawer[index]) {
    delete drawer[index];
    drawer[index] = NULL;
  }
  pthread_mutex_unlock(&(drawMutex[index]));
}

void drawCleanup(void *data)
{
  Drawer *drawer = (Drawer*)data;
  drawer->sampler->destroyDrawer(drawer->index);
}

void *drawThreadCB(void *data) 
{
  pthread_cleanup_push(drawCleanup,data);
  Drawer *drawer = (Drawer*)data;

  drawer->run();

  pthread_cleanup_pop(1);

  pthread_exit(NULL);
  return NULL;
}

void SBSMSampler :: drawSampleFile(int sIndex)
{
  debug("draw start\n");
  
  pthread_mutex_lock(&masterRenderMutex);
  if(gui->setWaveRenderer(sIndex)) {
    pthread_mutex_unlock(&masterRenderMutex);
    return;
  }
  string sbsmsname;
  string name;
  p->getFileName(sIndex,sbsmsname);
  p->getSampleName(sIndex,name);

  debug("draw 0\n");
  if(fileSamplesMap.find(sbsmsname) == fileSamplesMap.end()) {
    pthread_mutex_unlock(&masterRenderMutex);
    return;
  }

  debug("draw 1\n");
  countType samplesToProcess = fileSamplesMap[sbsmsname];
  SBSMSDrawRenderer *r = gui->getNewWaveRenderer(sIndex,samplesToProcess);
  if(!r) {
    pthread_mutex_unlock(&masterRenderMutex);
    return;
  }
  gui->putWaveRenderer(sIndex);
  
  debug("draw 2\n");

  pthread_mutex_lock(&(drawMutex[sIndex]));
  if(bDrawThread[sIndex]) {
    pthread_mutex_unlock(&(drawMutex[sIndex]));
    pthread_mutex_unlock(&masterRenderMutex);
    pthread_join(drawThread[sIndex],NULL);    
    pthread_mutex_lock(&masterRenderMutex);
  } else {    
    pthread_mutex_unlock(&(drawMutex[sIndex]));
  }

  debug("draw 3\n");

  pthread_mutex_lock(&(drawMutex[sIndex]));
  drawer[sIndex] = new Drawer(sIndex, this, sbsmsname, name, r);
  int rc = pthread_create(&(drawThread[sIndex]), NULL, drawThreadCB, (void*)drawer[sIndex]);
  if(rc) abort();
  bDrawThread[sIndex] = true;
  pthread_mutex_unlock(&(drawMutex[sIndex]));

  pthread_mutex_unlock(&masterRenderMutex);
}

void SBSMSampler :: drawSampleFile(Drawer *drawer)
{
  pthread_mutex_lock(&masterRenderMutex);

  int sIndex = drawer->index;
  string sbsmsname(drawer->sbsmsname);
  string name(drawer->name);
  SBSMSDrawRenderer *r = drawer->r;

  if(fileTIndexMap.find(sbsmsname) != fileTIndexMap.end()) {
    int tIndex = fileTIndexMap[sbsmsname];
    pthread_mutex_lock(&(renderMutex[tIndex]));
    if(bRenderThread[tIndex]) {
      pthread_mutex_unlock(&(renderMutex[tIndex]));
      pthread_mutex_unlock(&masterRenderMutex);
      pthread_join(renderThread[tIndex],NULL);
      pthread_mutex_lock(&masterRenderMutex);
    } else {
      pthread_mutex_unlock(&(renderMutex[tIndex]));
    }
  }

  debug("render joined\n");
  
  if(fileSamplesMap.find(sbsmsname) == fileSamplesMap.end()) {
    pthread_mutex_unlock(&masterRenderMutex);
    return;
  }

  countType samplesToProcess = fileSamplesMap[sbsmsname];
  if(fileSBSMSMap.find(sbsmsname) == fileSBSMSMap.end()) {
    pthread_mutex_unlock(&masterRenderMutex);
    return;
  }
  
  SBSMS *sbsms = fileSBSMSMap[sbsmsname];
  sbsms->addRenderer(r);

  int tIndex = tIndexQueue.front();
  tIndexQueue.pop();

  pthread_mutex_lock(&(renderMutex[tIndex]));
  SBSMSInterfaceVariableRate *iface = new SBSMSInterfaceVariableRate(samplesToProcess);
  renderer[tIndex] = new Renderer(true,sIndex,tIndex,"",sbsmsname,name,this,sbsms,iface,r);
  int rc = pthread_create(&(renderThread[tIndex]), NULL, renderThreadCB, (void*)renderer[tIndex]);
  if(rc) abort();
  fileRenderingSet.insert(sbsmsname);
  fileTIndexMap[sbsmsname] = tIndex;
  bRenderThread[tIndex] = true;
  pthread_mutex_unlock(&(renderMutex[tIndex]));
  pthread_mutex_unlock(&masterRenderMutex);
}

float SBSMSampler :: getBeatPeriod()
{
  return beatPeriod;
}

void SBSMSampler :: setSampleRate(float sampleRate)
{
  AudioEffectX :: setSampleRate(sampleRate);
  p->setSampleRate(sampleRate);
}

void SBSMSampler :: setBlockSize(int blockSize) 
{
  AudioEffectX :: setBlockSize(blockSize);
  if(outBuf) free(outBuf);
  outBuf = (float*) calloc(blockSize*2,sizeof(float));
  p->setBlockSize(blockSize);
}

SBSMSample *SBSMSampler :: newSample()
{
  SBSMSample *s = p->newSample();
  if(s) {
    s->setSampleRate(getSampleRate());
    s->setBlockSize(getBlockSize());
    gui->newSample(s->getIndex());
  }
  return s;
}

void SBSMSampler :: removeSample(int sIndex)
{
  pthread_mutex_lock(&masterRenderMutex);
  string ss;
  p->getFileName(sIndex,ss);
  fileSampleIndexMap[ss].erase(sIndex);
  if(fileSampleIndexMap[ss].empty()) {
    if(fileRenderingSet.find(ss) != fileRenderingSet.end() && fileTIndexMap.find(ss) != fileTIndexMap.end()) {
      int tIndex = fileTIndexMap[ss];
      stopRender(tIndex);
    }
  }
  pthread_mutex_unlock(&masterRenderMutex);
  gui->removeSample(sIndex);
  p->removeSample(sIndex);
}

void SBSMSampler :: addVoice(SBSMSVoice *v) 
{ 
  if(voiceList) {
    v->next = voiceList;
    v->prev = NULL;
    voiceList->prev = v;
    voiceList = v;
  } else {
    v->prev = NULL;
    v->next = NULL;
    voiceList = v;
  }
}

void SBSMSampler :: removeVoice(SBSMSVoice *v)
{	
	if(v == voiceList) {
		voiceList = v->next;
	}
	SBSMSVoice *p = v->prev;
	SBSMSVoice *n = v->next;  
	if(p) p->next = n;
	if(n) n->prev = p;
}

void SBSMSampler :: process(float **inputs, float **outputs, int samples, int offset) 
{
  while(samples) {
    int block;
    block = stride - nstrided;
    block = min(block,samples);
    if(latency) block = min(block,latency);


    for(int k=0;k<block;k++) {
      sideBuf[k][0] = inputs[2][offset+k];
      sideBuf[k][1] = inputs[3][offset+k];
    }
    inputBuf[0]->write(inputs[0]+offset,block);
    inputBuf[1]->write(inputs[1]+offset,block);
    nstrided += block;
    grainBuf->write(sideBuf,block);

    if(latency) latency -= block;
    if(nstrided == stride) {
      nstrided = 0;
      if(grainBuf->nReadable()) {
        if(sideGrain) {
          grainBuf->forget(sideGrain);
        }
        sideGrain = grainBuf->read(grainBuf->readPos);
        sideGrain->analyze();
        c2even(sideGrain->x,x0,nfft);
        c2odd(sideGrain->x,x1,nfft);
        for(int k=0; k<=nfft/2; k++) {
          sideGrain->x[k][0] = norm2(x0[k]);
          sideGrain->x[k][1] = norm2(x1[k]);
        }
        grainBuf->reference(sideGrain);
        grainBuf->advance(1);
      }
    }

    if(latency) {
      for(int k=0;k<block;k++) {
        outputs[0][offset+k] = 0;
        outputs[1][offset+k] = 0;
      }
    } else {

      float *delayedBuf[2];
      delayedBuf[0] = inputBuf[0]->getReadBuf();
      delayedBuf[1] = inputBuf[1]->getReadBuf();
      processBlock(delayedBuf,outputs,block,offset);
      inputBuf[0]->advance(block);
      inputBuf[1]->advance(block);
    }

    offset += block;
    samples -= block;
  }
}

void SBSMSampler :: processBlock(float **inputs, float **outputs, int samples, int offset) 
{
  memset(outBuf,0,(samples<<1)*sizeof(float));
  for(SBSMSVoice *v = voiceList; v; v = v->next) { 
    v->preprocess(samples);
  } 
  for(SBSMSVoice *v = voiceList; v; v = v->next) { 
    v->process(inputs,outBuf,samples,sideGrain);
  } 
  for(int k=0;k<samples;k++) {
    int k2 = k<<1;
    outputs[0][offset+k] = outBuf[k2];
    outputs[1][offset+k] = outBuf[k2+1];
  }
}

void SBSMSampler :: processReplacing(float** inputs, float** outputs, VstInt32 samples)
{ 
  int delta = 0;
  BlockEvents end;
  end.delta = samples;
  end.eventStatus = midiNull;
  blockEvents[numBlockEvents++] = end;    
 
  for(SBSMSVoice *v = voiceList; v; v = v->next) {  
	  if(!v->isPlaying() && v->isReady()) {
      v->sample->returnVoice(v);
      removeVoice(v);
    }
  }
  
  VstTimeInfo *time = getTimeInfo(kVstTempoValid|kVstTimeSigValid|kVstClockValid)
;  this->beatPeriod = 60.0 / time->tempo;
  p->setTime(time);

  for(int i=0;i<numBlockEvents;i++) {    
    
    BlockEvents event = blockEvents[i];
    int nextDelta = event.delta;
	
    if(event.eventStatus == midiNote) {
      if(event.byte2) {
        SBSMSVoice *newVoices[NUM_SAMPLES*NUM_VOICES];
        p->triggerOn(event.byte1,event.byte2,event.channel,newVoices,initialLatency);
        for(int k=0;k<NUM_SAMPLES*NUM_VOICES;k++) {
          if(newVoices[k])
            addVoice(newVoices[k]);
          else 
            break;
        }
      } else {	
        p->triggerOff(event.byte1,event.channel,initialLatency);
      }
    } else if(event.eventStatus == midiPitchbend) {
      float pbend = (float)(128*event.byte2 + event.byte1-8192)/8192.0f;
      p->setPitchbend(pbend,event.channel);  
    } else if(event.eventStatus == midiChannelAfterTouch) {
      //p->setChannelAfterTouch((float)event.byte1/127.0f,event.channel);
    } else if(event.eventStatus == midiPolyphonicAfterTouch) {
      //p->setPolyphonicAfterTouch(event.byte1,(float)event.byte2/127.0f,event.channel);
    } else if(event.eventStatus == midiControl) {
      switch(event.byte1) {
      case 1:
        float value = (float)event.byte2/127.0f;
        p->setModWheel(value,event.channel);
        gui->setChannelParameter(event.channel, pRate, value);
        break;
      }
    } else if(event.eventStatus == midiNotesOff) {
      for(SBSMSVoice *v = voiceList; v; v = v->next) {  
		    v->triggerOff(initialLatency);
      }
    }
    process(inputs,outputs, nextDelta - delta, delta);
    delta = nextDelta;
  }  
  numBlockEvents = 0;

  set<float> posSet[NUM_SAMPLES];
  for(SBSMSVoice *v = voiceList; v; v = v->next) {  
    //printf("pos = %g\n",v->getCurrPos());
    posSet[v->getSampleIndex()].insert(v->getCurrPos());
  }

  for(int i=0; i<NUM_SAMPLES; i++) {
    gui->setCurrPos(i, posSet[i]);
  }
}

VstInt32 SBSMSampler :: processEvents (VstEvents* events)
{
  processMIDIEvents(events, blockEvents, &numBlockEvents);
  return 1;
}
