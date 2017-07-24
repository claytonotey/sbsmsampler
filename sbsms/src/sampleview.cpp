#include "sampleview.h"
#include "convert.h"
#include "sbsmsampler.h"
#include "render.h"
#include <string>
#include <map>
using namespace std;


/*
  UI -> valueChanged -> sampler->setParameterAutomated -> sample->setParameter -> data.params -> sample->update
*/

SampleView :: SampleView(ProgramView *pv, CRect rect, CFrame *frame, CBitmap *bitmap) : CViewContainer(rect, frame, bitmap)
{
  setMode (kOnlyDirtyUpdate);
  this->pv = pv;
  this->sampler = pv->sampler;
  
  CRect size, size2;
  CPoint point;

  int x,y,w,h;
  int x2,y2,w2,h2;

  bmOnOff = new CBitmap(kOnOff);
  bmTempo = new CBitmap(kTempo);
  //bmSideband = new CBitmap(kSideband);

  // wave display
  bmPos = new CBitmap(kWaveSlider);
  size(248,11,675,135);
  waveDisplay = new CWaveDisplaySliders(frame, size, this, tagWaveDisplay, tagLeftPos, tagRightPos, tagStartPos, bmPos);
  addView(waveDisplay);

  // knobs
  bmKnob = new CBitmap(kKnob);

  x = 5;
  y = 101;
  w = bmKnob->getWidth();
  h = bmKnob->getHeight()/59;  
  int ht = h + 16;
  int wt = w + 10;

  // attack
  size(x, y, x+w, y+h);
  point(0, 0);
  knobAttack = new RightClickAnimKnob(size, this, tagAttack, 59, bmKnob->getHeight()/59, bmKnob, point);
  addView(knobAttack);

  x += 49;
  // release
  size(x, y, x+w, y+h);
  point(0, 0);
  knobRelease = new RightClickAnimKnob(size, this, tagRelease, 59, bmKnob->getHeight()/59, bmKnob, point);
  addView(knobRelease);

  x += 49;
  // rate  
  size(x, y, x+wt, y+ht);
  size2(x, y, x+w, y+h);
  point(0, 0);
  knobRate = new TempoCtrl(size, size2, this, frame, tagRate, 59, bmKnob->getHeight()/59, bmKnob, point, &tempo.s, bmTempo, tagRateMode);
  knobRate->setDefaultValue(0.5f);
  addView(knobRate);
  
  // next row
  x = 5;
  y = 164;

  // am freq 0
  size(x, y, x+wt, y+ht);
  size2(x, y, x+w, y+h);
  point(0, 0);
  knobAMFreq0 = new TempoCtrl(size, size2, this, frame, tagAMFreq0, 59, bmKnob->getHeight()/59, bmKnob, point, &tempo.s, bmTempo, tagAMFreq0Mode);
  knobAMFreq0->setDefaultValue(0.5f);
  addView(knobAMFreq0);

  x += 42;
  // am freq 1
  size(x, y, x+w, y+h);
  point(0, 0);
  knobAMFreq1 = new RightClickAnimKnob(size, this, tagAMFreq1, 59, bmKnob->getHeight()/59, bmKnob, point);
  knobAMFreq1->setDefaultValue(0.5f);
  addView(knobAMFreq1);

  // am button
  x2 = x + 10;
  y2 = y + 32;
  w2 = bmOnOff->getWidth();
  h2 = bmOnOff->getHeight()/2;
  size(x2, y2, x2+w2, y2+h2);
  onoffAMFreq1Mode = new COnOffButton(size, this, tagAMFreq1Mode, bmOnOff);
  addView(onoffAMFreq1Mode);
 
  x += 48;
  // FM freq 0
  size(x, y, x+wt, y+ht);
  size2(x, y, x+w, y+h);
  point(0, 0);
  knobFMFreq0 = new TempoCtrl(size, size2, this, frame, tagFMFreq0, 59, bmKnob->getHeight()/59, bmKnob, point, &tempo.s, bmTempo, tagFMFreq0Mode);
  knobFMFreq0->setDefaultValue(0.5f);
  addView(knobFMFreq0);

  x += 42;
  // FM freq 1
  size(x, y, x+w, y+h);
  point(0, 0);
  knobFMFreq1 = new RightClickAnimKnob(size, this, tagFMFreq1, 59, bmKnob->getHeight()/59, bmKnob, point);
  knobFMFreq1->setDefaultValue(0.5f);
  addView(knobFMFreq1);

  // FM button
  x2 = x + 10;
  y2 = y + 32;
  w2 = bmOnOff->getWidth();
  h2 = bmOnOff->getHeight()/2;
  size(x2, y2, x2+w2, y2+h2);
  onoffFMFreq1Mode = new COnOffButton(size, this, tagFMFreq1Mode, bmOnOff);
  addView(onoffFMFreq1Mode);

  x += 48;
  // Sidband mod
  size(x, y, x+w, y+h);
  point(0, 0);
  knobSidebandMod = new RightClickAnimKnob(size, this, tagSidebandMod, 59, bmKnob->getHeight()/59, bmKnob, point);
  knobSidebandMod->setDefaultValue(0.5f);
  addView(knobSidebandMod);

  x += 48;
  // Sidband BW
  size(x, y, x+w, y+h);
  point(0, 0);
  knobSidebandBW = new RightClickAnimKnob(size, this, tagSidebandBW, 59, bmKnob->getHeight()/59, bmKnob, point);
  knobSidebandBW->setDefaultValue(0.5f);
  addView(knobSidebandBW);

  x += 48;
  // Sidband BW Scale
  size(x, y, x+w, y+h);
  point(0, 0);
  knobSidebandBWScale = new RightClickAnimKnob(size, this, tagSidebandBWScale, 59, bmKnob->getHeight()/59, bmKnob, point);
  knobSidebandBWScale->setDefaultValue(0.5f);
  addView(knobSidebandBWScale);

  x += 48;
  // Sideband Release
  size(x, y, x+w, y+h);
  point(0, 0);
  knobSidebandRelease = new RightClickAnimKnob(size, this, tagSidebandRelease, 59, bmKnob->getHeight()/59, bmKnob, point);
  knobSidebandRelease->setDefaultValue(0.5f);
  addView(knobSidebandRelease);

  x += 48;
  // Track Threshold
  size(x, y, x+w, y+h);
  point(0, 0);
  knobThresh = new RightClickAnimKnob(size, this, tagThresh, 59, bmKnob->getHeight()/59, bmKnob, point);
  knobThresh->setDefaultValue(0.0f);
  addView(knobThresh);

  // Second row
  x = 5;
  y = 232;
  // am depth 0
  size(x, y, x+w, y+h);
  point(0, 0);
  knobAMDepth0 = new RightClickAnimKnob(size, this, tagAMDepth0, 59, bmKnob->getHeight()/59, bmKnob, point);
  knobAMDepth0->setDefaultValue(0.0f);
  addView(knobAMDepth0);

   x += 42;
  // am depth 1
  size(x, y, x+w, y+h);
  point(0, 0);
  knobAMDepth1 = new RightClickAnimKnob(size, this, tagAMDepth1, 59, bmKnob->getHeight()/59, bmKnob, point);
  knobAMDepth1->setDefaultValue(0.5f);
  addView(knobAMDepth1);

  x2 = x + 10;
  y2 = y + 32;
  w2 = bmOnOff->getWidth();
  h2 = bmOnOff->getHeight()/2;
  size(x2, y2, x2+w2, y2+h2);
  onoffAMDepth1Mode = new COnOffButton(size, this, tagAMDepth1Mode, bmOnOff);
  addView(onoffAMDepth1Mode);

   x += 48;
  // FM depth 0
  size(x, y, x+w, y+h);
  point(0, 0);
  knobFMDepth0 = new RightClickAnimKnob(size, this, tagFMDepth0, 59, bmKnob->getHeight()/59, bmKnob, point);
  knobFMDepth0->setDefaultValue(0.0f);
  addView(knobFMDepth0);

   x += 42;
  // FM depth 1
  size(x, y, x+w, y+h);
  point(0, 0);
  knobFMDepth1 = new RightClickAnimKnob(size, this, tagFMDepth1, 59, bmKnob->getHeight()/59, bmKnob, point);
  knobFMDepth1->setDefaultValue(0.5f);
  addView(knobFMDepth1);

  x2 = x + 10;
  y2 = y + 32;
  w2 = bmOnOff->getWidth();
  h2 = bmOnOff->getHeight()/2;
  size(x2, y2, x2+w2, y2+h2);
  onoffFMDepth1Mode = new COnOffButton(size, this, tagFMDepth1Mode, bmOnOff);
  addView(onoffFMDepth1Mode);



  x += 48;
  // Mod Pivot
  size(x, y, x+w, y+h);
  point(0, 0);
  knobModPivot = new RightClickAnimKnob(size, this, tagModPivot, 59, bmKnob->getHeight()/59, bmKnob, point);
  knobModPivot->setDefaultValue(0.5f);
  addView(knobModPivot);

  x += 48;
  // Sideband Env 
  size(x, y, x+w, y+h);
  point(0, 0);
  knobSidebandEnv = new RightClickAnimKnob(size, this, tagSidebandEnv, 59, bmKnob->getHeight()/59, bmKnob, point);
  knobSidebandEnv->setDefaultValue(0.0f);
  addView(knobSidebandEnv);

  x += 48;
  // Particle Force
  size(x, y, x+w, y+h);
  point(0, 0);
  knobForce = new RightClickAnimKnob(size, this, tagForce, 59, bmKnob->getHeight()/59, bmKnob, point);
  knobForce->setDefaultValue(0.0f);
  addView(knobForce);

  x += 48;
  // Particle Drag
  size(x, y, x+w, y+h);
  point(0, 0);
  knobDrag = new RightClickAnimKnob(size, this, tagDrag, 59, bmKnob->getHeight()/59, bmKnob, point);
  knobDrag->setDefaultValue(0.0f);
  addView(knobDrag);

  x += 48;
  // Particle Spring
  size(x, y, x+w, y+h);
  point(0, 0);
  knobSpring = new RightClickAnimKnob(size, this, tagSpring, 59, bmKnob->getHeight()/59, bmKnob, point);
  knobSpring->setDefaultValue(0.0f);
  addView(knobSpring);

  x = 13;
  y = 41;
  h = 12;
  w = 28;

  // key base
  size(x, y, x+w, y+h);
  optionKeyBase = new COptionMenu(size, this, tagKeyBase, 0, 0, kPopupStyle);
  for(int k=0;k<12;k++)
    optionKeyBase->addEntry(keyName[k],-1);
  addView(optionKeyBase);
	optionKeyBase->setFont(kNormalFontSmall);

  x += 49;
  // key lo
  size(x, y, x+w, y+h);
  optionKeyLo = new COptionMenu(size, this, tagKeyLo, 0, 0, kPopupStyle);
  for(int k=0;k<12;k++)
    optionKeyLo->addEntry(keyName[k],-1);
  addView(optionKeyLo);
	optionKeyLo->setFont(kNormalFontSmall);

  x += 49;
  // key hi
  size(x, y, x+w, y+h);
  optionKeyHi = new COptionMenu(size, this, tagKeyHi, 0, 0, kPopupStyle);
  for(int k=0;k<12;k++)
    optionKeyHi->addEntry(keyName[k],-1);
  addView(optionKeyHi);
	optionKeyHi->setFont(kNormalFontSmall);

  x = 13;
  y = 60;
  // octave base
  size(x, y, x+w, y+h);
  optionOctaveBase = new COptionMenu(size, this, tagOctaveBase, 0, 0, kPopupStyle);
  for(int k=-2;k<=8;k++) {
    char text[8];
    sprintf(text,"%d",k);
    optionOctaveBase->addEntry(text,-1);
  }
  addView(optionOctaveBase);
	optionOctaveBase->setFont(kNormalFontSmall);

  x += 49;

  // octave lo
  size(x, y, x+w, y+h);
  optionOctaveLo = new COptionMenu(size, this, tagOctaveLo, 0, 0, kPopupStyle);
  for(int k=-2;k<=8;k++) {
    char text[8];
    sprintf(text,"%d",k);
    optionOctaveLo->addEntry(text,-1);
  }
  addView(optionOctaveLo);
	optionOctaveLo->setFont(kNormalFontSmall);

  x += 49;
  
  // octave hi
  size(x, y, x+w, y+h);
  optionOctaveHi = new COptionMenu(size, this, tagOctaveHi, 0, 0, kPopupStyle);
  for(int k=-2;k<=8;k++) {
    char text[8];
    sprintf(text,"%d",k);
    optionOctaveHi->addEntry(text,-1);
  }
  addView(optionOctaveHi);
	optionOctaveHi->setFont(kNormalFontSmall);

  x = 159;
  y = 42;
 
  // channel
  size(x, y, x+w, y+h);
  optionChannel = new COptionMenu(size, this, tagChannel, 0, 0, kPopupStyle);
  for(int k=1;k<=16;k++) {
    char text[8];
    sprintf(text,"%d",k);
    optionChannel->addEntry(text,-1);
  }
  optionChannel->setFont(kNormalFontSmall);
  addView(optionChannel);

  y = 73;
  // pitchbend range
  size(x, y, x+w, y+h);
  optionPitchbendRange = new COptionMenu(size, this, tagPitchbendRange, 0, 0, kPopupStyle);
  for(int k=1;k<=24;k++) {
    char text[8];
    sprintf(text,"%d",k);
    optionPitchbendRange->addEntry(text,-1);
  }
  optionPitchbendRange->setFont(kNormalFontSmall);
  addView(optionPitchbendRange);
  
  y = 104;
  // loop switch
  size(x, y, x+w, y+h);
  optionLoop = new COptionMenu(size, this, tagLoop, 0, 0, kPopupStyle); 
  optionLoop->addEntry((char*)"|>|",-1);
  optionLoop->addEntry((char*)"|<|",-1);  
  optionLoop->addEntry((char*)"><",-1);  
  optionLoop->addEntry((char*)"<>",-1);
  optionLoop->addEntry((char*)"|-|",-1);
  optionLoop->addEntry((char*)">-<",-1);
  optionLoop->setFont(kNormalFontSmall);
  addView(optionLoop);

  // follow mode on/off
  x = 164;
  y = 134;
  w = bmOnOff->getWidth();
  h = bmOnOff->getHeight()/2;
  size(x, y, x+w, y+h);
  onoffFollowMode = new COnOffButton(size, this, tagFollowMode, bmOnOff);
  addView(onoffFollowMode);
  
  // volume 
  bmSlider = new CBitmap(kSlider);

  x = 210;
  y = 44;
  w = bmSlider->getWidth();
  h = 104;

  size(x, y, x+w, y+h);
  point(0,0);
  sliderVolume = new RightClickVSlider(size, this, tagVolume, y, y+h-bmSlider->getHeight(), bmSlider, NULL, point);
  addView(sliderVolume);
  
  // file
  bmBrowse = new CBitmap(kBrowse);

  x = 208;
  y = 5;
  w = bmBrowse->getWidth();
  h = bmBrowse->getHeight()/2;

  size(x, y, x+w, y+h);
  point(0,0);
  buttonOpen = new CKickButton(size, this, tagOpen, bmBrowse, point);
  addView(buttonOpen);

  x = 32;
  y = 6;
  w = 172;
  h = 12;

  size(x, y, x+w, y+h);
  textFile = new CStaticText(size);
  textFile->setFontColor(kWhiteCColor);
  textFile->setTransparency(false);
  addView(textFile);
	textFile->setFont(kNormalFontSmall);

  // synth mode
  x = 456;
  y = 148;
  h = 12;
  w = 96;
  size(x, y, x+w, y+h);
  optionSynthMode = new COptionMenu(size, this, tagSynthMode, 0, 0, kPopupStyle);
  optionSynthMode->addEntry((char*)"oscillate",-1);
  optionSynthMode->addEntry((char*)"bandpass",-1);
  optionSynthMode->addEntry((char*)"comb",-1);
  optionSynthMode->addEntry((char*)"decimate",-1);
  optionSynthMode->addEntry((char*)"granulate",-1);
  optionSynthMode->addEntry((char*)"dwgs",-1);
  addView(optionSynthMode);
	optionSynthMode->setFont(kNormalFontSmall);

  bmOscCtrl = new CBitmap(kOscCtrl);
  bmBandpassCtrl = new CBitmap(kBandpassCtrl);
  bmCombCtrl = new CBitmap(kCombCtrl);
  bmDecimateCtrl = new CBitmap(kDecimateCtrl);
  bmGranCtrl = new CBitmap(kGranCtrl);
  bmDWGSCtrl = new CBitmap(kDWGSCtrl);

  x = 428;
  y = 162;
  w = bmOscCtrl->getWidth();
  h = bmOscCtrl->getHeight();
  size(x,y,x+w,y+h);

  genCtrl[0] = oscCtrl = new OscCtrl(this,size,frame,bmOscCtrl);
  genCtrl[1] = bandpassCtrl = new BandpassCtrl(this,size,frame,bmBandpassCtrl);
  genCtrl[2] = combCtrl = new CombCtrl(this,size,frame,bmCombCtrl);
  genCtrl[3] = decimateCtrl = new DecimateCtrl(this,size,frame,bmDecimateCtrl);
  genCtrl[4] = granCtrl = new GranCtrl(this,size,frame,bmGranCtrl);
  genCtrl[5] = dwgsCtrl = new DWGSCtrl(this,size,frame,bmDWGSCtrl);
  
  currGenCtrl = oscCtrl;
  //addView(currGenCtrl);
}

