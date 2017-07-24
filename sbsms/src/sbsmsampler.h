#ifndef SBSMSAMPLER_H_
#define SBSMSAMPLER_H_

#include "audioeffectx.h"
#include "sbsmsamplergui.h"
#include "vstprogram.h"
#include "midi.h"
#include "vstsbsms.h"
#include "pthread.h"

#include <sys/stat.h>
#include <string>
#include <map>
#include <set>
#include <queue>
using namespace std;

class Renderer;
class Drawer;

class SBSMSampler : public AudioEffectX
{
 public:
	SBSMSampler(audioMasterCallback audioMaster);
	~SBSMSampler();

	virtual void setSampleRate (float sampleRate); 
	virtual void setBlockSize (VstInt32 blockSize);
	
	static bool bInit;
	///	Processes the input data in inputs, fills outputs with the plugin's output (replacing).
	virtual void processReplacing(float **inputs, float **outputs, int sampleFrames);
	///	Called from the host for every (audio) processing block.
	virtual int processEvents(VstEvents* ev);
	
	///	Fills data with the current preset data as a chunk, to send to the host.
	virtual int getChunk(void** data, bool isPreset);
	/// Sets the plugin's presets according to data.
	virtual int setChunk(void* data, int byteSize, bool isPreset);

	///	Sets the parameters according to the preset index, program.
	virtual void setProgram(int program);
	///	Sets the current program name.
	virtual void setProgramName(char *name);
	///	Fills name with the current program name.
	virtual void getProgramName(char *name);
	///	Fills text with the indexed program name.
	virtual bool getProgramNameIndexed(int category, int index, char* text);
	///	Copies the current program to the program indexed at destination.
	virtual bool copyProgram(int destination);

	///	Sets the indexed parameter to value.
	virtual void setParameter(int index, float value);
	///	Returns the indexed parameter value.
	virtual float getParameter(int index);
	///	Fills label with the indexed parameter's units (e.g. dB etc.).
	virtual void getParameterLabel(int index, char *label);
	///	Fills text with a textual representation of the indexed parameters current value.
	virtual void getParameterDisplay(int index, char *text);
	///	Fills text with the name of the indexed parameter.
	virtual void getParameterName(int index, char *text);
  virtual bool string2parameter(int index, char *text);

	///	Called by the host to determine whether the plugin can handle certain operations.
	virtual int canDo (char* text);
	///	Fills name with the plugin's name.
	virtual bool getEffectName(char* name);
	///	Fills text with the name of who made it.
	virtual bool getVendorString(char* text);
	///	Fills text with the name of the plugin.
	virtual bool getProductString(char* text);
	///	Returns the version of the plugin.
	virtual int getVendorVersion();

	virtual int getNumMidiInputChannels();
	virtual int getNumMidiOutputChannels();

  virtual void resume();
  
  int getSidebandSpectrumSize();

  void renderSampleFile(int index, const string &sbsmsname, const string &name);
  void convertAndRenderSampleFile(int index, const string &filename, const string &sbsmsname, const string &name);
  void drawSampleFile(int index);
  SBSMSample *newSample();
  void removeSample(int index);
  int getBlockSize();
  float getSampleRate();
  float getBeatPeriod();

  void destroyRenderer(int index);
  void destroyDrawer(int index);

  SBSMSamplerGUI *gui;
  friend class Renderer;
  friend class Drawer;
protected:	
  void process(float **inputs, float **outputs, int samples, int offset);
  void processBlock(float **inputs, float **outputs, int samples, int offset);
  void addVoice(SBSMSVoice *v);
  void removeVoice(SBSMSVoice *v);
  void stopRender(int index);
  void clearMaps(const string &sbsmsname);
  void drawSampleFile(Drawer *drawer);

 
  queue<int> tIndexQueue;
  set<string> fileRenderingSet;
  map<string,int> fileTIndexMap;
  map<string, set<int> > fileSampleIndexMap;
  map<string,SBSMS*> fileSBSMSMap;
  map<string,time_t> fileMTimeMap;
  map<string,countType> fileSamplesMap;
  SBSMSProgram* programs[NUM_PROGRAMS];
  int curProgram;
  SBSMSProgram* p;
  SBSMSVoice *voiceList;
  float *outBuf;
  audio *sideBuf;
  ArrayRingBuffer<float> *inputBuf[2];
  float beatPeriod;
  int nfft;
  int nstrided;
  int stride;
  int latency;
  int initialLatency;
  grain *sideGrain;
  GrainBuf *grainBuf;
  audio *x0;
  audio *x1;

  pthread_mutex_t masterRenderMutex;
  pthread_mutex_t renderMutex[NUM_SAMPLES];
  pthread_t renderThread[NUM_SAMPLES];
  Renderer *renderer[NUM_SAMPLES];
  bool bRenderThread[NUM_SAMPLES];

  pthread_mutex_t drawMutex[NUM_SAMPLES];
  pthread_t drawThread[NUM_SAMPLES];
  Drawer *drawer[NUM_SAMPLES];
  bool bDrawThread[NUM_SAMPLES];

  // MIDI essentials...
  BlockEvents *blockEvents;
  int numBlockEvents;
};

#endif
