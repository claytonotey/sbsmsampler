#include "config.h"
#ifdef HAVE_PORTAUDIO

#include "sbsms.h"
#include "sbsmsplayer.h"
#include <math.h>
#include <stdlib.h>
#include <string.h>
#include "audiobuffer.h"

class PlayWriter {
public:
  sbsmsplayer *player;
  PlayWriter(sbsmsplayer *player) {
    this->player = player;
  }
  void write() {
    player->write();
  }
  void writingComplete() {
    player->writingComplete();
  }
};

void sbsmsplayer :: writingComplete()
{
  rb->flush();
  rb->writingComplete();
  pthread_mutex_lock(&writeMutex);
  sbsms->reset(iface);
  pthread_mutex_unlock(&writeMutex);
}

void *writeThreadCB(void *data) {
  PlayWriter *writer = (PlayWriter*)data;
  while(writer->player->isWriting()) {
    writer->write();
  }
  writer->writingComplete();
  pthread_exit(NULL);
  return NULL;
}

class PlayReader {
public:
  sbsmsplayer *player;
  PlayReader(sbsmsplayer *player) {
    this->player = player;
  }
  long read(float *buf, long n, bool bFlush) {
    return player->read(buf,n,bFlush);
  }
  void readingComplete() {
    player->rb->readingComplete();
  }
};

static int audioCB(const void *inputBuffer, void *outputBuffer,
                   unsigned long framesPerBuffer,
                   const PaStreamCallbackTimeInfo* timeInfo,
                   PaStreamCallbackFlags statusFlags,
                   void *userData )
{
  PlayReader *reader = (PlayReader*)userData;
  float *out = (float*)outputBuffer;
  int channels = reader->player->getChannels();
  unsigned long pos = 0;
  long ret = reader->player->isPlaying()?1:0;
  if(reader->player->isDonePlaying()) reader->readingComplete();
  bool bFlush;
  if(!reader->player->isWriting()) bFlush = true;
  else bFlush = false;
  long chunk;
  while(pos<framesPerBuffer && ret) {
    chunk = framesPerBuffer-pos;
    ret = reader->read(out+pos*channels,chunk,bFlush);
    pos += ret;
  }
  if(!ret) {
    memset(out+pos*channels,0,sizeof(float)*(framesPerBuffer-pos)*channels);
    reader->player->pause();
  }
  return paContinue;
}

sbsmsplayer :: sbsmsplayer()
{
  playWriter = new PlayWriter(this);
  playReader = new PlayReader(this);
  rb = NULL;
  fbuf = NULL;
  abuf = NULL;
  sbsms = NULL;
  iface = NULL;
  stream = NULL;
  Fs = 44100;
  denv = 1.0f / (0.001f * (real)Fs);
  SBSMS :: init(13);
  PaError err;
  err = Pa_Initialize();
  if( err != paNoError ) abort();
  bPlaying = false;
  bDonePlaying = false;
  bPlayedToEnd = false;
  bOpen = false;
  bWriteThread = false;
  pthread_mutex_init(&playMutex, NULL);
  pthread_mutex_init(&writeMutex, NULL);
  samplesPlayed = 0;
  volume = 0.9f;
  rate = 1.0f;
  pitch = 1.0f;
}

sbsmsplayer :: ~sbsmsplayer()
{
  PaError err;
  err = Pa_Terminate();
  if( err != paNoError ) abort();
  close();
  delete playWriter;
  delete playReader;
}


long sbsmsplayer :: read(float *buf, long n, bool bFlush)
{
  n = rb->read(buf,n,bFlush);
  samplesPlayed += n;
  return n;
}

long sbsmsplayer :: write()
{
  pthread_mutex_lock(&writeMutex);
  long nWrite = sbsms->synthFromFile(iface, abuf, blockSize);
  pthread_mutex_unlock(&writeMutex);
  audio_convert_from(fbuf,0,abuf,0,nWrite);
  if(nWrite) {
    if(channels==1) {
      for(int k=0;k<nWrite;k++) {	
        int k2 = k<<1;
        fbuf[k] = volume*env*fbuf[k2];
        env += denv;
        if(env > 1.0f) env = 1.0f;
      }
    } else if(channels==2) {
      for(int k=0;k<nWrite;k++) {
        int k2 = k<<1;
        fbuf[k2] = volume*env*fbuf[k2];
        fbuf[k2+1] = volume*env*fbuf[k2+1];
        env += denv;
        if(env > 1.0f) env = 1.0f;
      }
    }
    rb->write(fbuf,nWrite);
  } else {
    bWriting = false;
    bPlayedToEnd = true;
  }
  return nWrite;
}

bool sbsmsplayer :: play()
{
  if(!bOpen) return false;
  if(isPlaying()) return false;
  env = 0.0f;
  pthread_mutex_lock(&playMutex);
  bDonePlaying = false;
  bPlayedToEnd = false;
  bWriting = true;
  while(write() && !rb->isFull());
  bPlaying = true;
  int rc = pthread_create(&writeThread, NULL, writeThreadCB, (void*)playWriter);
  if(rc) {
    fprintf(stderr,"ERROR; return code from pthread_create() is %d\n", rc);
    exit(-1);
  }
  bWriteThread = true;
  pthread_mutex_unlock(&playMutex);
  return true;
}

bool sbsmsplayer :: pause()
{
  if(!isPlaying()) return false;
  bDonePlaying = true;
  bPlaying = false;
  bWriting = false;
  if(bWriteThread) {
    pthread_join(writeThread,NULL);
  }
  seek((countType)(getPos()*samplesToProcess));
  return true;
}

