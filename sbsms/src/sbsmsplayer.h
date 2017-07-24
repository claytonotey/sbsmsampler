#include "config.h"
#ifdef HAVE_PORTAUDIO

#ifndef SBSMSPLAYER_H
#define SBSMSPLAYER_H

#include "portaudio.h"
#include <pthread.h>
#include "audiobuffer.h"
#include <stdio.h>
#include "sbsms.h"

using namespace _sbsms_;

class PlayWriter;
class PlayReader;

class sbsmsplayer {
 public:
  sbsmsplayer();
  ~sbsmsplayer();

  bool open(const char *filename);
  void close();
  bool play();
  bool pause();
  bool isPlaying();
  bool isWriting();
  bool isDonePlaying();
  bool isPlayedToEnd();
  double getTime();
  double getDuration();
  int getChannels();
  double getPos();
  bool setPos(double pos);
  real getVolume();
  void setVolume(real vol);
  real getRate();
  void setRate(real rate);
  real getPitch();
  void setPitch(real pitch);

  friend class PlayWriter;
  friend class PlayReader;
 protected:
  PlayWriter *playWriter;
  PlayReader *playReader;
  void seek(countType n);
  long read(float *buf, long n, bool bFlush);
  long write();
  void writingComplete();
  int channels;
  AudioBuffer *rb;
  SBSMS *sbsms;
  SBSMSInterfaceVariableRate *iface;
  void setLength();
  countType samplesPlayed;
  countType samplesToProcess;
  countType samplesInOutput;
  real env;
  real denv;
  real volume;
  real rate;
  real pitch;
  double timeConversion;
  double duration;
  bool bWriteThread;
  bool bOpen;
  bool bPlaying;
  bool bDonePlaying;
  bool bPlayedToEnd;
  bool bWriting;
  PaStream *stream;
  int Fs;
  pthread_t writeThread;
  pthread_mutex_t playMutex;
  pthread_mutex_t writeMutex;
  long blockSize;
  float *fbuf;
  audio *abuf;
};

#endif

#endif
