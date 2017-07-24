#include "sample.h"
#include "audioeffect.h"
#include <string.h>
#include "sbsmsampler.h"
#include <math.h>
#include "real.h"
#include "utils.h"

SBSMSample :: SBSMSample(SBSMSampler *sampler, int index, ProgramSynthesizer *programSynth)
{  
  synth.programSynth = programSynth;
  this->sampler = sampler;
  this->index = index;
  for(int k=0;k<NUM_VOICES;k++) {
    voices[k] = new SBSMSVoice(this, sampler->getBlockSize(), sampler->getSampleRate(), &synth, sampler->getSidebandSpectrumSize());
    synth.voiceSynths.push_back(voices[k]->voiceSynth);
	  voices[k]->index = k;
    indexQueue.push(k);
  }
  for(int k=0;k<NUM_NOTES;k++) {
    notes[k] = NULL;
  }
  numVoices = 0;
  bOpen = false;
  lastVoice = NULL;
  *(data.name) = 0;
  *(data.filename) = 0;
  setParameter(pLoop, 0 / 6.0f);
  setParameter(pFollowMode, 0 / 2.0f);
  setParameter(pAttack, 0.3f);
  setParameter(pRelease, 0.3f);
  setParameter(pRate, 0.5f);
  setParameter(pRateMode, 0.0f);
  setParameter(pAMFreq0, 0.5f);
  setParameter(pAMFreq0Mode, 0.0f);
  setParameter(pAMFreq1, 0.5f);
  setParameter(pAMFreq1Mode, 0.0f);
  setParameter(pAMDepth0, 0.0f);
  setParameter(pAMDepth1, 0.5f);
  setParameter(pAMDepth1Mode, 0.0f);
  setParameter(pFMFreq0, 0.5f);
  setParameter(pFMFreq0Mode, 0.0f);
  setParameter(pFMFreq1, 0.5f);
  setParameter(pFMFreq1Mode, 0.0f);
  setParameter(pFMDepth0, 0.0f);
  setParameter(pFMDepth1, 0.5f);
  setParameter(pFMDepth1Mode, 0.0f);
  setParameter(pDistortion0, 0.0f);
  setParameter(pDistortion1, 0.5f);
  setParameter(pDistortion1Mode, 0.0f);
  setParameter(pVolume, 0.75f);
  setParameter(pKeyBase, 0.0f / 12.0f);
  setParameter(pOctaveBase, (4+2)/11.0f);
  setParameter(pKeyLo, 0.0f / 12.0f);
  setParameter(pOctaveLo, (-2+2)/11.0f);
  setParameter(pKeyHi, 7/12.0f);
  setParameter(pOctaveHi, (8+2)/11.0f);
  setParameter(pChannel, 0 / 16.0f);
  setParameter(pPitchbendRange, 12 / 24.0f);
  setParameter(pLeftPos, 0.0f);
  setParameter(pRightPos, 1.0f);
  setParameter(pStartPos, 0.0f);
  setParameter(pFilterQ0, 0.5f);
  setParameter(pFilterQ1, 0.5f);
  setParameter(pFilterQ1Mode, 0.0f);
  setParameter(pSidebandMod, 0.5f);
  setParameter(pSidebandBW, 0.5f);
  setParameter(pSidebandBWScale, 0.5f);
  setParameter(pSidebandRelease, 0.5f);
  setParameter(pSidebandEnv, 0.0f);
  setParameter(pModPivot, 0.5f);
  setParameter(pSynthMode, 0.0f);
  setParameter(pCombFB0, 0.5f);
  setParameter(pCombFB1, 0.5f);
  setParameter(pCombFB1Mode, 0.0f);
  setParameter(pDecBits0, 0.5f);
  setParameter(pDecBits1, 0.5f);
  setParameter(pDecBits1Mode, 0.0f);
  setParameter(pGranMode, 0.0f);
  setParameter(pGranRate0, 0.0f);
  setParameter(pGranRate1, 0.5f);
  setParameter(pGranRate1Mode, 0.0f);
  setParameter(pGranSmooth0, 0.0f);
  setParameter(pGranSmooth1, 0.5f);
  setParameter(pGranSmooth1Mode, 0.0f);
  setParameter(pDWGSDecay0, 0.5f);
  setParameter(pDWGSDecay1, 0.5f);
  setParameter(pDWGSDecay1Mode, 0.5f);
  setParameter(pDWGSLopass0, 0.5f);
  setParameter(pDWGSLopass1, 0.5f);
  setParameter(pDWGSLopass1Mode, 0.0f);
  setParameter(pDWGSStringPos0, 0.5f);
  setParameter(pDWGSStringPos1, 0.5f);
  setParameter(pDWGSStringPos1Mode, 0.0f);
  setParameter(pThresh, 0.0f);
  setParameter(pForce, 0.0f);
  setParameter(pDrag, 0.0f);
  setParameter(pSpring, 0.0f);
}

SBSMSample :: ~SBSMSample()
{
  for(int k=0;k<NUM_VOICES;k++) {
    delete voices[k];
  }
}

int SBSMSample :: getIndex()
{
  return index;
}

void SBSMSample :: update()
{
  updatePitchbendRange();
  updateLoop();
  updateFollowMode();
  updateAttack();
  updateRelease();
  //updateRate();  
  updateRateMode();  
  //updateAMFreq0();
  updateAMFreq0Mode();
  updateAMFreq1();
  updateAMFreq1Mode();
  updateAMDepth0();
  updateAMDepth1();
  updateAMDepth1Mode();
  //  updateFMFreq0();
  updateFMFreq0Mode();
  updateFMFreq1();
  updateFMFreq1Mode();
  updateFMDepth0();
  updateFMDepth1();
  updateFMDepth1Mode();
  updateDistortion0();
  updateDistortion1();
  updateDistortion1Mode();
  updateVolume();
  updateKeyBase();
  updateOctaveBase();
  updateKeyLo();
  updateOctaveLo();
  updateKeyHi();
  updateOctaveHi();
  updateLeftPos();
  updateRightPos();
  updateStartPos();
  updateSynthMode();
  updateFilterQ0();
  updateFilterQ1();
  updateFilterQ1Mode();
  updateSidebandMod();
  updateSidebandBW();
  updateSidebandBWScale();
  updateSidebandRelease();
  updateSidebandEnv();
  updateModPivot();
  updateCombFB0();
  updateCombFB1();
  updateCombFB1Mode();
  updateDecBits0();
  updateDecBits1();
  updateDecBits1Mode();
  updateGranMode();
  updateGranRate0();
  updateGranRate1();
  updateGranRate1Mode();
  updateGranSmooth0();
  updateGranSmooth1();
  updateGranSmooth1Mode();
  updateDWGSDecay0();
  updateDWGSDecay1();
  updateDWGSDecay1Mode();
  updateDWGSLopass0();
  updateDWGSLopass1();
  updateDWGSLopass1Mode();
  updateDWGSStringPos0();
  updateDWGSStringPos1();
  updateDWGSStringPos1Mode();
  updateThresh();
  updateForce();
  updateDrag();
  updateSpring();
}

void SBSMSample :: setBlockSize(int value)
{
  for(int k=0;k<NUM_VOICES;k++) {
	  voices[k]->setBlockSize(value);
  }
}

void SBSMSample :: setSampleRate(float value)
{
  Fs = value;
  updateAttack();
  updateRelease();
  updateAMFreq0();
  updateFMFreq0();
  updateSidebandRelease();
  for(int k=0;k<NUM_VOICES;k++) {
	  voices[k]->setSampleRate(value);
  }
}

void SBSMSample :: resume()
{
  for(int k=0;k<NUM_VOICES;k++) {
	  voices[k]->resume();
  }
}

