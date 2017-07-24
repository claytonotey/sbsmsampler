#include "sbsms.h"
#include "voice.h"
#include <math.h>
#include "real.h"
#include <stdlib.h>
#include <string.h>
#include "audiobuffer.h"
#include "vstsbsms.h"
#include <pthread.h>
#include "sample.h"
#include "utils.h"
#include <assert.h>
#include <unistd.h>

#include <xmmintrin.h>

#include <algorithm>
using namespace std;

void brk() 
{
  usleep(10);
}

SBSMSVoice :: SBSMSVoice(SBSMSample *sample, int blockSize, float Fs, SampleSynthesizer *sampleSynth, int spectN) 
{
  //printf("spectN = %d\n",spectN);
  this->sample = sample;
  this->blockSize = blockSize;
  this->Fs = Fs;
  this->voiceSynth = new VoiceSynthesizer(sampleSynth,this,spectN);
  this->granuGrain = NULL;
  latencyOn = 0;
  latencyOff = 0;
  maxGrainSize = 0;
  blockPos = 0;
  currPos = 0.0f;
  currSample = 0;
  samplesToProcess = 0;
  channels = 1;
  rb = NULL;
  outBuf = NULL;
  fbuf = NULL;
  abuf = NULL;
  sbsms = NULL;
  sbsmsTransition = NULL;
  iface = NULL;
  transitionBuf[0] = transitionBuf[1] = NULL;
  bTransition = false;
  bPlaying = false;
  bOpen = false;
  bActive = false;
  bReady = false;
  pthread_mutex_init(&playMutex, NULL);
  pthread_mutex_init(&playCondMutex, NULL);
  pthread_mutex_init(&sbsmsMutex, NULL);
  pthread_cond_init(&playCond,NULL);
  pthread_mutex_init(&inputCondMutex, NULL);
  pthread_cond_init(&inputCond,NULL);
  currPos = 0.0f;
  leftPos = 0.0f;
  rightPos = 1.0f;
  startPos = 0.0f;
  startSample = 0;
  loop = loopForwardLoop;
  bLoop = false;
  volume = 1.0f;
  rate = 1.0f;
  pbendPitch = 1.0f;
  nInputSamples = 0;
  nInputSamples2 = 0;
  nTransition = 0;
  nDoneTransition = 0;
  nDoneTransition1 = 0;
  nDoneTransition2 = 0;
}

SBSMSVoice :: ~SBSMSVoice()
{
  delete voiceSynth;
  if(granuGrain) delete granuGrain;
  close();
}

int SBSMSVoice :: getSampleIndex()
{
  return sample->getIndex();
}

void SBSMSVoice :: wait()
{
  pthread_mutex_lock(&playCondMutex);      
  bReady = true;
  pthread_cond_wait(&playCond,&playCondMutex);
  bReady = false;
  pthread_mutex_unlock(&playCondMutex);
}

void *writeThreadCB(void *data) {
  SBSMSVoice *voice = (SBSMSVoice*)data;
  _mm_setcsr( _mm_getcsr() | 0x8040 );

  while(voice->isActive()) {
    if(!voice->isPlaying()) {
      voice->wait();
    }

    while(voice->isPlaying()) {
      voice->write();
    }
  
    voice->writingComplete();
  }

  pthread_exit(NULL);
  return NULL;
}

void SBSMSVoice :: writingComplete()
{
  blockPos = 0;
  rb->flush();
  rb->writingComplete();  
  pthread_mutex_lock(&sbsmsMutex);
  sbsms->reset(iface,true);
  sbsmsTransition->reset(iface,true);
  pthread_mutex_lock(&inputCondMutex);
  nInputSamples = 0;
  nInputSamples2 = 0;
  pthread_mutex_unlock(&inputCondMutex);
  pthread_mutex_unlock(&sbsmsMutex);
}

bool SBSMSVoice :: isActive()
{
  return bActive;
}

bool SBSMSVoice :: isReady()
{
  return bReady;
}

void SBSMSVoice :: triggerOn(float notePitch, float noteVol, float baseFreq, int latency)
{
  this->notePitch = notePitch;
  this->noteVol = noteVol;
  float pitch = notePitch*pbendPitch*44100.0f/Fs;
  if(iface) iface->setPitch(pitch);
  if(granuGrain) granuGrain->setBaseFreq(pitch, TWOPI*baseFreq/44100.0f);
  this->latencyOn = latency;
  this->latencyOff = 0;
  seekStart();
  bAttack = true;
  bRelease = false;
  env = 0.0f;
  play();
}