SampleView :: ~SampleView()
{
  debug("~sv\n");
  for(int k=0; k<6; k++) {
    if(genCtrl[k] != currGenCtrl) {
      genCtrl[k]->forget();
    }
  }
  bmOscCtrl->forget();
  bmBandpassCtrl->forget();
  bmCombCtrl->forget();
  bmDecimateCtrl->forget();
  bmGranCtrl->forget();
  bmDWGSCtrl->forget();
  bmOnOff->forget();
  bmPos->forget();
	bmBrowse->forget();
	bmKnob->forget();
	bmSlider->forget();
  bmTempo->forget();
}

#ifdef WINDOWS
char seperator[2] = "\\";
#else
char seperator[2] = "/";
#endif

bool getBaseName(char *basename, char *path)
{
  char *subpath = path;
  char *next = subpath;
  while(next) {
    next = strstr(subpath,seperator);
    if(next) subpath = next+1;
  }
  strcpy(basename, subpath);
  char *suffix = strstr(basename, ".");
  if(suffix) *suffix = 0;
  return true;
}

bool getDirName(char *dirname, char *path)
{
  char *subpath = path;
  char *next = subpath;
  while(next) {
    next = strstr(subpath,seperator);
    if(next) subpath = next+1;
  }
  strcpy(dirname,path);
  dirname[subpath-path] = 0;
  return true;
}