void SBSMSample :: setParameter(int param, float value)
{
  data.params[param] = value;
  switch(param) {
  case pRate:
    updateRate();
    break;
  case pRateMode:
    updateRateMode();
    break;
  case pStartPos:
    updateStartPos();
    break;
  case pFilterQ0:
    updateFilterQ0();
    break;
  case pFilterQ1:
    updateFilterQ1();
    break;
  case pFilterQ1Mode:
    updateFilterQ1Mode();
    break;
  case pSidebandMod:
    updateSidebandMod();
    break;
  case pSidebandBW:
    updateSidebandBW();
    break;
  case pSidebandBWScale:
    updateSidebandBWScale();
    break;
  case pSidebandRelease:
    updateSidebandRelease();
    break;
  case pSidebandEnv:
    updateSidebandEnv();
    break;
  case pModPivot:
    updateModPivot();
    break;
  case pAMFreq0:
    updateAMFreq0();
    break;
  case pAMFreq0Mode:
    updateAMFreq0Mode();
    break;
  case pAMFreq1:
    updateAMFreq1();
    break;
  case pAMFreq1Mode:
    updateAMFreq1Mode();
    break;
  case pAMDepth0:
    updateAMDepth0();
    break;
  case pAMDepth1:
    updateAMDepth1();
    break;
  case pAMDepth1Mode:
    updateAMDepth1Mode();
    break;
  case pFMFreq0:
    updateFMFreq0();
    break;
  case pFMFreq0Mode:
    updateFMFreq0Mode();
    break;
  case pFMFreq1:
    updateFMFreq1();
    break;
  case pFMFreq1Mode:
    updateFMFreq1Mode();
    break;
  case pFMDepth0:
    updateFMDepth0();
    break;
  case pFMDepth1:
    updateFMDepth1();
    break;
  case pFMDepth1Mode:
    updateFMDepth1Mode();
    break;
  case pDistortion0:
    updateDistortion0();
    break;
  case pDistortion1:
    updateDistortion1();
    break;
  case pDistortion1Mode:
    updateDistortion1Mode();
    break;
  case pAttack:
    updateAttack();
    break;  
  case pRelease:
    updateRelease();
    break;    
  case pVolume:
    updateVolume();
    break;
  case pLeftPos:
    updateLeftPos();
    break;
  case pRightPos:
    updateRightPos();
    break;
  case pKeyBase:
    updateKeyBase();
    break;
  case pOctaveBase:
    updateOctaveBase();
    break;
  case pKeyLo:
    updateKeyLo();
    break;
  case pOctaveLo:
    updateOctaveLo();
    break;
  case pKeyHi:
    updateKeyHi();
    break;
  case pOctaveHi:
    updateOctaveHi();
    break;
  case pChannel:
    updateChannel();
    break;
  case pPitchbendRange:
    updatePitchbendRange();
    break;
  case pLoop:
    updateLoop();
    break;
  case pFollowMode:
    updateFollowMode();
    break;
  case pSynthMode:
    updateSynthMode();
    break;
  case pCombFB0:
    updateCombFB0();
    break;
  case pCombFB1:
    updateCombFB1();
    break;
  case pCombFB1Mode:
    updateCombFB1Mode();
    break;
  case pDecBits0:
    updateDecBits0();
    break;
  case pDecBits1:
    updateDecBits1();
    break;
  case pDecBits1Mode:
    updateDecBits1Mode();
    break;
  case pGranMode:
    updateGranMode();
    break;
  case pGranRate0:
    updateGranRate0();
    break;
  case pGranRate1:
    updateGranRate1();
    break;
  case pGranRate1Mode:
    updateGranRate1Mode();
    break;
  case pGranSmooth0:
    updateGranSmooth0();
    break;
  case pGranSmooth1:
    updateGranSmooth1();
    break;
  case pGranSmooth1Mode:
    updateGranSmooth1Mode();
    break;
  case pDWGSDecay0:
    updateDWGSDecay0();
    break;
  case pDWGSDecay1:
    updateDWGSDecay1();
    break;
  case pDWGSDecay1Mode:
    updateDWGSDecay1Mode();
    break;
  case pDWGSLopass0:
    updateDWGSLopass0();
    break;
  case pDWGSLopass1:
    updateDWGSLopass1();
    break;
  case pDWGSLopass1Mode:
    updateDWGSLopass1Mode();
    break;
  case pDWGSStringPos0:
    updateDWGSStringPos0();
    break;
  case pDWGSStringPos1:
    updateDWGSStringPos1();
    break;
  case pDWGSStringPos1Mode:
    updateDWGSStringPos1Mode();
    break;
  case pThresh:
    updateThresh();
    break;
  case pForce:
    updateForce();
    break;
  case pDrag:
    updateDrag();
    break;
  case pSpring:
    updateSpring();
    break;
  }
}

bool SBSMSample :: string2parameter(int param, char *text)
{
  float fValue;
  int iValue;
  switch(param) {
  case pRate:
    if(sscanf(text,"%f",&fValue)) return setRate(fValue);
    break;
  case pRateMode:
    if(sscanf(text,"%d",&iValue)) return setRateMode(iValue);
    break;
  case pStartPos:
    if(sscanf(text,"%f",&fValue)) return setStartPos(fValue);
    break;
  case pFilterQ0:
    if(sscanf(text,"%f",&fValue)) return setFilterQ0(fValue);
    break;
  case pFilterQ1:
    if(sscanf(text,"%f",&fValue)) return setFilterQ1(fValue);
    break;
  case pFilterQ1Mode:
    if(sscanf(text,"%d",&iValue)) return setFilterQ1Mode(iValue);
    break;
  case pSidebandMod:
    if(sscanf(text,"%f",&fValue)) return setSidebandMod(fValue);
    break;
  case pSidebandBW:
    if(sscanf(text,"%f",&fValue)) return setSidebandBW(fValue);
    break;
  case pSidebandBWScale:
    if(sscanf(text,"%f",&fValue)) return setSidebandBWScale(fValue);
    break;
  case pSidebandRelease:
    if(sscanf(text,"%f",&fValue)) return setSidebandRelease(fValue);
    break;
  case pSidebandEnv:
    if(sscanf(text,"%f",&fValue)) return setSidebandEnv(fValue);
    break;
  case pModPivot:
    if(sscanf(text,"%f",&fValue)) return setModPivot(fValue);
    break;
  case pAMFreq0:
    if(sscanf(text,"%f",&fValue)) return setAMFreq0(fValue / 11025.0f);
    break;
  case pAMFreq0Mode:
    if(sscanf(text,"%d",&iValue)) return setAMFreq0Mode(iValue);
    break;
  case pAMFreq1:
    if(sscanf(text,"%f",&fValue)) return setAMFreq1(fValue);
    break;
  case pAMFreq1Mode:
    if(sscanf(text,"%d",&iValue)) return setAMFreq1Mode(iValue);
    break;
  case pAMDepth0:
    if(sscanf(text,"%f",&fValue)) return setAMDepth0(fValue);
    break;
  case pAMDepth1:
    if(sscanf(text,"%f",&fValue)) return setAMDepth1(fValue);
    break;
  case pAMDepth1Mode:
    if(sscanf(text,"%d",&iValue)) return setAMDepth1Mode(iValue);
    break;
  case pFMFreq0:
    if(sscanf(text,"%f",&fValue)) return setFMFreq0(fValue / 11025.0f);
    break;
  case pFMFreq0Mode:
    if(sscanf(text,"%d",&iValue)) return setFMFreq0Mode(iValue);
    break;
  case pFMFreq1:
    if(sscanf(text,"%f",&fValue)) return setFMFreq1(fValue);
    break;
  case pFMFreq1Mode:
    if(sscanf(text,"%d",&iValue)) return setFMFreq1Mode(iValue);
    break;
  case pFMDepth0:
    if(sscanf(text,"%f",&fValue)) return setFMDepth0(fValue);
    break;
  case pFMDepth1:
    if(sscanf(text,"%f",&fValue)) return setFMDepth1(fValue);
    break;
  case pFMDepth1Mode:
    if(sscanf(text,"%d",&iValue)) return setFMDepth1Mode(fValue);
    break;
  case pDistortion0:
    if(sscanf(text,"%f",&fValue)) return setDistortion0(fValue);
    break;
  case pDistortion1:
    if(sscanf(text,"%f",&fValue)) return setDistortion1(fValue);
    break;
  case pDistortion1Mode:
    if(sscanf(text,"%d",&iValue)) return setDistortion1Mode(iValue);
    break;
  case pAttack:
    if(sscanf(text,"%f",&fValue)) return setAttack(fValue);
    break;  
  case pRelease:
    if(sscanf(text,"%f",&fValue)) return setRelease(fValue);
    break;    
  case pVolume:
    if(sscanf(text,"%f",&fValue)) return setVolume(fValue);
    break;
  case pLeftPos:
    if(sscanf(text,"%f",&fValue)) return setLeftPos(fValue);
    break;
  case pRightPos:
    if(sscanf(text,"%f",&fValue)) return setRightPos(fValue);
    break;
  case pKeyBase:
    if(sscanf(text,"%d",&iValue)) return setKeyBase(iValue);
    break;
  case pOctaveBase:
    if(sscanf(text,"%d",&iValue)) return setOctaveBase(iValue);
    break;
  case pKeyLo:
    if(sscanf(text,"%d",&iValue)) return setKeyLo(iValue);
    break;
  case pOctaveLo: 
    if(sscanf(text,"%d",&iValue)) return setOctaveLo(iValue);
    break;
  case pKeyHi:  
    if(sscanf(text,"%d",&iValue)) return setKeyHi(iValue);
    break;
  case pOctaveHi:
    if(sscanf(text,"%d",&iValue)) return setOctaveHi(iValue);
    break;
  case pChannel:  
    if(sscanf(text,"%d",&iValue)) return setChannel(iValue-1);
    break;
  case pPitchbendRange:
    if(sscanf(text,"%d",&iValue)) return setPitchbendRange(iValue);
    break;
  case pLoop:
    if(sscanf(text,"%d",&iValue)) return setLoop(iValue);
    break; 
  case pFollowMode:
    if(sscanf(text,"%d",&iValue)) return setFollowMode(iValue);
    break;
  case pSynthMode:
    if(sscanf(text,"%f",&fValue)) return setSynthMode(fValue);
    break;
  case pCombFB0:
    if(sscanf(text,"%f",&fValue)) return setCombFB0(fValue);
    break;
  case pCombFB1:
    if(sscanf(text,"%f",&fValue)) return setCombFB1(fValue);
    break;
  case pCombFB1Mode:
    if(sscanf(text,"%d",&iValue)) return setCombFB1Mode(iValue);
    break;
  case pDecBits0:
    if(sscanf(text,"%f",&fValue)) return setDecBits0(fValue);
    break;
  case pDecBits1:
    if(sscanf(text,"%f",&fValue)) return setDecBits1(fValue);
    break;
  case pDecBits1Mode:
    if(sscanf(text,"%d",&iValue)) return setDecBits1Mode(iValue);
    break;
  case pGranMode:
    if(sscanf(text,"%f",&fValue)) return setGranMode(fValue);
    break;
  case pGranRate0:
    if(sscanf(text,"%f",&fValue)) return setGranRate0(fValue);
    break;
  case pGranRate1:
    if(sscanf(text,"%f",&fValue)) return setGranRate1(fValue);
    break;
  case pGranRate1Mode:
    if(sscanf(text,"%d",&iValue)) return setGranRate1Mode(iValue);
    break;
  case pGranSmooth0:
    if(sscanf(text,"%f",&fValue)) return setGranSmooth0(fValue);
    break;
  case pGranSmooth1:
    if(sscanf(text,"%f",&fValue)) return setGranSmooth1(fValue);
    break;
  case pGranSmooth1Mode:
    if(sscanf(text,"%d",&iValue)) return setGranSmooth1Mode(iValue);
    break;
  case pDWGSDecay0:
    if(sscanf(text,"%f",&fValue)) return setDWGSDecay0(fValue);
    break;
  case pDWGSDecay1:
    if(sscanf(text,"%f",&fValue)) return setDWGSDecay1(fValue);
    break;
  case pDWGSDecay1Mode:
    if(sscanf(text,"%d",&iValue)) return setDWGSDecay1Mode(iValue);
    break;
  case pDWGSLopass0:
    if(sscanf(text,"%f",&fValue)) return setDWGSLopass0(fValue);
    break;
  case pDWGSLopass1:
    if(sscanf(text,"%f",&fValue)) return setDWGSLopass1(fValue);
    break;
  case pDWGSLopass1Mode:
    if(sscanf(text,"%d",&iValue)) return setDWGSLopass1Mode(iValue);
    break;
  case pDWGSStringPos0:
    if(sscanf(text,"%f",&fValue)) return setDWGSStringPos0(fValue);
    break;
  case pDWGSStringPos1:
    if(sscanf(text,"%f",&fValue)) return setDWGSStringPos1(fValue);
    break;
  case pDWGSStringPos1Mode:
    if(sscanf(text,"%d",&iValue)) return setDWGSStringPos1Mode(iValue);
    break;
  case pThresh:
    if(sscanf(text,"%g",&fValue)) return setThresh(fValue);
    break;
  case pForce:
    if(sscanf(text,"%g",&fValue)) return setForce(fValue);
    break;
  case pDrag:
    if(sscanf(text,"%g",&fValue)) return setDrag(fValue);
    break;
  case pSpring:
    if(sscanf(text,"%g",&fValue)) return setSpring(fValue);
    break;
  }
  return false;
}