void SBSMSVoice :: triggerOff(int latency)
{
  if(latency) {
    this->latencyOff = latency;
  } else {
    bAttack = false;
    bRelease = true;
  }
}

void SBSMSVoice :: forceOff()
{
  env = 0.0;
  triggerOff(0);
}

void SBSMSVoice :: write()
{
  //printf("n1 = %ld\n",nInputSamples);

  pthread_mutex_lock(&inputCondMutex);
  if(nInputSamples <= 0) {
    pthread_cond_wait(&inputCond,&inputCondMutex);
  }
  pthread_mutex_unlock(&inputCondMutex);      

  //printf("n2 = %ld %ld\n",nInputSamples,nInputSamples2);

  pthread_mutex_lock(&sbsmsMutex);
  long nDone = 0;

  if(bTransition && !nTransition) {
    setupTransition();
    bTransition = false;
  }

  if(nTransition) {
    //long blockPos1 = blockPos;
    //long blockPos2 = blockPos;
    long n1 = -1;
    long n2 = -1;
    printf("transition start %ld %ld %ld\n",nInputSamples,nDoneTransition,nTransition);
    long nStart = nDoneTransition;

    while(n2 && nDoneTransition < nTransition) {

      pthread_mutex_lock(&inputCondMutex);
      n1 = min(nTransition-nDoneTransition1,blockSize - blockPos);
      n1 = min(n1,nInputSamples);
      pthread_mutex_unlock(&inputCondMutex);

      pthread_mutex_lock(&inputCondMutex);
      n2 = min(nTransition-nDoneTransition2,blockSize - blockPos);
      n2 = min(n2,nInputSamples);
      pthread_mutex_unlock(&inputCondMutex);

      printf("transition synth %ld %ld\n",n1,n2);
      n1 = sbsms->synthFromMemory(iface,bufTransition1+nDoneTransition1,n1,voiceSynth);

      bGoingBackwards = (rate < 0);      
      if(bGoingBackwards) {
        printf("n1 = %ld, n2 = %ld\n",n1,n2);
        brk();
      }

      n1 = abs(n1);
      if(n1 == 0) {
        if(bLoop) {    
          sbsms->reset(iface);
          if(bGoingBackwards) {
            sbsms->seek(iface,rightSample);
          } else {
            sbsms->seek(iface,leftSample);
          }
        } else {
          long nr = nTransition-nDoneTransition1;
          //printf("null n1 = %ld\n",n1);
          for(int c=0; c<channels; c++) {
            memset(bufTransition1[c]+nDoneTransition1,0,nr);
          }
        }
      }

      pthread_mutex_lock(&inputCondMutex);
      //nInputSamples -= n1;
      pthread_mutex_unlock(&inputCondMutex);
      //blockPos1 += n1;
      //while(blockPos1 >= blockSize) blockPos1 -= blockSize;

      n2 = sbsmsTransition->synthFromMemory(iface,bufTransition2+nDoneTransition2,n2,voiceSynth);
      bGoingBackwards = bGoingBackwards || (rate < 0);
      if(bGoingBackwards) {
        printf("n1 = %ld, n2 = %ld\n",n1,n2);
        brk();
      }
      n2 = abs(n2);
      
      printf("transition synthed %ld %ld\n",n1,n2);
      if(n2 == 0) {
        if(bLoop) {    
          sbsmsTransition->reset(iface);
          if(bGoingBackwards) {
            sbsmsTransition->seek(iface,rightSample);
          } else {
            sbsmsTransition->seek(iface,leftSample);
          }
        } else {
          
        }
      }
      pthread_mutex_lock(&inputCondMutex);
      nInputSamples -= n2;
      pthread_mutex_unlock(&inputCondMutex);


      blockPos += n2;
      while(blockPos >= blockSize) blockPos -= blockSize;
    
      nDoneTransition1 += n1;
      nDoneTransition2 += n2;
      long n = min(nDoneTransition1,nDoneTransition2) - nDoneTransition;
      nDoneTransition = min(nDoneTransition1,nDoneTransition2);
      
      nDone += n;
      printf("transitioned %ld %ld %ld\n",nDoneTransition, nDoneTransition1,nDoneTransition2);
    }

    printf("t1 = %g\n",transition1);
    printf("nDone = %ld\n",nDone);
    blockPos += nDone;
    while(blockPos >= blockSize) blockPos -= blockSize;      
    float dTransition = 1.0f / (float)nTransition;
    for(int i=0; i<nDone; i++) {
      for(int c=0; c<channels; c++) {
        abuf[i][c] = transition1 * bufTransition1[nStart+i][c] + transition2 * bufTransition2[nStart+i][c];
      }
      transition1 -= dTransition;
      transition2 += dTransition;
    }

    printf("t2 = %g\n",transition1);
    if(nDoneTransition == nTransition) {
      //swap sbsms
      SBSMS *tmp = sbsms;
      sbsms = sbsmsTransition;
      sbsmsTransition = tmp;
      startSample = nextStartSample;
      nTransition = 0;
    }
  } else {
    long n = blockSize - blockPos;
    pthread_mutex_lock(&inputCondMutex);
    n = min(n,nInputSamples);
    pthread_mutex_unlock(&inputCondMutex);
    n = sbsms->synthFromMemory(iface,abuf,n,voiceSynth);
    
    //printf("n = %ld\n",n);
    bGoingBackwards = (rate < 0);
    
    if(bGoingBackwards) {
      printf("n = %ld\n",n);
      brk();
    }

    nDone = abs(n);
    sbsmsTransition->readInputs(NULL, nDone, true);

    pthread_mutex_lock(&inputCondMutex);
    nInputSamples -= nDone;
    pthread_mutex_unlock(&inputCondMutex);
    blockPos += nDone;
    if(blockPos >= blockSize) blockPos -= blockSize;
    if(nDone == 0) {
      if(bLoop) {    
        sbsms->reset(iface);
        if(bGoingBackwards) {
          sbsms->seek(iface,rightSample);
        } else {
          sbsms->seek(iface,leftSample);
        }
      } else if(bStill) {
        nDone = blockSize - blockPos;
        memset(abuf,0,nDone*sizeof(audio));
      } else {
        rb->writingComplete();
      }
    }
  }
  currSample = sbsms->getSamplePosFromMemory();
  float rate = this->rate * 44100.0f / Fs;
  if(rate != 0.0f) {
    currSample -= (int)((float)rb->getSamplesQueued() * rate);
    currSample = max(leftSample,min(rightSample,currSample));
  }
  currPos = (float)currSample / (float)samplesToProcess;
  pthread_mutex_unlock(&sbsmsMutex);
  if(nDone) {
    audio_convert_from(fbuf,0,abuf,0,nDone,channels);
    rb->write(fbuf,nDone);    
  }
}

