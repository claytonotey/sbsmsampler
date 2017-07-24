#include "vstsbsms.h"
#include "sbsms.h"
#include "real.h"
#include "subband.h"
#include "track.h"
#include "utils.h"
#include <stdlib.h>
#include <string.h>
#include "config.h"
#ifdef MULTITHREADED
#include <pthread.h>
#endif
#include <algorithm>
using namespace std;

void jitter() {
  //usleep(rand() % 1000);
}

namespace _sbsms_ {


audio *make_audio_buf(long n) {
  return (audio*)calloc(n,sizeof(audio));
}

void free_audio_buf(audio *buf) {
  free(buf);
}

long copy_audio_buf(audio *to, long off1, audio *from, long off2, long n)
{
  memcpy(to+off1,from+off2,n*sizeof(audio));
  return n;
}

long audio_convert_from(float *to, long off1, audio *from, long off2, long n, int channels)
{
  int n2 = n+off2;
  if(channels == 2) {
    int k1 = off1<<1;
    for(int k=off2;k<n2;k++) {
      to[k1++] = (float)from[k][0];
      to[k1++] = (float)from[k][1];
    }
  } else {
    int k1 = off1;
    for(int k=off2;k<n2;k++) {
      to[k1++] = (float)from[k][0];
    }
  }
  return n;
}

long audio_convert_to(audio *to, long off1, float *from, long off2, long n, int channels)
{
  int n1 = n + off1;
  if(channels == 2) {
    int k2 = off2<<1;
    for(int k=off1;k<n1;k++) {
      to[k][0] = (float)from[k2++];
      to[k][1] = (float)from[k2++];
    }
  } else {
    int k2 = off2;
    for(int k=off1;k<n1;k++) {
      to[k][0] = (float)from[k2];
      to[k][1] = (float)from[k2++];
    }
  }
  return n;
}

#define SBSMS_QUALITY_N_PARAMS 52

const SBSMSQualityParams SBSMSQualityStandard = {
  8,4,
  {384,384,512,480,384,288,256,256,0,0},
  {128,96,72,60,48,36,28,16,0,0},
  {192,168,144,120,96,72,56,32,0,0},
  {256,256,192,160,128,96,72,48,0,0},
  {1,1,2,1,2,1,2,1,0,0}
};

const SBSMSQualityParams SBSMSQualityFast = {
  7,8,
  {256,256,288,240,192,144,168,0,0,0},
  {128,128,72,60,48,36,28,0,0,0},
  {192,168,144,120,96,72,56,0,0,0},
  {256,256,192,160,128,96,72,0,0,0},
  {1,1,2,2,1,2,1,0,0,0}
};

SBSMSQuality :: SBSMSQuality(const SBSMSQualityParams *params)
{
  this->params = *params;
}

long SBSMSQuality :: getFrameSize()
{
  return (1<<(params.bands-1)) * params.H;
}

long SBSMSQuality :: getMaxPresamples()
{
  long prepad = 0;
  int M_MAX = 1<<(params.bands-1);
  int m = M_MAX;
  int i;
  for(i=0; i<params.bands-1; i++) {
    m >>= 1;
    prepad += m * params.N[i];
  }
  i = params.bands - 1;
  int framesize = M_MAX * params.H;

  prepad = max(prepad,M_MAX * params.N[i] / 2L);

  long frames = prepad / framesize;
  if(prepad%framesize) frames++;
  prepad = frames * framesize;
  return prepad;
}

class SBSMSRendererImp : public SMSRenderer {
public:
  SBSMSRendererImp(SBSMSRenderer *renderer);
  void startTime(int c, countType samplePos, countType time, int n, float h2, float pos);
  void renderTrack(int c, Track *t, float f0, float f1, SBSMSynthesizer *synth);

protected:
  countType samplePos[2];
  countType time[2];
  int n[2];
  SBSMSRenderer *renderer;
};

SBSMSRenderer :: SBSMSRenderer()
{
  imp = new SBSMSRendererImp(this);
}

SBSMSRenderer :: ~SBSMSRenderer()
{
  delete imp;
}

SBSMSRendererImp :: SBSMSRendererImp(SBSMSRenderer *renderer)
{
  this->renderer = renderer;
}

void SBSMSRendererImp :: startTime(int c, countType samplePos, countType time, int n, float h2, float pos)
{
  this->samplePos[c] = samplePos;
  this->time[c] = time;
  this->n[c] = n;
}

void SBSMSRendererImp :: renderTrack(int c, Track *t, float f0, float f1, SBSMSynthesizer *synth)
{
  TrackPoint *tp0 = t->getTrackPoint(time[c]);
  TrackPoint *tp1 = t->getTrackPoint(time[c]+1);
  if(tp0 && tp1) {
    float m = 1.0f / (float)t->getM();
    renderer->render(c,samplePos[c],n[c],tp0->f*m,tp1->f*m,tp0->y*m,tp1->y*m);
  }
}

class ThreadInterface;
class SBSMSImp {
public:
  friend class SBSMS;