void SBSMSample :: getParameterDisplay(int param, char *text)
{
  char text2[128];
  
  switch(param) {
  case pRate:
    sprintf(text2,"%.4g",rate);
    break;
  case pRateMode:
    sprintf(text2,"%d",rateMode);
    break;
  case pStartPos:
    sprintf(text2,"%g",startPos);
    break;
  case pFilterQ0:
    sprintf(text2,"%g",synth.Q0);
    break;
 case pFilterQ1:
    sprintf(text2,"%g",synth.Q1);
    break;
 case pFilterQ1Mode:
    sprintf(text2,"%d",synth.Q1Mode);
    break;
  case pSidebandMod:
    sprintf(text2,"%g",synth.sidebandMod);
    break;
  case pSidebandBW:
    sprintf(text2,"%g",synth.sidebandBW);
    break;
  case pSidebandBWScale:
    sprintf(text2,"%g",synth.sidebandBWScale);
    break;
  case pSidebandRelease:
    sprintf(text2,"%g",synth.sidebandRelease);
    break;
  case pSidebandEnv:
    sprintf(text2,"%g",synth.sidebandEnv);
    break;
  case pModPivot:
    sprintf(text2,"%g",synth.modPivot);
    break;
  case pAMFreq0:
    sprintf(text2,"%.4g",synth.fAM0 * 11025.0f);
    break;
  case pAMFreq0Mode:
    sprintf(text2,"%d",fAM0Mode);
    break;
  case pAMFreq1:
    sprintf(text2,"%g",synth.fAM1);
    break;
  case pAMFreq1Mode:
    sprintf(text2,"%d",synth.fAM1Mode);
    break;
  case pAMDepth0:
    sprintf(text2,"%g",synth.mAM0);
    break;
  case pAMDepth1:
    sprintf(text2,"%g",synth.mAM1);
    break;
  case pAMDepth1Mode:
    sprintf(text2,"%d",synth.mAM1Mode);
    break;
  case pFMFreq0:
    sprintf(text2,"%.4g",synth.fFM0 * 11025.0f);
    break;
  case pFMFreq0Mode:
    sprintf(text2,"%d",fFM0Mode);
    break;
  case pFMFreq1:
    sprintf(text2,"%g",synth.fFM1);
    break;
  case pFMFreq1Mode:
    sprintf(text2,"%d",synth.fFM1Mode);
    break;
  case pFMDepth0:
    sprintf(text2,"%g",synth.mFM0);
    break;
  case pFMDepth1:
    sprintf(text2,"%g",synth.mFM1);
    break;
  case pFMDepth1Mode:
    sprintf(text2,"%d",synth.mFM1Mode);
    break;
  case pDistortion0:
    sprintf(text2,"%g",synth.dist0);
    break;
  case pDistortion1:
    sprintf(text2,"%g",synth.dist1);
    break;
  case pDistortion1Mode:
    sprintf(text2,"%d",synth.dist1Mode);
    break;
  case pAttack:
    sprintf(text2,"%g",attack);
    break;  
  case pRelease:
    sprintf(text2,"%g",release);
    break;    
  case pVolume:
    sprintf(text2,"%g",volume);
    break;
  case pLeftPos:
    sprintf(text2,"%g",leftPos);
    break;
  case pRightPos:
    sprintf(text2,"%g",rightPos);
    break;
  case pKeyBase:
    sprintf(text2,"%d",keyBase);
    break;
  case pOctaveBase:
    sprintf(text2,"%d",octaveBase);
    break;
  case pKeyLo:
    sprintf(text2,"%d",keyLo);
    break;
  case pOctaveLo: 
    sprintf(text2,"%d",octaveLo);
    break;
  case pKeyHi:  
    sprintf(text2,"%d",keyHi);
    break;
  case pOctaveHi:
    sprintf(text2,"%d",octaveHi);
    break;
  case pChannel:  
    sprintf(text2,"%d",channel+1);
    break;
  case pPitchbendRange:
    sprintf(text2,"%d",pitchbendRange);
    break;
  case pLoop:
    sprintf(text2,"%d",loop);
    break;
  case pFollowMode:
    sprintf(text2,"%d",followMode);
    break;
  case pSynthMode:
    sprintf(text2,"%d",synth.synthMode);
    break;
  case pCombFB0:
    sprintf(text2,"%g",synth.combFB0);
    break;
  case pCombFB1:
    sprintf(text2,"%g",synth.combFB1);
    break;
  case pCombFB1Mode:
    sprintf(text2,"%d",synth.combFB1Mode);
    break;
  case pDecBits0:
    sprintf(text2,"%g",synth.decBits0);
    break;
  case pDecBits1:
    sprintf(text2,"%g",synth.decBits1);
    break;
  case pDecBits1Mode:
    sprintf(text2,"%d",synth.decBits1Mode);
    break;
  case pGranMode:
    sprintf(text2,"%d",synth.granMode);
    break;
  case pGranRate0:
    sprintf(text2,"%g",synth.granRate0);
    break;
  case pGranRate1:
    sprintf(text2,"%g",synth.granRate1);
    break;
  case pGranRate1Mode:
    sprintf(text2,"%d",synth.granRate1Mode);
    break;
  case pGranSmooth0:
    sprintf(text2,"%g",synth.granSmooth0);
    break;
  case pGranSmooth1:
    sprintf(text2,"%g",synth.granSmooth1);
    break;
  case pGranSmooth1Mode:
    sprintf(text2,"%d",synth.granSmooth1Mode);
    break;
  case pDWGSDecay0:
    sprintf(text2,"%g",synth.dwgsDecay0);
    break;
  case pDWGSDecay1:
    sprintf(text2,"%g",synth.dwgsDecay1);
    break;
  case pDWGSDecay1Mode:
    sprintf(text2,"%d",synth.dwgsDecay1Mode);
    break;
  case pDWGSLopass0:
    sprintf(text2,"%g",synth.dwgsLopass0);
    break;
  case pDWGSLopass1:
    sprintf(text2,"%g",synth.dwgsLopass1);
    break;
  case pDWGSLopass1Mode:
    sprintf(text2,"%d",synth.dwgsLopass1Mode);
    break;
  case pDWGSStringPos0:
    sprintf(text2,"%g",synth.dwgsStringPos0);
    break;
  case pDWGSStringPos1:
    sprintf(text2,"%g",synth.dwgsStringPos1);
    break;
  case pDWGSStringPos1Mode:
    sprintf(text2,"%d",synth.dwgsStringPos1Mode);
    break;
  case pThresh:
    sprintf(text2,"%g",synth.mThresh);
    break;
  case pForce:
    sprintf(text2,"%g",synth.mForce);
    break;
  case pDrag:
    sprintf(text2,"%g",synth.mDrag);
    break;
  case pSpring:
    sprintf(text2,"%g",synth.mSpring);
    break;
  }
  vst_strncpy(text,text2,kVstMaxLabelLen);
}