void SBSMSVoice :: preprocess(int samples)
{
  pthread_mutex_lock(&sbsmsMutex);
  if(sbsms) sbsms->particles(iface,voiceSynth);
  pthread_mutex_unlock(&sbsmsMutex);
}

void SBSMSVoice :: resume()
{
}

void SBSMSVoice :: process(float **inputs, float *out, int samples, grain *sideGrain)
{ 
  int ret = isPlaying()?1:0;
  int offset = min(latencyOn,samples);
  int chunk = samples - offset;
  latencyOn -= offset;

  //printf("writeIn1 %d\n",chunk);
  pthread_mutex_lock(&sbsmsMutex);
  if(sbsms) sbsms->writeInputs(inputs, chunk, offset);
  if(granuGrain) granuGrain->writeInputs(inputs, chunk, offset);
  if(voiceSynth) voiceSynth->setSideGrain(sideGrain,samples);
  if(sbsmsTransition) sbsmsTransition->writeInputs(inputs, chunk, offset);
  pthread_mutex_unlock(&sbsmsMutex);

  pthread_mutex_lock(&inputCondMutex);
  nInputSamples += chunk;
  pthread_cond_broadcast(&inputCond);
  pthread_mutex_unlock(&inputCondMutex);



  bool bStop = false;  
  int pos = offset;
  while(pos<samples && ret) {
    chunk = samples-pos;
    ret = rb->read(outBuf+pos*channels,chunk);
    pos += ret;
    if(!ret) bStop = true;
  }
  for(int k=offset;k<pos;k++) {
    float f = env*volume*noteVol;
    int k2 = k<<1;
    if(channels == 2) {
      out[k2] += f*outBuf[k2];
      out[k2+1] += f*outBuf[k2+1];
    } else {
      out[k2] += f*outBuf[k];
      out[k2+1] += f*outBuf[k];
    }
    if(volume < sample->volume) {
      volume *= volUp;
      if(volume > sample->volume) volume = sample->volume;
    } else if(volume > sample->volume) {
      volume *= volDown;
      if(volume < sample->volume) volume = sample->volume;
    }
    if(bAttack) {
      env += sample->dattack;
      if(env > 1.0f) {
        bAttack = false;
        env = 1.0f;
      }
    } else if(bRelease) {
      env -= sample->drelease;
      if(env < 0.0f)  {
         bStop = true;
         env = 0.0f;
      }
    }
    if(latencyOff) {
      if(latencyOff == 1) {
        bAttack = false;
        bRelease = true;
      }
      latencyOff--;
    }
  }
  if(bStop) {   
    stop();
  }
}