void sbsmsplayer :: close()
{
  if(stream) {
    PaError err;
    err = Pa_StopStream( stream );
    if( err != paNoError ) abort();
    err = Pa_CloseStream( stream );
    if( err != paNoError ) abort();
    stream = NULL;
  }
  if(bWriteThread) {
    pthread_join(writeThread,NULL);
  }
  if(rb) { delete rb; rb = NULL; }
  if(fbuf) { free(fbuf); fbuf = NULL; }
  if(abuf) { free(abuf); abuf = NULL; }
  if(sbsms) { delete sbsms; sbsms = NULL; }
  if(iface) { delete iface; iface = NULL; }
  bOpen = false;
  bWriting = false;
  bPlaying = false;
  bDonePlaying = false;
}

bool sbsmsplayer :: open(const char *filenameIn)
{
  sbsms = new SBSMS(filenameIn,&samplesToProcess,true,false);
  if(sbsms->getError()) {
    delete sbsms;
    sbsms = NULL;
    fprintf(stderr,"Cannot open file %s\n",filenameIn);
    return false;
  }
  sbsms->setLeftPos(0);
  sbsms->setRightPos(samplesToProcess);
  duration = (double)samplesToProcess / (double)Fs;
  iface = new SBSMSInterfaceVariableRate(samplesToProcess);
  iface->setRate(rate);
  iface->setPitch(pitch);
  channels = sbsms->getChannels();
  
  blockSize = sbsms->getInputFrameSize();
  rb = new AudioBuffer(8*blockSize,channels);
  fbuf = (float*)calloc(blockSize*2,sizeof(float));
  abuf = (audio*)calloc(blockSize,sizeof(audio));
  bOpen = true;
  bPlaying = false;
  bWriting = false;
  bDonePlaying = false;
  bPlayedToEnd = false;
  samplesPlayed = 0;
  samplesInOutput = (countType)((double)samplesToProcess/rate);
  timeConversion = (double)samplesToProcess/((double)samplesInOutput * (double)Fs);

  PaError err;
  PaStreamParameters outputParameters;
  outputParameters.device = Pa_GetDefaultOutputDevice();
  outputParameters.channelCount = channels;
  outputParameters.sampleFormat = paFloat32;
  outputParameters.suggestedLatency = (double)(4*blockSize)/(double)Fs;
  outputParameters.hostApiSpecificStreamInfo = NULL;
  PaStreamFlags flags = paNoFlag;
  err = Pa_OpenStream( &stream,
                       NULL,
                       &outputParameters,
                       Fs,
                       blockSize,
                       flags,
                       audioCB,
                       playReader);
  if( err != paNoError ) {
    fprintf(stderr,"pa error %d\n",err);
    abort();
  }
  err = Pa_StartStream( stream );
  if( err != paNoError ) {
    fprintf(stderr,"pa error %d\n",err);
    abort();
  }
  return true;
}

bool sbsmsplayer :: isPlaying()
{  
  return bPlaying;
}

bool sbsmsplayer :: isWriting()
{
  return bWriting;
}

bool sbsmsplayer :: isDonePlaying()
{
  return bDonePlaying;
}

bool sbsmsplayer :: isPlayedToEnd()
{
  return bDonePlaying && bPlayedToEnd;
}

int sbsmsplayer :: getChannels()
{
  return channels;
}

double sbsmsplayer :: getPos()
{
  return (double)samplesPlayed/(double)samplesInOutput;
}

double sbsmsplayer :: getTime()
{
  return (double)samplesPlayed * timeConversion;
}
 
double sbsmsplayer :: getDuration()
{
  return duration;
}

void sbsmsplayer :: seek(countType samplePos)
{
  pthread_mutex_lock(&writeMutex);
  sbsms->seek(iface,samplePos);
  pthread_mutex_unlock(&writeMutex);    
}

bool sbsmsplayer :: setPos(double pos)
{
  if(isPlaying()) return false;
  if(!bOpen) return false;
  countType samplePos = (countType)(pos*samplesToProcess);
  if(samplePos > samplesToProcess) samplePos = samplesToProcess;
  else if(samplePos < 0) samplePos = 0;
  pthread_mutex_lock(&playMutex);
  if(bWriteThread) {
    pthread_join(writeThread,NULL);
  }
  samplesInOutput = (countType)((double)samplesToProcess/rate);
  timeConversion = (double)samplesToProcess/((double)samplesInOutput * (double)Fs);
  samplesPlayed = (countType)(pos*(double)samplesToProcess/rate);
  if(pos < 1.0f) bDonePlaying = false;
  pthread_mutex_unlock(&playMutex);
  seek(samplePos);
  return true;
}

real sbsmsplayer :: getVolume()
{
  return volume;
}

void sbsmsplayer :: setVolume(real vol)
{
  this->volume = vol;
}

real sbsmsplayer :: getRate()
{
  return rate;
}
  
void sbsmsplayer :: setRate(real rate)
{
  this->rate = rate;
  if(iface) {
    iface->setRate(rate);
    if(rate == 0.0f) {
      rate = 1e-6f;
    }
    pthread_mutex_lock(&playMutex);
    double pos = (double)samplesPlayed/(double)samplesInOutput;
    samplesInOutput = (countType)((double)samplesToProcess / (double)rate);
    timeConversion = (double)samplesToProcess/((double)samplesInOutput * (double)Fs);
    samplesPlayed = (countType)(pos * (double)samplesToProcess / fabs((double)rate));
    pthread_mutex_unlock(&playMutex);
    printf("rate %lld %lld\n",samplesPlayed,samplesInOutput);
  }
}

real sbsmsplayer :: getPitch()
{
  return pitch;
}

void sbsmsplayer :: setPitch(real pitch)
{
  if(iface) iface->setPitch(pitch);
  this->pitch = pitch;
}

#endif