float SBSMSample :: getParameter(int param)
{
  return data.params[param];
}

void SBSMSample :: updateAttack()
{
  attack = 0.004f * powf(2.0f, 10.0f * data.params[pAttack]);
  dattack = 1.0f / (attack * Fs);
}

bool SBSMSample :: setAttack(float value)
{
  float dataValue = log2f(250.0f * value) * 0.1f;
  if(dataValue >= 0.0f && dataValue <= 1.0f) {
    data.params[pAttack] = dataValue;
    updateAttack();
    return true;
  } else {
    return false;
  }
}

void SBSMSample :: updateRelease()
{
  release = 0.004f * powf(2.0f, 10.0f * data.params[pRelease]);
  drelease = drelease = 1.0f / (release * Fs);
}

bool SBSMSample :: setRelease(float value)
{
  float dataValue = log2f(250.0f * value) * 0.1f;
  if(dataValue >= 0.0f && dataValue <= 1.0f) {
    data.params[pRelease] = dataValue;
    updateRelease();
    return true;
  } else {
    return false;
  }
}

void SBSMSample :: updateRate()
{
  float value = data.params[pRate];
  if(rateMode == 0) {
    if(loop == loopStill || loop == loopStillLoop) {
      if(value < 0.4f) {
        rate = -4.0f * powf(2.0f, -7.5f * value);
      } else if(value < 0.498f){
        rate = -0.5f * powf(2.0f, -40.0f * (value - 0.4f));
      } else if(value < 0.502f) {
        rate = 0.0f;
      } else if(value < 0.6f) {
        rate = 0.5f * powf(2.0f, -40.0f * (0.6f - value));
      } else {
        rate = 4.0f * powf(2.0f, -7.5f * (1.0f - value));
      }
    } else {
      if(value < 0.001f) {
        rate = 0.0f;
      } else if(value < 0.1f) {
        rate = 0.5f * powf(2.0f, -40.0f * (0.1f - value));
      } else if(value < 0.5f) {
      rate = powf(2.0f, -2.5f * (0.5f - value));
      } else {
        rate = powf(2.0f, 2.0f * (value - 0.5f));
      }
      if(loop == loopBackward || loop == loopBackwardLoop) {
        rate = -rate;
      }
    }
  } else {
    float length;
    if(loop == loopBackward || loop == loopBackwardLoop) {
      length = startPos - leftPos;
    } else {
      length = rightPos - startPos;
    }
    length *= samplesToProcess / 44100.0;
    float tmp = tempo.s[lrintf(value * (tempo.s.size()-1))].first;
    rate = length / (sampler->getBeatPeriod() * tmp);
    rate = min(rate,12.0f);
    if(loop == loopBackward || loop == loopBackwardLoop) {
      rate = -rate;
    }
  }
  for(int k=0;k<NUM_VOICES;k++) {
	  voices[k]->setRate(rate);
  }
}

bool SBSMSample :: setRate(float value)
{
  float dataValue;
  if(rateMode == 0) {
    if(loop == loopStill || loop == loopStillLoop) {
      if(value < -0.5f) {
        dataValue = log2f(value / -4.0f) / -7.5f;
      } else if(value < 0.0f) {
        dataValue = log2f(value / -0.5f) / -40.0f + 0.4f;
      } else if(value == 0.0f) {
        dataValue = 0.5f;
      } else if(value < 0.5f) {
        dataValue = 0.6f - log2f(value / 0.5f) / -40.0f;
      } else {
        dataValue = 1.0f - log2f(value / 4.0f) / -7.5f;
      }
    } else {
      if(loop == loopBackward || loop == loopBackwardLoop) {
        value = -value;
      }
      if(value == 0.0f) {
        dataValue = 0.0f;
      } else if(value < 0.5f) {
        dataValue = 0.1f - log2f(value / 0.5f) / -40.0f ;
      } else if(value < 1.0f) {
        dataValue = 0.5f - log2f(value) / -2.5f;
      } else {
        dataValue = log2f(value) / 2.0f + 0.5f;
      }
    }
  } else {
    if(loop == loopBackward || loop == loopBackwardLoop) {
      value = -value;
    }
    float length;
    if(loop == loopBackward || loop == loopBackwardLoop) {
      length = startPos - leftPos;
    } else {
      length = rightPos - startPos;
    }
    length *= samplesToProcess / 44100.0;
    float tmp = value == 0.0?0.0:length / (value * sampler->getBeatPeriod());
    dataValue = closestTempo(tmp);
  }
  if(dataValue >= 0.0f && dataValue <= 1.0f) {
    data.params[pRate] = dataValue;
    updateRate();
    return true;
  } else {
    return false;
  }
}

void SBSMSample :: updateRateMode()
{	
  rateMode = lrintf(data.params[pRateMode]);
  updateRate();
}

bool SBSMSample :: setRateMode(float value)
{
  float dataValue = value;
  if(dataValue >= 0.0f && dataValue <= 1.0f) {
    data.params[pRateMode] = dataValue;
    updateRateMode();
    return true;
  } else {
    return false;
  }
}

void SBSMSample :: updateAMFreq0()
{	
  float f; 
  if(fAM0Mode == 0) {
    f = .2 * powf(2.0f, 8.0f * data.params[pAMFreq0]);
  } else {
    f = data.params[pAMFreq0];
    f = tempo.s[lrintf(f * (tempo.s.size()-1))].first;
    f = 1.0 / (f * sampler->getBeatPeriod());
  }
  synth.fAM0 = f * 4.0 / Fs;
}

bool SBSMSample :: setAMFreq0(float value)
{
  float f = value * Fs / 4.0;
  float dataValue;
  if(fAM0Mode == 0) {
    dataValue = log2f(5.0f * value) / 8.0f;
  } else {
    dataValue = 1.0 / (f * sampler->getBeatPeriod());
    dataValue = closestTempo(dataValue);
  }
  if(dataValue >= 0.0f && dataValue <= 1.0f) {
    data.params[pAMFreq0] = dataValue;
    updateAMFreq0();
    return true;
  } else {
    return false;
  }
}

void SBSMSample :: updateAMFreq0Mode()
{	
  fAM0Mode = lrintf(data.params[pAMFreq0Mode]);
  updateAMFreq0();
}

bool SBSMSample :: setAMFreq0Mode(float value)
{
  float dataValue = value;
  if(dataValue >= 0.0f && dataValue <= 1.0f) {
    data.params[pAMFreq0Mode] = dataValue;
    updateAMFreq0Mode();
    return true;
  } else {
    return false;
  }
}

void SBSMSample :: updateAMFreq1()
{	
  float value = data.params[pAMFreq1];
  if(value == 0.5f) {
    synth.fAM1 = 0.0f;
  } else if(value < 0.5f) {
    synth.fAM1 = -powf(2.0f, -12.0f * value);
  } else {
    synth.fAM1 = powf(2.0f, 12.0f * (value - 1.0f));
  }
}

bool SBSMSample :: setAMFreq1(float value)
{
  float dataValue;
  if(value == 0.0f) {
    dataValue = 0.5f;
  } else if(value < 0.0f) {
    dataValue = log2f(-value) / -12.0f;
  } else {
    dataValue = log2f(value) / 12.0f + 1.0f;
  }
  if(dataValue >= 0.0f && dataValue <= 1.0f) {
    data.params[pAMFreq1] = dataValue;
    updateAMFreq1();
    return true;
  } else {
    return false;
  }
}

void SBSMSample :: updateAMFreq1Mode()
{	
  synth.fAM1Mode = lrintf(data.params[pAMFreq1Mode]);
}

bool SBSMSample :: setAMFreq1Mode(float value)
{
  float dataValue = value;
  if(dataValue >= 0.0f && dataValue <= 1.0f) {
    data.params[pAMFreq1Mode] = dataValue;
    updateAMFreq1Mode();
    return true;
  } else {
    return false;
  }
}

void SBSMSample :: updateAMDepth0()
{	
  synth.mAM0 = data.params[pAMDepth0];
}

bool SBSMSample :: setAMDepth0(float value)
{
  float dataValue = value;
  if(dataValue >= 0.0f && dataValue <= 1.0f) {
    data.params[pAMDepth0] = dataValue;
    updateAMDepth0();
    return true;
  } else {
    return false;
  }
}