bool SBSMSVoice :: play()
{
  if(!bOpen) return false;
  bPlaying = true;
  nTransition = 0;
  pthread_cond_broadcast(&playCond);  
  return true;
}

bool SBSMSVoice :: stop()
{
  bPlaying = false;
  if(bOpen) {
    rb->readingComplete();
  }
  return true;
}

void SBSMSVoice :: close()
{
  if(bActive) {
    bActive = false;
    stop();
    pthread_cond_broadcast(&playCond);
    pthread_join(writeThread, NULL);
  }

  if(rb) { delete rb; rb = NULL; }
  if(outBuf) { free(outBuf); outBuf = NULL; }
  if(abuf) { free(abuf); abuf = NULL; }
  if(fbuf) { free(fbuf); fbuf = NULL; }
  pthread_mutex_lock(&sbsmsMutex);
  if(sbsms) { delete sbsms; sbsms = NULL; }
  if(sbsmsTransition) { delete sbsmsTransition; sbsmsTransition = NULL; }
  if(iface) { delete iface; iface = NULL; }
  for(int c = 0; c<channels; c++) {
    if(transitionBuf[c]) { delete transitionBuf[c]; transitionBuf[c] = NULL; }
  }
  pthread_mutex_unlock(&sbsmsMutex);

  bOpen = false;
  bPlaying = false;
  bActive = false;
}

bool SBSMSVoice :: open(SBSMS *sbsmsSrc, countType samplesToProcess)
{
  pthread_mutex_lock(&sbsmsMutex);
  sbsms = new SBSMS(sbsmsSrc);
  sbsmsTransition = new SBSMS(sbsmsSrc);
  this->samplesToProcess = samplesToProcess;
  iface = new SBSMSInterfaceVariableRate(samplesToProcess);
  pthread_mutex_unlock(&sbsmsMutex);
  iface->setRate(rate);
  channels = sbsms->getChannels();
  if(granuGrain) delete granuGrain;
  granuGrain = new Granugrain(channels,maxGrainSize);
  voiceSynth->setGrain(granuGrain);
  bOpen = true;
  setLeftPos(leftPos);
  setRightPos(rightPos);
  startSample = (countType)(startPos*samplesToProcess);
  setLoop(loop);
  setBlockSize(blockSize);
  setSampleRate(Fs);
  currPos = 0.0f;
  currSample = 0;
  nTransition = 0;
  for(int c=0; c<channels; c++) {
    if(transitionBuf[c]) { delete transitionBuf[c]; }
    transitionBuf[c] = new ArrayRingBuffer<float>(512);
  }
  bActive = true;
  int rc = pthread_create(&writeThread, NULL, writeThreadCB, (void*)this);
  if(rc) abort();

  return true;
}

bool SBSMSVoice :: isPlaying()
{  
  return bPlaying;
}

void SBSMSVoice :: setLeftPos(float pos)
{
  pthread_mutex_lock(&sbsmsMutex);
  leftPos = pos;
  leftSample = (countType)(pos*samplesToProcess);
  if(sbsms) sbsms->setLeftPos(leftSample);
  if(sbsmsTransition) sbsmsTransition->setLeftPos(leftSample);
  pthread_mutex_unlock(&sbsmsMutex);
}