  SBSMSImp(int channels, SBSMSQuality *quality, bool bPreAnalyze, bool bSynthesize);
  SBSMSImp(const char *filename, countType *samplesToInput, bool bSynthesize, bool bKeepInMemory);
  SBSMSImp(SBSMSImp *source);
  ~SBSMSImp();

  inline long read(SBSMSInterface *iface, audio *buf, long n);
  inline void particles(SBSMSInterface *iface, SBSMSynthesizer *synth);
  inline long synthFromMemory(SBSMSInterface *iface, audio *buf, long n, SBSMSynthesizer *synthMod);
  inline long synthFromFile(SBSMSInterface *iface, audio *buf, long n);
  inline void addRenderer(SMSRenderer *renderer);
  inline void removeRenderer(SMSRenderer *renderer);
  inline long renderFrame(SBSMSInterface *iface);
  inline long preAnalyze(SBSMSInterface *iface);
  inline void reset(SBSMSInterface *iface, bool flushInput = false);
  inline void readFooter(long frames);
  inline void seek(SBSMSInterface *iface,countType samplePos);  
  inline bool openWrite(const char *filename);
  inline void closeWrite(SBSMSInterface *iface);
  inline void closeRead();
  inline bool isWriteOpen();
  inline bool isReadOpen();
  inline countType getSamplePosFromMemory();


  SubBand *top;  
#ifdef MULTITHREADED
  friend class ThreadInterface;
  ThreadInterface *threadInterface;
#endif

protected:
  SBSMSImp *source;
  float getInputTime(SBSMSInterface *iface);
  long write(SBSMSInterface *);
  SMSRenderer *renderer;
  FILE *fpIn;  
  FILE *fpOut;
  SBSMSError error;
  bool bKeepInMemory;
  vector<off_t> *frameByteOffset;
  vector<countType> *frameSampleOffset;
  TrackAllocator *ta;  
  PeakAllocator **pa;
  long nPrepad;
  countType nSamplesInputed;
  countType nSamplesOutputed;
  bool bAnalysis;
  unsigned int version;
  int channels;
  SBSMSQuality *quality;
  audio *ina;
};

#ifdef MULTITHREADED

struct channel_thread_data {
  int c;
  ThreadInterface *threadInterface;
};

struct analyze_thread_data {
  int i;
  ThreadInterface *threadInterface;
};

class ThreadInterface {
public:
  friend class SBSMSImp;
  ThreadInterface(SBSMSImp *sbsms, bool bAnalyze, bool bSynthesize);  
  ~ThreadInterface();

  void signalReadWrite();
  void signalAnalyze();
  void signalExtract(int c);
  void signalMark(int c);
  void signalAssign(int c);
  void signalRender(int c);

  void waitReadWrite();
  void waitAnalyze(int i);
  void waitExtract(int c);
  void waitAssign(int c);
  void waitRender(int c);

  SubBand *top;
  int channels;

  pthread_mutex_t readWriteMutex;
  pthread_cond_t readWriteCond;

  bool bAnalyzeThread;
  pthread_t analyzeThread[3];
  pthread_mutex_t analyzeMutex[3];
  pthread_cond_t analyzeCond[3];

  bool bExtractThread;
  pthread_t extractThread[2];
  pthread_mutex_t extractMutex[2];
  pthread_cond_t extractCond[2];

  bool bAssignThread;
  pthread_t assignThread[2];
  pthread_mutex_t assignMutex[2];
  pthread_cond_t assignCond[2];

  bool bRenderThread;
  pthread_t renderThread[2];
  pthread_mutex_t renderMutex[2];
  pthread_cond_t renderCond[2];