void SBSMSample :: updateAMDepth1()
{	
  float value = data.params[pAMDepth1];
  if(value == 0.5f) {
    synth.mAM1 = 0.0f;
  } else if(value < 0.5f) {
    synth.mAM1 = -powf(2.0f, -10.0f * value);
  } else {
    synth.mAM1 = powf(2.0f, 10.0f * (value - 1.0f));
  }
}

bool SBSMSample :: setAMDepth1(float value)
{
  float dataValue;
  if(value == 0.0f) {
    dataValue = 0.5f;
  } else if(value < 0.0f) {
    dataValue = log2f(-value) / -10.0f;
  } else {
    dataValue = log2f(value) / 10.0f + 1.0f;
  }
  if(dataValue >= 0.0f && dataValue <= 1.0f) {
    data.params[pAMDepth1] = dataValue;
    updateAMDepth1();
    return true;
  } else {
    return false;
  }
}

void SBSMSample :: updateAMDepth1Mode()
{	
  synth.mAM1Mode = lrintf(data.params[pAMDepth1Mode]);
}

bool SBSMSample :: setAMDepth1Mode(float value)
{
  float dataValue = value;
  if(dataValue >= 0.0f && dataValue <= 1.0f) {
    data.params[pAMDepth1Mode] = dataValue;
    updateAMDepth1Mode();
    return true;
  } else {
    return false;
  }
}

void SBSMSample :: updateFMFreq0()
{	
  float f;
  if(fFM0Mode == 0) {
    f = .2 * powf(2.0f, 8.0f * data.params[pFMFreq0]);
  } else {
    f = data.params[pFMFreq0];
    f = tempo.s[lrintf(f * (tempo.s.size()-1))].first;
    f = 1.0 / (f * sampler->getBeatPeriod());
  }
  synth.fFM0 = f * 4.0 / Fs;
}

bool SBSMSample :: setFMFreq0(float value)
{
  float f = value * Fs / 4.0;
  float dataValue;
  if(fFM0Mode == 0) {
    dataValue = log2f(5.0f * value) / 8.0f;
  } else {
    dataValue = 1.0 / (f * sampler->getBeatPeriod());
    dataValue = closestTempo(dataValue);
  }
  if(dataValue >= 0.0f && dataValue <= 1.0f) {
    data.params[pFMFreq0] = dataValue;
    updateFMFreq0();
    return true;
  } else {
    return false;
  }
}

void SBSMSample :: updateFMFreq0Mode()
{	
  fFM0Mode = lrintf(data.params[pFMFreq0Mode]);
  updateFMFreq0();
}

bool SBSMSample :: setFMFreq0Mode(float value)
{
  float dataValue = value;
  if(dataValue >= 0.0f && dataValue <= 1.0f) {
    data.params[pFMFreq0Mode] = dataValue;
    updateFMFreq0Mode();
    return true;
  } else {
    return false;
  }
}

void SBSMSample :: updateFMFreq1()
{	
  float value = data.params[pFMFreq1];
  if(value == 0.5f) {
    synth.fFM1 = 0.0f;
  } else if(value < 0.5f) {
    synth.fFM1 = -powf(2.0f, -12.0f * value);
  } else {
    synth.fFM1 = powf(2.0f, 12.0f * (value - 1.0f));
  }
}

bool SBSMSample :: setFMFreq1(float value)
{
  float dataValue;
  if(value == 0.0f) {
    dataValue = 0.5f;
  } else if(value < 0.0f) {
    dataValue = log2f(-value) / -12.0f;
  } else {
    dataValue = log2f(value) / 12.0f + 1.0f;
  }
  if(dataValue >= 0.0f && dataValue <= 1.0f) {
    data.params[pFMFreq1] = dataValue;
    updateFMFreq1();
    return true;
  } else {
    return false;
  }
}

void SBSMSample :: updateFMFreq1Mode()
{	
  synth.fFM1Mode = lrintf(data.params[pFMFreq1Mode]);
}

bool SBSMSample :: setFMFreq1Mode(float value)
{
  float dataValue = value;
  if(dataValue >= 0.0f && dataValue <= 1.0f) {
    data.params[pFMFreq1Mode] = dataValue;
    updateFMFreq1Mode();
    return true;
  } else {
    return false;
  }
}

void SBSMSample :: updateFMDepth0()
{	
  float value = data.params[pFMDepth0];
  synth.mFM0 = (value == 0.0f?0.0f:0.1f * powf(2.0f, 6.0f * (value - 1.0f)));
}

bool SBSMSample :: setFMDepth0(float value)
{
  float dataValue;
  if(value == 0.0f) {
    dataValue = 0.0f;
  } else {
    dataValue = log2f(10.0f * value) / 6.0f + 1.0f;
  }
  if(dataValue >= 0.0f && dataValue <= 1.0f) {
    data.params[pFMDepth0] = dataValue;
    updateFMDepth0();
    return true;
  } else {
    return false;
  }
}

void SBSMSample :: updateFMDepth1()
{	
  float value = data.params[pFMDepth1];
  if(value == 0.5f) {
    synth.mFM1 = 0.0f;
  } else if(value < 0.5f) {
    synth.mFM1 = -powf(2.0f, -8.0f * value);
  } else {
    synth.mFM1 = powf(2.0f, 8.0f * (value - 1.0f));
  }
}

bool SBSMSample :: setFMDepth1(float value)
{
  float dataValue;
  if(value == 0.0f) {
    dataValue = 0.5f;
  } else if(value < 0.0f) {
    dataValue = log2f(-value) / -8.0f;
  } else {
    dataValue = log2f(value) / 8.0f + 1.0f;
  }
  if(dataValue >= 0.0f && dataValue <= 1.0f) {
    data.params[pFMDepth1] = dataValue;
    updateFMDepth1();
    return true;
  } else {
    return false;
  }
}

void SBSMSample :: updateFMDepth1Mode()
{	
  synth.mFM1Mode = lrintf(data.params[pFMDepth1Mode]);
}

bool SBSMSample :: setFMDepth1Mode(float value)
{
  float dataValue = value;
  if(dataValue >= 0.0f && dataValue <= 1.0f) {
    data.params[pFMDepth1Mode] = dataValue;
    updateFMDepth1Mode();
    return true;
  } else {
    return false;
  }
}

void SBSMSample :: updateDistortion0()
{	
  synth.dist0 = 127.0f * data.params[pDistortion0];
}

bool SBSMSample :: setDistortion0(float value)
{
  float dataValue = value / 127.0f;
  if(dataValue >= 0.0f && dataValue <= 1.0f) {
    data.params[pDistortion0] = dataValue;
    updateDistortion0();
    return true;
  } else {
    return false;
  }
}

void SBSMSample :: updateDistortion1()
{	
  float value = data.params[pDistortion1];
  if(value == 0.5f) {
    synth.dist1 = 0.0f;
  } else if(value < 0.5f) {
    synth.dist1 = -powf(2.0f, -14.0f * value);
  } else {
    synth.dist1 = powf(2.0f, 14.0f * (value - 1.0f));
  }
}

bool SBSMSample :: setDistortion1(float value)
{
  float dataValue;
  if(value == 0.0f) {
    dataValue = 0.5f;
  } else if(value < 0.0f) {
    dataValue = log2f(-value) / -14.0f;
  } else {
    dataValue = log2f(value) / 14.0f + 1.0f;
  }
  if(dataValue >= 0.0f && dataValue <= 1.0f) {
    data.params[pDistortion1] = dataValue;
    updateDistortion1();
    return true;
  } else {
    return false;
  }
}

void SBSMSample :: updateDistortion1Mode()
{	
  synth.dist1Mode = lrintf(data.params[pDistortion1Mode]);
}

bool SBSMSample :: setDistortion1Mode(float value)
{
  float dataValue = value;
  if(dataValue >= 0.0f && dataValue <= 1.0f) {
    data.params[pDistortion1Mode] = dataValue;
    updateDistortion1Mode();
    return true;
  } else {
    return false;
  }
}

void SBSMSample :: updateFilterQ0()
{
  synth.Q0 = powf(10.0,3.5f*data.params[pFilterQ0]);
  //synth.mForce = 1e-2 * powf(10.0,3.5f*data.params[pFilterQ0]);
}

bool SBSMSample :: setFilterQ0(float value)
{
  float dataValue = log10f(value) / 3.5f;
  if(dataValue >= 0.0f && dataValue <= 1.0f) {
    data.params[pFilterQ0] = dataValue;
    updateFilterQ0();
    return true;
  } else {
    return false;
  }
}

void SBSMSample :: updateFilterQ1()
{
  float value = data.params[pFilterQ1];
  if(value == 0.5f) {
    synth.Q1 = 0.0f;
  } else if(value < 0.5f) {
    synth.Q1 = 2.0f * (value - 0.5f);
  } else {
    synth.Q1 = 2.0f * (value - 0.5f);
  }
  synth.mDrag = 1e-1 * powf(10.0f,3*value);
}

