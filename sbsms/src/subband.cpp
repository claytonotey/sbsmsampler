#include "subband.h"
#include "real.h"
#include "sbsms.h"
#include "utils.h"
#include <algorithm>
using namespace std;

namespace _sbsms_ {

#define SUB_BUF_SIZE 512

SubBand :: SubBand(SubBand *parent, int M, int channels, SBSMSQuality *quality, long prepad, 
                   bool bPreAnalyze, bool bSynthesize,
                   TrackAllocator *ta, PeakAllocator **pa, SubBand *source)
{
  this->source = source;
  int k = ilog2(M);
  if(k<quality->params.bands-1) 
    sub = new SubBand(this,M*2,channels,quality,prepad,bPreAnalyze,bSynthesize,ta,pa,source?source->sub:NULL);
  else 
    sub = NULL;
  this->quality = quality;
  this->channels = channels;
  this->parent = parent;
  this->M_MAX = 1<<(quality->params.bands-1);
  this->N = quality->params.N[k];
  this->N0 = quality->params.N0[k];  
  this->N1 = quality->params.N1[k];  
  this->N2 = quality->params.N2[k];
  this->M = M;
  this->s = 4;
  this->res = quality->params.res[k];
  this->prepad = prepad;

  nGrainsPerFrame = res;
  if(sub) nGrainsPerFrame *= sub->nGrainsPerFrame;
  
  if(sub) resTotal = 2*sub->resTotal;
  else resTotal = 1;

#ifdef MULTITHREADED
  writeSlack = 6;
  analyzeSlack = 6;
  extractSlack = 6;
  markSlack = 6;
  assignSlack = 6;
  renderSlack = 6;
#else
  writeSlack = 2;
  analyzeSlack = 2;
  extractSlack = 2;
  markSlack = 2;
  assignSlack = 2;
  renderSlack = 2;
#endif

  init();

  // sms 
  grains[0] = new GrainBuf(N, h, N0, hannpoisson);
  grains[1] = new GrainBuf(N, h, N1, hannpoisson);
  grains[2] = new GrainBuf(N, h, N2, hannpoisson);

  for(int c=0; c<channels; c++) {
    analyzedGrains[0][c] = new GrainBuf(N, h, N0, hannpoisson);
    analyzedGrains[1][c] = new GrainBuf(N, h, N1, hannpoisson);
    analyzedGrains[2][c] = new GrainBuf(N, h, N2, hannpoisson);
  }

  if(!parent && bPreAnalyze) {
    this->bPreAnalyze = true;
    grainsPre = new GrainBuf(N, h, N/2, hann);
    x1[0] = make_audio_buf(N);
    x1[1] = make_audio_buf(N);
    x2[0] = make_audio_buf(N);
    x2[1] = make_audio_buf(N);
  } else {
    this->bPreAnalyze = false;
    grainsPre = NULL;
  }

  if(sub) {
    // output 
    samplesSubOut = new SampleBuf(0);

    // receive
    grainsIn = new GrainBuf(N, N/s, N, hann);

    // samples to sub
    samplesSubIn = new SampleBuf(N/2);
    hsub = N/2/s;

    downSampledGrainAllocator = new GrainAllocator(N/2,N/2,hann);
  }

#ifdef MULTITHREADED
  pthread_mutex_init(&dataMutex, NULL);
  for(int i=0; i<3; i++) {
    pthread_mutex_init(&grainMutex[i], NULL);
  }
#endif

  if(ta->getType() == FileTrackType) {
    nRenderLatencyOriginal = 1;
    SMS0 *sms0 = new SMS0(N,M,M_MAX,h,res,N0,N1,N2,channels,ta->ta0,pa);
    sms = sms0;
    smsFile = sms0;
    smsMemory = NULL;
    smsAnalyze = NULL;
  } else if(ta->getType() == MemoryTrackType) {
    nRenderLatencyOriginal = 1;
    SMS1 *sms1 = new SMS1(N,M,M_MAX,h,res,N0,N1,N2,channels,ta->ta1,pa,(SMS1*)(source?source->sms:NULL));
    sms = sms1;
    smsFile = sms1;
    smsMemory = sms1;
    smsAnalyze = NULL;
  } else if(ta->getType() == AnalysisTrackType) {
    nRenderLatencyOriginal = 4;
    SMS2 *sms2 = new SMS2(N,M,M_MAX,h,res,N0,N1,N2,channels,ta->ta2,pa);
    sms = sms2;
    smsFile = NULL;
    smsMemory = NULL;
    smsAnalyze = sms2;
  }

  if(source) {
    stretchMod = source->stretchMod;
  } else {
    stretchMod = new RingBuffer<float>;
  }

  if(bSynthesize) {
    synthRenderer = new SMSSynthRenderer(channels,M);
    renderers.push_back(synthRenderer);
    if(sub) {
      outMixer = new Mixer(synthRenderer,samplesSubOut);
    } else {
      outMixer = synthRenderer;
    }
  } else {
    synthRenderer = NULL;
    outMixer = NULL;
  }
}

void SubBand :: addRenderer(SMSRenderer *renderer)
{
  if(sub) sub->addRenderer(renderer);
  renderers.push_back(renderer);
}

void SubBand :: removeRenderer(SMSRenderer *renderer)
{
  if(sub) sub->removeRenderer(renderer);
  renderers.remove(renderer);
}

void SubBand :: reset(bool flushInput) {
  //printf("reset\n");
  if(sub) sub->reset(flushInput);
  init();
  outputFrameSize.clear();
  for(int c=0; c<channels; c++) {
    bSynthStarted[c] = false;
    synthFramePos[c] = 0;
    bSynthGrainStart[c] = true;
    nGrainsSynthed[c] = 0;
    stretchRender[c].clear();
    pitchRender[c].clear();
  }
  for(int i=0; i<3; i++) {
    grains[i]->clear();
    for(int c=0; c<channels; c++) {
      analyzedGrains[i][c]->clear();
    }
  }
  if(sub) {
    grainsIn->clear();
    samplesSubIn->clear();
    samplesSubOut->clear();
  }
  sms->reset();
  if(synthRenderer) synthRenderer->reset(flushInput);
}

void SubBand :: init()
{
  int ssms = (N*nGrainsPerFrame)/(resTotal*quality->params.H);
  h = N/ssms;
  inputFrameSize = h * nGrainsPerFrame;
  nToWriteForGrain = h;
  nReadFromOutputFrame = 0;
  lastStretchMod = 1.0f;
  nRenderLatency = nRenderLatencyOriginal;
  nGrainsToDrop = (prepad * nGrainsPerFrame) / (M_MAX * quality->params.H) - ssms/2;
  for(int i=0; i<3; i++) {
    nFramesAnalyzed[i] = 0;
  }
  for(int c=0; c<channels; c++) {
    nFramesExtracted[c] = 0;
    nFramesMarked[c] = 0;
    nFramesAssigned[c] = 0;
    nFramesRendered[c] = 0;
    nGrainsMarked[c] = 0;
    nGrainsAssigned[c] = 0;
    nGrainsStarted[c] = 0;
    nGrainsAdvanced[c] = 0;
  }
  samplePos = 0;
  nToDrop = 0;
  nFramesRead = 0;
  inputSamplesQueued = 0;
  outputSamplesQueued = 0;
  gPrev = NULL;
  totalSizef = 0.0;
  bWritingComplete = false;
}

SubBand :: ~SubBand() 
{
  for(int i=0; i<3; i++) {
    delete grains[i];
    for(int c=0; c<channels; c++) {
      delete analyzedGrains[i][c];
    }
  }
  delete sms;
  if(grainsPre) {
    delete grainsPre;
    free_audio_buf(x1[0]);
    free_audio_buf(x1[1]);
    free_audio_buf(x2[0]);
    free_audio_buf(x2[1]);
  }
  if(!source) {
    delete stretchMod;
  }
  if(sub) {
    delete sub;
    delete grainsIn;
    delete samplesSubIn;
    delete samplesSubOut;
    delete outMixer;
    delete downSampledGrainAllocator;
  }
  if(synthRenderer) delete synthRenderer;
}

void SubBand :: setStretch(float stretch)
{
  if(!parent) {
    float oFrameSizef = (stretch==0.0f?1.0f:stretch)*(float)inputFrameSize;
    totalSizef += oFrameSizef;
    long oFrameSizei = lrintf(totalSizef);
    totalSizef -= oFrameSizei;
    outputFrameSize.write(oFrameSizei);
    inputSamplesQueued -= inputFrameSize;
    outputSamplesQueued += (oFrameSizei<0?-oFrameSizei:oFrameSizei);
  }
  for(int c=0; c<channels; c++) {
    stretchRender[c].write(stretch);
  }
  if(sub) sub->setStretch(stretch);
}

void SubBand :: setStretchMod(float stretchMod)
{
#ifdef MULTITHREADED
    pthread_mutex_lock(&dataMutex);
#endif    
    this->stretchMod->write(stretchMod);
#ifdef MULTITHREADED
    pthread_mutex_unlock(&dataMutex);
#endif    
}

void SubBand :: setPitch(float f)
{
#ifdef MULTITHREADED
    pthread_mutex_lock(&dataMutex);
#endif    
  if(sub) sub->setPitch(f);
  for(int c=0; c<channels; c++) {
    pitchRender[c].write(f);
  }
#ifdef MULTITHREADED
    pthread_mutex_unlock(&dataMutex);
#endif    
}

void SubBand :: stepAnalyzeFrame(int i)
{
  if(sub) sub->stepAnalyzeFrame(i);
  nFramesAnalyzed[i]++;
}

void SubBand :: stepExtractFrame(int c)
{
  if(sub) sub->stepExtractFrame(c);
  nFramesExtracted[c]++;
}

void SubBand :: stepMarkFrame(int c)
{
  if(sub) sub->stepMarkFrame(c);
  nFramesMarked[c]++;
}

void SubBand :: stepAssignFrame(int c)
{
  if(sub) sub->stepAssignFrame(c);
  nFramesAssigned[c]++;
}

void SubBand :: stepRenderFrame(int c)
{
  if(sub) sub->stepRenderFrame(c);
#ifdef MULTITHREADED
  pthread_mutex_lock(&dataMutex);
#endif
  stretchRender[c].advance(1);
  pitchRender[c].advance(1);
#ifdef MULTITHREADED
  pthread_mutex_unlock(&dataMutex);
#endif
  nFramesRendered[c]++;
}

void SubBand :: stepReadFrame()
{
  if(sub) sub->stepReadFrame();
  nFramesRead++;
}

bool SubBand :: writeInit()
{
  long n = getFramesAtFront(0);
  n = min(n,getFramesAtFront(1));
  n = min(n,getFramesAtFront(2));  
  return (n <= writeSlack);
}

long SubBand :: readInit()
{
  long n = nFramesRendered[0];
  for(int c=1; c<channels; c++) {
    n = max(0L,min(1L,min(n,nFramesRendered[c] - nFramesRead)));
  }
  if(sub) n = min(n,sub->readInit());
  return n;
}

long SubBand :: analyzeInit(int i, bool bSet, long n)
{
  if(!parent) {
    n = getFramesAtFront(i);
    for(int c=0; c<channels; c++) {
      n = max(0L,min(1L,min(n,
                            analyzeSlack-(long)(nFramesAnalyzed[i]-nFramesExtracted[c]))));
    }
  }
  if(bSet) {
    nGrainsToAnalyze[i] = n * nGrainsPerFrame;
    if(sub) {
      sub->analyzeInit(i,bSet,n);
    }
  }
  return n;
}

long SubBand :: extractInit(int c, bool bSet)
{
  long n;
  if(sub) n = res*sub->extractInit(c,bSet);
  if(!sub) {
    n = max(0L,min(1L,extractSlack-(long)(nFramesExtracted[c]-nFramesMarked[c])));
    for(int i=0; i<3; i++) {
      n = max(0L,min(1L,min(n,(long)(nFramesAnalyzed[i]-nFramesExtracted[c]))));
    }
  }
  if(bSet) {
    nGrainsToExtract[c] = n;
  }
  return n;
}

long SubBand :: markInit(int c, bool bSet)
{
  long n;
  if(sub) n = res*sub->markInit(c,bSet);
  if(!sub) n = max(0L,min(1L,min((long)(nFramesExtracted[c]-nFramesMarked[c])-1,
                                 markSlack-(long)(nFramesMarked[c]-nFramesAssigned[c]))));
  if(bSet) {
    nGrainsToMark[c] = n;
  }
  return n;
}

long SubBand :: assignInit(int c, bool bSet)
{
  long n;
  if(sub) n = res*sub->assignInit(c,bSet);
  if(!sub) n = max(0L,min(1L,min((long)(nFramesMarked[c]-nFramesAssigned[c])-1,
                                 assignSlack+nRenderLatency-(long)(nFramesAssigned[c]-nFramesRendered[c]))));
  if(bSet) {
    nGrainsToAdvance[c] = n;
    nGrainsToAssign[c] = n;
    if(n) {
      if(nFramesAssigned[c]==0) {
        smsAnalyze->assignTrackPoints(nGrainsAssigned[c]++,
                                      parent?parent->smsAnalyze:NULL,
                                      sub?sub->smsAnalyze:NULL,c);
        smsAnalyze->startNewTracks(nGrainsStarted[c]++,c);
      }      
    }
  }
  return n;
}

long SubBand :: renderInit(int c, bool bSet) 
{
  long n;
  if(sub) n = res*sub->renderInit(c,bSet);
  if(!sub) n = max(0L,min(1L,min((long)(nFramesAssigned[c]-nFramesRendered[c])-nRenderLatency,
                                 renderSlack-(long)(nFramesRendered[c]-nFramesRead))));
  if(bSet) {
    nGrainsRendered[c] = 0;
    nGrainsToRender[c] = n;
  }
  return n;
}

long SubBand :: writeFromFileInit(int c, bool bSet)
{
  long n = 1;
  if(sub) n = res*sub->writeFromFileInit(c,bSet);
  if(!sub) {
    n = min(n,max(0L,min(1L,(long)(nFramesInFile-nFramesAssigned[c]))));
  }
  if(bSet) {
    nGrainsAssigned[c] = 0;
    nGrainsToAssign[c] = n;
  }
  return n;
}

void SubBand :: analyze(int i)
{
  if(sub) sub->analyze(i);
  vector<grain*> gV;

#ifdef MULTITHREADED
  pthread_mutex_lock(&grainMutex[i]);
#endif
  for(int k=grains[i]->readPos;k<grains[i]->readPos+nGrainsToAnalyze[i];k++) {
    grain *g = grains[i]->read(k);
    gV.push_back(g);
  }
#ifdef MULTITHREADED
  pthread_mutex_unlock(&grainMutex[i]);
#endif

  for(int k=0;k<nGrainsToAnalyze[i];k++) {
    gV[k]->analyze();
  }

#ifdef MULTITHREADED
  pthread_mutex_lock(&grainMutex[i]);
#endif
  for(int k=0;k<nGrainsToAnalyze[i];k++) {
    for(int c=0; c<channels; c++) {
      analyzedGrains[i][c]->write(gV[k]);
    }
  }
  grains[i]->advance(nGrainsToAnalyze[i]);
#ifdef MULTITHREADED
  pthread_mutex_unlock(&grainMutex[i]);
#endif
}

void SubBand :: extract(int c)
{
  if(sub) sub->extract(c);
  vector<grain*> gV[3];

  for(int i=0; i<3; i++) {
#ifdef MULTITHREADED
    pthread_mutex_lock(&grainMutex[i]);
#endif    
    for(int k=analyzedGrains[i][c]->readPos;k<analyzedGrains[i][c]->readPos+nGrainsToExtract[c];k++) {
      grain *g = analyzedGrains[i][c]->read(k);
      gV[i].push_back(g);
    }
#ifdef MULTITHREADED
    pthread_mutex_unlock(&grainMutex[i]);
#endif
  }

  for(int k=0;k<nGrainsToExtract[c];k++) {
    grain *g0 = gV[0][k];
    grain *g1 = gV[1][k];
    grain *g2 = gV[2][k];
    smsAnalyze->addTrackPoints(g0,g1,g2,c);
  }

  for(int i=0; i<3; i++) {
#ifdef MULTITHREADED
    pthread_mutex_lock(&grainMutex[i]);
#endif
    analyzedGrains[i][c]->advance(nGrainsToExtract[c]);
#ifdef MULTITHREADED
    pthread_mutex_unlock(&grainMutex[i]);
#endif
  }
}

void SubBand :: mark(int c)
{
  long ntodo = parent?1:nGrainsToMark[c];
  long ndone = 0;
  while(ndone<ntodo) {  
    smsAnalyze->markDuplicates(nGrainsMarked[c],
                          parent?parent->smsAnalyze:NULL,
                          sub?sub->smsAnalyze:NULL,c);
    if(nGrainsMarked[c]%res==1 || res==1) {
      if(sub) sub->mark(c);
    }
    ndone++;
    nGrainsMarked[c]++;
  }
}

void SubBand :: assign(int c) 
{
  long ntodo = parent?1:nGrainsToAssign[c];
  long ndone = 0;
  while(ndone<ntodo) {  
    if(nGrainsAssigned[c]%res==0) {
      if(sub) sub->assign(c);
      smsAnalyze->assignTrackPoints(nGrainsAssigned[c]++,
                               parent?parent->smsAnalyze:NULL,
                               sub?sub->smsAnalyze:NULL,c);
      if(!parent) start(c);
    } else {
      smsAnalyze->assignTrackPoints(nGrainsAssigned[c]++,
                               parent?parent->smsAnalyze:NULL,
                               sub?sub->smsAnalyze:NULL,c);
    }
    if(parent && parent->res != 1) {
      parent->smsAnalyze->startNewTracks(parent->nGrainsStarted[c]++,c);
    }
    ndone++;
  }
}

void SubBand :: start(int c)
{
  if(!parent||!sub||nGrainsAssigned[c]%2==1||res==1) {
    smsAnalyze->startNewTracks(nGrainsStarted[c]++,c);
  }
  if(sub&&(nGrainsAssigned[c]%2==1||res==1)) {
    sub->start(c);
  }
}

void SubBand :: advance(int c)
{
  long ntodo = parent?1:nGrainsToAdvance[c];
  long ndone = 0;
  while(ndone<ntodo) {
    if(nGrainsAdvanced[c]%res==0)
      if(sub) sub->advance(c);
    smsAnalyze->advanceTrackPoints(c);
    nGrainsMarked[c]--;
    nGrainsAssigned[c]--;
    nGrainsStarted[c]--;
    nGrainsAdvanced[c]++;
    ndone++;
  }
}

void SubBand :: readSubSamples()
{
  if(sub) sub->readSubSamples();
  if(sub) {
    audio fromSub[SUB_BUF_SIZE];
    long nFromSub = 0;
    do {
      nFromSub = sub->outMixer->read(fromSub,SUB_BUF_SIZE);
      samplesSubOut->write(fromSub, nFromSub);
    } while(nFromSub>0);
  }
}

long SubBand :: read(audio *buf, long n) 
{
  if(bWritingComplete && !inputSamplesQueued && !outputSamplesQueued) {
    memset(buf,0,n*sizeof(audio));
    return n;
  }

  long nRead = 0;
  long nToRead = n;
  readSubSamples();
  while(nToRead && nRead < n && outputFrameSize.nReadable()) {
    long nToReadFromOutputFrame = outputFrameSize.read();
    nToRead = min(n-nRead,nToReadFromOutputFrame-nReadFromOutputFrame);
    nToRead = outMixer->read(buf+nRead, nToRead);
    outputSamplesQueued -= (float)nToRead;
    nReadFromOutputFrame += nToRead;
    nRead += nToRead;
    if(nReadFromOutputFrame == nToReadFromOutputFrame) {
      nReadFromOutputFrame = 0;
      outputFrameSize.advance(1);
      stepReadFrame();
    }
  }
  return nRead;
}

long SubBand :: renderSynchronous() 
{
  float stretchMod;
  if(this->stretchMod->nReadable() > nFramesRead) {
    stretchMod = this->stretchMod->read(this->stretchMod->readPos+nFramesRead);
  } else {
    stretchMod = 1.0f;
  }
  for(list<SMSRenderer*>::iterator i = renderers.begin(); i != renderers.end(); ++i) {
    SMSRenderer *renderer = *i;
    renderer->startFrame(stretchMod);
  }
  for(int c=0; c<channels; c++) {    
    renderInit(c,true);
    render(c);
    stepRenderFrame(c);
  }
  for(list<SMSRenderer*>::iterator i = renderers.begin(); i != renderers.end(); ++i) {
    SMSRenderer *renderer = *i;
    renderer->endFrame();
  }
  long samples = outputFrameSize.read(outputFrameSize.readPos+nFramesRead);
  outputSamplesQueued -= samples;
  stepReadFrame();
  return samples;
}

void SubBand :: prepadInputs(int samples)
{
  if(synthRenderer) synthRenderer->prepadInputs(samples);
  if(sub) sub->prepadInputs(samples);
}

void SubBand :: writeInputs(float **inputs, int samples, int offset)
{
  if(synthRenderer) synthRenderer->writeInputs(inputs, samples, offset);
  if(sub) sub->writeInputs(inputs, samples, offset);
}

long SubBand :: readInputs(ArrayRingBuffer<float> *buf[2], int samples, bool bAdvance)
{
  if(synthRenderer) synthRenderer->readInputs(buf, samples, bAdvance);
}

void SubBand :: setLeftPos(countType pos)
{
  sms->setLeftPos(pos);
}

void SubBand :: setRightPos(countType pos)
{
  sms->setRightPos(pos);
}

void SubBand :: particlePopulate(int c, float pitch, SBSMSynthesizer *synth)
{
  if(sub) sub->particlePopulate(c,pitch,synth);
  sms->particlePopulate(c,pitch,synth);
}

/*
void SubBand :: particleFormants(int c, float pitch, SBSMSynthesizer *synth)
{
  
}
*/

void SubBand :: particles(float pitch, SBSMSynthesizer *synth) 
{
  bool bOutputFrameStart = (nReadFromOutputFrame == 0L);
  if(synth && synth->isPopulateRequired()) {
    for(int c=0; c<channels; c++) {
      synth->particleInit(c);
      particlePopulate(c,pitch,synth);
    }
  }
}

long SubBand :: synthFromMemory(audio *buf, long n, float stretch, float pitch, SBSMSynthesizer *synth) 
{
  if(!n) return 0;
  long nSynth;
  for(int c=0; c<channels; c++) {
    float stretch2 = stretch;
    //n = min(n,inputFrameSize-nReadFromOutputFrame);
    nSynth = synthFromMemoryInit(c,n,&stretch2);
    this->synth(c,nSynth,stretch2,pitch,synth);
    //printf("%ld %ld %ld\n",n,nSynth,nReadFromOutputFrame);
  }
  nReadFromOutputFrame += nSynth;
  while(nReadFromOutputFrame >= inputFrameSize) {
    nReadFromOutputFrame -= inputFrameSize;
  }
  long nRead = 0;
  long nToRead = -1;
  readSubSamples();
  while(nToRead && nRead < n) {
    nToRead = n - nRead;
    nToRead = outMixer->read(buf+nRead, nToRead);
    nRead += nToRead;
  }
  return nRead;
}

long SubBand :: getDrop(float stretch)
{
  if(!bSynthStarted[0]) {
    bool bBackwards = (stretch<0.0f);
    long pos = (samplePos%inputFrameSize);
    if(bBackwards) {
      nToDrop = inputFrameSize - pos;
    } else {
      nToDrop = pos;
    }
  }
  return nToDrop;
}

long SubBand :: synthFromMemoryInit(int c, long n, float *stretch)
{
  if(!parent && 
     (sms->isPastLeft(c) && *stretch < 0.0f ||
      sms->isPastRight(c) && *stretch >= 0.0f)) return 0;

  if(!bSynthStarted[c]) {
    bBackwards[c] = (*stretch<0.0f);
    if(!parent) {
      int pos = samplePos%inputFrameSize;
      nToDrop = 0;
      if(bBackwards[c]) {
        if(pos) {
          synthFramePos[c]++;
          sms->seek(synthFramePos[c]*nGrainsPerFrame);
          nToDrop = inputFrameSize - pos;
        }
      } else {
        if(pos) {
          nToDrop = pos;
        }
      }
    }
    if(nToDrop) {
      /*
      float drop0[512];
      float drop1[512];
      float *drop[2];
      drop[0] = drop0;
      drop[1] = drop1;
      memset(drop0,0,inputFrameSize*sizeof(float));
      memset(drop1,0,inputFrameSize*sizeof(float));
      writeInputs((float**)drop,nToDrop,0);*/
      prepadInputs(nToDrop);
      //printf("drop - %ld\n",nToDrop);
      //assert(false);
    }
    smsMemory->assignTrackPointsFromMemory(c,bBackwards[c],0);
    bSynthStarted[c] = true;
  }
  int ism = synthFramePos[0];
  if(bSynthGrainStart[c]) {
    bool bBackwardsNext = (*stretch<0.0f);
    if(bBackwards[c] != bBackwardsNext) {
      sms->pruneTracks(c);
    }
    bBackwards[c] = bBackwardsNext;
    bSynthGrainStart[c] = false;
    if(bBackwards[c] && nGrainsSynthed[c]%nGrainsPerFrame == 0) {
      ism = ism - 1;
    }
    if(!smsMemory->assignTrackPointsFromMemory(c,bBackwards[c],bBackwards[c]?-1:1)) {
      n = 0;
    }
  }
  if(!parent && nToDrop) {
    n = min(n,nToDrop);
    if(bBackwards[c]) {
      *stretch = -1.0f;
    } else {
      *stretch = 1.0f;
    }
  } else if(!parent && this->stretchMod->nReadable() > ism && ism >= 0) {
    *stretch *= this->stretchMod->read(this->stretchMod->readPos + ism);
  }
  if(!parent) {
    long n0 = sms->synthInit(c,n,bBackwards[c],*stretch);
    n = min(n,n0);
  }
  if(n == 0) bSynthGrainStart[c] = true;
  if(sub) sub->synthFromMemoryInit(c,n,stretch);
  return n;
}

long SubBand :: synthFromFile(audio *buf, long n, float stretch, float pitch, FILE *fp) 
{
  if(!n) return 0;
  long nSynth = synthFromFileInit(n,&stretch,pitch,fp);
  for(int c=0; c<channels; c++) {
    synth(c,nSynth,stretch,pitch,NULL);
  }
  long nRead = 0;
  long nToRead = -1;
  readSubSamples();
  while(nToRead && nRead < n) {
    nToRead = n - nRead;
    nToRead = outMixer->read(buf+nRead, nToRead);
    nRead += nToRead;
  }
  return nRead;
}

long SubBand :: synthFromFileInit(long n, float *stretch, float pitch, FILE *fp)
{
  if(sms->isPastRight(0)) return 0;

  if(!bSynthStarted[0]) {
    for(int c=0; c<channels; c++) {
      bBackwards[c] = false;
    }
    nToDrop = samplePos%inputFrameSize;
    if(!writeFromFile(fp,*stretch,pitch)) return 0;
  }

  if(bSynthGrainStart[0] && nGrainsSynthed[0]%nGrainsPerFrame == 0) {
    bSynthGrainStart[0] = false;
    if(bSynthStarted[0]) {
      for(int c=0; c<channels; c++) {
        stepRenderFrame(c);
      }
      stepReadFrame();
    }
    if(!writeFromFile(fp,*stretch,pitch)) return 0;
  }

  bSynthStarted[0] = true;

  int ism = synthFramePos[0];
  if(nToDrop) {
    n = min(n,nToDrop);
    *stretch = 1.0f;
  } else if(this->stretchMod->nReadable() > ism) {
    *stretch *= this->stretchMod->read(this->stretchMod->readPos + ism);
  }

  long n0;
  for(int c=0; c<channels; c++) {  
    n0 = sms->synthInit(c,n,false,*stretch);
  }
  n = min(n,n0);
  return n;
}

long SubBand :: synth(int c, long n, float stretch, float pitch, SBSMSynthesizer *synthMod) {
  if(sub) sub->synth(c,n,stretch,pitch,synthMod);
  int grainCompleted = sms->synthTracks(c,n,synthRenderer,bBackwards[c],stretch,pitch,synthMod);
  if(!parent && grainCompleted) {
    stepSynthGrain(c,bBackwards[c]?-grainCompleted:grainCompleted);
  }
  return n;
}

void SubBand :: stepSynthGrain(int c, int n)
{
  int n0;
  if(bBackwards[c]) {
    if(n == -1) {
      nGrainsSynthed[c]--;
      n0 = -1;
    } else {
      n0 = 0;
    }
  } else {
    if(n == 1) {
      nGrainsSynthed[c]++;
      n0 = 1;
    } else {
      n0 = 0;
    } 
  }
  if(sub && nGrainsSynthed[c]%res == 0) sub->stepSynthGrain(c,n);
  sms->stepSynth(c,n0);
  synthFramePos[c] = nGrainsSynthed[c] / nGrainsPerFrame;
  bSynthGrainStart[c] = true;
}

void SubBand :: render(int c)
{
#ifdef MULTITHREADED
  pthread_mutex_lock(&dataMutex);
#endif
  float stretch = stretchRender[c].read();
  float f0 = pitchRender[c].read(pitchRender[c].readPos);
  float f1;
  if(pitchRender[c].nReadable()>=2) {
    f1 = pitchRender[c].read(pitchRender[c].readPos+1);
  } else {
    f1 = f0;
  }
#ifdef MULTITHREADED
  pthread_mutex_unlock(&dataMutex);
#endif
  long ntodo = parent?1:nGrainsToRender[c];
  long ndone = 0;
  long nRenderedTotal = 0;

  while(ndone<ntodo) {
    if(nGrainsRendered[c]%res==0)
      if(sub) sub->render(c);
    float df = (f1-f0)/(float)nGrainsToRender[c];
    sms->renderTracks(c,renderers,stretch,f0+nGrainsRendered[c]*df,f0+(nGrainsRendered[c]+1)*df);
    int pos = samplePos%inputFrameSize;
    if(pos && !bSynthStarted[c]) {
      nToDrop = lrintf(pos * stretch);
    }
    bSynthStarted[c] = true;
    nGrainsRendered[c]++;
    ndone++;
  }
}

long SubBand :: write(audio *inBuf, long n, float stretch, float pitch)
{
  long nWritten = 0;

  while(nWritten<n) {
    long nToWrite = min(nToWriteForGrain,n-nWritten);
    long ng;
    for(int i=0; i<3; i++) {
#ifdef MULTITHREADED
      pthread_mutex_lock(&grainMutex[i]);
#endif      
      ng = grains[i]->write(inBuf+nWritten, nToWrite);
#ifdef MULTITHREADED
      pthread_mutex_unlock(&grainMutex[i]);
#endif
    }
    inputSamplesQueued += nToWrite;
    nWritten += nToWrite;
    nToWriteForGrain -= nToWrite;
    if(nToWriteForGrain == 0) {
      nToWriteForGrain = h;
    }
    long nAdvance = 0;
    for(int k=0;k<ng;k++) {
      if(nGrainsToDrop) {
        nGrainsToDrop--;
        nAdvance++;
      } else {
        if(!parent && nGrainsWritten%nGrainsPerFrame == 0) {
          if(bPreAnalyze && stretchPreAnalysis.nReadable()) {
            lastStretchMod = stretchPreAnalysis.read();
            stretchPreAnalysis.advance(1);
          } else {
            lastStretchMod = 1.0f;
          }
          setStretch(lastStretchMod*stretch);
          setStretchMod(lastStretchMod);
          setPitch(pitch);
        }
        nGrainsWritten++;
      }
    }
    if(nAdvance) {
      for(int i=0; i<3; i++) {
#ifdef MULTITHREADED
        pthread_mutex_lock(&grainMutex[i]);
#endif
        grains[i]->advance(nAdvance);
#ifdef MULTITHREADED
        pthread_mutex_unlock(&grainMutex[i]);
#endif
      }
    }
  }
  
  if(sub) {
    grainsIn->write(inBuf, n);
    long nGrainsRead = 0;
    for(int k=grainsIn->readPos;k<grainsIn->writePos;k++) {
      grain *g = grainsIn->read(k); g->analyze();
      grain *gdown = downSampledGrainAllocator->create();
      g->downsample(gdown);
      samplesSubIn->write(gdown, hsub);
      downSampledGrainAllocator->forget(gdown);
      nGrainsRead++;
    }
    grainsIn->advance(nGrainsRead);  
    long nWriteToSub = samplesSubIn->nReadable();
    audio *subBuf = samplesSubIn->getReadBuf();
    nWriteToSub = sub->write(subBuf,nWriteToSub,stretch,pitch);
    samplesSubIn->advance(nWriteToSub);
  }
  return n;
}

void SubBand :: process(bool bRender)
{
  for(int i=0; i<3; i++) {
    if(analyzeInit(i,true)) {
      analyze(i);
      stepAnalyzeFrame(i);
    }
  }

  for(int c=0; c<channels; c++) {
    if(extractInit(c,true)) {
      extract(c);
      stepExtractFrame(c);
    }

    if(markInit(c,true)) {
      mark(c);
      stepMarkFrame(c);
    }
    
    if(assignInit(c,true)) {
      assign(c);
      advance(c);
      stepAssignFrame(c);
    }

    if(bRender) {
      if(renderInit(c,true)) {
        render(c);
        stepRenderFrame(c);
      }
    }
  }
}

long SubBand :: getFramesInFile() 
{
  return nFramesInFile;
}

void SubBand :: setFramesInFile(long frames) 
{
  if(sub) sub->setFramesInFile(frames);
  nFramesInFile = frames;
}

long SubBand :: writeFromMemory(float stretch, float pitch)
{
  bool bReady = true;
  for(int c=0; c<channels; c++) {
    if(!writeFromFileInit(c,false)) {
      bReady = false;
      break;
    }
  }

  if(bReady) {
    inputSamplesQueued += inputFrameSize;
    setPitch(pitch);
    if(stretchMod->nReadable() > nFramesAssigned[0]) {
      setStretch(stretchMod->read(stretchMod->readPos + nFramesAssigned[0]) * stretch);
    } else {
      setStretch(stretch);
    }
    for(int c=0; c<channels; c++) { 
      writeFromFileInit(c,true);
      writeFromMemory(c);
      stepAssignFrame(c);
    }
    return inputFrameSize;
  } else {
    return 0;
  }
}

long SubBand :: writeFromMemory(int c)
{
  long ntodo = parent?1:nGrainsToAssign[c];
  long ndone = 0;

  while(ndone<ntodo) {
    if(nGrainsAssigned[c]%res==0)
      if(sub) sub->writeFromMemory(c);
    smsMemory->assignTrackPointsFromMemory(c);
    nGrainsAssigned[c]++;
    ndone++;
  }
  return ndone;
}

countType SubBand :: getSamplePosFromMemory()
{
  return sms->getSamplePos();
}

long SubBand :: writeFromFile(FILE *fp, float stretch, float pitch)
{
  bool bReady = true;
  for(int c=0; c<channels; c++) {
    if(!writeFromFileInit(c,false)) {
      bReady = false;
      break;
    }
  }
  if(bReady) {
    int sm = fread_16_little_endian(fp);
    float stretchMod = sm==0?1.0f:decodeLinear(sm,MINSTRETCHMOD,MAXSTRETCHMOD);
    for(int c=0; c<channels; c++) {
      writeFromFileInit(c,true);
      writeFromFile(fp,c);
      stepAssignFrame(c);
    }
    inputSamplesQueued += inputFrameSize;
    setStretch(stretch*stretchMod);
    setStretchMod(stretchMod);
    setPitch(pitch);
    return inputFrameSize;
  } else {
    return 0;
  }
}

long SubBand :: writeFromFile(FILE *fp, int c)
{
  long ntodo = parent?1:nGrainsToAssign[c];
  long ndone = 0;

  while(ndone<ntodo) {
    if(nGrainsAssigned[c]%res==0)
      if(sub) sub->writeFromFile(fp,c);
    smsFile->assignTrackPointsFromFile(fp,c);
    nGrainsAssigned[c]++;
    ndone++;
  }
  return ndone;
}

long SubBand :: getFramePos()
{
  return nFramesRead;  
}

void SubBand :: seek(long framePos, countType samplePos) {
  if(sub) sub->seek(framePos,samplePos);
  this->samplePos = samplePos;
  sms->seek(framePos*nGrainsPerFrame);
  for(int c=0; c<channels; c++) {
    nFramesAssigned[c] = framePos;
    nFramesRendered[c] = framePos;
    synthFramePos[c] = framePos;
    nGrainsSynthed[c] = framePos*nGrainsPerFrame;
  }
  nFramesRead = framePos;
}

long SubBand :: getOutputSamplesQueued()
{
  return (long)lrintf(outputSamplesQueued);
}

long SubBand :: getInputSamplesQueued()
{
  return inputSamplesQueued;
}

long SubBand :: getFramesAtFront(int i)
{
  long n;
#ifdef MULTITHREADED
  pthread_mutex_lock(&grainMutex[i]);
#endif
  n = grains[i]->nReadable() / nGrainsPerFrame;
  if(sub) n = min(n,sub->getFramesAtFront(i));
#ifdef MULTITHREADED
  pthread_mutex_unlock(&grainMutex[i]);
#endif
  return n;
}

long SubBand :: getInputFrameSize()
{
  return inputFrameSize;
}

bool SubBand :: isDone()
{
  bool bDone = true;
  for(int c=0; c<channels; c++) {
    if(!nFramesRead || nFramesRead < nFramesAssigned[c]) {
      bDone = false;
      break;
    }
  }
  return bDone;
}

void SubBand :: writingComplete()
{
  if(sub) sub->writingComplete();
  nRenderLatency = 0;
  bWritingComplete = true;
}

long SubBand :: preAnalyze(audio *buf, long n, float stretch)
{
  long nWritten = 0;
  long samplesProcessed = 0;
  while(nWritten<n) {
    long nToWrite = min((long)(grainsPre->h),n-nWritten);
    grainsPre->write(buf+nWritten,nToWrite);
    nWritten += nToWrite;
    long nGrainsRead = 0;
    for(int k=grainsPre->readPos;k<grainsPre->writePos;k++) {
      if(nGrainsToDrop) {
        nGrainsToDrop--;
      } else {
        if(!parent && (nGrainsWritten)%nGrainsPerFrame == 0) {
          grain *g = grainsPre->read(k); g->analyze();
          grainsPre->reference(g);
          float o = calculateOnset(gPrev,g);
          onset.write(o);
          if(gPrev) grainsPre->forget(gPrev);
          gPrev = g;
          samplesProcessed += inputFrameSize;
        }
        nGrainsWritten++;
      }
      nGrainsRead++;
    }
    grainsPre->advance(nGrainsRead);
  }
  return samplesProcessed;
}

float SubBand :: getOnset(long k)
{
  float tot = 2.0f*onset.read(k);
  float c = 2.0f;
  if(k-1>=onset.readPos) {
    tot += onset.read(k-1);
    c += 1.0f;
  }
  if(k+1<onset.writePos) {
    tot += onset.read(k+1);
    c += 1.0f;
  }
  return tot / c;
}

void SubBand :: preAnalyzeComplete()
{
  if(gPrev) {
    grainsPre->forget(gPrev);
    gPrev = NULL;
  }

  long k0 = onset.readPos;
  long k1 = onset.writePos;
  long kstart = 0;
  long kend = 0;

  for(long k=k0;k<k1;k++) {
    bool bOnset = false;
    if(k==k0) {
      bOnset = true;
    } else {
      float o0 = getOnset(k);
      if(o0 < 0.1f) continue;
      float o1 = getOnset(k-1);
      if(o0 < 1.1f*o1) continue;
      if(o0 < 0.22f) continue;
      bOnset = ((o0 > 0.4f) || (o0>1.4f*o1));
      if(!bOnset && k>k0+1) {
        float o2 = getOnset(k-2);
        bOnset = ((o0 > 1.2f*o1) && (o1 > 1.2f * o2));
        if(!bOnset && k>k0+2) {
          float o3 = getOnset(k-3);
          bOnset = ((o0 > 0.3f) && (o1 > 1.1f * o2) && (o2 > 1.1f * o3));
        }
      }
    }
    if(bOnset) {
      if(kend != 0) {
        calculateStretchMod(kstart,kend);
      }
      kstart = kend;
      kend = k;
    }
  }
  calculateStretchMod(kstart,k1);
  stretchPreAnalysis.write(1.0f);
  stretchPreAnalysis.advance(1);
  onset.clear();
}

void SubBand :: calculateStretchMod(long kstart, long kend)
{
  float oMax = 0.0f;
  float dTotal = 0.0f;

  for(long k=kstart;k<kend;k++) {
    float o = getOnset(k);
    if(o > oMax) oMax = o;
  }

  oMax *= 1.3f;

  for(long k=kstart;k<kend;k++) {
    float o = getOnset(k);
    float d = oMax - o;
    dTotal += d;
  }

  float stretchAllot = (float)(kend-kstart);
  for(long k=kstart;k<kend;k++) {
    float o = getOnset(k);
    float d = oMax - o;
    float stretchMod;
    if(dTotal == 0.0 || k+1==kend) {
      stretchMod = stretchAllot;
    } else {
      stretchMod = d/dTotal*stretchAllot;
    }
	if(stretchMod > MAXSTRETCHMOD) stretchMod = MAXSTRETCHMOD;

    dTotal -= d;
    stretchAllot -= stretchMod;
    stretchPreAnalysis.write(stretchMod);
  }
}

float SubBand :: calculateOnset(grain *g1, grain *g2)
{  
  c2even(g2->x, x2[0], g2->N);
  c2odd(g2->x, x2[1], g2->N);
  float o = 1.0f;
  if(g1!=NULL) {
    int Nover2 = N/2;
    int nOnset = 0;
    int nThresh = 0;
    for(int k=0;k<Nover2;k++) {
      float m2 = norm2(x2[0][k]) + norm2(x2[1][k]);
      float m1 = norm2(x1[0][k]) + norm2(x1[1][k]);
      bool bThresh = (m2 > 1e-6);
      bool bOnset = (m2 > 2.0f*m1) && bThresh;
      if(bOnset) nOnset++;
      if(bThresh) nThresh++;
    }
    if(nThresh == 0)
      o = 0.0f;
    else
      o = (float)nOnset/(float)nThresh;
  } 
  memcpy(x1[0],x2[0],g2->N*sizeof(audio));
  memcpy(x1[1],x2[1],g2->N*sizeof(audio));
  return o;
}

}