  channel_thread_data channelData[2];
  analyze_thread_data analyzeData[3];
  bool bActive;
};

void *analyzeThreadCB(void *data) {
  analyze_thread_data *analyzeData = (analyze_thread_data*)data;
  ThreadInterface *threadInterface = analyzeData->threadInterface;
  SubBand *top = threadInterface->top;
  int i = analyzeData->i;
  int channels = threadInterface->channels;

  while(threadInterface->bActive) {

    threadInterface->waitAnalyze(i);
    jitter();

    if(top->analyzeInit(i,true)) {

      jitter();
      top->analyze(i);
      jitter();
      top->stepAnalyzeFrame(i);
      jitter();
 
      threadInterface->signalReadWrite();
      for(int c=0; c<channels; c++) {
        threadInterface->signalExtract(c);
      }
      jitter();     
    }
  }
  pthread_exit(NULL);
  return NULL;
}

void *extractThreadCB(void *data) {
  channel_thread_data *channelData = (channel_thread_data*)data;
  ThreadInterface *threadInterface = channelData->threadInterface;
  SubBand *top = threadInterface->top;
  int c = channelData->c;

  while(threadInterface->bActive) {

    jitter();

    threadInterface->waitExtract(c);

    if(top->extractInit(c,true)) {

      jitter();
      top->extract(c);
      jitter();
      top->stepExtractFrame(c);
      jitter();

      threadInterface->signalAnalyze();
      
      threadInterface->signalMark(c);        
    }
  }
  pthread_exit(NULL);
  return NULL;
}

void *assignThreadCB(void *data) {
  channel_thread_data *channelData = (channel_thread_data*)data;
  ThreadInterface *threadInterface = channelData->threadInterface;
  SubBand *top = threadInterface->top;
  int c = channelData->c;

  while(threadInterface->bActive) {

    threadInterface->waitAssign(c);

    if(top->markInit(c,true)) {
      jitter();
      top->mark(c);
      jitter();
      top->stepMarkFrame(c);      
      jitter();

      threadInterface->signalExtract(c);
    }

    jitter();

    if(top->assignInit(c,true)) {
      jitter();
      top->assign(c);
      jitter();
      top->advance(c);
      jitter();
      top->stepAssignFrame(c);
      jitter();

      if(threadInterface->bRenderThread) {
        threadInterface->signalRender(c);
      } else {
        threadInterface->signalReadWrite();
      }
    }
  }
  pthread_exit(NULL);
  return NULL;
}

void *renderThreadCB(void *data) {
  channel_thread_data *channelData = (channel_thread_data*)data;
  ThreadInterface *threadInterface = channelData->threadInterface;
  SubBand *top = threadInterface->top;
  int c = channelData->c;

  while(threadInterface->bActive) {
    jitter();
    threadInterface->waitRender(c);
    jitter();
    if(top->renderInit(c,true)) {
      jitter();
      top->render(c);
      jitter();
      top->stepRenderFrame(c);
      jitter();

      threadInterface->signalAssign(c);
      threadInterface->signalReadWrite();
    }
  }
  pthread_exit(NULL);
  return NULL;
}

ThreadInterface :: ThreadInterface(SBSMSImp *sbsms, bool bAnalyze, bool bSynthesize) 
{
  this->top = sbsms->top;
  this->channels = sbsms->channels;
  bActive = true;
  
  pthread_cond_init(&readWriteCond, NULL);
  pthread_mutex_init(&readWriteMutex, NULL);
  
  if(!bAnalyze) bSynthesize = false;
  if(bAnalyze) {
    bAnalyzeThread = true;
    bExtractThread = true;
    bAssignThread = true;
  } else {
    bAnalyzeThread = false;
    bExtractThread = false;
    bAssignThread = false;
  }  
  if(bSynthesize) {
    bRenderThread = true;
  } else {
    bRenderThread = false;
  }
  
  if(bAnalyzeThread) {
    for(int i=0; i<3; i++) {
      analyzeData[i].i = i;
      analyzeData[i].threadInterface = this;
      pthread_cond_init(&analyzeCond[i], NULL);
      pthread_mutex_init(&analyzeMutex[i], NULL);
    }
  }
  
  for(int c=0; c<channels; c++) {
    channelData[c].c = c;
    channelData[c].threadInterface = this;
    if(bExtractThread) {
      pthread_cond_init(&extractCond[c], NULL);
      pthread_mutex_init(&extractMutex[c], NULL);
    }
    if(bAssignThread) {
      pthread_cond_init(&assignCond[c], NULL);    
      pthread_mutex_init(&assignMutex[c], NULL);
    }
    if(bRenderThread) {
      pthread_cond_init(&renderCond[c], NULL);
      pthread_mutex_init(&renderMutex[c], NULL);
    }
  }
  
  if(bAnalyzeThread) {
    for(int i=0; i<3; i++) {
      pthread_create(&analyzeThread[i], NULL, analyzeThreadCB, (void*)&analyzeData[i]);
    }
  }
  
  for(int c=0; c<channels; c++) {
    if(bExtractThread) {
      pthread_create(&extractThread[c], NULL, extractThreadCB, (void*)&channelData[c]);
    }
    if(bAssignThread) {
      pthread_create(&assignThread[c], NULL, assignThreadCB, (void*)&channelData[c]);
    }
    if(bRenderThread) {
      pthread_create(&renderThread[c], NULL, renderThreadCB, (void*)&channelData[c]);
    }
  }
}

ThreadInterface :: ~ThreadInterface() 
{
  bActive = false;
  if(bAnalyzeThread) {
    for(int i=0; i<3; i++) {
      pthread_mutex_lock(&analyzeMutex[i]);
      pthread_cond_broadcast(&analyzeCond[i]);
      pthread_mutex_unlock(&analyzeMutex[i]);
      pthread_join(analyzeThread[i],NULL);
    }
  }
  for(int c=0; c<channels; c++) {
    if(bExtractThread) {
      pthread_mutex_lock(&extractMutex[c]);
      pthread_cond_broadcast(&extractCond[c]);
      pthread_mutex_unlock(&extractMutex[c]);
      pthread_join(extractThread[c],NULL);
    }
    if(bAssignThread) {
      pthread_mutex_lock(&assignMutex[c]);
      pthread_cond_broadcast(&assignCond[c]);
      pthread_mutex_unlock(&assignMutex[c]);
      pthread_join(assignThread[c],NULL);
    }
    if(bRenderThread) {
      pthread_mutex_lock(&renderMutex[c]);
      pthread_cond_broadcast(&renderCond[c]);
      pthread_mutex_unlock(&renderMutex[c]);
      pthread_join(renderThread[c],NULL);
    }
  }
}

void ThreadInterface :: signalReadWrite() 
{
  pthread_mutex_lock(&readWriteMutex);
  jitter();
  bool bReady;
  if(bRenderThread) {
    bReady = (top->writeInit() || top->readInit());
  } else {
    if(top->writeInit()) {
      bReady = true;  
    } else {
      bReady = true;
      for(int c=0; c<channels; c++) {
        if(!top->renderInit(c,false)) {
          bReady = false;
          break;
        }
      }
    }
  }
  if(bReady) {
    jitter();
    pthread_cond_broadcast(&readWriteCond);
    jitter();
  }
  pthread_mutex_unlock(&readWriteMutex);
}

void ThreadInterface :: signalAnalyze() 
{
  for(int i=0; i<3; i++) {
    pthread_mutex_lock(&analyzeMutex[i]);
    jitter();
    if(top->analyzeInit(i,false)) {
      jitter();
      pthread_cond_broadcast(&analyzeCond[i]);
      jitter();
    }
    pthread_mutex_unlock(&analyzeMutex[i]);
    jitter();
  }
}

void ThreadInterface :: signalExtract(int c) {
  pthread_mutex_lock(&extractMutex[c]);
  jitter();
  if(top->extractInit(c,false)) {
    jitter();
    pthread_cond_broadcast(&extractCond[c]);
    jitter();
  }
  pthread_mutex_unlock(&extractMutex[c]);
}

void ThreadInterface :: signalMark(int c) {
  pthread_mutex_lock(&assignMutex[c]);
  jitter();
  if(top->markInit(c,false)) {
    jitter();
    pthread_cond_broadcast(&assignCond[c]);
    jitter();
  }
  pthread_mutex_unlock(&assignMutex[c]);
}

void ThreadInterface :: signalAssign(int c) {
  pthread_mutex_lock(&assignMutex[c]);
  jitter();
  if(top->assignInit(c,false)) {
    jitter();
    pthread_cond_broadcast(&assignCond[c]);
    jitter();
  }
  pthread_mutex_unlock(&assignMutex[c]);
}

void ThreadInterface :: signalRender(int c) {
  pthread_mutex_lock(&renderMutex[c]);
  jitter();
  if(top->renderInit(c,false)) {
    jitter();
    pthread_cond_broadcast(&renderCond[c]);
  }
  jitter();
  pthread_mutex_unlock(&renderMutex[c]);
  jitter();
}

void ThreadInterface :: waitReadWrite() {
  pthread_mutex_lock(&readWriteMutex);
  bool bReady;
  if(bRenderThread) {
    bReady = (top->writeInit() || top->readInit());
  } else {
    if(top->writeInit()) {
      bReady = true;  
    } else {
      bReady = true;
      for(int c=0; c<channels; c++) {
        if(!top->renderInit(c,false)) {
          bReady = false;
          break;
        }
      }
    }
  }
  if(!bReady) {
    jitter();
    pthread_cond_wait(&readWriteCond,&readWriteMutex);
  }
  pthread_mutex_unlock(&readWriteMutex);
  jitter();
}

void ThreadInterface :: waitAnalyze(int i) {
  pthread_mutex_lock(&analyzeMutex[i]);
  if(!top->analyzeInit(i,false)) {
    pthread_cond_wait(&analyzeCond[i],&analyzeMutex[i]);
  }
  pthread_mutex_unlock(&analyzeMutex[i]);
}

void ThreadInterface :: waitExtract(int c) {
  pthread_mutex_lock(&extractMutex[c]);
  if(!top->extractInit(c,false)) {
    pthread_cond_wait(&extractCond[c],&extractMutex[c]);
  }
  pthread_mutex_unlock(&extractMutex[c]);
}

void ThreadInterface :: waitAssign(int c) {
  pthread_mutex_lock(&assignMutex[c]);
  if(!top->markInit(c,false) && !top->assignInit(c,false)) {
    pthread_cond_wait(&assignCond[c],&assignMutex[c]);
  }
  pthread_mutex_unlock(&assignMutex[c]);
}

void ThreadInterface :: waitRender(int c) {
  pthread_mutex_lock(&renderMutex[c]);
  jitter();
  if(!top->renderInit(c,false)) {
    jitter();
    pthread_cond_wait(&renderCond[c],&renderMutex[c]);
  }
  jitter();
  pthread_mutex_unlock(&renderMutex[c]);
}

#endif

void SBSMS :: init(int size)
{
  cosInit(size);
}

void SBSMS :: init(int size, int n, float *tab)
{
  cosInit(size,n,tab);
}

void SBSMS :: finalize()
{
  cosDestroy();
}

void SBSMS :: reset(SBSMSInterface *iface, bool flushInput) { imp->reset(iface,flushInput); }
void SBSMSImp :: reset(SBSMSInterface *iface, bool flushInput)
{
  ta->init();
  nSamplesInputed = 0;
  nSamplesOutputed = 0;
  if(bAnalysis) {
    long prepad = quality->getMaxPresamples();
    nPrepad = max(0L,prepad - (iface?iface->getPresamples():0L));
  } else {
    nPrepad = 0;
  }
  top->reset(flushInput);
}

SBSMS :: SBSMS(int channels, SBSMSQuality *quality, bool bPreAnalyze, bool bSynthesize)
{ imp = new SBSMSImp(channels,quality,bPreAnalyze,bSynthesize); }
SBSMSImp :: SBSMSImp(int channels, SBSMSQuality *quality, bool bPreAnalyze, bool bSynthesize)
{
  this->source = NULL;
  fpIn = NULL;
  fpOut = NULL;
  version = SBSMS_VERSION;
  this->channels = channels;
  this->quality = new SBSMSQuality(&quality->params);
  bAnalysis = true;
  error = SBSMSErrorNone;
  ta = new TrackAllocator(AnalysisTrackType);
  pa = new PeakAllocator*[channels];
  for(int c=0; c<channels; c++) {
    pa[c] = new PeakAllocator();
  }
  renderer = NULL;
  long prepad = quality->getMaxPresamples();  
  frameByteOffset = NULL;
  frameSampleOffset = NULL;
  top = new SubBand(NULL,1,channels,quality,prepad,bPreAnalyze,bSynthesize,ta,pa);
  ina = make_audio_buf(quality->getFrameSize());  
  reset(NULL);
#ifdef MULTITHREADED
  threadInterface = new ThreadInterface(this,true,bSynthesize);
#endif
}

SBSMS :: SBSMS(const char *filename, countType *samplesToInput, bool bSynthesize, bool bKeepInMemory)
{ imp = new SBSMSImp(filename, samplesToInput, bSynthesize, bKeepInMemory); }
SBSMSImp :: SBSMSImp(const char *filename, countType *samplesToInput, bool bSynthesize, bool bKeepInMemory)
{
  this->source = NULL;
  FOPEN(fpIn,filename,"rb");
  if(!fpIn) {
    error = SBSMSErrorFileOpen;
    return;
  }
  fpOut = NULL;
  this->bKeepInMemory = bKeepInMemory;
  version = fread_32_little_endian(fpIn);
  if(samplesToInput) {
    *samplesToInput = fread_64_little_endian(fpIn);
  } else {
    fread_64_little_endian(fpIn);
  }
  long framesToInput = fread_32_little_endian(fpIn);
  channels = fread_16_little_endian(fpIn);
  SBSMSQualityParams qualityParams;
  int *p = (int*)&qualityParams;
  for(int i=0; i<SBSMS_QUALITY_N_PARAMS; i++) {
    p[i] = fread_16_little_endian(fpIn);
  }
  quality = new SBSMSQuality(&qualityParams);
  int maxTracks = fread_16_little_endian(fpIn);
  bAnalysis = false;
  error = SBSMSErrorNone;
  ta = new TrackAllocator(bKeepInMemory?MemoryTrackType:FileTrackType,maxTracks);
  pa = NULL;
  renderer = NULL;
  top = new SubBand(NULL,1,channels,quality,0,false,bSynthesize,ta,pa);
  top->setFramesInFile(framesToInput);  
  this->frameByteOffset = new vector<off_t>;
  this->frameSampleOffset = new vector<countType>;
  readFooter(framesToInput);
  FSEEK(fpIn,SBSMS_OFFSET_DATA,SEEK_SET);
  ina = make_audio_buf(quality->getFrameSize());
  reset(NULL);
#ifdef MULTITHREADED
  threadInterface = new ThreadInterface(this,false,false);
#endif
}

SBSMS :: SBSMS(SBSMS *source) { imp = new SBSMSImp(source->imp); }
SBSMSImp :: SBSMSImp(SBSMSImp *source)
{
  printf("new sbsms\n");
  this->bKeepInMemory = true;
  this->source = source;
  frameByteOffset = source->frameByteOffset;
  frameSampleOffset = source->frameSampleOffset;
  fpIn = NULL;
  fpOut = NULL;
  version = source->version;
  this->channels = source->channels;
  this->quality = new SBSMSQuality(&source->quality->params);
  bAnalysis = false;
  error = SBSMSErrorNone;
  ta = new TrackAllocator(MemoryTrackType,source->ta->size());
  pa = NULL;
  renderer = NULL;
  top = new SubBand(NULL,1,channels,quality,0,false,true,ta,pa,source->top);  
  top->setFramesInFile(source->top->nFramesInFile);
  ina = make_audio_buf(quality->getFrameSize());
  reset(NULL);
#ifdef MULTITHREADED
  threadInterface = new ThreadInterface(this,false,false);
#endif
}

SBSMS :: ~SBSMS() { delete imp; }
SBSMSImp :: ~SBSMSImp()
{
#ifdef MULTITHREADED
  if(threadInterface) delete threadInterface;
#endif
  if(top) delete top;
  if(ta) delete ta;  
  if(ina) free_audio_buf(ina);
  if(pa) {
    for(int c=0; c<channels; c++) {
      delete pa[c];
    }
    delete [] pa;
  }
  if(!source && frameByteOffset) delete frameByteOffset;
  if(!source && frameSampleOffset) delete frameSampleOffset;
  if(fpIn) fclose(fpIn);
  if(fpOut) fclose(fpOut);
  if(renderer) delete renderer;
  if(quality) delete quality;
}

void SBSMS :: addRenderer(SBSMSRenderer *renderer) { imp->addRenderer(renderer->imp); }
void SBSMSImp :: addRenderer(SMSRenderer *renderer)
{
  top->addRenderer(renderer);
}

void SBSMS :: removeRenderer(SBSMSRenderer *renderer) { imp->removeRenderer(renderer->imp); }
void SBSMSImp :: removeRenderer(SMSRenderer *renderer)
{
  top->removeRenderer(renderer);
}

SBSMSError SBSMS :: getError()
{
  return imp->error;
}

float SBSMSImp :: getInputTime(SBSMSInterface *iface)
{
  countType samples = nSamplesInputed;
  long presamples = iface->getPresamples();
  if(presamples >= samples) {
    return 0.0f;
  } else {
    return (float)((double)(samples - presamples) / iface->getSamplesToInput());
  }
}

long SBSMS :: preAnalyze(SBSMSInterface *iface) { return imp->preAnalyze(iface); }
long SBSMSImp :: preAnalyze(SBSMSInterface *iface)
{
  long nToWrite = 0;

  float t = getInputTime(iface);
  float stretch = iface->getStretch(t);

  if(nPrepad) {
    stretch = 1.0f;
    nToWrite = min(quality->getFrameSize(),nPrepad);
    memset(ina,0,nToWrite*sizeof(audio));
    nPrepad -= nToWrite;
  } else {
    nToWrite = iface->samples(ina,quality->getFrameSize());
    nSamplesInputed += nToWrite;
    if(nToWrite == 0) {
      nToWrite = quality->getFrameSize();
      memset(ina,0,nToWrite*sizeof(audio));
    }
  }
  long nAnalyzed = top->preAnalyze(ina, nToWrite, stretch);
  nSamplesOutputed += nAnalyzed;
  if(nSamplesOutputed >= iface->getSamplesToInput()) {
    top->preAnalyzeComplete();
  }
  return nAnalyzed;
}

long SBSMSImp :: write(SBSMSInterface *iface)
{
  long nWrite = 0;

  float t = getInputTime(iface);
  float stretch = iface->getStretch(t);
  float pitch = iface->getPitch(t);
  assert(stretch > 0.0f);

  if(bAnalysis) {
    if(nPrepad) {
      stretch = 1.0f;
      nWrite = min(quality->getFrameSize(),nPrepad);
      memset(ina,0,nWrite*sizeof(audio));
      nPrepad -= nWrite;
    } else {
      nWrite = iface->samples(ina,quality->getFrameSize());
      nSamplesInputed += nWrite;
      if(nWrite == 0) {
        nWrite = quality->getFrameSize();
        memset(ina,0,nWrite*sizeof(audio));
      }
    }
    nWrite = top->write(ina, nWrite, stretch, pitch);
  } else {
    if(fpIn) {
      nWrite = top->writeFromFile(fpIn, stretch, pitch);
    } else if(bKeepInMemory) {
      nWrite = top->writeFromMemory(stretch,pitch);
    } else {
      nWrite = 0;
    }
    nSamplesInputed += nWrite;
    if(nWrite == 0) {
      top->writingComplete();
    }
  }
  return nWrite;
}

long SBSMS :: read(SBSMSInterface *iface, audio *buf, long n) { return imp->read(iface,buf,n); }
long SBSMSImp :: read(SBSMSInterface *iface, audio *buf, long n)
{
  long nReadTotal = 0;
  while(nReadTotal < n) {
    long nRead;
    if(top->nToDrop) {
      audio nullBuf[512];
      nRead = min(512L,top->nToDrop);
      nRead = top->read(nullBuf,nRead);
      top->nToDrop -= nRead;
    } else {
      nRead = n - nReadTotal;
      nRead = top->read(buf+nReadTotal,nRead);
      nReadTotal += nRead;
    }
    if(nRead) {
#ifdef MULTITHREADED
      if(threadInterface->bRenderThread) {
        for(int c=0; c<channels; c++) {
          threadInterface->signalRender(c);
        }
      }
#endif
    } else {
#ifdef MULTITHREADED
      jitter();
      threadInterface->waitReadWrite();
#endif
      if(top->writeInit()) {
        write(iface);
#ifdef MULTITHREADED
        if(threadInterface->bAnalyzeThread) {
          threadInterface->signalAnalyze();
        }
#endif
      }
    }
#ifdef MULTITHREADED
    if(!(threadInterface->bAnalyzeThread &&
         threadInterface->bExtractThread && 
         threadInterface->bAssignThread)) {
      top->process(!threadInterface->bRenderThread);
    }
    jitter();
    if(!threadInterface->bRenderThread && threadInterface->bAssignThread) {
      for(int c=0; c<channels; c++) {     
        threadInterface->signalAssign(c);
      }
    }
#else
    top->process(true);
#endif
    nSamplesOutputed += nRead;
  }
  return nReadTotal;
}

void SBSMS :: particles(SBSMSInterface *iface, SBSMSynthesizer *synth) { imp->particles(iface,synth); }
void SBSMSImp :: particles(SBSMSInterface *iface, SBSMSynthesizer *synth) {
  float t = getInputTime(iface);
  float pitch = iface->getPitch(t);
  top->particles(pitch,synth);
}

long SBSMS :: synthFromMemory(SBSMSInterface *iface, audio *buf, long n, SBSMSynthesizer *synthMod) { return imp->synthFromMemory(iface,buf,n,synthMod); }
long SBSMSImp :: synthFromMemory(SBSMSInterface *iface, audio *buf, long n, SBSMSynthesizer *synthMod)
{
  long nReadTotal = 0;
  float t = getInputTime(iface);
  float stretch = iface->getStretch(t);
  float pitch = iface->getPitch(t);

  long nRead = -1;
  while(nRead && nReadTotal < n) {
    long nToDrop = top->getDrop(stretch);
    if(nToDrop) {
      audio nullBuf[512];
      nRead = min(512L,top->nToDrop);
      char str[1024];
      
      nRead = top->synthFromMemory(nullBuf,nRead,stretch,pitch,synthMod);
      printf("nToDrop = %ld, nRead = %ld, nReadTotal = %ld\n",top->nToDrop,nRead, nReadTotal);

      top->nToDrop -= nRead;
    } else {
      nRead = n - nReadTotal;
      nRead = top->synthFromMemory(buf+nReadTotal,nRead,stretch,pitch,synthMod);
      nReadTotal += nRead;
    }
  }

  nSamplesInputed += nReadTotal;

  //printf("nReadTotal = %ld\n",nReadTotal);
  return stretch<0.0f?-nReadTotal:nReadTotal;
}

long SBSMS :: synthFromFile(SBSMSInterface *iface, audio *buf, long n) { return imp->synthFromFile(iface,buf,n); }
long SBSMSImp :: synthFromFile(SBSMSInterface *iface, audio *buf, long n)
{
  long nReadTotal = 0;
  float t = getInputTime(iface);
  float stretch = iface->getStretch(t);
  float pitch = iface->getPitch(t);

  long nRead = -1;
  while(nRead && nReadTotal < n) {
    if(top->nToDrop) {
      audio nullBuf[512];
      nRead = min(512L,top->nToDrop);
      nRead = top->synthFromFile(nullBuf,nRead,stretch,pitch,fpIn);
      top->nToDrop -= nRead;
    } else {
      nRead = n - nReadTotal;
      nRead = top->synthFromFile(buf+nReadTotal,nRead,stretch,pitch,fpIn);
      nReadTotal += nRead;
    }
  }
  return nReadTotal;
}

long SBSMS :: renderFrame(SBSMSInterface *iface) { return imp->renderFrame(iface); }
long SBSMSImp :: renderFrame(SBSMSInterface *iface)
{
  long nRendered = 0;
  while(!nRendered && !top->isDone()) {
    bool bReady = true;
    for(int c=0; c<channels; c++) {
      if(!top->renderInit(c,false)) {
        bReady = false;
        break;
      }
    }
    if(bReady) {
      nRendered = top->renderSynchronous();
    }
    if(nRendered) {
#ifdef MULTITHREADED
      if(threadInterface->bAssignThread) {
        for(int c=0; c<channels; c++) {
          threadInterface->signalAssign(c);
        }
      }
#endif
    } else {
#ifdef MULTITHREADED
      jitter();
      threadInterface->waitReadWrite();  
#endif
      
      if(top->writeInit()) {
        jitter();
        write(iface);
        jitter();
      }
      
#ifdef MULTITHREADED
      if(threadInterface->bAnalyzeThread) {
        threadInterface->signalAnalyze();
      }
#endif
    }
#ifdef MULTITHREADED
    if(!(threadInterface->bAnalyzeThread &&
         threadInterface->bExtractThread && 
         threadInterface->bAssignThread)) {
      top->process(false);
    }
#else
    top->process(false);
#endif
    nSamplesOutputed += nRendered;
  }
  return nRendered;
}

bool SBSMS :: isReadOpen() { return imp->isReadOpen(); }
bool SBSMSImp :: isReadOpen()
{
  return (fpIn != NULL);
}

bool SBSMS :: isWriteOpen() { return imp->isWriteOpen(); }
bool SBSMSImp :: isWriteOpen()
{
  return (fpOut != NULL);
}

bool SBSMS :: openWrite(const char *fileName) { return imp->openWrite(fileName); }
bool SBSMSImp :: openWrite(const char *fileName)
{
  FOPEN(fpOut,fileName,"wb");
  if(!fpOut) {
    error = SBSMSErrorFileOpen;
    return false; 
  }
  fwrite_32_little_endian(version,fpOut);
  fwrite_64_little_endian(0LL,fpOut);
  fwrite_32_little_endian(0L,fpOut);
  fwrite_16_little_endian(channels,fpOut);
  int *p = (int*)&(quality->params);
  for(int i=0; i<SBSMS_QUALITY_N_PARAMS; i++) {
    fwrite_16_little_endian(p[i],fpOut);
  } 
  fwrite_16_little_endian(0,fpOut);
  if(renderer) {
    removeRenderer(renderer);
    delete renderer;
  }
  renderer = new SMSFileRenderer(channels,fpOut,ta);
  addRenderer(renderer);
  return true;
}

void SBSMS :: closeRead() { imp->closeRead(); }
void SBSMSImp :: closeRead()
{
  if(fpIn) {
    fclose(fpIn);
    fpIn = NULL;
  }
}

void SBSMS :: closeWrite(SBSMSInterface *iface) { imp->closeWrite(iface); }
void SBSMSImp :: closeWrite(SBSMSInterface *iface)
{
  if(fpOut) {
    FSEEK(fpOut,SBSMS_OFFSET_SAMPLES,SEEK_SET);
    fwrite_64_little_endian(iface->getSamplesToOutput(),fpOut);
    renderer->end();
    removeRenderer(renderer);
    delete renderer;
    renderer = NULL;
    fclose(fpOut);
    fpOut = NULL;
  }
} 

void SBSMSImp :: readFooter(long frames)
{
  off_t footerSize = frames*8;
  FSEEK(fpIn,-footerSize,SEEK_END);
  off_t samplesAcc = 0;
  int frameSize = top->getInputFrameSize();
  for(int k=0;k<frames;k++) {
    off_t offset = (off_t)fread_64_little_endian(fpIn);
    frameByteOffset->push_back(offset);
    frameSampleOffset->push_back(samplesAcc);
    samplesAcc += frameSize;
  }
}

void SBSMS :: prepadInputs(int samples) { imp->top->prepadInputs(samples); }

void SBSMS :: writeInputs(float **inputs, int samples, int offset) { imp->top->writeInputs(inputs, samples, offset); }

long SBSMS :: readInputs(ArrayRingBuffer<float> *buf[2], int samples, bool bAdvance) { return imp->top->readInputs(buf, samples, bAdvance); }

void SBSMS :: setLeftPos(countType pos) { imp->top->setLeftPos(pos); }
void SBSMS :: setRightPos(countType pos) { imp->top->setRightPos(pos); }

void SBSMS :: seek(SBSMSInterface *iface, countType samplePos) { imp->seek(iface,samplePos); }
void SBSMSImp :: seek(SBSMSInterface *iface, countType samplePos)
{
  long i = (long) (samplePos / top->getInputFrameSize());
  if(fpIn) FSEEK(fpIn,frameByteOffset->at(i),SEEK_SET);  
  nSamplesInputed = frameSampleOffset->at(i);
  float t = getInputTime(iface);
  float stretch = iface->getStretch(t);
  nSamplesOutputed = (countType)((double)nSamplesInputed * (double)stretch);
  top->seek(i,samplePos);
}

unsigned int SBSMS :: getVersion()
{
  return imp->version;
}

int SBSMS :: getChannels()
{
  return imp->channels;
}

SBSMSQuality *SBSMS :: getQuality()
{
  return imp->quality;
}

long SBSMS :: getInputFrameSize()
{
  return imp->top->getInputFrameSize();
}

countType SBSMS :: getSamplePosFromMemory() { return imp->getSamplePosFromMemory(); }
countType SBSMSImp :: getSamplePosFromMemory() 
{
  return top->getSamplePosFromMemory();
}

class SBSMSInterfaceVariableRateImp {
public:
  SBSMSInterfaceVariableRateImp(countType samplesToInput);
  inline countType getSamplesToInput();
  inline countType getSamplesToOutput();
  friend class SBSMSInterfaceVariableRate;
protected:
  double stretch;
  float pitch;
  countType samplesToInput;
};

SBSMSInterfaceVariableRate :: SBSMSInterfaceVariableRate(countType samplesToInput)
{
  imp = new SBSMSInterfaceVariableRateImp(samplesToInput);
}

SBSMSInterfaceVariableRate :: ~SBSMSInterfaceVariableRate()
{
  delete imp;
}

SBSMSInterfaceVariableRateImp :: SBSMSInterfaceVariableRateImp(countType samplesToInput)
{
  this->samplesToInput = samplesToInput;
  this->stretch = 1.0;
  this->pitch = 1.0;
}

float SBSMSInterfaceVariableRate :: getStretch(float t)
{
  return (float)imp->stretch;
}

float SBSMSInterfaceVariableRate :: getPitch(float t)
{
  return imp->pitch;
}

void SBSMSInterfaceVariableRate :: setRate(float rate)
{
  imp->stretch = (rate == 0.0f?0.0f:1.0 / (double)rate);
}

void SBSMSInterfaceVariableRate :: setPitch(float pitch)
{
  imp->pitch = pitch;
}

long SBSMSInterfaceVariableRate :: getPresamples() 
{
  return 0L;
}

countType SBSMSInterfaceVariableRate :: getSamplesToInput() { return imp->getSamplesToInput(); }
countType SBSMSInterfaceVariableRateImp :: getSamplesToInput()
{
  return samplesToInput;
}

countType SBSMSInterfaceVariableRate :: getSamplesToOutput() { return imp->getSamplesToOutput(); }
countType SBSMSInterfaceVariableRateImp :: getSamplesToOutput()
{
  return (countType)((double)samplesToInput * stretch);
}

}