bool SBSMSample :: setFilterQ1(float value)
{
  float dataValue;
  if(value == 0.0f) {
    dataValue = 0.5f;
  } else if(value < 0.0f) {
    dataValue = 0.5f * value + 0.5f;
  } else {
    dataValue = 0.5f * value + 0.5f;
  }
  if(dataValue >= 0.0f && dataValue <= 1.0f) {
    data.params[pFilterQ1] = dataValue;
    updateFilterQ1();
    return true;
  } else {
    return false;
  }
}

void SBSMSample :: updateFilterQ1Mode()
{	
  synth.Q1Mode = lrintf(data.params[pFilterQ1Mode]);
}

bool SBSMSample :: setFilterQ1Mode(float value)
{
  float dataValue = value;
  if(dataValue >= 0.0f && dataValue <= 1.0f) {
    data.params[pFilterQ1Mode] = dataValue;
    updateFilterQ1Mode();
    return true;
  } else {
    return false;
  }
}

void SBSMSample :: updateSidebandMod()
{
  float value = data.params[pSidebandMod];
  synth.sidebandMod = pow10f(6.0f * value - 1.0f);
}

bool SBSMSample :: setSidebandMod(float value)
{
  float dataValue = (log10f(value) + 1.0f) / 6.0f;
  if(dataValue >= 0.0f && dataValue <= 1.0f) {
    data.params[pSidebandMod] = dataValue;
    updateSidebandMod();
    return true;
  } else {
    return false;
  }
}

void SBSMSample :: updateSidebandBW()
{
  synth.sidebandBW = pow10f(1.0 - 4.0 * data.params[pSidebandBW]);
}

bool SBSMSample :: setSidebandBW(float value)
{
  float dataValue = (1.0 - log10f(value)) / 4.0;
  if(dataValue >= 0.0f && dataValue <= 1.0f) {
    data.params[pSidebandBW] = dataValue;
    updateSidebandBW();
    return true;
  } else {
    return false;
  }
}

void SBSMSample :: updateSidebandBWScale()
{
  synth.sidebandBWScale = 0.03 * pow10f(3.0 * data.params[pSidebandBWScale]);
}

bool SBSMSample :: setSidebandBWScale(float value)
{
  float dataValue = log10f(value / .03) / 3.0;
  if(dataValue >= 0.0f && dataValue <= 1.0f) {
    data.params[pSidebandBWScale] = dataValue;
    updateSidebandBWScale();
    return true;
  } else {
    return false;
  }
}

void SBSMSample :: updateSidebandRelease()
{
  synth.sidebandRelease = pow10f(-pow10f(2.8 - 4.0 * data.params[pSidebandRelease])/Fs);
}

bool SBSMSample :: setSidebandRelease(float value)
{
  float dataValue = (2.8 - log10f(-log10f(value)*Fs)) / 4.0;
  if(dataValue >= 0.0f && dataValue <= 1.0f) {
    data.params[pSidebandRelease] = dataValue;
    updateSidebandRelease();
    return true;
  } else {
    return false;
  }
}

void SBSMSample :: updateModPivot()
{
  float value = data.params[pModPivot];
  synth.modPivot = -2.769563268113285f + (5.0f * (value - 0.5f));
}

bool SBSMSample :: setModPivot(float value)
{
  float dataValue = (value - -2.769563268113285f) / 5.0f + 0.5f;
  if(dataValue >= 0.0f && dataValue <= 1.0f) {
    data.params[pModPivot] = dataValue;
    updateModPivot();
    return true;
  } else {
    return false;
  }
}

void SBSMSample :: updateSidebandEnv()
{
  synth.sidebandEnv = data.params[pSidebandEnv];
}

bool SBSMSample :: setSidebandEnv(float value)
{
  float dataValue = value;
  if(dataValue >= 0.0f && dataValue <= 1.0f) {
    data.params[pSidebandEnv] = dataValue;
    updateSidebandEnv();
    return true;
  } else {
    return false;
  }
}

void SBSMSample :: updateVolume()
{	
  volume = 0.001 * exp(9.210340371976184 * data.params[pVolume]);
}

bool SBSMSample :: setVolume(float value)
{
  float dataValue = log10f(1000.0 * value) / 4.0;
  if(dataValue >= 0.0f && dataValue <= 1.0f) {
    data.params[pVolume] = dataValue;
    updateVolume();
    return true;
  } else {
    return false;
  }
}

void SBSMSample :: updatePitchbendRange()
{
  pitchbendRange = lrintf((PITCHBEND_MAX_RANGE-1) * data.params[pPitchbendRange]) + 1;
}

bool SBSMSample :: setPitchbendRange(int value)
{
  float dataValue = (value-1) / (float)(PITCHBEND_MAX_RANGE - 1);
  if(dataValue >= 0.0f && dataValue <= 1.0f) {
    data.params[pPitchbendRange] = dataValue;
    updatePitchbendRange();
    return true;
  } else {
    return false;
  }
}

void SBSMSample :: updateLeftPos()
{
  leftPos = data.params[pLeftPos];
  updateRate();
  for(int k=0;k<NUM_VOICES;k++) {
	  voices[k]->setLeftPos(leftPos);
  }
}

bool SBSMSample :: setLeftPos(float value)
{
  float dataValue = value;
  if(dataValue >= 0.0f && dataValue <= 1.0f) {
    data.params[pLeftPos] = dataValue;
    updateLeftPos();
    return true;
  } else {
    return false;
  }
}

void SBSMSample :: updateRightPos()
{
  rightPos = data.params[pRightPos];
  updateRate();
  for(int k=0;k<NUM_VOICES;k++) {
	  voices[k]->setRightPos(rightPos);
  }
}

bool SBSMSample :: setRightPos(float value)
{
  float dataValue = value;
  if(dataValue >= 0.0f && dataValue <= 1.0f) {
    data.params[pRightPos] = dataValue;
    updateRightPos();
    return true;
  } else {
    return false;
  }
}

void SBSMSample :: updateStartPos()
{
  startPos = max(data.params[pLeftPos],min(data.params[pRightPos],data.params[pStartPos]));
  updateRate();
  for(int k=0;k<NUM_VOICES;k++) {
	  voices[k]->setStartPos(startPos);
  }
}


bool SBSMSample :: setStartPos(float value)
{
  float dataValue = value;
  if(dataValue >= 0.0f && dataValue <= 1.0f) {
    data.params[pStartPos] = dataValue;
    updateStartPos();
    return true;
  } else {
    return false;
  }
}

void SBSMSample :: updateSynthMode()
{
  synth.synthMode = lrintf(6.0f * data.params[pSynthMode]);
}

bool SBSMSample :: setSynthMode(float value)
{
  float dataValue = value / 6.0f;
  if(dataValue >= 0.0f && dataValue <= 1.0f) {
    data.params[pSynthMode] = dataValue;
    updateSynthMode();
    return true;
  } else {
    return false;
  }
}

void SBSMSample :: updateCombFB0()
{
  // 1 / (1-fb) = v
  // v - fb v = 1
  // fb  = (v - 1) / v
  float v = 64.0 * data.params[pCombFB0];
  synth.combFB0 = v / (1.0 + v);
}

bool SBSMSample :: setCombFB0(float value)
{
  float dataValue = 1.0 / (1.0 - value) - 1.0;
  if(dataValue >= 0.0f && dataValue <= 1.0f) {
    data.params[pCombFB0] = dataValue;
    updateCombFB0();
    return true;
  } else {
    return false;
  }
}

void SBSMSample :: updateCombFB1()
{
  synth.combFB1 = 2.0f * (data.params[pCombFB1] - 0.5f);
}

bool SBSMSample :: setCombFB1(float value)
{
  float dataValue = 0.5f * (value + 1.0f);
  if(dataValue >= 0.0f && dataValue <= 1.0f) {
    data.params[pCombFB1] = dataValue;
    updateCombFB1();
    return true;
  } else {
    return false;
  }
}

void SBSMSample :: updateCombFB1Mode()
{	
  synth.combFB1Mode = lrintf(data.params[pCombFB1Mode]);
}

bool SBSMSample :: setCombFB1Mode(float value)
{
  float dataValue = value;
  if(dataValue >= 0.0f && dataValue <= 1.0f) {
    data.params[pCombFB1Mode] = dataValue;
    updateCombFB1Mode();
    return true;
  } else {
    return false;
  }
}

void SBSMSample :: updateDecBits0()
{
  synth.decBits0 = 4.0 + 28.0 * data.params[pDecBits0];
}

bool SBSMSample :: setDecBits0(float value)
{
  float dataValue = (value - 4.0) / 28.0f;
  if(dataValue >= 0.0f && dataValue <= 1.0f) {
    data.params[pDecBits0] = dataValue;
    updateDecBits0();
    return true;
  } else {
    return false;
  }
}

void SBSMSample :: updateDecBits1()
{
  synth.decBits1 = 2.0f * (data.params[pDecBits1] - 0.5);
}

bool SBSMSample :: setDecBits1(float value)
{
  float dataValue = 0.5f * (value + 1.0f);
  if(dataValue >= 0.0f && dataValue <= 1.0f) {
    data.params[pDecBits1] = dataValue;
    updateDecBits1();
    return true;
  } else {
    return false;
  }
}

