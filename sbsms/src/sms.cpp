#include "vstsbsms.h"
#include "real.h"
#include "utils.h"
#include <math.h>
#include "sms.h"
#include "buffer.h"
#include <limits.h>

void blob() {
  usleep(10);
}

using namespace std;
namespace _sbsms_ {


SMS0 :: SMS0(int N, int M, int M_MAX, int h, int res, int N0, int N1, int N2, int channels, TrackAllocatorImp<TrackPoint> *ta, PeakAllocator **pa)
  : SMSBase<TrackPoint>(N,M,M_MAX,h,res,N0,N1,N2,channels,ta,pa) {}

SMS1 :: SMS1(int N, int M, int M_MAX, int h, int res, int N0, int N1, int N2, int channels, TrackAllocatorImp<TrackPoint1> *ta, PeakAllocator **pa, SMS1 *source)
  : SMSBase<TrackPoint1>(N,M,M_MAX,h,res,N0,N1,N2,channels,ta,pa)
{
  if(source) {
    this->source = source;
    for(int c=0; c<channels; c++) {
      trackPointCache[c] = source->trackPointCache[c];
      trackPointIndexAtTime[c] = source->trackPointIndexAtTime[c];
      nTracksAtTime[c] = source->nTracksAtTime[c];
    }
  } else {
    for(int c=0; c<channels; c++) {
      this->source = NULL;
      trackPointCache[c] = new vector<TrackPoint1*>;
      trackPointIndexAtTime[c] = new vector<long>;
      nTracksAtTime[c] = new vector<int>;
    }
  }
}

SMS1 :: ~SMS1()
{
  if(!source) {
    for(int c=0; c<channels; c++) {
      for(unsigned int i=0; i<trackPointCache[c]->size(); i++) {
        delete trackPointCache[c]->at(i);
      }
      delete trackPointCache[c];
      delete trackPointIndexAtTime[c];
      delete nTracksAtTime[c];
    }
  }
}

SMS2 :: SMS2(int N, int M, int M_MAX, int h, int res, int N0, int N1, int N2, int channels, TrackAllocatorImp<TrackPoint2> *ta, PeakAllocator **pa)
  : SMSBase<TrackPoint2>(N,M,M_MAX,h,res,N0,N1,N2,channels,ta,pa) {}

template<class T>
SMSBase<T> :: SMSBase(int N, int M, int M_MAX, int h, int res, int N0, int N1, int N2, int channels, TrackAllocatorImp<T> *ta, PeakAllocator **pa)
{
  this->N = N;
  this->M = M;
  this->h = h;
  this->h1 = (double)(h * M);
  this->Nover2 = N/2;
  this->pad0 = (float)N1/(float)N0;
  this->pad2 = (float)N1/(float)N2;
  this->res = res;
  this->channels = channels;
  this->ta = ta;
  this->pa = pa;

  float m = (float)M/(float)M_MAX;  
  bool bTerminal = (M == M_MAX);
  float pad = (float)N/(float)N1;

  // Threshold (relative to max magnitude) for finding peaks
  peakThresh = .00001f;
  // Max differency in frequency for continuing a track
  maxDF2 = square(.001f*(float)h1);
  // Max figure of merit for continuing a track (see merit)
  maxMerit2 = maxDF2 + square(.10f);
  // Coefficient for dB magnitude changes in figure of merit for continuing a track
  dMCoeff = 0.01f;
  // Max figure of merit for matching peaks for band stitching
  maxMerit2Match = square(.08f);
  // Max difference in frequency for matching peaks for band stitching
  maxDF2Match = square(.06f);
  // Coefficient for dB magnitude changes in figure of merit for band stitching
  dMCoeffMatch = 0.02f;
    // Min number of trackpoints in a track for synthesis
  minNpts = 3;
  // Max width of a peak for determining spectral energy
  peakWidth0 = 2*(lrintf(ceil(pad*pad0*(float)N/96.0f)+1)/2);
  // Max width of a peak for determining spectral energy
  peakWidth1 = 2*(lrintf(ceil(pad*(float)N/96.0f)+1)/2);
  // Beginning and end of band
  if(bTerminal) kStart = 1; 
  else kStart = N/4-peakWidth0;
  if(M==1) kEnd = N/2;
  else kEnd = N/2-peakWidth0;
  // Max frequency for matching peaks for band stitching
  if(bTerminal) maxFMatch = 0.0f;
  else maxFMatch = 0.5f * PI;
  // Min frequency for matching peaks for band stitching
  if(M==1) minFMatch = PI;
  else minFMatch = (kEnd-peakWidth0)*TWOPI/(float)N;
  // Min ratio of peak "top" to total peak energy
  minPeakTopRatio = 0.05f;
  // Ratio for favoring continuing a track in local band over stitching to another band
  localFavorRatio = 1.1f;
  // Max ratio of (peak energy given to other peaks) to (peak energy)
  maxDecRatio = 1.5f;

  // Normalization from spectrum energy to peak amplitude
  // hann
  // mNorm = 1.0f/6.0f * pad * square(8.0f / (float)(N));
  // hann-poisson
  mNorm =  16.061113032124002f * pad / square((float)N);

  // max magnitude (for determining peaks)
  magmax = 0;

  for(int c=0; c<channels; c++) {
    addtime[c] = 0;
    marktime[c] = 0;
    assigntime[c] = 0;
    synthtime[c] = 0;
    dec[c] = (float*)calloc(N,sizeof(float));
    mag0[c] = (float*)calloc(N,sizeof(float));
    mag1[c] = (float*)calloc(N,sizeof(float));
    mag2[c] = (float*)calloc(N,sizeof(float));
    trackPointListBuffer[c] = new PointerBuffer< list<T*> >();
    samplePos[c] = 0;
    samplePosCum[c] = 0.0;
    h2cum[c] = 0.0;
    synthOffset[c] = 0.0;
    grainPos[c] = 0.0f;
    grainLength[c] = h1;
    x0[c] = make_audio_buf(N);
    x1[c] = make_audio_buf(N);
    x2[c] = make_audio_buf(N);
  }
  peak0 = (float*)calloc(2*N,sizeof(float));
  peak1 = (float*)calloc(2*N,sizeof(float));
  bPeakSet = false;

#ifdef MULTITHREADED
  pthread_mutex_init(&tplbMutex[0],NULL);
  pthread_mutex_init(&tplbMutex[1],NULL);
  pthread_mutex_init(&trackMutex[0],NULL);
  pthread_mutex_init(&trackMutex[1],NULL);
#endif
}

template<class T>
SMSBase<T> :: ~SMSBase()
{
  reset();
  free(peak0);
  free(peak1);
  for(int c=0;c<channels;c++) {
    for(long k=trackPointListBuffer[c]->readPos; k<trackPointListBuffer[c]->writePos; k++) {
      list<T*> *tpl = trackPointListBuffer[c]->read(k);
      for(typename list<T*>::iterator tpi = tpl->begin();
          tpi != tpl->end();
          tpi++) {
        delete (*tpi);
      }
    }    
    free(mag0[c]);
    free(mag1[c]);   
    free(mag2[c]);
    free(dec[c]);
    free_audio_buf(x0[c]);
    free_audio_buf(x1[c]);
    free_audio_buf(x2[c]);
    delete trackPointListBuffer[c];
  }
}

template<class T>
void SMSBase<T> :: reset()
{
  for(int c=0;c<channels;c++) {
    for(typename list<TrackImp<T>*>::iterator tt=trax[c].begin(); 
        tt != trax[c].end(); ) {
      TrackImp<T> *t = (*tt);
      typename list<TrackImp<T>*>::iterator eraseMe = tt;
      tt++;
      trax[c].erase(eraseMe);
      ta->destroy(t);
    }
    samplePos[c] = 0;
    samplePosCum[c] = 0.0;
    h2cum[c] = 0.0;
    synthOffset[c] = 0.0;
    grainPos[c] = 0.0f;
    grainLength[c] = h1;
    addtime[c] = 0;
    assigntime[c] = 0;
    marktime[c] = 0;
    synthtime[c] = 0;
  }
}

template<class T>
void SMSBase<T> :: seek(long time)
{
  for(int c=0;c<channels;c++) {
    samplePos[c] = ((countType)time) * (countType)(h * M);
    addtime[c] = time;
    assigntime[c] = time;
    marktime[c] = time;
    synthtime[c] = time;
  }
}

template<class T>
void SMSBase<T> :: assignTrackPoint(T *tp, char flags, int index, int precursorIndex, int c, bool bBackwards)
{
  TrackImp<T> *t = ta->getTrack(index);
  bool bStart;
  bool tailStart;
  bool bEnd;
  bool tailEnd;
  bool bPrecursor = (precursorIndex >= 0);
  if(bBackwards) {
    bStart = (flags & 4)?true:false;
    tailStart = (flags & 8)?true:false;	
    bEnd = (flags & 1)?true:false;
    tailEnd = (flags & 2)?true:false;
  } else {
    bStart = (flags & 1)?true:false;
    tailStart = (flags & 2)?true:false;	
    bEnd = (flags & 4)?true:false;
    tailEnd = (flags & 8)?true:false;
  }
  if(t && !bStart) {
    t->push_back(tp);
    if(bEnd) {
      t->endTrack(tailEnd);
    }
  } else {
    TrackImp<T> *t1;
    if(bPrecursor) {
      TrackImp<T> *precursor = ta->getTrack(precursorIndex);
      t1 = ta->create(precursor,M,res,h,index);
      if(precursor) {
        precursor->descendant = t1;
      }
    } else {
      t1 = ta->create(NULL,M,res,h,index);
    }
    if((!bStart && !t)) {
      tp->y = 0.0f;
    }
    t1->startTrack(tp,bStart&&tailStart);
    addTrack(c,t1);
  }
}

SMSSynthRenderer :: SMSSynthRenderer(int channels, int M)
{
  this->channels = channels;
  this->M = M;
  for(int c=0; c<channels; c++) {
    ins[c] = new ArrayRingBuffer<float>(0);
    sines[c] = new SampleBuf(0);
    synthBuf[c] = (float*)calloc(8192,sizeof(float));
    inBuf[c] = (float*)calloc(8192,sizeof(float));
    inputPad[c] = 0;
    nPad[c] = 0;
  }
#ifdef MULTITHREADED
  pthread_mutex_init(&bufferMutex,NULL);
#endif
}

SMSSynthRenderer :: ~SMSSynthRenderer()
{
  for(int c=0; c<channels; c++) {
    delete ins[c];
    delete sines[c];
    free(synthBuf[c]);
    free(inBuf[c]);
  }
}

long SMSSynthRenderer :: readInputs(ArrayRingBuffer<float> *buf[2], int samples, bool bAdvance)
{
#ifdef MULTITHREADED
  pthread_mutex_lock(&bufferMutex);
#endif
  long n = samples;
  for(int c=0;c<channels;c++) {
    n = min(n,ins[c]->nReadable());
  }
  for(int c=0;c<channels;c++) {
    if(buf) buf[c]->write(ins[c]->getReadBuf(),n);
    if(bAdvance) ins[c]->advance(n);
  }
#ifdef MULTITHREADED
  pthread_mutex_unlock(&bufferMutex);
#endif
  return n;
}

void SMSSynthRenderer :: prepadInputs(int samples)
{
#ifdef MULTITHREADED
  pthread_mutex_lock(&bufferMutex);
#endif
  for(int c=0;c<channels;c++) {
    inputPad[c] += samples;
  }
#ifdef MULTITHREADED
  pthread_mutex_unlock(&bufferMutex);
#endif
}

void SMSSynthRenderer :: writeInputs(float **inputs, int samples, int offset)
{
#ifdef MULTITHREADED
  pthread_mutex_lock(&bufferMutex);
#endif
  for(int c=0;c<channels;c++) {
    ins[c]->write(inputs[c]+offset,samples);
  }
#ifdef MULTITHREADED
  pthread_mutex_unlock(&bufferMutex);
#endif
}

void SMSSynthRenderer :: reset(bool flushInput)
{
  for(int c=0;c<channels;c++) {
    //printf("reset %ld %d\n",ins[c]->readPos,ins[c]->writePos);
    if(flushInput) ins[c]->clear();
    sines[c]->clear();
    inputPad[c] = 0;
  }
}

void SMSSynthRenderer :: startTime(int c, countType samplePos, countType time, int n, float h2, float pos)
{
  this->time[c] = time;
  this->n[c] = n;
  this->h2[c] = h2;
  this->pos[c] = pos;
  memset(synthBuf[c],0,abs(n)*sizeof(float));
#ifdef MULTITHREADED
  pthread_mutex_lock(&bufferMutex);
#endif
  /*
  if(this->n[c] > ins[c]->nReadable()) {
    char str[1024];
    sprintf(str,"%d %ld\n",this->n[c], ins[c]->nReadable());
    debug(str);
    assert(false);
  }
  */
  int nin = abs(n);
  nPad[c] = min(nin,inputPad[c]);
  memset(inBuf[c],0,nPad[c]*sizeof(float));
  nin -= nPad[c];
  inputPad[c] -= nPad[c];
  memcpy(inBuf[c]+nPad[c],ins[c]->getReadBuf(),nin*sizeof(float));
#ifdef MULTITHREADED
  pthread_mutex_unlock(&bufferMutex);
#endif
}

void SMSSynthRenderer :: renderTrack(int c, Track *t, float f0, float f1, SBSMSynthesizer *synth) {
  if(synth) {
    synth->synth(c,inBuf[c],synthBuf[c],time[c],h2[c],pos[c],n[c],f0,f1,t);
    //synth->particleStep(c,n[c],t);
  } else {
    t->synth(c,synthBuf[c],time[c],h2[c],pos[c],n[c],f0,f1);
  }
}

void SMSSynthRenderer :: endTime(int c)
{
#ifdef MULTITHREADED
  pthread_mutex_lock(&bufferMutex);
#endif
  int n = abs(this->n[c]);
  sines[c]->grow(n);
  long j = sines[c]->writePos;

  for(int k=0; k<n; k++) {
    sines[c]->buf[j++][c] += synthBuf[c][k];
  }
  //printf("endTime %d %ld %d %d\n",n,ins[c]->nReadable(),nPad[c],M);
  sines[c]->writePos += n;
  assert(n-nPad[c] <= ins[c]->nReadable());

  //XXX this assert failed when switching gens alot 7/22/2017
  ins[c]->advance(n-nPad[c]);
#ifdef MULTITHREADED
  pthread_mutex_unlock(&bufferMutex);
#endif
}

long SMSSynthRenderer :: read(audio *out, long n)
{
#ifdef MULTITHREADED
  pthread_mutex_lock(&bufferMutex);
#endif
  n = min(n,sines[0]->nReadable());
  for(int c=1; c<channels; c++) {
    n = min(n,sines[c]->nReadable());
  }
  for(int c=0; c<channels; c++) {
    audio *buf = sines[c]->getReadBuf();
    for(int k=0; k<n; k++) {
        out[k][c] = buf[k][c];
    }
    sines[c]->advance(n);
  }

#ifdef MULTITHREADED
  pthread_mutex_unlock(&bufferMutex);
#endif
  return n;
}

SMSFileRenderer :: SMSFileRenderer(int channels, FILE *fp, TrackAllocator *ta)
{
  this->channels = channels;
  this->fp = fp;
  this->ta = ta;
}

void SMSFileRenderer :: startFrame(float stretchMod)
{
  frameBytes = 0;
  if(stretchMod == 1.0f) {
    fwrite_16_little_endian(0,fp); frameBytes += 2;
  } else {
    fwrite_16_little_endian(encodeLinear(stretchMod,MINSTRETCHMOD,MAXSTRETCHMOD),fp); frameBytes += 2;
  }
}

void SMSFileRenderer :: startTime(int c, countType samplePos, countType time, int n, float h2, float pos)
{
  nTracks[c] = 0;
  this->time = time;
  fgetpos(fp,&start[c]);
  fwrite_16_little_endian(nTracks[c],fp); frameBytes += 2;
}

void SMSFileRenderer :: renderTrack(int c, Track *t, float f0, float f1, SBSMSynthesizer *synth)
{
  if(!(t->isTailStart() && t->isStart(time)) &&
     !(t->isTailEnd() && t->isEnd(time))) {
    TrackPoint *tp = t->getTrackPoint(time);
    if(tp) {      
      char flags = 0;       
      Track *precursor = t->getPrecursor();
      if((t->isTailStart() && t->isStart(time-1)) ||
         (!t->isTailStart() && t->isStart(time)))
        flags += 1;
      if(t->isTailStart())
        flags += 2;
      if((t->isTailEnd() && t->isEnd(time+1)) ||
         (!t->isTailEnd() && t->isEnd(time)))			
        flags += 4;
      if(t->isTailEnd())
        flags += 8;
      if(precursor)
        flags += 16;
      int tpBytes = 9;
      if(precursor) tpBytes += 2;
      fwrite_8_little_endian(flags,fp); 
      fwrite_16_little_endian(encodeLinear(tp->f,0.0f,PI),fp);
      fwrite_16_little_endian(encodeMu(tp->y/(float)t->getM()),fp);
      fwrite_16_little_endian(encodeLinear(tp->ph,-PI,PI),fp);
      fwrite_16_little_endian(t->getIndex(),fp);
      if(precursor) {
        fwrite_16_little_endian(precursor->getIndex(),fp);
      }
      frameBytes += tpBytes;
      nTracks[c]++;
    }
  }
}

void SMSFileRenderer :: endTime(int c)
{
  fpos_t end;
  fgetpos(fp,&end);
  fsetpos(fp,&start[c]);
  fwrite_16_little_endian(nTracks[c],fp);
  fsetpos(fp,&end);
}

void SMSFileRenderer :: endFrame()
{
  frameBytesBuf.write(frameBytes);
}

void SMSFileRenderer :: end()
{
  int maxTracks = ta->size();
  long frames = 0;
  off_t offset = SBSMS_OFFSET_MAXTRACKS;
  FSEEK(fp,offset,SEEK_SET);
  fwrite_16_little_endian(maxTracks,fp);
  FSEEK(fp,0,SEEK_END);
  offset = SBSMS_OFFSET_DATA;

  for(int k=frameBytesBuf.readPos; k<frameBytesBuf.writePos;k++) {
    int bytes = frameBytesBuf.read(k);
    fwrite_64_little_endian(offset,fp);
    offset += bytes;
    frames++;
  }

  offset = SBSMS_OFFSET_FRAMES;
  FSEEK(fp,offset,SEEK_SET);
  fwrite_32_little_endian(frames,fp);
}

template<class T>
void SMSBase<T> :: setLeftPos(countType pos)
{
  leftPos = pos;
}

template<class T>
void SMSBase<T> :: setRightPos(countType pos)
{
  rightPos = pos;
}

template<class T>
bool SMSBase<T> :: isPastLeft(int c)
{
  return samplePos[c] <= leftPos;
}

template<class T>
bool SMSBase<T> :: isPastRight(int c)
{
  return samplePos[c] >= rightPos;
}

template<class T>
long SMSBase<T> :: synthInit(int c, int n, bool bBackwards, float stretch)
{
  if(stretch == 0.0f) return n;
  if(bBackwards) stretch = -stretch;
  bool bBackwardsInGrain = (stretch < 0.0f);
  if(bBackwards != bBackwardsInGrain) {
    n = max(0,min(n,(int)lrint(((samplePos[c] - leftPos) * fabsf(stretch)))));
  } else {
    n = max(0,min(n,(int)lrint(((rightPos - samplePos[c]) * fabsf(stretch)))));
  }
  if(bBackwardsInGrain) {
    n = min(n, max(0,(int)lrint(ceil(synthOffset[c] * fabsf(stretch)))));
  } else {
    n = min(n, max(0,(int)lrint(ceil((h1 - synthOffset[c]) * fabsf(stretch)))));
  }
  //if(c==0) printf("init %d %g %d %d %d %g %g %lld\n",M,stretch,bBackwards,bBackwardsInGrain,n,synthOffset[c],grainPos[c],samplePos[c]);
  return n;
}


template<class T>
void SMSBase<T> :: particlePopulate(int c, float pitch, SBSMSynthesizer *synth)
{
#ifdef MULTITHREADED
  pthread_mutex_lock(&trackMutex[c]);
#endif
  for(typename list<TrackImp<T>*>::iterator tt=trax[c].begin(); 
      tt != trax[c].end();) {
    TrackImp<T> *t = (*tt);      
    if(synthtime[c] >= t->getStart()) {
      synth->particlePopulate(c,pitch,grainPos[c],synthtime[c],t);
      tt++;
    } else {
      break;
    }
  }
#ifdef MULTITHREADED
  pthread_mutex_unlock(&trackMutex[c]);
#endif
}

template<class T>
int SMSBase<T> :: synthTracks(int c, int n, SMSRenderer *renderer, bool bBackwards, float stretch, float pitch, SBSMSynthesizer *synth)
{
#ifdef MULTITHREADED
  pthread_mutex_lock(&trackMutex[c]);
#endif
  bool bBackwardsInGrain = (bBackwards != (stretch < 0.0f));
  int nInGrain = bBackwardsInGrain?-n:n;
  float stretchAbs = fabsf(stretch);
  float h2 = (stretch == 0.0f?0.0f:(float)(stretch * h1));
  renderer->startTime(c,samplePos[c],synthtime[c],nInGrain,h2,grainPos[c]);
  for(typename list<TrackImp<T>*>::iterator tt=trax[c].begin(); 
      tt != trax[c].end();) {
    TrackImp<T> *t = (*tt);      
    if(synthtime[c] >= t->getStart()) {
      renderer->renderTrack(c,t,pitch,pitch,synth);
      if(synthtime[c] == t->getStart()) t->popPrecursor();
      tt++;
    } else {
      break;
    }
  }
#ifdef MULTITHREADED
  pthread_mutex_unlock(&trackMutex[c]);
#endif
  renderer->endTime(c);

  if(stretch != 0.0f) {
    synthOffset[c] += (double)nInGrain / stretchAbs;
    grainPos[c] += (float)nInGrain / (float)(stretchAbs * grainLength[c]);
    if(M == 1) {
      double n1 = (double)nInGrain / stretch;
      samplePosCum[c] += n1;
      int n1i = lrint(samplePosCum[c]);
      samplePosCum[c] -= (double)n1i;
      samplePos[c] += n1i;
      if(samplePos[c] < leftPos) samplePos[c] = leftPos;
      else if(samplePos[c] > rightPos) samplePos[c] = rightPos;
    }
  }

  if(n && synthOffset[c] <= 0.0) {
    //if(c == 0) printf("synth -1 %d %d %g %d %d %g %g %d\n",M,bBackwards,stretch,n,nInGrain,synthOffset[c],grainPos[c],bBackwardsInGrain);
    return -1;
  } else if(n && synthOffset[c] >= h1) {
    //if(c == 0) printf("synth 1 %d %d %g %d %d %g %g %d\n",M,bBackwards,stretch,n,nInGrain,synthOffset[c],grainPos[c],bBackwardsInGrain);
    return 1;
  } else {
    //if(c == 0) printf("synth 0 %d %d %g %d %d %g %g %d\n",M,bBackwards,stretch,n,nInGrain,synthOffset[c],grainPos[c],bBackwardsInGrain);
    return 0;
  }
}

template<class T>
void SMSBase<T> :: stepSynth(int c, int n)
{
  addtime[c] += n;
  if(n == 0) {
    grainPos[c] = 0.0f;
    grainLength[c] = h1 + synthOffset[c];
    pruneTracks(c);
    assigntime[c]--;
  } else {
    synthOffset[c] -= h1;
    grainPos[c] = 0.0f;
    grainLength[c] = h1 - synthOffset[c];
    synthtime[c]++;
    for(typename list<TrackImp<T>*>::iterator tt=trax[c].begin(); 
        tt != trax[c].end();) {
      TrackImp<T> *t = (*tt);      
      if(t->isEnded() && synthtime[c] >= t->getEnd() && !t->getDescendant()) {
        typename list<TrackImp<T>*>::iterator eraseMe = tt;
        tt++;
        trax[c].erase(eraseMe);
        ta->destroy(t);
      } else {
        tt++;
      }
    }
  }
  //if(c==0) printf("step %d %d %g %g %lld %lld %lld %lld\n",M,n,synthOffset[c],samplePosCum[c],samplePos[c],addtime[c],assigntime[c],synthtime[c]);
}

template<class T>
void SMSBase<T> :: pruneTracks(int c)
{
#ifdef MULTITHREADED
  pthread_mutex_lock(&trackMutex[c]);
#endif
  for(typename list<TrackImp<T>*>::iterator tt=trax[c].begin(); 
      tt != trax[c].end();) {
    TrackImp<T> *t = (*tt);      
    if(synthtime[c] <= t->getStart()) {
      typename list<TrackImp<T>*>::iterator eraseMe = tt;
      tt++;
      trax[c].erase(eraseMe);
      ta->destroy(t);
    } else {
      t->prune(synthtime[c]);
      tt++;
    }
  }
#ifdef MULTITHREADED
  pthread_mutex_unlock(&trackMutex[c]);
#endif
}

template<class T>
countType SMSBase<T> :: getSamplePos()
{
  return samplePos[0];
}

template<class T>
void SMSBase<T> :: renderTracks(int c, list<SMSRenderer*> &renderers, float a, float f0, float f1)
{
#ifdef MULTITHREADED
  pthread_mutex_lock(&trackMutex[c]);
#endif
  double h2 = a * h1;
  h2cum[c] += h2;
  h2i[c] = lrint(h2cum[c]);
  h2cum[c] -= h2i[c]; 
  for(list<SMSRenderer*>::iterator i = renderers.begin(); i != renderers.end(); ++i) {
    SMSRenderer *renderer = *i;
    renderer->startTime(c,samplePos[c],synthtime[c],h2i[c],(float)h2i[c],0.0f);
  }
  for(typename list<TrackImp<T>*>::iterator tt=trax[c].begin(); 
      tt != trax[c].end();) {
    TrackImp<T> *t = (*tt);      
    if(t->isEnded() && synthtime[c] > t->getEnd()) {
      if(!t->descendant) {
        typename list<TrackImp<T>*>::iterator eraseMe = tt;
        tt++;
        trax[c].erase(eraseMe);
        ta->destroy(t);
      } else {
        tt++;
      }
    } else if(synthtime[c] >= t->getStart()) {
      for(list<SMSRenderer*>::iterator i = renderers.begin(); i != renderers.end(); ++i) {
        SMSRenderer *renderer = *i;
        renderer->renderTrack(c,t,f0,f1,NULL);
      }
      if(synthtime[c] == t->getStart()) t->popPrecursor();
      tt++;
    } else {
      break;
    }
  }
#ifdef MULTITHREADED
  pthread_mutex_unlock(&trackMutex[c]);
#endif  
  for(list<SMSRenderer*>::iterator i = renderers.begin(); i != renderers.end(); ++i) {
    SMSRenderer *renderer = *i;  
    renderer->endTime(c);
  }
  samplePos[c] += h2i[c];
  synthtime[c]++;
}

template<class T>
void SMSBase<T> :: addTrack(int c, TrackImp<T> *t)
{
#ifdef MULTITHREADED
  pthread_mutex_lock(&trackMutex[c]);
#endif
  typename list<TrackImp<T>*>::reverse_iterator tt=trax[c].rbegin();
  while(tt!=trax[c].rend()) {
    TrackImp<T> *t0 = *tt;
    if(t->getStart() >= t0->getStart()) {
      break;
    }
    tt++;
  }
  trax[c].insert(tt.base(),t);
#ifdef MULTITHREADED
  pthread_mutex_unlock(&trackMutex[c]);
#endif
}

void SMS0 :: assignTrackPointsFromFile(FILE *fp, int c)
{
  int nTracks = fread_16_little_endian(fp);
  for(int k=0;k<nTracks;k++) {   
    char flags = fread_8_little_endian(fp);		
    bool bPrecursor = (flags & 16)?true:false;
    int f = fread_16_little_endian(fp);
    int y = fread_16_little_endian(fp);
    int ph = fread_16_little_endian(fp);
    int index = fread_16_little_endian(fp);
    TrackPoint *tp = new TrackPoint();
    tp->f = decodeLinear(f,0.0f,PI);
    tp->y = decodeMu(y)*(float)M;
    tp->ph = decodeLinear(ph,-PI,PI);
    tp->time = assigntime[c];
    int precursorIndex;
    if(bPrecursor) {
      precursorIndex = fread_16_little_endian(fp);
    } else {
      precursorIndex = -1;
    }
    assignTrackPoint(tp,flags,index,precursorIndex,c,false);
  }
  assigntime[c]++;
}

void SMS1 :: assignTrackPointsFromFile(FILE *fp, int c)
{
  int nTracks = fread_16_little_endian(fp);
  trackPointIndexAtTime[c]->push_back((int)trackPointCache[c]->size());
  nTracksAtTime[c]->push_back(nTracks);
  for(int k=0;k<nTracks;k++) {   
    char flags = fread_8_little_endian(fp);		
    bool bPrecursor = (flags & 16)?true:false;
    int f = fread_16_little_endian(fp);
    int y = fread_16_little_endian(fp);
    int ph = fread_16_little_endian(fp);
    int index = fread_16_little_endian(fp);
    TrackPoint1 *tp = new TrackPoint1();
    tp->f = decodeLinear(f,0.0f,PI);
    tp->y = decodeMu(y)*(float)M;
    tp->ph = decodeLinear(ph,-PI,PI);
    tp->time = addtime[c];
    tp->flags = flags;
    tp->index = index;
    if(bPrecursor) {
      tp->precursorIndex = fread_16_little_endian(fp);
    } else {
      tp->precursorIndex = -1;
    }
    tp->descendantIndex = -1;
    trackPointCache[c]->push_back(tp);
    bool bStart = flags & 1;
    if(!bStart) {
      TrackImp<TrackPoint1> *t = ta->getTrack(index);
      if(t) {
        TrackPoint1 *back = t->back();
        if(back) {
          tp->descendantIndex = back->descendantIndex;
        }
      }
    }
    assignTrackPoint(tp,tp->flags,tp->index,tp->precursorIndex,c,false);
    if(bPrecursor) {
      TrackImp<TrackPoint1> *precursor = ta->getTrack(tp->precursorIndex);
      if(precursor) {
        precursor->back()->descendantIndex = index;
      }
    }
  }
  addtime[c]++;
}
 
bool SMS1 :: assignTrackPointsFromMemory(int c)
{
  long time = (long)addtime[c];
  if((long)trackPointIndexAtTime[c]->size() > time) {
    long trackPointIndex = trackPointIndexAtTime[c]->at(time);
    int nTracks = nTracksAtTime[c]->at(time);
    for(int k=0;k<nTracks;k++) {
      TrackPoint1 *tp = trackPointCache[c]->at(trackPointIndex+k);
      tp->time = assigntime[c];
      assignTrackPoint(tp,tp->flags,tp->index,tp->descendantIndex,c,false);
    }
    assigntime[c]++;
    addtime[c]++;
    return true;
  } else {
    return false;
  }
}

bool SMS1 :: assignTrackPointsFromMemory(int c, bool bBackwards, int offset)
{
  //if(c==0) printf("assign %d %lld %lld %ld\n",M,addtime[c],assigntime[c],offset);
  long time = (long)addtime[c] + offset;
  if((long)trackPointIndexAtTime[c]->size() > time && time >= 0) {
    long trackPointIndex = trackPointIndexAtTime[c]->at(time);
    int nTracks = nTracksAtTime[c]->at(time);
    for(int k=0;k<nTracks;k++) {
      TrackPoint1 *tp = trackPointCache[c]->at(trackPointIndex+k);
      tp->time = assigntime[c];
      assignTrackPoint(tp,tp->flags,tp->index,bBackwards?tp->descendantIndex:tp->precursorIndex,c,bBackwards);
    }
    for(list<TrackImp<TrackPoint1>*>::iterator tt=trax[c].begin(); 
        tt != trax[c].end();
        tt++) {
      TrackImp<TrackPoint1> *t = (*tt);      
      if(!t->isEnded() && t->back()->time < assigntime[c]) {
        t->endTrack(true);
      }
    }
    assigntime[c]++;
    return true;
  } else {
    if(time < 0) {
      samplePos[c] = 0;
      samplePosCum[c] = 0.0;
    } else if((long)trackPointIndexAtTime[c]->size() <= time) {
      samplePos[c] = (countType)trackPointIndexAtTime[c]->size() * (countType)(h * M);
      samplePosCum[c] = 0.0;
    }
    return false;
  }
}

inline float SMS2 :: merit(TrackPoint2 *tp0, TrackPoint2 *tp1, float m0, float m1, float dti2, float dMCoeff, float *df, float maxDF2)
{
  (*df) = m1*tp1->f - m0*tp0->f;
  float df2 = square(*df);
  if(df2 > maxDF2) return df2;
  if(tp0->y==0.0f || tp1->y==0.0f) return df2;
  float dM = dBApprox((m1*tp1->y)/(m0*tp0->y));
  return (df2+square(dMCoeff*dM));
}

bool SMS2 :: nearestTrackPoint(list<TrackPoint2*> *tpl, TrackPoint2 *tp0, float m0, float m1, float dti2, list<TrackPoint2*>::iterator *minpos, float *minMerit2, float maxMerit2, float maxDF2) 
{
  (*minMerit2) = 1e9;
  if(tpl == NULL) return false;
  bool bSet = false;

  for(list<TrackPoint2*>::iterator tpi=tpl->begin(); 
      tpi!=tpl->end();
      ++tpi) {

    TrackPoint2 *tp1 = (*tpi);
    float df;
    float Merit2 = merit(tp0,tp1,m0,m1,dti2,dMCoeff,&df,maxDF2);
    if(m0!=m1) Merit2*=localFavorRatio;
    if(Merit2 < (*minMerit2) && Merit2 < maxMerit2 && square(df) < maxDF2) {
      (*minMerit2) = Merit2;
      (*minpos) = tpi;
      bSet = true;
    }
  }
  return bSet;
}

bool SMS2 :: connectTrackPoints(TrackPoint2 *tp0, TrackPoint2 *tp1, SMS2 *hi, SMS2 *lo, float dtlo, int c)
{
  bool isConn;
  bool allowCont;
  if(tp0->M == tp1->M) {
    allowCont = contTrack(tp0->owner, tp1, c);
    isConn = allowCont;
  } else if(tp0->M < tp1->M) {
    isConn = true;
    allowCont = lo->adoptTrack(tp0->owner, this, tp1, 0.5f, dtlo, c);
  } else {
    isConn = true;
    allowCont = hi->adoptTrack(tp0->owner, this, tp1, 2.0f, 1.0, c);
  }
  tp0->bConnected = true;
  if(isConn) {
    tp1->bConnected = true;
    if(tp0->dupcont != NULL) {
      TrackPoint2 *dup = tp0->dupcont;
      dup->bConnected = true;
      if(!dup->owner) {
        dup->bDelete = true;
      }
    }
    
    TrackPoint2 *dup2 = tp0->dup[2];
    if(dup2 && dup2 != tp1 && !dup2->owner) {
      dup2->bDelete = true;
    }

    for(int d=0;d<3;d++) {
      if(tp1->dup[d] && !tp1->dup[d]->owner && (d<2 || tp1->dup[d]->M < tp1->M)) {
        tp1->dup[d]->bDelete = true;
      }
    }
  }
  return allowCont;
}

bool SMS2 :: contTrack(TrackImp<TrackPoint2> *t, TrackPoint2 *tp, int c)
{
#ifdef MULTITHREADED
  pthread_mutex_lock(&trackMutex[c]);
#endif
  t->push_back(tp);
#ifdef MULTITHREADED
  pthread_mutex_unlock(&trackMutex[c]);
#endif
  return true;
}      

bool SMS2 :: adoptTrack(TrackImp<TrackPoint2> *precursor, 
                        SMS2 *lender,
                        TrackPoint2 *tp,
                        float m,
                        float dt,
                        int c)
{
#ifdef MULTITHREADED
  pthread_mutex_lock(&lender->trackMutex[c]);
#endif
  TrackPoint2 *last = precursor->back();
#ifdef MULTITHREADED
  pthread_mutex_unlock(&lender->trackMutex[c]);
#endif
  if(tp->M>last->M) {
    if(dt==1.0) {
      if(lender->res==2) {
        TrackPoint2 *tp0 = new TrackPoint2();
        tp0->y = 0.5f*tp->y;
        tp0->f = 0.5f*tp->f;
        tp0->ph = tp->ph;
        tp0->time = last->time+1;
        tp0->M = last->M;
#ifdef MULTITHREADED
        pthread_mutex_lock(&lender->trackMutex[c]);
#endif
        precursor->push_back(tp0); 
#ifdef MULTITHREADED
        pthread_mutex_unlock(&lender->trackMutex[c]);
#endif
        if(!tp->owner) 
          tp->bDelete = true;
      } else {
        TrackPoint2 *tpend = new TrackPoint2();
        tpend->y = 0;
        tpend->f = 0.5f*tp->f;
        tpend->ph = tp->ph;
        tpend->time = last->time+1;
        tpend->M = last->M;

        TrackPoint2 *tp0 = new TrackPoint2();
        tp0->y = 0;
        tp0->f = last->f*2.0f;
        tp0->ph = last->ph;
        tp0->time = last->time/lender->res;
        tp0->M = M;
        TrackImp<TrackPoint2> *t = ta->create(precursor,M,res,h);
#ifdef MULTITHREADED
        pthread_mutex_lock(&lender->trackMutex[c]);
#endif
        precursor->push_back(tpend);
        precursor->endTrack(false);
        precursor->descendant = t;
#ifdef MULTITHREADED
        pthread_mutex_unlock(&lender->trackMutex[c]);
#endif
        t->startTrack(tp0,false);
        t->push_back(tp);
        addTrack(c,t);
      }
    } else if(dt==2.0) {
      TrackImp<TrackPoint2> *t = ta->create(precursor,M,res,h);
      TrackPoint2 *tpend0 = new TrackPoint2();
      TrackPoint2 *tpend1 = new TrackPoint2();
      TrackPoint2 *tp0 = new TrackPoint2();
      tp0->y = 0;
      tp0->f = last->f*2.0f;
      tp0->ph = last->ph;
      tp0->M = M;
      tp0->time = last->time/lender->res;
      
      tpend0->y = 0.5f*last->y;
      tpend0->f = 0.5f*(last->f+0.5f*tp->f);
      tpend0->ph = canon(last->ph + 0.5f*(last->f+tpend0->f)*precursor->getH());
      tpend0->time = last->time+1;
      tpend0->M = last->M;

      tpend1->y = 0;
      tpend1->f = 0.5f*tp->f;
      tpend1->ph = tp->ph;
      tpend1->time = last->time+2;
      tpend1->M = last->M;

      t->startTrack(tp0,false);
      t->push_back(tp);
#ifdef MULTITHREADED
      pthread_mutex_lock(&lender->trackMutex[c]);
#endif
      precursor->push_back(tpend0);
      precursor->push_back(tpend1);
      precursor->endTrack(false);
      precursor->descendant = t;
#ifdef MULTITHREADED
      pthread_mutex_unlock(&lender->trackMutex[c]);
#endif
      addTrack(c,t);
    } else {
      abort();
    }
  } else {
    TrackImp<TrackPoint2> *t = ta->create(precursor,M,res,h);

    TrackPoint2 *tp0 = new TrackPoint2();
    TrackPoint2 *tpend = new TrackPoint2();
    
    tp0->y = 0;
    tp0->f = 0.5f*last->f;
    tp0->ph = last->ph;
    tp0->time = last->time*res;
    tp0->M = M;
    
    tpend->y = 0;
    tpend->f = tp->f*2.0f;
    tpend->time = last->time+1;
    tpend->ph = tp->ph;
    tpend->M = last->M;

    t->startTrack(tp0,false);
    if(res==2) {
      TrackPoint2 *tp1 = new TrackPoint2();
      tp1->y = 0.5f*tp->y;
      tp1->f = 0.5f*(0.5f*last->f+tp->f);
      tp1->ph = canon(tp0->ph + 0.5f*(tp0->f + tp1->f)*precursor->getH());
      tp1->time = last->time*res+1;
      tp1->M = M;
      t->push_back(tp1);
    }
    t->push_back(tp);
#ifdef MULTITHREADED
    pthread_mutex_lock(&lender->trackMutex[c]);
#endif
    precursor->push_back(tpend);
    precursor->endTrack(false);
    precursor->descendant = t;
#ifdef MULTITHREADED
    pthread_mutex_unlock(&lender->trackMutex[c]);
#endif
    addTrack(c,t);
  }
  return true;
}

void SMS2 :: markDuplicates(long offset, SMSAnalyze *hi_, SMSAnalyze *lo_, int c)
{
  SMS2 *hi = (SMS2*)hi_;
  SMS2 *lo = (SMS2*)lo_;
  if(offset%res==0) {
    markDuplicatesLo(offset,lo,0,c);
  } else {
    markDuplicatesLo(offset,lo,0,c);
    markDuplicatesLo(offset,lo,1,c);
  }
}

void SMS2 :: markDuplicatesLo(long offset, SMS2 *lo, long offsetlo, int c)
{
  if(!lo) return;
#ifdef MULTITHREADED
  pthread_mutex_lock(&lo->tplbMutex[c]);
#endif
  list<TrackPoint2*> *trackPointsL1 = 
    lo->trackPointListBuffer[c]->read(lo->trackPointListBuffer[c]->readPos+offset/res+offsetlo);
#ifdef MULTITHREADED
  pthread_mutex_unlock(&lo->tplbMutex[c]);
#endif
#ifdef MULTITHREADED
    pthread_mutex_lock(&tplbMutex[c]);
#endif
  list<TrackPoint2*> *trackPointsM1 = 
    trackPointListBuffer[c]->read(trackPointListBuffer[c]->readPos+offset);
#ifdef MULTITHREADED
  pthread_mutex_unlock(&tplbMutex[c]);
#endif
  
  list<TrackPoint2*> trackPoints0;
  
  for(list<TrackPoint2*>::iterator tpi=trackPointsM1->begin(); 
      tpi!=trackPointsM1->end();
      tpi++) {
    
    if((*tpi)->f > maxFMatch) break;
    trackPoints0.push_back((*tpi));
  }
  
  bool bDone = false;
  bool bLastDitch = false;
  
  while(!bDone) {      
    int nToCont = 0;
    int nCont = 0;
    
    for(list<TrackPoint2*>::iterator tpi = trackPoints0.begin(); 
        tpi!=trackPoints0.end();
        tpi++) {
      float F;
      list<TrackPoint2*>::iterator minL0; bool minL0Set = nearestTrackPoint(trackPointsL1,*tpi,1.0,0.5,1.0,&minL0,&F,maxMerit2Match, maxDF2Match);
      if(minL0Set) {
        nToCont++;
        (*tpi)->cont = (*minL0);
      } else {
        (*tpi)->cont = NULL;
      }
    }
      
    if(trackPointsL1) {
      for(list<TrackPoint2*>::reverse_iterator tpi = trackPointsL1->rbegin(); 
          tpi!=trackPointsL1->rend();
          tpi++) {      
        float F;
        (*tpi)->cont = NULL;
        if((*tpi)->f < minFMatch) break;
        list<TrackPoint2*>::iterator minL1; bool minL1Set = nearestTrackPoint(&trackPoints0,*tpi,0.5,1.0,1.0,&minL1,&F,maxMerit2Match, maxDF2Match);
        if(minL1Set) (*tpi)->cont = *minL1;
      }
    }
    
    for(list<TrackPoint2*>::iterator tpi = trackPoints0.begin(); 
        tpi!=trackPoints0.end();
        ) {
      TrackPoint2 *tp0 = (*tpi);
      TrackPoint2 *tp1 = tp0->cont;
      if(tp1 != NULL) {
        if(bLastDitch || tp1->cont == tp0) {
          nCont++;
          bool bAlreadyMarked = false;
          if(offset%res == 0) {
            if(tp1->dup[1] != NULL || tp0->dup[1] != NULL)
              bAlreadyMarked = true;
          } else {
            if(tp1->dup[2-2*offsetlo] != NULL || tp0->dup[2*offsetlo] != NULL)
              bAlreadyMarked = true;
          }
          if(!bAlreadyMarked) {
            if(offset%res == 0) {
              tp1->dup[1] = tp0;
              tp0->dup[1] = tp1;
            } else {
              tp1->dup[2-2*offsetlo] = tp0;
              tp0->dup[2*offsetlo] = tp1;
            }
          }
          list<TrackPoint2*>::iterator eraseMe = tpi;
          tpi++;
          trackPoints0.erase(eraseMe);	    
          continue;
        }
      }
      tpi++;
    }
    bDone = (nToCont-nCont == 0);
    bLastDitch = (!bDone && nCont==0);
  }
  marktime[c]++;
}

long SMS2 :: assignTrackPoints(long offset, SMSAnalyze *hi_, SMSAnalyze *lo_, int c)
{  
  SMS2 *hi = (SMS2*)hi_;
  SMS2 *lo = (SMS2*)lo_;
  if(offset%res == 0)
    assignTrackPoints_(offset,hi,lo,1.0,0,c);
  else
    assignTrackPoints_(offset,hi,lo,2.0,1,c);
  return 1;
}

long SMS2 :: assignTrackPoints_(long offset, SMS2 *hi, SMS2 *lo, float dtlo, long offsetlo, int c)
{
#ifdef MULTITHREADED
  if(hi) pthread_mutex_lock(&hi->tplbMutex[c]);
#endif
  list<TrackPoint2*> *trackPointsH1 = 
    hi?hi->trackPointListBuffer[c]->read(hi->trackPointListBuffer[c]->readPos+hi->res*offset):NULL;  
#ifdef MULTITHREADED 
  if(hi) pthread_mutex_unlock(&hi->tplbMutex[c]);
#endif

#ifdef MULTITHREADED
  if(lo) pthread_mutex_lock(&lo->tplbMutex[c]);
#endif
  list<TrackPoint2*> *trackPointsL1 = 
    lo?lo->trackPointListBuffer[c]->read(lo->trackPointListBuffer[c]->readPos+offset/res+offsetlo):NULL;
#ifdef MULTITHREADED
	if(lo) pthread_mutex_unlock(&lo->tplbMutex[c]);
#endif

#ifdef MULTITHREADED
  pthread_mutex_lock(&tplbMutex[c]);
#endif
  list<TrackPoint2*> *trackPointsM1 = 
    trackPointListBuffer[c]->read(trackPointListBuffer[c]->readPos+offset);
#ifdef MULTITHREADED
  pthread_mutex_unlock(&tplbMutex[c]);	
#endif
  
  list<TrackPoint2*> trackPoints0;
  
#ifdef MULTITHREADED
  pthread_mutex_lock(&trackMutex[c]);
#endif
  for(list<TrackImp<TrackPoint2>*>::iterator tt=trax[c].begin(); 
      tt != trax[c].end(); 
      tt++) {
    TrackImp<TrackPoint2> *t = (*tt);
    t->bEnd = true;
    if(t->isEnded() || t->back()->time+1 != assigntime[c]) {
      t->bEnd = false;
      continue; 
    }
    trackPoints0.push_back(t->back());
  }
#ifdef MULTITHREADED
  pthread_mutex_unlock(&trackMutex[c]);
#endif

  for(list<TrackPoint2*>::iterator tpi=trackPointsM1->begin(); 
      tpi!=trackPointsM1->end();) {
    TrackPoint2 *tp = (*tpi);
    if(tp->bDelete) {
      list<TrackPoint2*>::iterator eraseMe = tpi;
      tpi++;
      trackPointsM1->erase(eraseMe);
      delete tp;
    } else {
      tpi++;
    }
  }
  
  if(trackPointsL1) {
    for(list<TrackPoint2*>::iterator tpi=trackPointsL1->begin(); 
        tpi!=trackPointsL1->end(); ) {
      TrackPoint2 *tp = (*tpi);
      if(tp->bDelete) {
        list<TrackPoint2*>::iterator eraseMe = tpi;
        tpi++;
        trackPointsL1->erase(eraseMe);
        delete tp;
      } else {
        tpi++;
      }
    }
  }

  if(trackPointsH1) {
    for(list<TrackPoint2*>::iterator tpi=trackPointsH1->begin(); 
        tpi!=trackPointsH1->end(); ) {
      TrackPoint2 *tp = (*tpi);
      if(tp->bDelete) {
        list<TrackPoint2*>::iterator eraseMe = tpi;
        tpi++;
        trackPointsH1->erase(eraseMe);
        delete tp;
      } else {
        tpi++;
      }
    }
  }

  bool bDone = false;
  bool bLastDitch = false;
  
  while(!bDone) {
    int nToCont = 0;
    int nCont = 0;
    
    for(list<TrackPoint2*>::iterator tpi=trackPointsM1->begin(); 
        tpi!=trackPointsM1->end();
        tpi++) {
      float F;	
      (*tpi)->bConnect = false;
      (*tpi)->bConnected = false;
      list<TrackPoint2*>::iterator minM1; bool minM1Set = nearestTrackPoint(&trackPoints0,*tpi,1.0,1.0,1.0,&minM1,&F,maxMerit2,maxDF2);
      if(minM1Set) (*tpi)->cont = *minM1;
      else (*tpi)->cont = NULL;
    }
    
    if(trackPointsL1) {
      for(list<TrackPoint2*>::reverse_iterator tpi = trackPointsL1->rbegin(); 
          tpi!=trackPointsL1->rend();
          tpi++) {      
        (*tpi)->cont = NULL;
        (*tpi)->bConnect = false;
        (*tpi)->bConnected = false;
        if((*tpi)->f < minFMatch) break;
        float F;
        list<TrackPoint2*>::iterator minL1; bool minL1Set = nearestTrackPoint(&trackPoints0,*tpi,0.5f,1.0f,square(1.0f/dtlo),&minL1,&F,maxMerit2,maxDF2);
        if(minL1Set) (*tpi)->cont = *minL1;
      }
    }
    
    if(trackPointsH1) {
      for(list<TrackPoint2*>::iterator tpi = trackPointsH1->begin(); 
          tpi!=trackPointsH1->end();
          tpi++) {      
        (*tpi)->cont = NULL;
        (*tpi)->bConnect = false;
        (*tpi)->bConnected = false;
        float F;
        if((*tpi)->f < maxFMatch) {
          list<TrackPoint2*>::iterator minH1; bool minH1Set = nearestTrackPoint(&trackPoints0,*tpi,2.0f,1.0f,1.0f,&minH1,&F,maxMerit2,maxDF2);
          if(minH1Set) (*tpi)->cont = *minH1;
        }
      }
    }
    
    for(list<TrackPoint2*>::iterator tpi = trackPoints0.begin(); 
        tpi!=trackPoints0.end();
        tpi++) {
      (*tpi)->dupcont = NULL;
      (*tpi)->bConnected = false;
      float FM0, FL0, FH0;
      list<TrackPoint2*>::iterator minM0; bool minM0Set = nearestTrackPoint(trackPointsM1,*tpi,1.0f,1.0f,1.0f,&minM0,&FM0,maxMerit2,maxDF2);
      list<TrackPoint2*>::iterator minL0; bool minL0Set = nearestTrackPoint(trackPointsL1,*tpi,1.0f,0.5f,square(1.0f/dtlo),&minL0,&FL0,maxMerit2,maxDF2);
      list<TrackPoint2*>::iterator minH0; bool minH0Set = nearestTrackPoint(trackPointsH1,*tpi,1.0f,2.0f,1.0f,&minH0,&FH0,maxMerit2,maxDF2);
      
      if(minM0Set &&
         ((FM0<=FH0 && FM0<=FL0)
          ||(minL0Set && FL0<=FH0 && FL0<=FM0 && (*minL0)->dup[1-offsetlo] == (*minM0))
          ||(minH0Set && FH0<=FL0 && FH0<=FM0 && (*minH0)->dup[1] == (*minM0)))) {
        if(offsetlo == 0 && minL0Set && (*minL0)->dup[1] == (*minM0)) {	    
          (*tpi)->dupcont = *minL0;
        } else if(minH0Set && (*minH0)->dup[1] == (*minM0)) {
          (*tpi)->dupcont = *minH0;
        }
        (*tpi)->contF = FM0;
        (*tpi)->cont = *minM0;
        nToCont++;
      } else if(minL0Set && FL0<=FM0 && FL0<=FH0) {
        if(minM0Set && (*minL0)->dup[1-offsetlo] == (*minM0)) {
          (*tpi)->dupcont = *minM0;
        }
        (*tpi)->contF = FL0;
        (*tpi)->cont = *minL0;
        nToCont++;
      } else if(minH0Set && FH0<=FM0 && FH0<=FL0) {
        if(minM0Set && (*minH0)->dup[1] == (*minM0)) {
          (*tpi)->dupcont = *minM0;
        }
        (*tpi)->contF = FH0;
        (*tpi)->cont = *minH0;
        nToCont++;
      } else {
        (*tpi)->cont = NULL;
      }
    }

    // Nominal connections - may be conflicts to be resolved
    for(list<TrackPoint2*>::iterator tpi = trackPoints0.begin(); 
        tpi!=trackPoints0.end();
        tpi++ ) {
      TrackPoint2 *tp0 = (*tpi);
      TrackPoint2 *tp1 = tp0->cont;
      if(tp1 != NULL) {
        if(bLastDitch || (tp1->cont == tp0)) {
          tp1->bConnect = true;
        }
      }
    }
    
    // Make connections and resolve conflicts between duplicates and between last ditch connections
    for(list<TrackPoint2*>::iterator tpi = trackPoints0.begin(); 
        tpi!=trackPoints0.end(); ) {
      
      TrackPoint2 *tp0 = (*tpi);
      TrackPoint2 *tp1 = tp0->cont;
      
      if(tp0->bDelete) { 
        list<TrackPoint2*>::iterator eraseMe = tpi;
        tpi++;
        trackPoints0.erase(eraseMe);
        continue;
      }
      if(tp1 != NULL && tp1->bConnect) {
        if(tp0->dupcont != NULL && tp0->dupcont->bConnect) {
          if(!tp1->bConnected && !tp0->dupcont->bConnected) {
            if(!tp0->bConnected && !tp1->bDelete && (tp0->dupcont->cont == NULL || tp0->contF <= tp0->dupcont->cont->contF)) {
              nCont++;
              if(connectTrackPoints(tp0,tp1,hi,lo,dtlo,c))
                tp0->owner->bEnd = false;
              tp0->dupcont->bConnect = false;
            } else if(tp0->dupcont->cont != NULL && !tp0->dupcont->cont->bConnected && !tp0->dupcont->bDelete && !tp0->dupcont->cont->bDelete) {
              nCont++;
              if(connectTrackPoints(tp0->dupcont->cont,tp0->dupcont,hi,lo,dtlo,c))
                tp0->dupcont->cont->owner->bEnd = false;
              tp1->bConnect = false;
            }
          }
        } else if(!tp0->bConnected && !tp1->bConnected && !tp1->bDelete) {
          nCont++;
          if(connectTrackPoints(tp0,tp1,hi,lo,dtlo,c)) {
            tp0->owner->bEnd = false;
          }
        }
      }
      if(tp0->bConnected || tp0->bDelete) {
        list<TrackPoint2*>::iterator eraseMe = tpi;
        tpi++;
        trackPoints0.erase(eraseMe);
      } else {
        tpi++;
      }
    }
    
    for(list<TrackPoint2*>::iterator tpi=trackPointsM1->begin(); 
        tpi!=trackPointsM1->end(); ) {
      TrackPoint2 *tp = (*tpi);
      if(tp->bConnected ||tp->bDelete) {
        list<TrackPoint2*>::iterator eraseMe = tpi;
        tpi++;
        trackPointsM1->erase(eraseMe);
      } else {
        tpi++;
      }
      if(tp->bDelete) {
        delete tp;
      }
    }
    
    if(trackPointsL1) {
      for(list<TrackPoint2*>::iterator tpi=trackPointsL1->begin(); 
          tpi!=trackPointsL1->end(); ) {      
        TrackPoint2 *tp = (*tpi);
        if(tp->bConnected||tp->bDelete) {
          list<TrackPoint2*>::iterator eraseMe = tpi;
          tpi++;
          trackPointsL1->erase(eraseMe);
        } else {
          tpi++;
        }
        if(tp->bDelete) {
          delete tp;
        }
      }
    }
    
    if(trackPointsH1) {
      for(list<TrackPoint2*>::iterator tpi=trackPointsH1->begin(); 
          tpi!=trackPointsH1->end(); ) {    
        TrackPoint2 *tp = (*tpi);
        if(tp->bConnected || tp->bDelete) {
          list<TrackPoint2*>::iterator eraseMe = tpi;
          tpi++;
          trackPointsH1->erase(eraseMe);
        } else {
          tpi++;
        }
        if(tp->bDelete) {
          delete tp;
        }
      }
    }
    bDone = (nToCont-nCont == 0);
    bLastDitch = (!bDone && nCont==0);
  }
  
#ifdef MULTITHREADED
  pthread_mutex_lock(&trackMutex[c]);
#endif
  for(list< TrackImp<TrackPoint2>* >::iterator tt=trax[c].begin(); 
      tt != trax[c].end(); ) {
    TrackImp<TrackPoint2> *t = (*tt);      
    if(t->bEnd) {
      t->endTrack(true);
      if(t->isEnded() && t->size() < minNpts && !t->precursor && !t->descendant) {
        list<TrackImp<TrackPoint2>*>::iterator eraseMe = tt;
        tt++;
        trax[c].erase(eraseMe);
        ta->destroy(t);  
      } else {
        tt++;
      }
    } else {
      tt++;
    }
  }
#ifdef MULTITHREADED
  pthread_mutex_unlock(&trackMutex[c]);
#endif
  assigntime[c]++;
  return 1;
}

long SMS2 :: startNewTracks(long offset, int c)
{
#ifdef MULTITHREADED
  pthread_mutex_lock(&tplbMutex[c]);
#endif
  list<TrackPoint2*> *trackPoints = trackPointListBuffer[c]->read(trackPointListBuffer[c]->readPos+offset);
#ifdef MULTITHREADED
  pthread_mutex_unlock(&tplbMutex[c]);
#endif

  for(list<TrackPoint2*>::iterator tpi=trackPoints->begin(); 
      tpi!=trackPoints->end(); ) {
    TrackPoint2 *tp = (*tpi);
    
    if(!tp->bDelete) {
      TrackImp<TrackPoint2> *t = ta->create(NULL,M,res,h);
      t->startTrack(tp,true);
      addTrack(c,t);	
      for(int d=0;d<2;d++) {
        if(tp->dup[d])
          if(!tp->dup[d]->owner)
            tp->dup[d]->bDelete = true;
      }
    } else {
      delete tp;
    }                 
    list<TrackPoint2*>::iterator eraseMe = tpi;
    tpi++;
    trackPoints->erase(eraseMe);
  }
  return 1;
}

void SMS2 :: advanceTrackPoints(int c)
{
#ifdef MULTITHREADED
  pthread_mutex_lock(&tplbMutex[c]);
#endif
  trackPointListBuffer[c]->advance(1);
#ifdef MULTITHREADED
  pthread_mutex_unlock(&tplbMutex[c]);
#endif
}

long SMS2 :: addTrackPoints(grain *g0, grain *g1, grain *g2, int c)
{
  // set magnitude profile of single constant frequency peak
  if(!bPeakSet) {
    for(int k=-Nover2;k<=Nover2;k++) {
      peak0[k+N] = norm2(g0->peak[(k+N)%N]);
      peak1[k+N] = norm2(g1->peak[(k+N)%N]);
    }
    bPeakSet = true;
  }

  if(c == 0) {
    c2even(g0->x, x0[0], N);
    c2even(g1->x, x1[0], N);
    c2even(g2->x, x2[0], N);
  } else {
    c2odd(g0->x, x0[1], N);
    c2odd(g1->x, x1[1], N);
    c2odd(g2->x, x2[1], N);
  }
  calcmags(mag0[c], x0[c], pad0);
  calcmags(mag1[c], x1[c], 1.0f);
  calcmags(mag2[c], x2[c], pad2);
  extractTrackPoints(x1[c],mag0[c],mag1[c],mag2[c],addtime[c],trackPointListBuffer[c],c);
  addtime[c]++;
  return 1;
}

void SMS2 :: extractTrackPoints(audio *x, 
                                float *mag0, float *mag1, float *mag2,
                                countType addtime,
                                PointerBuffer< list<TrackPoint2*> > *trackPointListBuffer,
                                int c)
{
  float thmin = square(peakThresh)*magmax;
  list<peak*> peaks;
  peak* trough1 = NULL;
  peak* trough2 = NULL;

  peak *t0 = pa[c]->create();
  t0->k = 1;
  t0->x = (float)1;
  t0->y = mag1[0];
  t0->tp = NULL;
  t0->tp2 = NULL;
  t0->tn = NULL;
  t0->tn2 = NULL;

  peak *t1 = pa[c]->create();
  t1->k = Nover2-1;
  t1->x = (float)(Nover2-1);
  t1->y = mag1[Nover2-1];
  t1->tp = NULL;
  t1->tp2 = NULL;
  t1->tn = NULL;
  t1->tn2 = NULL;

  trough2 = t0;

  for(int k=1;k<Nover2-1;k++) {
    if( (mag2[k] > thmin)
        &&
        (mag2[k] > mag2[k-1])
        &&
        (mag2[k] >= mag2[k+1])
        ) {
      if(k>=kStart-peakWidth0 && k<=kEnd+peakWidth0) {
        peak *p = makePeak(mag1,mag2,k,c);
        p->tn2 = t1;
        if(trough1 && trough1->tn == NULL) {
          p->tp = trough1;
          trough1->tn = p;
        }
        if(trough2 && trough2->tn2 == NULL) {
          p->tp2 = trough2;
          trough2->tn2 = p;
        }
        peaks.push_back(p);
      }
    } else if((mag2[k] <= mag2[k-1])
              &&
              (mag2[k] <= mag2[k+1])
              ) {
      peak *p = makePeak(mag1,mag2,k,c);
      trough2 = p;
      if(!peaks.empty()) { 
        peaks.back()->tn2 = p;
        p->tp2 = peaks.back();
      }
    }
    if((mag1[k] <= mag1[k-1])
       &&
       (mag1[k] <= mag1[k+1])
       ) {      
      peak *p = makePeak(mag1,mag1,k,c);
      trough1 = p;
      if(!peaks.empty() && peaks.back()->tn == NULL) {
        peaks.back()->tn = p;
        p->tp = peaks.back();
      }
    }
  }

  adjustPeaks(peaks,
              mag0,
              mag1,
              mag2,
              dec[c]);
  
  list<TrackPoint2*> *trackPoints = new list<TrackPoint2*>;
  for(list<peak*>::iterator pp=peaks.begin();
      pp!=peaks.end();
      pp++) {
    
    peak *p = (*pp);
    if(p->k>=kStart && p->k<=kEnd && p->m>0.0f) {
      float y = sqrt(mNorm * p->m);
      TrackPoint2 *tp = new TrackPoint2(x,p->x,y,N,M,addtime);
      trackPoints->push_back(tp);
    }
  }
  pa[c]->destroyAll();
#ifdef MULTITHREADED
  pthread_mutex_lock(&tplbMutex[c]);
#endif
  trackPointListBuffer->write(trackPoints);
#ifdef MULTITHREADED
  pthread_mutex_unlock(&tplbMutex[c]);
#endif
}

void SMS2 :: calcmags(float *mag, audio *x, float q) {
  for(int k=0;k<=Nover2;k++) {
    float m = norm2(x[k])*q;
    mag[k] = m;
    if(magmax < m) magmax = m;
  }
}

peak *SMS2 :: makePeak(float *mag, float *mag2, int k, int c)
{
  float v0,v1,v2;
  v0 = mag2[k-1];
  v1 = mag2[k];
  v2 = mag2[k+1];
  float y1 = v1-v0;
  float y2 = v2-v0;
  float a = 0.5f*(y2 - 2.0f * y1);
  float b = (a==0.0f?2.0f:1.0f-y1/a);
  peak *p = pa[c]->create();
  p->x = (k-1)+0.5f*b;
  p->k = k;
  int k0 = lrintf(p->x);
  float kf = k0<p->x?p->x-k0:k0-p->x;
  int k1 = k0<p->x?k0+1:k0-1;
  p->y = (1.0f-kf)*mag[k0] + kf*mag[k1];
  p->tp = NULL;
  p->tp2 = NULL;
  p->tn = NULL;
  p->tn2 = NULL;
  return p;
}

void SMS2 :: adjustPeaks(list<peak*> &peaks,
                        float *mag0,
                        float *mag1,
                        float *mag2,
                        float *dec)
{
  for(int k=0;k<N;k++) {
    dec[k] = 0.0f;
  }

  for(list<peak *>::iterator tpi = peaks.begin();
      tpi != peaks.end(); ) {
    list<peak*>::iterator eraseMe = tpi;
    peak *p = (*tpi);

    peak *pp = NULL;
    if(tpi!=peaks.begin()) {
      pp = *(--tpi);
      tpi++;
    }

    peak *pn = NULL;
    tpi++;
    if(tpi != peaks.end()) {
      pn = (*tpi);
    }

    if(p->tp && pp && pp->tn
       &&
       p->x - p->tp->x > p->x - p->tp2->x
       &&
       pp->tn->x - pp->x > pp->tn2->x - pp->x) {
    } else {
      p->tp = p->tp2;
    }
    
    if(p->tn && pn && pn->tp
       &&
       p->tn->x - p->x > p->tn2->x - p->x
       &&
       pn->x - pn->tp->x > pn->x - pn->tp2->x) {
    } else {
      p->tn = p->tn2;
    }

    peak *tp = p->tp;
    peak *tn = p->tn;

    int k1 = p->k;
    int k0 = tp->k;
    float kf0;
    if(k0 < k1-peakWidth1) {
      k0 = k1-peakWidth1;
      kf0 = 0;
    } else {
      kf0 = k0 > tp->x ? k0 - tp->x : tp->x - k0;
    }
    int k2 = tn->k;
    float kf2;
    if(k2 > k1+peakWidth1) {
      k2 = k1+peakWidth1;
      kf2 = 0;
    } else {
      kf2 = k2 > tn->x ? k2 - tn->x : tn->x - k2;
    }

    float m = 0.0f;
    float mtop = 0.0f;

    float mbase = mag2[k0] + mag2[k2];
    if(k0 < tp->x) {
      float m0 = ((mag2[k0])+(mag2[k0+1]))*(1.0f-kf0);
      m += m0;
      mtop += max(0.0f,m0 - mbase*(1.0f-kf0));
    } else {
      float m0 = ((mag2[k0])+(mag2[k0+1]));
      m += m0;
      mtop += max(0.0f,m0 - mbase);
      m0 = ((mag2[k0-1])+(mag2[k0]))*kf0;
      m += m0;
      mtop += max(0.0f,m0 - mbase*kf0);
    }
    
    if(k2 < tn->x) {
      float m0 = ((mag2[k2-1])+(mag2[k2]));
      m += m0;
      mtop += max(0.0f,m0 - mbase);
      m0 = ((mag2[k2])+(mag2[k2+1]))*kf2;
      m += m0;
      mtop += max(0.0f,m0 - mbase*kf2);
    } else {
      float m0 = ((mag2[k2-1])+(mag2[k2]))*(1.0f-kf2);
      m += m0;
      mtop += max(0.0f,m0 - mbase*(1.0f-kf2));
    }
    
    for(int k=k0+1;k<k2-1;k++) {
      float m0 = ((mag2[k])+(mag2[k+1]));
      m += m0;
      mtop += max(0.0f,m0 - mbase);
    }

    if(mtop < minPeakTopRatio*m) {
      peaks.erase(eraseMe);
      if(pp && (!pn || pp->y > pn->y)) {
        pp->tn = p->tn;
        pp->tn2 = p->tn2;
      } else if(pn && (!pp || pn->y > pp->y)) {
        pn->tp = p->tp;
        pn->tp2 = p->tp2;
      }
    }
  }

  for(list<peak *>::iterator tpi = peaks.begin();
      tpi != peaks.end();
      tpi++) {
    peak *p = (*tpi);
    
    peak *tp = p->tp;
    peak *tn = p->tn;
       
    int k1 = p->k;
    int ko1 = k1 > p->x ? -1 : 1;
    float kf1 = k1 > p->x ? k1 - p->x : p->x - k1; 
    int k0 = tp->k;
    float kf0;
    if(k0 < k1-peakWidth1) {
      k0 = k1-peakWidth1;
      kf0 = 0;
    } else {
      kf0 = (k0 > tp->x ? k0 - tp->x : tp->x - k0);
    }
    int k2 = tn->k;
    float kf2;
    if(k2 > k1+peakWidth1) {
      k2 = k1+peakWidth1;
      kf2 = 0;
    } else {
      kf2 = k2 > tn->x ? k2 - tn->x : tn->x - k2;
    }

    float m1 = 0.0f;

    if(k0 < tp->x) {
      m1 += ((mag1[k0])+(mag1[k0+1]))*(1.0f-kf0);
    } else {
      m1 += ((mag1[k0])+(mag1[k0+1]));
      m1 += ((mag1[k0-1])+(mag1[k0]))*kf0;
    }
    
    if(k2 < tn->x) {
      m1 += ((mag1[k2-1])+(mag1[k2]));
      m1 += ((mag1[k2])+(mag1[k2+1]))*kf2;
    } else {
      m1 += ((mag1[k2-1])+(mag1[k2]))*(1.0f-kf2);
    }
    
    for(int k=k0+1;k<k2-1;k++) {
      m1 += ((mag1[k])+(mag1[k+1]));
    }
    
    m1 *= 0.5f;

    for(int k=max(1,k1-peakWidth1+1);k<=k0;k++) {
      float d = (1.0f-kf1)*peak1[k-k1+N] + kf1*peak1[k-k1+N+ko1];
      float m = d*(p->y);
      m1 += m;
      dec[k] += m;
    }
    int k3 = min(k1+peakWidth1,Nover2-1);
    for(int k=k2;k<k3;k++) {
      float d = (1.0f-kf1)*peak1[k-k1+N] + kf1*peak1[k-k1+N+ko1];
      float m = d*(p->y);
      m1 += m;
      dec[k] += m;
    }
    p->m = m1;
  }

  peak *p0 = NULL;
  peak *p1 = NULL;

  for(list<peak *>::iterator tpi = peaks.begin();
      tpi != peaks.end(); ) {
    peak *p = (*tpi);

    list<peak*>::iterator eraseMe = tpi;
    peak *pp = NULL;
    if(tpi!=peaks.begin()) {
      pp = *(--tpi);
      tpi++;
    }
    peak *pn = NULL;
    tpi++;
    if(tpi != peaks.end()) {
      pn = (*tpi);
    }

    peak *tp;
    peak *tn;
    tp = p->tp;
    tn = p->tn;

    int k1 = p->k;
    int k0 = tp->k;
    float kf0;
    if(k0 < k1-peakWidth1) {
      k0 = k1-peakWidth1;
      kf0 = 0;
    } else {
      kf0 = k0 > tp->x ? k0 - tp->x : tp->x - k0;
    }
    int k2 = tn->k;
    float kf2;
    if(k2 > k1+peakWidth1) {
      k2 = k1+peakWidth1;
      kf2 = 0;
    } else {
      kf2 = k2 > tn->x ? k2 - tn->x : tn->x - k2;
    }

    float m = p->m;
    float mdec = 0.0f;

    if(k0 < tp->x) {
      mdec += (dec[k0]+dec[k0+1])*(1.0f-kf0);
    } else {
      mdec += (dec[k0]+dec[k0+1]);
      mdec += (dec[k0-1]+dec[k0])*kf0;
    }
    
    if(k2 < tn->x) {
      mdec += (dec[k2-1]+dec[k2]);
      mdec += (dec[k2]+dec[k2+1])*kf2;
    } else {
      mdec += (dec[k2-1]+dec[k2])*(1.0f-kf2);
    }
    
    for(int k=k0+1;k<k2-1;k++) {
      mdec += (dec[k]+dec[k+1]);
    }

    mdec *= 0.5;

    if(mdec > maxDecRatio*m) {
      peaks.erase(eraseMe);
      m -= mdec;
      if(pp && pn) {
        float d = square(pn->x - p->x) + square(pp->x - p->x);
        pp->m += m*square(pn->x - p->x)/d;
        pn->m += m*square(pp->x - p->x)/d;
      } else if(pp) {
        pp->m += m;
      } else if(pn) {
        pn->m += m;
      }
      continue;
    } else {
      m -= mdec;
      p->m = m;
      if(p0 == NULL && p->k >= kStart) {
        p0 = p;
      }
      if(p->k <= kEnd) {
        p1 = p;
      }
    }
  }

  if(p0 && p1) {
    int pk0 = p0->k;
    int pk1 = p1->k;
    int tk0 = p0->tp->k;
    int tk1 = p1->tn->k;
    float px0 = p0->x;
    float px1 = p1->x;
    float tx0 = p0->tp->x;
    float tx1 = p1->tn->x;   
    
    int pko0 = pk0 > px0 ? -1 : 1;
    float kf0 = pk0 > px0 ? pk0 - px0 : px0 - pk0;
    int pko1 = pk1 > px1 ? -1 : 1;
    float kf1 = pk1 > px1 ? pk1 - px1 : px1 - pk1;

    float m0 = 0.0f;
    float m1 = 0.0f;

    for(int k=tk0+1;k<tk1-1;k++) {
      m0 += mag0[k] + mag0[k+1];
      m1 += mag1[k] + mag1[k+1];
    }

    if(tk0 < pk0-peakWidth0) {
      int tk00 = pk0-peakWidth0;
      m0 += mag0[tk00] + mag0[tk00+1];
    } else {
      if(tk0 > tx0) {
        m0 += mag0[tk0] + mag0[tk0+1];
        m0 += (mag0[tk0-1] + mag0[tk0]) * (tk0 - tx0);
      } else {
        m0 += (mag0[tk0] + mag0[tk0+1]) * ((tk0 + 1) - tx0); 
      }
    }

    if(tk0 < pk0-peakWidth1) {
      int tk01 = pk0-peakWidth1;
      m1 += mag1[tk01] + mag1[tk01+1];
    } else {
      if(tk0 > tx0) {
        m1 += mag1[tk0] + mag1[tk0+1];
        m1 += (mag1[tk0-1] + mag1[tk0]) * (tk0 - tx0);
      } else {
        m1 += (mag1[tk0] + mag1[tk0+1]) * ((tk0 + 1) - tx0); 
      }
    }
    
    if(tk1 > pk1+peakWidth0) {
      int tk10 = pk1+peakWidth0;
      m0 += mag0[tk10-1] + mag0[tk10];
    } else {
      if(tk1 < tx1) {
        m0 += mag0[tk1-1] + mag0[tk1];
        m0 += (mag0[tk1] + mag0[tk1+1]) * (tx1 - tk1);
      } else {
        m0 += (mag0[tk1-1] + mag0[tk1]) * (tx1 - (tk1 - 1));
      }
    }
    
    if(tk1 > pk1+peakWidth1) {
      int tk11 = pk1+peakWidth1;
      m1 += mag1[tk11-1] + mag1[tk11];
    } else {
      if(tk1 < tx1) {
        m1 += mag1[tk1-1] + mag1[tk1];
        m1 += (mag1[tk1] + mag1[tk1+1]) * (tx1 - tk1);
      } else {
        m1 += (mag1[tk1-1] + mag1[tk1]) * (tx1 - (tk1 - 1));
      }
    }

    for(int k=max(1,pk0-peakWidth0+1);k<=tk0;k++) {
      float d = (1.0f-kf0)*peak0[k-pk0+N] + kf0*peak0[k-pk0+N+pko0];
      float m = d*(p0->y) / pad0;
      m0 += m;
    }

    for(int k=max(1,pk0-peakWidth1+1);k<=tk1;k++) {
      float d = (1.0f-kf0)*peak1[k-pk0+N] + kf0*peak1[k-pk0+N+pko0];
      float m = d*(p0->y);
      m1 += m;
    }

    int k3 = min(pk1+peakWidth0,Nover2-1);
    for(int k=tk1;k<k3;k++) {
      float d = (1.0f-kf1)*peak0[k-pk1+N] + kf1*peak0[k-pk1+N+pko1];
      float m = d*(p1->y) / pad0;
      m0 += m;
    }

    k3 = min(pk1+peakWidth1,Nover2-1);
    for(int k=tk1;k<k3;k++) {
      float d = (1.0f-kf1)*peak1[k-pk1+N] + kf1*peak1[k-pk1+N+pko1];
      float m = d*(p1->y);
      m1 += m;
    }
    
    float s = (m1 == 0.0f?1.0f:m0/m1);

    for(list<peak *>::iterator tpi = peaks.begin();
        tpi != peaks.end();
        tpi++) {
      peak *p = (*tpi);      
      p->m *= s;
    }
  }
}

}
