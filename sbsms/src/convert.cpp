#include "config.h"
#include "grain.h"
#include "synth.h"
#include <math.h>
#include "convert.h"
#include "sbsms.h"
#include "real.h"
#include <stdlib.h>
#include <algorithm>
using namespace std;
using namespace _sbsms_;

#ifdef HAVE_SNDFILE
#include "import.h"
#include "pcm.h"
#define BWAV 1
#endif

#ifdef _WIN32
#define FSCANF fscanf_s
#define FOPEN(fp,fn,mode) fopen_s(&fp,fn,mode)
#else
#define FSCANF fscanf
#define FOPEN(fp,fn,mode) (fp = fopen(fn,mode))
#endif

class SBSMSInterfaceDecoderImp {
public:
  SBSMSInterfaceDecoderImp(Slide *rateSlide, Slide *pitchSlide, bool bPitchInputReference,
                           countType samples, long preSamples,
                           SBSMSQuality *quality, AudioDecoder *decoder, float pitch);

  virtual ~SBSMSInterfaceDecoderImp();
  inline long samples(audio *buf, long n);
  
  Resampler *preResampler;
  float *buf;
  audio *abuf;
  float pitch;
  countType samplesIn;
  long block;
#ifdef BWAV
  AudioDecoder *decoder;
#else
  FILE *decoder;
#endif
};

long convertResampleCB(void *cb_data, SBSMSFrame *data) 
{
   SBSMSInterfaceDecoderImp *iface = (SBSMSInterfaceDecoderImp*) cb_data;
#ifdef BWAV
  long n_read_in = iface->decoder->read(iface->buf,iface->block);
  audio_convert_to(iface->abuf,0,iface->buf,0,n_read_in);
#else
  long n_read_in = 0;
  for(int k=0;k<iface->block;k++) {
    if(FSCANF(iface->decoder,"%g %g\n",&(iface->abuf[k][0]),&(iface->abuf[k][1])) == EOF)
      break;
    n_read_in++;
  }
#endif  
  iface->samplesIn += n_read_in;
  data->size = n_read_in;
  data->ratio0 = iface->pitch;
  data->ratio1 = iface->pitch;
  data->buf = iface->abuf;

  return n_read_in;
}

SBSMSInterfaceDecoderImp :: SBSMSInterfaceDecoderImp(Slide *rateSlide, Slide *pitchSlide, bool bPitchInputReference,
                                                     countType samples, long preSamples,
                                                     SBSMSQuality *quality, AudioDecoder *decoder, float pitch)
{
  this->preResampler = new Resampler(convertResampleCB, this, pitch==1.0f?SlideIdentity:SlideConstant);
  this->pitch = pitch;
  this->decoder = decoder;
  this->block = quality->getFrameSize();
  samplesIn = 0;
  buf = (float*)calloc(block*2,sizeof(float));
  abuf = (audio*)calloc(block,sizeof(audio));
}

SBSMSInterfaceDecoderImp :: ~SBSMSInterfaceDecoderImp()
{
  free(buf);
  free(abuf);
  delete preResampler;
}
  
long SBSMSInterfaceDecoderImp :: samples(audio *buf, long n)
{ 
  return preResampler->read(buf, n);
}
  
SBSMSInterfaceDecoder :: SBSMSInterfaceDecoder(Slide *rateSlide, Slide *pitchSlide, bool bPitchInputReference,
                                               countType samples, long preSamples,
                                               SBSMSQuality *quality, AudioDecoder *decoder, float pitch) 
  : SBSMSInterfaceSliding(rateSlide,pitchSlide,bPitchInputReference,samples,preSamples,quality) 
{
  imp = new SBSMSInterfaceDecoderImp(rateSlide,pitchSlide,bPitchInputReference,samples,preSamples,quality,decoder,pitch);
}

SBSMSInterfaceDecoder :: ~SBSMSInterfaceDecoder()
{
  delete imp;
}

long SBSMSInterfaceDecoder :: samples(audio *buf, long n)
{
  return imp->samples(buf,n);
}

#ifndef BWAV
long lineCount(FILE *fp) {
  long n = 0;
  char c = 0;
  do {
    c = fgetc(fp);
    if (c == '\n') n++;
  } while (c != EOF);
  fseek(fp,0,SEEK_SET);
  return n;
}
#endif