void SBSMSVoice :: setRightPos(float pos)
{
  pthread_mutex_lock(&sbsmsMutex);
  rightPos = pos;
  rightSample = (countType)(pos*samplesToProcess);
  if(sbsms) sbsms->setRightPos(rightSample);
  if(sbsmsTransition) sbsmsTransition->setRightPos(rightSample);
  pthread_mutex_unlock(&sbsmsMutex);
}

countType SBSMSVoice :: getCurrSample()
{
  return currSample;
}

float SBSMSVoice :: getCurrPos()
{
  return currPos;
}

void SBSMSVoice :: setStartSample(countType samplePos)
{
  nextStartSample = samplePos;
  startSample = samplePos;
}

void SBSMSVoice :: setTime(VstTimeInfo *time)
{
  maxGrainSize = lrintf(time->sampleRate * 60.0/time->tempo * time->timeSigNumerator);
}

void SBSMSVoice :: setupTransition()
{
  sbsmsTransition->reset(iface,false);
  sbsmsTransition->seek(iface,nextStartSample);

  pthread_mutex_lock(&inputCondMutex);
  transition1 = 1.0f;
  transition2 = 0.0f;
  nDoneTransition = 0;
  nDoneTransition1 = 0;
  nDoneTransition2 = 0;
  //nInputSamples2 = nin;
  nTransition = min(blockSize,min(256L,lrintf(Fs*0.004f)));
  pthread_mutex_unlock(&inputCondMutex);
}

void SBSMSVoice :: setStartPos(float pos)
{
  pthread_mutex_lock(&sbsmsMutex);
  startPos = pos;
  nextStartSample = (countType)(pos*samplesToProcess);
  if(sbsms) {
    if(isPlaying()) {
      if(!nTransition) {
        //printf("start pos\n");
        startSample = nextStartSample;
        bTransition = true;
      }
    } else {
      startSample = nextStartSample;
      sbsms->reset(iface);
      sbsms->seek(iface,startSample);
    }
  } else {
    startSample = nextStartSample;
  }

  pthread_mutex_unlock(&sbsmsMutex);
}

void SBSMSVoice :: setLoop(int loop)
{
  this->loop = loop;
  bLoop = (loop == loopForwardLoop || loop == loopBackwardLoop || loop == loopStillLoop);
  bBackwards = (loop == loopBackward || loop == loopBackwardLoop);
  if(bBackwards && startSample == 0) {
    startSample = rightSample;
  } else if(!bBackwards && startSample == rightSample) {
    startSample = leftSample;
  }
  bStill = (loop == loopStill || loop == loopStillLoop);
}

void SBSMSVoice :: seekStart()
{
  pthread_mutex_lock(&sbsmsMutex);
  if(sbsms) {
    sbsms->reset(iface);
    sbsms->seek(iface,startSample);
    granuGrain->reset(maxGrainSize);
  }
  pthread_mutex_unlock(&sbsmsMutex);
}

void SBSMSVoice :: setBlockSize(int value)
{
  this->blockSize = value;
  this->blockPos = 0;
  if(rb) delete rb; 
  rb = new AudioBuffer(blockSize*4,channels);
  if(outBuf) free(outBuf);
  outBuf = (float*)calloc(blockSize*channels,sizeof(float)); 
  if(fbuf) free(fbuf);
  fbuf = (float*)calloc(blockSize*channels,sizeof(float));
  if(abuf) free(abuf);
  abuf = (audio*)calloc(blockSize,sizeof(audio));
}

void SBSMSVoice :: setSampleRate(float value)
{
  this->Fs = value;
  if(iface) iface->setRate(rate*44100.0f/Fs);
  if(iface) iface->setPitch(notePitch*pbendPitch*44100.0f/Fs);
  volUp = exp(1.0f / (0.004f * Fs));
  volDown = 1.0 / volUp;
}

void SBSMSVoice :: setPitchbend(float pbendPitch)
{
  this->pbendPitch = pbendPitch;
  if(iface) iface->setPitch(notePitch*pbendPitch*44100.0f/Fs);
  if(granuGrain) granuGrain->setPitchbend(pbendPitch);
}
  
void SBSMSVoice :: setRate(float rate)
{
  this->rate = rate;
  if(iface) iface->setRate(rate*44100.0f/Fs);
}