void SBSMSample :: updateDecBits1Mode()
{	
  synth.decBits1Mode = lrintf(data.params[pDecBits1Mode]);
}

bool SBSMSample :: setDecBits1Mode(float value)
{
  float dataValue = value;
  if(dataValue >= 0.0f && dataValue <= 1.0f) {
    data.params[pDecBits1Mode] = dataValue;
    updateDecBits1Mode();
    return true;
  } else {
    return false;
  }
}

void SBSMSample :: updateGranMode()
{
  synth.granMode = lrintf(6.0f * data.params[pGranMode]);
}

bool SBSMSample :: setGranMode(float value)
{
  float dataValue = value / 6.0f;
  if(dataValue >= 0.0f && dataValue <= 1.0f) {
    data.params[pGranMode] = dataValue;
    updateGranMode();
    return true;
  } else {
    return false;
  }
}

void SBSMSample :: updateGranRate0()
{
  synth.granRate0 = data.params[pGranRate0];
}

bool SBSMSample :: setGranRate0(float value)
{
  float dataValue = value;
  if(dataValue >= 0.0f && dataValue <= 1.0f) {
    data.params[pGranRate0] = dataValue;
    updateGranRate0();
    return true;
  } else {
    return false;
  }
}

void SBSMSample :: updateGranRate1()
{
  synth.granRate1 = 2.0f * (data.params[pGranRate1] - 0.5);
}

bool SBSMSample :: setGranRate1(float value)
{
  float dataValue = 0.5f * (value + 1.0f);
  if(dataValue >= 0.0f && dataValue <= 1.0f) {
    data.params[pGranRate1] = dataValue;
    updateGranRate1();
    return true;
  } else {
    return false;
  }
}

void SBSMSample :: updateGranRate1Mode()
{	
  synth.granRate1Mode = lrintf(data.params[pGranRate1Mode]);
}

bool SBSMSample :: setGranRate1Mode(float value)
{
  float dataValue = value;
  if(dataValue >= 0.0f && dataValue <= 1.0f) {
    data.params[pGranRate1Mode] = dataValue;
    updateGranRate1Mode();
    return true;
  } else {
    return false;
  }
}

void SBSMSample :: updateGranSmooth0()
{
  synth.granSmooth0 = data.params[pGranSmooth0];
}

bool SBSMSample :: setGranSmooth0(float value)
{
  float dataValue = value;
  if(dataValue >= 0.0f && dataValue <= 1.0f) {
    data.params[pGranSmooth0] = dataValue;
    updateGranSmooth0();
    return true;
  } else {
    return false;
  }
}

void SBSMSample :: updateGranSmooth1()
{
  synth.granSmooth1 = 2.0f * (data.params[pGranSmooth1] - 0.5);
}

bool SBSMSample :: setGranSmooth1(float value)
{
  float dataValue = 0.5f * (value + 1.0f);
  if(dataValue >= 0.0f && dataValue <= 1.0f) {
    data.params[pGranSmooth1] = dataValue;
    updateGranSmooth1();
    return true;
  } else {
    return false;
  }
}

void SBSMSample :: updateGranSmooth1Mode()
{	
  synth.granSmooth1Mode = lrintf(data.params[pGranSmooth1Mode]);
}

bool SBSMSample :: setGranSmooth1Mode(float value)
{
  float dataValue = value;
  if(dataValue >= 0.0f && dataValue <= 1.0f) {
    data.params[pGranSmooth1Mode] = dataValue;
    updateGranSmooth1Mode();
    return true;
  } else {
    return false;
  }
}

void SBSMSample :: updateDWGSDecay0()
{
  synth.dwgsDecay0 = 0.04*pow(2.0,8.0*data.params[pDWGSDecay0]);
}

bool SBSMSample :: setDWGSDecay0(float value)
{
  float dataValue = log2f(25.0*value) / 8.0;
  if(dataValue >= 0.0f && dataValue <= 1.0f) {
    data.params[pDWGSDecay0] = dataValue;
    updateDWGSDecay0();
    return true;
  } else {
    return false;
  }
}

void SBSMSample :: updateDWGSDecay1()
{
  synth.dwgsDecay1 = 2.0f * (data.params[pDWGSDecay1] - 0.5);
}

bool SBSMSample :: setDWGSDecay1(float value)
{
  float dataValue = 0.5f * (value + 1.0f);
  if(dataValue >= 0.0f && dataValue <= 1.0f) {
    data.params[pDWGSDecay1] = dataValue;
    updateDWGSDecay1();
    return true;
  } else {
    return false;
  }
}

void SBSMSample :: updateDWGSDecay1Mode()
{	
  synth.dwgsDecay1Mode = lrintf(data.params[pDWGSDecay1Mode]);
}

bool SBSMSample :: setDWGSDecay1Mode(float value)
{
  float dataValue = value;
  if(dataValue >= 0.0f && dataValue <= 1.0f) {
    data.params[pDWGSDecay1Mode] = dataValue;
    updateDWGSDecay1Mode();
    return true;
  } else {
    return false;
  }
}

void SBSMSample :: updateDWGSLopass0()
{
  synth.dwgsLopass0 = 0.1*pow(2.0,8.0*data.params[pDWGSLopass0]);
}

bool SBSMSample :: setDWGSLopass0(float value)
{
  float dataValue = log2f(10.0*value) / 8.0;
  if(dataValue >= 0.0f && dataValue <= 1.0f) {
    data.params[pDWGSLopass0] = dataValue;
    updateDWGSLopass0();
    return true;
  } else {
    return false;
  }
}

void SBSMSample :: updateDWGSLopass1()
{
  synth.dwgsLopass1 = 2.0f * (data.params[pDWGSLopass1] - 0.5);
}

bool SBSMSample :: setDWGSLopass1(float value)
{
  float dataValue = 0.5f * (value + 1.0f);
  if(dataValue >= 0.0f && dataValue <= 1.0f) {
    data.params[pDWGSLopass1] = dataValue;
    updateDWGSLopass1();
    return true;
  } else {
    return false;
  }
}

void SBSMSample :: updateDWGSLopass1Mode()
{	
  synth.dwgsLopass1Mode = lrintf(data.params[pDWGSLopass1Mode]);
}

bool SBSMSample :: setDWGSLopass1Mode(float value)
{
  float dataValue = value;
  if(dataValue >= 0.0f && dataValue <= 1.0f) {
    data.params[pDWGSLopass1Mode] = dataValue;
    updateDWGSLopass1Mode();
    return true;
  } else {
    return false;
  }
}

void SBSMSample :: updateDWGSStringPos0()
{
  synth.dwgsStringPos0 = 0.5f * data.params[pDWGSStringPos0];
}

bool SBSMSample :: setDWGSStringPos0(float value)
{
  float dataValue = 2.0 * value;
  if(dataValue >= 0.0f && dataValue <= 1.0f) {
    data.params[pDWGSStringPos0] = dataValue;
    updateDWGSStringPos0();
    return true;
  } else {
    return false;
  }
}

void SBSMSample :: updateDWGSStringPos1()
{
  synth.dwgsStringPos1 = 2.0f * (data.params[pDWGSStringPos1] - 0.5);
}

bool SBSMSample :: setDWGSStringPos1(float value)
{
  float dataValue = 0.5f * (value + 1.0f);
  if(dataValue >= 0.0f && dataValue <= 1.0f) {
    data.params[pDWGSStringPos1] = dataValue;
    updateDWGSStringPos1();
    return true;
  } else {
    return false;
  }
}

void SBSMSample :: updateDWGSStringPos1Mode()
{	
  synth.dwgsStringPos1Mode = lrintf(data.params[pDWGSStringPos1Mode]);
}

bool SBSMSample :: setDWGSStringPos1Mode(float value)
{
  float dataValue = value;
  if(dataValue >= 0.0f && dataValue <= 1.0f) {
    data.params[pDWGSStringPos1Mode] = dataValue;
    updateDWGSStringPos1Mode();
    return true;
  } else {
    return false;
  }
}

void SBSMSample :: updateThresh()
{
  float v = data.params[pThresh];
  if(v == 0.0f) {
    synth.mThresh = 0.0f;
  } else {
    synth.mThresh = pow10f(6.0f * (v - 1.0f));
  }
}

bool SBSMSample :: setThresh(float value)
{
  float dataValue = log10f(value) / 6.0f + 1.0f;
  if(dataValue >= 0.0f && dataValue <= 1.0f) {
    data.params[pThresh] = dataValue;
    updateThresh();
    return true;
  } else {
    return false;
  }
}

void SBSMSample :: updateForce()
{
  synth.mForce = data.params[pForce];
}

bool SBSMSample :: setForce(float value)
{
  float dataValue = value;
  if(dataValue >= 0.0f && dataValue <= 1.0f) {
    data.params[pForce] = dataValue;
    updateForce();
    return true;
  } else {
    return false;
  }
}

void SBSMSample :: updateDrag()
{
  synth.mDrag = data.params[pDrag];
}

bool SBSMSample :: setDrag(float value)
{
  float dataValue = value;
  if(dataValue >= 0.0f && dataValue <= 1.0f) {
    data.params[pDrag] = dataValue;
    updateDrag();
    return true;
  } else {
    return false;
  }
}