bool getSuffix(char *suffix, char *path)
{
  char *subpath = path;
  char *next = subpath;
  while(next) {
    next = strstr(subpath,".");
    if(next) subpath = next+1;
  }
  strcpy(suffix,subpath);
  return true;
}

void SampleView :: selectFile()
{
  VstFileSelect vfs;
  
  VstFileType waveType("WAV File", ".wav", "wav", "wav", "wav", "audio/wav");
  VstFileType aiffType("AIFF File", ".aiff", "aiff", "aiff", "aiff", "audio/aiff");
  VstFileType aifType("AIF File", ".aif", "aif", "aif", "aif", "audio/aif");
  VstFileType mp3Type("MP3 File", ".mp3", "mp3", "mp3", "mp3", "audio/mp3");
  VstFileType sbsmsType("SBSMS File", ".sbsms", "sbsms", "sbsms", "sbsms", "audio/sbsms");
  VstFileType fileTypes[] = {waveType,aiffType,aifType,mp3Type,sbsmsType};
  
  vfs.command = kVstFileLoad;
  vfs.type = kVstFileType;
  vfs.nbFileTypes = 5;
  vfs.returnMultiplePaths = 0;
  vfs.fileTypes   = (VstFileType*)&fileTypes;
  vfs.returnPath  = new char[4096];
  vfs.sizeReturnPath = 4096;
  vfs.initialPath = 0;
  vfs.future[0] = 1;	// utf-8 path on macosx
  memset(vfs.returnPath,0,4096*sizeof(char));
  strcpy(vfs.title, "Choose an Audio File");

  debug("select 0\n");
  CFileSelector fs(sampler);
  fs.run(&vfs);
  char *filename = vfs.returnPath;
  if(*filename == 0) {
    delete [] vfs.returnPath;
    return;
  }
  printf("%s\n",filename);
    
  debug("select 1\n");
  char suffix[4096];
  getSuffix(suffix, filename);

  char basename[4096];
  getBaseName(basename, filename);

  char sbsmsname[4096];

  if( strcmp(suffix,"sbsms") != 0) {
    getDirName(sbsmsname, filename);
    strcat(sbsmsname,basename);
    strcat(sbsmsname,".sbsms");
    sampler->convertAndRenderSampleFile(index,string(filename),string(sbsmsname),string(basename));
  } else {
    strcpy(sbsmsname, filename);
    sampler->renderSampleFile(index, string(sbsmsname), string(basename));
  }

  delete [] vfs.returnPath;
}