bool sbsms_convert(const char *filenameIn, const char *filenameOut, bool bAnalyzeOnly, bool bSynthesizeOnly, bool bPreAnalyze, progress_cb progressCB, void *data, float rate0, float rate1, float pitch0, float pitch1, float volume)
{  
  bool status = true;
  float srOut = 44100.0f;
  int channels;
  countType samplesIn;
  float srIn;
  countType samplesToOutput;
  countType samplesToInput;
  bool bRead = false;
  bool bWrite = false;
  audio *abuf = NULL;
  float *fbuf = NULL;
  SBSMSInterface *iface = NULL;
  SBSMSInterface *ifacePre = NULL;
  SBSMS *sbsms = NULL;
  float preProgress = 0.0f;

  SBSMSQuality quality(&SBSMSQualityStandard);
  Slide rateSlide(SlideLinearInputRate,rate0,rate1);
  Slide pitchSlide(SlideLinearOutputRate,pitch0,pitch1);
  long blockSize;
#ifdef BWAV
  AudioDecoder *decoder = NULL;
  AudioDecoder *decoderPre = NULL;
#else
  FILE *decoder = NULL;
  FILE *decoderPre = NULL;
#endif
  if(bSynthesizeOnly) {
    samplesIn = 0;
    //sbsms = new SBSMS(filenameIn,&samplesToInput,true,false);   
    sbsms = new SBSMS(filenameIn,&samplesToInput,false,true);   
    if(sbsms->getError()) {
      printf("File: %s cannot be opened\n",filenameIn);
      status = false;
      goto cleanup;
    }
    bRead = true;
    channels = sbsms->getChannels();
    quality = *(sbsms->getQuality());
    iface = new SBSMSInterfaceSliding(&rateSlide,&pitchSlide,false,
                                      samplesToInput,0,&quality);
    while(sbsms->renderFrame(iface));
    sbsms = new SBSMS(sbsms);

    decoder = NULL;
  } else {
#ifdef BWAV
    decoder = import(filenameIn);
    if(!decoder) {
      printf("File: %s cannot be opened\n",filenameIn);
      exit(-1);
    }
    srIn = (float) decoder->getSampleRate();
    channels = decoder->getChannels();
    samplesIn = decoder->getFrames();
#else	
    FOPEN(decoder,filenameIn,"r");
    if(!decoder) {
      printf("File: %s cannot be opened\n",filenameIn);
      status = false;
      goto cleanup;
    }
    srIn = 44100.0;
    channels = 2;
    samplesIn = lineCount(decoder);
#endif
    samplesToInput = (countType) (samplesIn*srOut/srIn);    
    float pitch = (srOut == srIn?1.0f:(float)srOut / (float)srIn);
    iface = new SBSMSInterfaceDecoder(&rateSlide,&pitchSlide,false,
                                      samplesToInput,0,&quality,decoder,pitch);
    sbsms = new SBSMS(channels,&quality,bPreAnalyze,!bAnalyzeOnly);
  }
  
  if(bPreAnalyze && !bSynthesizeOnly) {
#ifdef BWAV
    decoderPre = import(filenameIn);
    if(!decoderPre) {
      printf("File: %s cannot be opened\n",filenameIn);
      status = false;
      goto cleanup;
    }
#else
    FOPEN(decoderPre,filenameIn,"r");
    if(!decoderPre) {
      printf("File: %s cannot be opened\n",filenameIn);
      status = false;
      goto cleanup;
    }
#endif
    float pitch = (srOut == srIn?1.0f:(float)srOut / (float)srIn);
    ifacePre = new SBSMSInterfaceDecoder(&rateSlide,&pitchSlide,false,
                                             samplesToInput,0,&quality,decoderPre,pitch);
  }
  
  samplesToOutput = iface->getSamplesToOutput();

  if(bPreAnalyze && !bSynthesizeOnly) {
    preProgress = 0.05f;
    sbsms->reset(ifacePre);
    countType pos = 0;
    countType lastPos = 0;
    long ret = 0;
    while(lastPos<samplesToInput) {
      long lastPercent=0;      
      ret = sbsms->preAnalyze(ifacePre);
      lastPos = pos;
      pos += ret;      
      float progress = (float)lastPos/(float)samplesToInput;
      progressCB(progress*preProgress,"Analysis",data);
    }
  }

  sbsms->reset(iface);

  blockSize = quality.getFrameSize();         

  if(bAnalyzeOnly) {
    countType pos = 0;
    countType lastPos = 0;
    long ret = -1;
    bool bOpen = sbsms->openWrite(filenameOut);
    if(!bOpen) {
      printf("File: %s cannot be opened\n",filenameOut);
      status = false;
      goto cleanup;
    }
    bWrite = true;
    bool bDone = false;
    while(!bDone) {
      long lastPercent=0;
      ret = sbsms->renderFrame(iface);
      bDone = !ret || lastPos >= samplesToOutput;
      lastPos = pos;
      pos += ret;
      float progress = (float)lastPos/(float)samplesToOutput;
      progressCB(preProgress + progress * (1.0f - preProgress),"Progress",data);
    }
    progressCB(1.0f,"Progress",data);
  } else {
    fbuf = (float*)calloc(blockSize*2,sizeof(float));
    abuf = (audio*)calloc(blockSize,sizeof(audio));    
#ifdef BWAV
    PcmWriter writer(filenameOut,samplesToOutput,(int)srOut,channels);
    if(writer.isError()) {
      printf("File: %s cannot be opened\n",filenameOut);
      status = false;
      goto cleanup;
    }
#else
    FILE *OUT;
	FOPEN(OUT,filenameOut,"w");
    if(!OUT) {
      printf("File: %s cannot be opened\n",filenameOut);
      status = false;
      goto cleanup;
    }
#endif

    countType pos = 0;
    long ret = -1;

    /*    
    sbsms->setLeftPos(0);
    sbsms->setRightPos(samplesToInput);
    int drop = 0;
    sbsms->seek(iface,drop);
    pos = drop;
    */

    float *in[2];
    in[0] = (float*)calloc(4096,sizeof(float));
    in[1] = (float*)calloc(4096,sizeof(float));

    while(pos<samplesToOutput) {
      //while(pos<samplesToOutput && ret) {
      long lastPercent=0;      
      //sbsms->writeInputs(in,blockSize,0);

      ret = sbsms->read(iface,abuf,blockSize);
      //SampleSynthesizer sampleSynth;
      //int spectN = 512;
      //VoiceSynthesizer voiceSynth(&sampleSynth,NULL,spectN);
      //ret = sbsms->synthFromMemory(iface,abuf,blockSize,&voiceSynth);
      //ret = abs(ret);
      //ret = sbsms->synthFromFile(iface,abuf,blockSize);
      //printf("ret %ld %lld %lld\n",ret,pos,samplesToOutput);

      if(pos+ret > samplesToOutput) {
        ret = (long)(samplesToOutput - pos);
      }
      audio_convert_from(fbuf,0,abuf,0,ret);
      if(channels==1) {
        for(int k=0;k<ret;k++) {
          int k2 = k<<1;
          fbuf[k] = volume*fbuf[k2];
        }
      } else if(channels==2) {
        for(int k=0;k<ret;k++) {
          int k2 = k<<1;
          fbuf[k2] = volume*fbuf[k2];
          fbuf[k2+1] = volume*fbuf[k2+1];
        }
      }
      
#ifdef BWAV
      if(ret) writer.write(fbuf, ret);
#else
      for(int k=0;k<ret;k++)
        fprintf(OUT,"%g %g\n",fbuf[2*k],fbuf[2*k+1]);
#endif
      pos += ret;
      
      float progress = (float)pos / (float)samplesToOutput;
      progressCB(progress,"Progress",data);
    }
    
#ifdef BWAV
    writer.close();
#else
    if(OUT) fclose(OUT);
#endif
  }
  
 cleanup:
#ifdef BWAV
  if(decoderPre) delete decoderPre;
  if(decoder) delete decoder;
#else
  if(decoderPre) fclose(decoderPre);
  if(decoder) fclose(decoder);
#endif
  if(fbuf) free(fbuf);
  if(abuf) free(abuf);
  if(bWrite) sbsms->closeWrite(iface);
  if(sbsms) delete sbsms;
  if(iface) delete iface;
  if(ifacePre) delete ifacePre;

  return status;
}
