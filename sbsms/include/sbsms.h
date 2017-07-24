#ifndef SBSMS_INCLUDE
#define SBSMS_INCLUDE

#include <stdio.h>

#define SBSMS_VERSION 10800L

namespace _sbsms_ {

class Track;

typedef long long int countType;
typedef float audio[2];

audio *make_audio_buf(long);
void free_audio_buf(audio *);
long copy_audio_buf(audio *, long off1, audio *, long off2, long n);
long audio_convert_from(float *to, long off1, audio *from, long off2, long n, int channels = 2);
long audio_convert_to(audio *to, long off1, float *from, long off2, long n, int channels = 2);

struct SBSMSQualityParams {
  int bands;
  int H;
  int N[10];
  int N0[10];
  int N1[10];
  int N2[10];
  int res[10];
};

class SBSMSQuality {
 public:
  SBSMSQuality(const SBSMSQualityParams *params);
  SBSMSQualityParams params;
  long getFrameSize();
  long getMaxPresamples();
};

extern const SBSMSQualityParams SBSMSQualityStandard;
extern const SBSMSQualityParams SBSMSQualityFast;

struct SBSMSFrame {
  float ratio0;
  float ratio1;
  audio *buf;
  long size;
};

class SBSMSynthesizer {
 public:
  virtual ~SBSMSynthesizer() {}
  virtual void particleStep(int c, int n, Track *t)=0;
  virtual float particleForce(int c, float w, float m)=0;
  virtual void particleInit(int c)=0;
  virtual void particlePopulate(int c, float offset, float pitch, countType synthtime, Track *t)=0;
  virtual bool isPopulateRequired()=0;
  virtual bool synth(int c,
                     float *in,
             float *out,
             countType synthtime,
             float h2,
             float offset,
             int n,
             float fScale0,
             float fScale1,
             Track *t)=0;

};

typedef long (*SBSMSResampleCB)(void *cbData, SBSMSFrame *frame);

class SBSMSInterface {
 public:
  virtual ~SBSMSInterface() {}
  virtual long samples(audio *buf, long n) { return 0; }
  virtual float getStretch(float t)=0;
  virtual float getPitch(float t)=0;
  virtual long getPresamples()=0;
  virtual countType getSamplesToInput()=0;
  virtual countType getSamplesToOutput()=0;
};

class SBSMSRendererImp;
class SBSMS;
class SBSMSImp;

class SBSMSRenderer {
 public:
  SBSMSRenderer();
  virtual ~SBSMSRenderer();
  virtual void render(int c, countType samplePos, long nSamples, float f0, float f1, float y0, float y1)=0;
  friend class SBSMSImp;
  friend class SBSMS;
 protected:
  SBSMSRendererImp *imp;
};

enum SBSMSError {
  SBSMSErrorNone = 0,
  SBSMSErrorFileOpen,
  SBSMSErrorFileNotSet,
  SBSMSErrorInvalidRate
};

template<class T>
class ArrayRingBuffer;

class SBSMS {
 public:
  static void init(int size);
  static void init(int size, int n, float *tab);
  static void finalize();

  SBSMS(int channels, SBSMSQuality *quality, bool bPreAnalyze, bool bSynthesize);
  SBSMS(const char *filename, countType *samplesToInput, bool bSynthesize, bool bKeepInMemory);
  SBSMS(SBSMS *source);
  ~SBSMS();

  long read(SBSMSInterface *iface, audio *buf, long n);
  void particles(SBSMSInterface *iface, SBSMSynthesizer *synth);
  long synthFromMemory(SBSMSInterface *iface, audio *buf, long n, SBSMSynthesizer *synth);
  long synthFromFile(SBSMSInterface *iface, audio *buf, long n);
  void addRenderer(SBSMSRenderer *renderer);
  void removeRenderer(SBSMSRenderer *renderer);
  long renderFrame(SBSMSInterface *iface);
  long preAnalyze(SBSMSInterface *iface);
  void reset(SBSMSInterface *iface, bool flushInput = false);
  void seek(SBSMSInterface *iface, countType samplePos);
  void prepadInputs(int samples);
  void writeInputs(float **inputs, int samples, int offset);
  long readInputs(ArrayRingBuffer<float> *buf[2], int samples, bool bAdvance = true);
  void setLeftPos(countType pos);
  void setRightPos(countType pos);
  countType getSamplePosFromMemory();
  long getInputFrameSize();
  SBSMSError getError();
  
  bool isReadOpen();
  void closeRead();
  bool isWriteOpen();
  bool openWrite(const char *filename);
  void closeWrite(SBSMSInterface *iface);

  unsigned int getVersion();
  int getChannels();
  SBSMSQuality *getQuality();

  friend class SBSMSImp;
 protected:
  SBSMSImp *imp;
};

enum SlideType {
  SlideIdentity = 0,
  SlideConstant,
  SlideLinearInputRate,
  SlideLinearOutputRate,
  SlideLinearInputStretch,
  SlideLinearOutputStretch,
  SlideGeometricInput,
  SlideGeometricOutput
};

class SlideImp;

class Slide {
 public:
  Slide(SlideType slideType, float rate0 = 1.0f, float rate1 = 1.0f, countType n = 0);
  ~Slide();
  float getTotalStretch();
  float getStretchedTime(float t);
  float getRate(float t);
  float getStretch(float t);
  float getRate();
  float getStretch();
  void step();
 protected:
  SlideImp *imp;
};

class SBSMSInterfaceVariableRateImp;

class SBSMSInterfaceVariableRate : public SBSMSInterface {
public:
  SBSMSInterfaceVariableRate(countType samplesToInput);
  virtual ~SBSMSInterfaceVariableRate();
  virtual float getStretch(float t);
  virtual float getPitch(float t);
  void setRate(float rate);
  void setPitch(float pitch);
  virtual long getPresamples();
  virtual countType getSamplesToInput();
  virtual countType getSamplesToOutput();

  friend class SBSMSInterfaceVariableRateImp;
protected:
  SBSMSInterfaceVariableRateImp *imp;
};
 
class SBSMSInterfaceSlidingImp;

class SBSMSInterfaceSliding : public SBSMSInterface {
public:
  SBSMSInterfaceSliding(Slide *rateSlide, Slide *pitchSlide, 
                        bool bPitchReferenceInput, 
                        countType samplesToInput, long preSamples,
                        SBSMSQuality *quality);
  virtual ~SBSMSInterfaceSliding();
  virtual float getStretch(float t);
  virtual float getPitch(float t);
  virtual long getPresamples();
  virtual countType getSamplesToInput();
  virtual countType getSamplesToOutput();

  friend class SBSMSInterfaceSlidingImp;
protected:
  SBSMSInterfaceSlidingImp *imp;
};


class SampleBuf;
class ResamplerImp;

class Resampler {
 public:
  Resampler(SBSMSResampleCB func, void *data, SlideType slideType = SlideConstant);
  Resampler(SampleBuf *in, float ratio, SlideType slideType = SlideConstant);
  ~Resampler();
  long read(audio *audioOut, long frames);
  void reset();
  long samplesInOutput();

 protected:
  ResamplerImp *imp;
};

}

#endif