SBSMSDrawRenderer *SampleView :: getNewWaveRenderer(countType samplesToProcess)
{
  return waveDisplay->getNewRenderer(samplesToProcess);
}

void SampleView :: setWaveRenderer(SBSMSDrawRenderer *r)
{
  waveDisplay->setRenderer(r);
}

SBSMSDrawRenderer *SampleView :: getWaveRenderer()
{
  return waveDisplay->getCurrentRenderer();
}

void SampleView :: updateRender()
{
  waveDisplay->setDirty2(true);
}

void SampleView :: setCurrPos(const set<float> &pos)
{
  waveDisplay->setCurrPos(pos);
}

void SampleView :: setSampleFile(const string &sbsmsname, const string &name)
{
  unsigned int maxLength = 38;
  if(sbsmsname.length() > maxLength) {
    char str[1024] = "...";
    strcat(str,sbsmsname.c_str()+sbsmsname.length()-(maxLength-3));
    textFile->setText(str);
  } else {
    textFile->setText((char*)sbsmsname.c_str());
  }
}

void SampleView :: valueChanged(CDrawContext* context, CControl* control)
{
  char txt[128];
	long tag = control->getTag();
  switch(tag) {
  case tagOpen:
    if(control->getValue() == 1.0)
      selectFile();
    break;
  case tagVolume:
    sampler->setParameterAutomated(index*NUM_PARAMS+pVolume, sliderVolume->getValue());
    break;
  case tagAttack:
    sampler->setParameterAutomated(index*NUM_PARAMS+pAttack, knobAttack->getValue());
    break;
  case tagRelease:
    sampler->setParameterAutomated(index*NUM_PARAMS+pRelease, knobRelease->getValue());
    break;
  case tagRate:
    sampler->setParameterAutomated(index*NUM_PARAMS+pRate, knobRate->getValue());
    sampler->getParameterDisplay(index*NUM_PARAMS+pRate, txt);
    knobRate->setText(txt);
    break;
  case tagRateMode:
    sampler->setParameterAutomated(index*NUM_PARAMS+pRateMode, knobRate->getMode());
    break;
  case tagAMFreq0:
    sampler->setParameterAutomated(index*NUM_PARAMS+pAMFreq0, knobAMFreq0->getValue());
    sampler->getParameterDisplay(index*NUM_PARAMS+pAMFreq0, txt);
    knobAMFreq0->setText(txt);
    break;
  case tagAMFreq0Mode:
    sampler->setParameterAutomated(index*NUM_PARAMS+pAMFreq0Mode, knobAMFreq0->getMode());
    break;
  case tagAMFreq1:
    sampler->setParameterAutomated(index*NUM_PARAMS+pAMFreq1, knobAMFreq1->getValue());
    break;
  case tagAMFreq1Mode:
    sampler->setParameterAutomated(index*NUM_PARAMS+pAMFreq1Mode, onoffAMFreq1Mode->getValue());
    break;
  case tagAMDepth0:
    sampler->setParameterAutomated(index*NUM_PARAMS+pAMDepth0, knobAMDepth0->getValue());
    break;
  case tagAMDepth1:
    sampler->setParameterAutomated(index*NUM_PARAMS+pAMDepth1, knobAMDepth1->getValue());
    break;
  case tagAMDepth1Mode:
    sampler->setParameterAutomated(index*NUM_PARAMS+pAMDepth1Mode, onoffAMDepth1Mode->getValue());
    break;
  case tagFMFreq0:
    sampler->setParameterAutomated(index*NUM_PARAMS+pFMFreq0, knobFMFreq0->getValue());
    sampler->getParameterDisplay(index*NUM_PARAMS+pFMFreq0, txt);
    knobFMFreq0->setText(txt);
    break;
  case tagFMFreq0Mode:
    sampler->setParameterAutomated(index*NUM_PARAMS+pFMFreq0Mode, knobFMFreq0->getMode());
    break;
  case tagFMFreq1:
    sampler->setParameterAutomated(index*NUM_PARAMS+pFMFreq1, knobFMFreq1->getValue());
    break;
  case tagFMFreq1Mode:
    sampler->setParameterAutomated(index*NUM_PARAMS+pFMFreq1Mode, onoffFMFreq1Mode->getValue());
    break;
  case tagFMDepth0:
    sampler->setParameterAutomated(index*NUM_PARAMS+pFMDepth0, knobFMDepth0->getValue());
    break;
  case tagFMDepth1:
    sampler->setParameterAutomated(index*NUM_PARAMS+pFMDepth1, knobFMDepth1->getValue());
    break;
  case tagFMDepth1Mode:
    sampler->setParameterAutomated(index*NUM_PARAMS+pFMDepth1Mode, onoffFMDepth1Mode->getValue());
    break;
  case tagDistortion0:
    sampler->setParameterAutomated(index*NUM_PARAMS+pDistortion0, oscCtrl->knobDistortion0->getValue());
    break;
  case tagDistortion1:
    sampler->setParameterAutomated(index*NUM_PARAMS+pDistortion1, oscCtrl->knobDistortion1->getValue());
    break;
  case tagDistortion1Mode:
    sampler->setParameterAutomated(index*NUM_PARAMS+pDistortion1Mode, oscCtrl->onoffDistortion1Mode->getValue());
    break;
  case tagFilterQ0:
    sampler->setParameterAutomated(index*NUM_PARAMS+pFilterQ0, bandpassCtrl->knobFilterQ0->getValue());
    break;
  case tagFilterQ1:
    sampler->setParameterAutomated(index*NUM_PARAMS+pFilterQ1, bandpassCtrl->knobFilterQ1->getValue());
    break;
  case tagFilterQ1Mode:
    sampler->setParameterAutomated(index*NUM_PARAMS+pFilterQ1Mode, bandpassCtrl->onoffFilterQ1Mode->getValue());
    break;
  case tagSidebandMod:
    sampler->setParameterAutomated(index*NUM_PARAMS+pSidebandMod, knobSidebandMod->getValue());
    break;
  case tagSidebandBW:
    sampler->setParameterAutomated(index*NUM_PARAMS+pSidebandBW, knobSidebandBW->getValue());
    break;
  case tagSidebandBWScale:
    sampler->setParameterAutomated(index*NUM_PARAMS+pSidebandBWScale, knobSidebandBWScale->getValue());
    break;
  case tagSidebandRelease:
    sampler->setParameterAutomated(index*NUM_PARAMS+pSidebandRelease, knobSidebandRelease->getValue());
    break;
  case tagSidebandEnv:
    sampler->setParameterAutomated(index*NUM_PARAMS+pSidebandEnv, knobSidebandEnv->getValue());
    break;
  case tagModPivot:
    sampler->setParameterAutomated(index*NUM_PARAMS+pModPivot, knobModPivot->getValue());
    break;
  case tagPitchbendRange:
    sampler->setParameterAutomated(index*NUM_PARAMS+pPitchbendRange, optionPitchbendRange->getCurrent()/24.0f);
    break;
  case tagLeftPos:
    sampler->setParameterAutomated(index*NUM_PARAMS+pLeftPos, waveDisplay->getLeftPos());
    break;
  case tagRightPos:
    sampler->setParameterAutomated(index*NUM_PARAMS+pRightPos, waveDisplay->getRightPos());
    break; 
  case tagStartPos:
    sampler->setParameterAutomated(index*NUM_PARAMS+pStartPos, waveDisplay->getStartPos());
    break;
  case tagLoop:
    sampler->setParameterAutomated(index*NUM_PARAMS+pLoop, optionLoop->getValue()/6.0f);
    break; 
  case tagFollowMode:
    sampler->setParameterAutomated(index*NUM_PARAMS+pFollowMode, onoffFollowMode->getValue()/2.0f);
    break;
  case tagKeyBase:
    sampler->setParameterAutomated(index*NUM_PARAMS+pKeyBase, optionKeyBase->getCurrent()/12.0f);
    break;
  case tagOctaveBase:
    sampler->setParameterAutomated(index*NUM_PARAMS+pOctaveBase, optionOctaveBase->getCurrent()/11.0f);
    break;
  case tagKeyLo:
    sampler->setParameterAutomated(index*NUM_PARAMS+pKeyLo, optionKeyLo->getCurrent()/12.0f);
    break;
  case tagOctaveLo:
    sampler->setParameterAutomated(index*NUM_PARAMS+pOctaveLo, optionOctaveLo->getCurrent()/11.0f);
    break;
  case tagKeyHi:
    sampler->setParameterAutomated(index*NUM_PARAMS+pKeyHi, optionKeyHi->getCurrent()/12.0f);
    break;
  case tagOctaveHi:
    sampler->setParameterAutomated(index*NUM_PARAMS+pOctaveHi, optionOctaveHi->getCurrent()/11.0f);
    break;
  case tagChannel:
    sampler->setParameterAutomated(index*NUM_PARAMS+pChannel, optionChannel->getCurrent()/16.0f);
    break;
  case tagSynthMode:
    sampler->setParameterAutomated(index*NUM_PARAMS+pSynthMode, optionSynthMode->getCurrent()/6.0f);
    break;
  case tagCombFB0:
    sampler->setParameterAutomated(index*NUM_PARAMS+pCombFB0, combCtrl->knobCombFB0->getValue());
    break;
  case tagCombFB1:
    sampler->setParameterAutomated(index*NUM_PARAMS+pCombFB1, combCtrl->knobCombFB1->getValue());
    break;
  case tagCombFB1Mode:
    sampler->setParameterAutomated(index*NUM_PARAMS+pCombFB1Mode, combCtrl->onoffCombFB1Mode->getValue());
    break;
  case tagDecBits0:
    sampler->setParameterAutomated(index*NUM_PARAMS+pDecBits0, decimateCtrl->knobDecBits0->getValue());
    break;
  case tagDecBits1:
    sampler->setParameterAutomated(index*NUM_PARAMS+pDecBits1, decimateCtrl->knobDecBits1->getValue());
    break;
  case tagDecBits1Mode:
    sampler->setParameterAutomated(index*NUM_PARAMS+pDecBits1Mode, decimateCtrl->onoffDecBits1Mode->getValue());
    break;
  case tagGranMode:
    sampler->setParameterAutomated(index*NUM_PARAMS+pGranMode, granCtrl->optionGranMode->getCurrent()/6.0f);
    break;
  case tagGranRate0:
    sampler->setParameterAutomated(index*NUM_PARAMS+pGranRate0, granCtrl->knobGranRate0->getValue());
    break;
  case tagGranRate1:
    sampler->setParameterAutomated(index*NUM_PARAMS+pGranRate1, granCtrl->knobGranRate1->getValue());
    break;
  case tagGranRate1Mode:
    sampler->setParameterAutomated(index*NUM_PARAMS+pGranRate1Mode, granCtrl->onoffGranRate1Mode->getValue());
    break;
  case tagGranSmooth0:
    sampler->setParameterAutomated(index*NUM_PARAMS+pGranSmooth0, granCtrl->knobGranSmooth0->getValue());
    break;
  case tagGranSmooth1:
    sampler->setParameterAutomated(index*NUM_PARAMS+pGranSmooth1, granCtrl->knobGranSmooth1->getValue());
    break;
  case tagGranSmooth1Mode:
    sampler->setParameterAutomated(index*NUM_PARAMS+pGranSmooth1Mode, granCtrl->onoffGranSmooth1Mode->getValue());
    break;
  case tagDWGSDecay0:
    sampler->setParameterAutomated(index*NUM_PARAMS+pDWGSDecay0, dwgsCtrl->knobDWGSDecay0->getValue());
    break;
  case tagDWGSDecay1:
    sampler->setParameterAutomated(index*NUM_PARAMS+pDWGSDecay1, dwgsCtrl->knobDWGSDecay1->getValue());
    break;
  case tagDWGSDecay1Mode:
    sampler->setParameterAutomated(index*NUM_PARAMS+pDWGSDecay1Mode, dwgsCtrl->onoffDWGSDecay1Mode->getValue());
    break;
  case tagDWGSLopass0:
    sampler->setParameterAutomated(index*NUM_PARAMS+pDWGSLopass0, dwgsCtrl->knobDWGSLopass0->getValue());
    break;
  case tagDWGSLopass1:
    sampler->setParameterAutomated(index*NUM_PARAMS+pDWGSLopass1, dwgsCtrl->knobDWGSLopass1->getValue());
    break;
  case tagDWGSLopass1Mode:
    sampler->setParameterAutomated(index*NUM_PARAMS+pDWGSLopass1Mode, dwgsCtrl->onoffDWGSLopass1Mode->getValue());
    break;
  case tagDWGSStringPos0:
    sampler->setParameterAutomated(index*NUM_PARAMS+pDWGSStringPos0, dwgsCtrl->knobDWGSStringPos0->getValue());
    break;
  case tagDWGSStringPos1:
    sampler->setParameterAutomated(index*NUM_PARAMS+pDWGSStringPos1, dwgsCtrl->knobDWGSStringPos1->getValue());
    break;
  case tagDWGSStringPos1Mode:
    sampler->setParameterAutomated(index*NUM_PARAMS+pDWGSStringPos1Mode, dwgsCtrl->onoffDWGSStringPos1Mode->getValue());
    break;
  case tagThresh:
    sampler->setParameterAutomated(index*NUM_PARAMS+pThresh, knobThresh->getValue());
    break;
  case tagForce:
    sampler->setParameterAutomated(index*NUM_PARAMS+pForce, knobForce->getValue());
    break;
  case tagDrag:
    sampler->setParameterAutomated(index*NUM_PARAMS+pDrag, knobDrag->getValue());
    break;
  case tagSpring:
    sampler->setParameterAutomated(index*NUM_PARAMS+pSpring, knobSpring->getValue());
    break;
  }    
}