void SBSMSample :: updateSpring()
{
  synth.mSpring = data.params[pSpring];
}

bool SBSMSample :: setSpring(float value)
{
  float dataValue = value;
  if(dataValue >= 0.0f && dataValue <= 1.0f) {
    data.params[pSpring] = dataValue;
    updateSpring();
    return true;
  } else {
    return false;
  }
}

void SBSMSample :: updateFollowMode()
{
  followMode = lrintf(2.0f * data.params[pFollowMode]);
}

bool SBSMSample :: setFollowMode(int value)
{
  float dataValue = value / 2.0f;
  if(dataValue >= 0.0f && dataValue <= 1.0f) {
    data.params[pFollowMode] = dataValue;
    updateFollowMode();
    return true;
  } else {
    return false;
  }
}

void SBSMSample :: updateLoop()
{
  loop = lrintf(6.0f * data.params[pLoop]);
  for(int k=0;k<NUM_VOICES;k++) {
	  voices[k]->setLoop(loop);
  }
  updateRate();
}

bool SBSMSample :: setLoop(int value)
{
  float dataValue = value / 6.0f;
  if(value >= 0 && value < 6) {
    data.params[pLoop] = dataValue;
    updateLoop();
    return true;
  } else {
    return false;
  }
}

void SBSMSample :: updateChannel()
{
  channel = lrintf(16.0f * data.params[pChannel]);
}

bool SBSMSample :: setChannel(int value)
{
  float dataValue = value / 16.0f;
  if(value >= 0 && value < 16) {
    channel = value;
    data.params[pChannel] = dataValue;
    return true;
  } else {
    return false;
  }
}

void SBSMSample :: updateNoteBase()
{
  noteBase = lrintf(11.0f * data.params[pOctaveBase]) * 12 + lrintf(12.0f * data.params[pKeyBase]);
}

void SBSMSample :: updateKeyBase()
{
  keyBase = lrintf(12.0f * data.params[pKeyBase]);
  updateNoteBase();
}

bool SBSMSample :: setKeyBase(int value)
{
  float dataValue = value / 12.0f;
  if(value >= 0 && value < 12) {
    keyBase = value;
    data.params[pKeyBase] = dataValue;
    updateNoteBase();
    return true;
  } else {
    return false;
  }
}

void SBSMSample :: updateOctaveBase()
{
  octaveBase = lrintf(11.0f * data.params[pOctaveBase]) - 2;
  updateNoteBase();
}

bool SBSMSample :: setOctaveBase(int value)
{
  float dataValue = (value + 2) / 11.0f;
  if(value >= -2 && value <= 8) {
    octaveBase = value;
    data.params[pOctaveBase] = dataValue;
    updateNoteBase();
    return true;
  } else {
    return false;
  }
}

void SBSMSample :: updateNoteLo()
{
  noteLo = lrintf(11.0f * data.params[pOctaveLo]) * 12 + lrintf(12.0f * data.params[pKeyLo]);
}

void SBSMSample :: updateKeyLo()
{
  keyLo = lrintf(12.0f * data.params[pKeyLo]);
  updateNoteLo();
}

bool SBSMSample :: setKeyLo(int value)
{
  float dataValue = value / 12.0f;
  if(value >= 0 && value < 12) {
    keyLo = value;
    data.params[pKeyLo] = dataValue;
    updateNoteLo();
    return true;
  } else {
    return false;
  }
}

void SBSMSample :: updateOctaveLo()
{
  octaveLo = lrintf(11.0f * data.params[pOctaveLo]) - 2;
  updateNoteLo();
}

bool SBSMSample :: setOctaveLo(int value)
{
  float dataValue = (value + 2) / 11.0f;
  if(value >= -2 && value <= 8) {
    octaveLo = value;
    data.params[pOctaveLo] = dataValue;
    updateNoteLo();
    return true;
  } else {
    return false;
  }
}

void SBSMSample :: updateNoteHi()
{
  noteHi = lrintf(11.0f * data.params[pOctaveHi]) * 12 + lrintf(12.0f * data.params[pKeyHi]);
}

void SBSMSample :: updateKeyHi()
{
  keyHi = lrintf(12.0f * data.params[pKeyHi]);
  updateNoteHi();
}

bool SBSMSample :: setKeyHi(int value)
{
  float dataValue = value / 12.0f;
  if(value >= 0 && value < 12) {
    keyHi = value;
    data.params[pKeyHi] = dataValue;
    updateNoteHi();
    return true;
  } else {
    return false;
  }
}

void SBSMSample :: updateOctaveHi()
{
  octaveHi = lrintf(11.0f * data.params[pOctaveHi]) - 2;
  updateNoteHi();
}

bool SBSMSample :: setOctaveHi(int value)
{
  float dataValue = (value + 2) / 11.0f;
  if(value >= -2 && value <= 8) {
    octaveHi = value;
    data.params[pOctaveHi] = dataValue;
    updateNoteHi();
    return true;
  } else {
    return false;
  }
}

void SBSMSample :: close()
{
  for(int k=0;k<NUM_VOICES;k++) {
    voices[k]->close();
  }
  bOpen = false;
}

void SBSMSample :: getName(string &str)
{
  str.assign(data.name);
}

void SBSMSample :: setName(const string &str)
{
   vst_strncpy(data.name, str.c_str(), kNameLength);
}

bool SBSMSample :: isFileName(const string &str)
{
  return (str.compare(data.filename) == 0);
}

void SBSMSample :: getFileName(string &str)
{
  str.assign(data.filename);
}

void SBSMSample :: setFileName(const string &str)
{
  vst_strncpy(data.filename, str.c_str(), kFileNameLength);
}

void SBSMSample :: setData(void *buf)
{
  memcpy(&data,buf,sizeof(SBSMSampleData));
}

void SBSMSample :: getData(void *buf)
{
  memcpy(buf,&data,sizeof(SBSMSampleData));
}

void SBSMSample :: setSample(SBSMS *sbsms, countType samplesToProcess)
{ 
  this->samplesToProcess = samplesToProcess;
  updateRate();
  if(sbsms) {
	  for(int k=0;k<NUM_VOICES;k++) {
      voices[k]->open(sbsms,samplesToProcess);
	  }
	  bOpen = true;
  }
}

int SBSMSample :: getNoteBase()
{
  return noteBase;
}

int SBSMSample :: getNoteLo()
{
  return noteLo;
}

int SBSMSample :: getNoteHi()
{
  return noteHi;
}

int SBSMSample :: getChannel()
{
  return channel;
}

bool SBSMSample :: isOpen()
{
	return bOpen;
}

SBSMSVoice* SBSMSample :: getAvailableVoice()
{
  if(indexQueue.empty())
    return NULL;
  else {
    SBSMSVoice *front = voices[indexQueue.front()];
    if(front->isReady()) {
      indexQueue.pop();
      numVoices++;
      return front;
    } else {
      return NULL;
    }
  }
}

void SBSMSample :: returnVoice(SBSMSVoice *v)
{
  numVoices--;
  indexQueue.push(v->index);
}

SBSMSVoice *SBSMSample :: triggerOn(int note, int velocity, int latency)
{
  if(!bOpen) return NULL;
  SBSMSVoice *v = notes[note];
  bool bFollow = (followMode && lastVoice && numVoices);
  if(v) {
    char str[1024];
    sprintf(str,"%d -> force %d\n",note,v->index);
    printf(str);
    v->forceOff();
  }
  v = getAvailableVoice();

  if(v) {      
    char str[1024];
    sprintf(str,"%d -> on %d\n",note,v->index);
    printf(str);
    float pitch = pow(2.0f,(note-noteBase)/12.0f);
    float baseFreq = pow(2.0f,(noteBase-68)/12.0f)*440.0;
    float vol = (float)velocity/127.0f;
    if(bFollow) {
      v->setStartSample(lastVoice->getCurrSample());
    }
    v->triggerOn(pitch, vol, baseFreq, latency);
    notes[note] = v;
    lastVoice = v;
  } else{     
    char str[1024];
    sprintf(str,"failed\n");
    printf(str);
  }

  return v;
}

void SBSMSample :: setTime(VstTimeInfo *time)
{
  for(int k=0;k<NUM_VOICES;k++) {
	  voices[k]->setTime(time);
  }  
}

void SBSMSample :: triggerOff(int note, int latency)
{
  if(!bOpen) return;
  SBSMSVoice *v = notes[note];
  if(v) {    
    char str[1024];
    sprintf(str,"%d -> off %d\n",note, v->index);
    printf(str);
    v->triggerOff(latency);
  }
  notes[note] = NULL;
}

void SBSMSample :: setPitchbend(float value)
{	  
  float pbend = pow(2.0f,(float)pitchbendRange*value/12.0f);
  for(int k=0;k<NUM_VOICES;k++) {
	  voices[k]->setPitchbend(pbend);
  }
}

void SBSMSample :: setChannelAfterTouch(float value)
{
}

void SBSMSample :: setPolyphonicAfterTouch(int note, float value)
{
}