void SampleView :: setGenCtrl(int mode)
{
  removeView(currGenCtrl,false);
  currGenCtrl = genCtrl[mode];
  addView(currGenCtrl);
}

void SampleView :: setParameter(int index, float value)
{
  int param = index%NUM_PARAMS;

  switch(param) {
  case pAttack:
    knobAttack->setValue(value);
    break;
  case pRelease:
    knobRelease->setValue(value);
    break;
  case pRate:
    knobRate->setValue(value);
    break;
  case pRateMode:
    knobRate->setMode(lrintf(value));
    break;
  case pAMFreq0:
    knobAMFreq0->setValue(value);
    break;
  case pAMFreq0Mode:
    knobAMFreq0->setMode(lrintf(value));
    break;
  case pAMFreq1:
    knobAMFreq1->setValue(value);
    break;
  case pAMFreq1Mode:
    onoffAMFreq1Mode->setValue(value);
    break;
  case pAMDepth0:
    knobAMDepth0->setValue(value);
    break;
  case pAMDepth1:
    knobAMDepth1->setValue(value);
    break;
  case pAMDepth1Mode:
    onoffAMDepth1Mode->setValue(lrintf(value));
    break;
  case pFMFreq0:
    knobFMFreq0->setValue(value);
    break;
  case pFMFreq0Mode:
    knobFMFreq0->setMode(lrintf(value));
    break;
  case pFMFreq1:
    knobFMFreq1->setValue(value);
    break;
  case pFMFreq1Mode:
    onoffFMFreq1Mode->setValue(lrintf(value));
    break;
  case pFMDepth0:
    knobFMDepth0->setValue(value);
    break;
  case pFMDepth1:
    knobFMDepth1->setValue(value);
    break;
  case pFMDepth1Mode:
    onoffFMDepth1Mode->setValue(lrintf(value));
    break;
  case pDistortion0:
    oscCtrl->knobDistortion0->setValue(value);
    break;
  case pDistortion1:
    oscCtrl->knobDistortion1->setValue(value);
    break;
  case pDistortion1Mode:
    oscCtrl->onoffDistortion1Mode->setValue(lrintf(value));
    break;
  case pFilterQ0:
    bandpassCtrl->knobFilterQ0->setValue(value);
    break;
  case pFilterQ1:
    bandpassCtrl->knobFilterQ1->setValue(value);
    break;
  case pFilterQ1Mode:
    bandpassCtrl->onoffFilterQ1Mode->setValue(lrintf(value));
    break;
  case pSidebandMod:
    knobSidebandMod->setValue(value);
    break;
  case pSidebandBW:
    knobSidebandBW->setValue(value);
    break;
  case pSidebandBWScale:
    knobSidebandBWScale->setValue(value);
    break;
  case pSidebandRelease:
    knobSidebandRelease->setValue(value);
    break;
  case pSidebandEnv:
    knobSidebandEnv->setValue(value);
    break;
  case pModPivot:
    knobModPivot->setValue(value);
    break;
  case pVolume:
    sliderVolume->setValue(value);
    break;
  case pKeyBase:
    optionKeyBase->setValue(value*12.0f);
    break;
  case pOctaveBase:
    optionOctaveBase->setValue(value*11.0f);
    break;
  case pKeyLo:
    optionKeyLo->setValue(value*12.0f);
    break;
  case pOctaveLo:
    optionOctaveLo->setValue(value*11.0f);
    break;
  case pKeyHi:
    optionKeyHi->setValue(value*12.0f);
    break;
  case pOctaveHi:
    optionOctaveHi->setValue(value*11.0f);
    break;
  case pChannel:
    optionChannel->setValue(value*16.0f);
    break;
  case pPitchbendRange:
    optionPitchbendRange->setValue(value*24.0f);
    break;
  case pLeftPos:
    waveDisplay->setLeftPos(value);
    break;
  case pRightPos:
    waveDisplay->setRightPos(value);
    break;
  case pStartPos:
    waveDisplay->setStartPos(value);
    break;
  case pLoop:
    optionLoop->setValue(value*6.0f);
    waveDisplay->setLoop(lrintf(value*6.0f));
    break;  
  case pFollowMode:
    onoffFollowMode->setValue(value*2.0f);
    break;
  case pSynthMode:
    optionSynthMode->setValue(value*6.0f);
    setGenCtrl(lrintf(value*6.0f));
    break;
  case pCombFB0:
    combCtrl->knobCombFB0->setValue(value);
    break;
  case pCombFB1:
    combCtrl->knobCombFB1->setValue(value);
    break;
  case pCombFB1Mode:
    combCtrl->onoffCombFB1Mode->setValue(lrintf(value));
    break;
  case pDecBits0:
    decimateCtrl->knobDecBits0->setValue(value);
    break;
  case pDecBits1:
    decimateCtrl->knobDecBits1->setValue(value);
    break;
  case pDecBits1Mode:
    decimateCtrl->onoffDecBits1Mode->setValue(lrintf(value));
    break;
  case pGranMode:
    granCtrl->optionGranMode->setValue(value*6.0f);
    break;
  case pGranRate0:
    granCtrl->knobGranRate0->setValue(value);
    break;
  case pGranRate1:
    granCtrl->knobGranRate1->setValue(value);
    break;
  case pGranRate1Mode:
    granCtrl->onoffGranRate1Mode->setValue(lrintf(value));
    break;
  case pGranSmooth0:
    granCtrl->knobGranSmooth0->setValue(value);
    break;
  case pGranSmooth1:
    granCtrl->knobGranSmooth1->setValue(value);
    break;
  case pGranSmooth1Mode:
    granCtrl->onoffGranSmooth1Mode->setValue(lrintf(value));
    break;
  case pDWGSDecay0:
    dwgsCtrl->knobDWGSDecay0->setValue(value);
    break;
  case pDWGSDecay1:
    dwgsCtrl->knobDWGSDecay1->setValue(value);
    break;
  case pDWGSDecay1Mode:
    dwgsCtrl->onoffDWGSDecay1Mode->setValue(lrintf(value));
    break;
  case pDWGSLopass0:
    dwgsCtrl->knobDWGSLopass0->setValue(value);
    break;
  case pDWGSLopass1:
    dwgsCtrl->knobDWGSLopass1->setValue(value);
    break;
  case pDWGSLopass1Mode:
    dwgsCtrl->onoffDWGSLopass1Mode->setValue(lrintf(value));
    break;
  case pDWGSStringPos0:
    dwgsCtrl->knobDWGSStringPos0->setValue(value);
    break;
  case pDWGSStringPos1:
    dwgsCtrl->knobDWGSStringPos1->setValue(value);
    break;
  case pDWGSStringPos1Mode:
    dwgsCtrl->onoffDWGSStringPos1Mode->setValue(lrintf(value));
    break;
  case pThresh:
    knobThresh->setValue(value);
    break;
  case pForce:
    knobForce->setValue(value);
    break;
  case pDrag:
    knobDrag->setValue(value);
    break;
  case pSpring:
    knobSpring->setValue(value);
    break;
  }
}
  
