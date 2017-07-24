#include "genctrl.h"
#include "sbsmsamplergui.h"

GenCtrl :: GenCtrl(CControlListener *listener, CRect size, CFrame *frame, CBitmap *bitmap) : CViewContainer(size, frame, bitmap)
{
  this->listener = listener;
  bmOnOff = new CBitmap(kOnOff);
  bmKnob = new CBitmap(kKnob);
}

GenCtrl :: ~GenCtrl()
{
  bmOnOff->forget();
	bmKnob->forget();
}

OscCtrl :: OscCtrl(CControlListener *listener, CRect rect, CFrame *frame, CBitmap *bitmap) : GenCtrl(listener, rect, frame, bitmap)
{
  setMode (kOnlyDirtyUpdate);
  CRect size;
  CPoint point;
  int x,y,w,h;
  int x2,y2,w2,h2;

  x = 5;
  y = 25;
  w = bmKnob->getWidth();
  h = bmKnob->getHeight()/59;  

  // Distortion 0
  size(x, y, x+w, y+h);
  point(0, 0);
  knobDistortion0 = new RightClickAnimKnob(size, listener, tagDistortion0, 59, bmKnob->getHeight()/59, bmKnob, point);
  knobDistortion0->setDefaultValue(0.0f);
  addView(knobDistortion0);

  x += 48;
  // Distortion 1
  size(x, y, x+w, y+h);
  point(0, 0);
  knobDistortion1 = new RightClickAnimKnob(size, listener, tagDistortion1, 59, bmKnob->getHeight()/59, bmKnob, point);
  knobDistortion1->setDefaultValue(0.5f);
  addView(knobDistortion1);

  x2 = x + 22;
  y2 = y + 35;
  w2 = bmOnOff->getWidth();
  h2 = bmOnOff->getHeight()/2;
  size(x2, y2, x2+w2, y2+h2);
  onoffDistortion1Mode = new COnOffButton(size, listener, tagDistortion1Mode, bmOnOff);
  addView(onoffDistortion1Mode);

}

CombCtrl :: CombCtrl(CControlListener *listener, CRect rect, CFrame *frame, CBitmap *bitmap) : GenCtrl(listener, rect, frame, bitmap)
{
  setMode (kOnlyDirtyUpdate);
  CRect size;
  CPoint point;
  int x,y,w,h;
  int x2,y2,w2,h2;

  x = 5;
  y = 5;
  w = bmKnob->getWidth();
  h = bmKnob->getHeight()/59;  

  // Comb FB0
  size(x, y, x+w, y+h);
  point(0, 0);
  knobCombFB0 = new RightClickAnimKnob(size, listener, tagCombFB0, 59, bmKnob->getHeight()/59, bmKnob, point);
  knobCombFB0->setDefaultValue(0.5f);
  addView(knobCombFB0);

  x += 48;
  // Comb FB1
  size(x, y, x+w, y+h);
  point(0, 0);
  knobCombFB1 = new RightClickAnimKnob(size, listener, tagCombFB1, 59, bmKnob->getHeight()/59, bmKnob, point);
  knobCombFB1->setDefaultValue(0.5f);
  addView(knobCombFB1);

  x2 = x + 22;
  y2 = y + 35;
  w2 = bmOnOff->getWidth();
  h2 = bmOnOff->getHeight()/2;
  size(x2, y2, x2+w2, y2+h2);
  onoffCombFB1Mode = new COnOffButton(size, listener, tagCombFB1Mode, bmOnOff);
  addView(onoffCombFB1Mode);
}

BandpassCtrl :: BandpassCtrl(CControlListener *listener, CRect rect, CFrame *frame, CBitmap *bitmap) : GenCtrl(listener, rect, frame, bitmap)
{
  setMode (kOnlyDirtyUpdate);
  CRect size;
  CPoint point;
  int x,y,w,h;
  int x2,y2,w2,h2;

  x = 5;
  y = 25;
  w = bmKnob->getWidth();
  h = bmKnob->getHeight()/59;  

  // Filter Q0
  size(x, y, x+w, y+h);
  point(0, 0);
  knobFilterQ0 = new RightClickAnimKnob(size, listener, tagFilterQ0, 59, bmKnob->getHeight()/59, bmKnob, point);
  knobFilterQ0->setDefaultValue(0.5f);
  addView(knobFilterQ0);

  x += 48;
  // Filter Q1
  size(x, y, x+w, y+h);
  point(0, 0);
  knobFilterQ1 = new RightClickAnimKnob(size, listener, tagFilterQ1, 59, bmKnob->getHeight()/59, bmKnob, point);
  knobFilterQ1->setDefaultValue(0.5f);
  addView(knobFilterQ1);

  x2 = x + 22;
  y2 = y + 35;
  w2 = bmOnOff->getWidth();
  h2 = bmOnOff->getHeight()/2;
  size(x2, y2, x2+w2, y2+h2);
  onoffFilterQ1Mode = new COnOffButton(size, listener, tagFilterQ1Mode, bmOnOff);
  addView(onoffFilterQ1Mode);
}

DecimateCtrl :: DecimateCtrl(CControlListener *listener, CRect rect, CFrame *frame, CBitmap *bitmap) : GenCtrl(listener, rect, frame, bitmap)
{
  setMode (kOnlyDirtyUpdate);
  CRect size;
  CPoint point;
  int x,y,w,h;
  int x2,y2,w2,h2;

  x = 5;
  y = 5;
  w = bmKnob->getWidth();
  h = bmKnob->getHeight()/59;  

  // Dec Bits0
  size(x, y, x+w, y+h);
  point(0, 0);
  knobDecBits0 = new RightClickAnimKnob(size, listener, tagDecBits0, 59, bmKnob->getHeight()/59, bmKnob, point);
  knobDecBits0->setDefaultValue(1.0f);
  addView(knobDecBits0);

  x += 48;
  // Dec Bits1
  size(x, y, x+w, y+h);
  point(0, 0);
  knobDecBits1 = new RightClickAnimKnob(size, listener, tagDecBits1, 59, bmKnob->getHeight()/59, bmKnob, point);
  knobDecBits1->setDefaultValue(0.5f);
  addView(knobDecBits1);

  x2 = x + 22;
  y2 = y + 35;
  w2 = bmOnOff->getWidth();
  h2 = bmOnOff->getHeight()/2;
  size(x2, y2, x2+w2, y2+h2);
  onoffDecBits1Mode = new COnOffButton(size, listener, tagDecBits1Mode, bmOnOff);
  addView(onoffDecBits1Mode);
}

GranCtrl :: GranCtrl(CControlListener *listener, CRect rect, CFrame *frame, CBitmap *bitmap) : GenCtrl(listener, rect, frame, bitmap)
{
  setMode (kOnlyDirtyUpdate);
  CRect size;
  CPoint point;
  int x,y,w,h;
  int x2,y2,w2,h2;

  // gran mode
  x = 5;
  y = 5;
  h = 12;
  w = 28;
  size(x, y, x+w, y+h);
  optionGranMode = new COptionMenu(size, listener, tagGranMode, 0, 0, kPopupStyle);
  optionGranMode->addEntry((char*)"lll",-1);
  optionGranMode->addEntry((char*)"lls",-1);
  optionGranMode->addEntry((char*)"lsl",-1);
  optionGranMode->addEntry((char*)"lss",-1);
  optionGranMode->addEntry((char*)"sll",-1);
  optionGranMode->addEntry((char*)"sls",-1);
  addView(optionGranMode);
	optionGranMode->setFont(kNormalFontSmall);


  x = 34;
  y = 5;
  w = bmKnob->getWidth();
  h = bmKnob->getHeight()/59;  

  // Gran rate0
  size(x, y, x+w, y+h);
  point(0, 0);
  knobGranRate0 = new RightClickAnimKnob(size, listener, tagGranRate0, 59, bmKnob->getHeight()/59, bmKnob, point);
  knobGranRate0->setDefaultValue(0.0f);
  addView(knobGranRate0);

  x += 48;
  // Gran rate1
  size(x, y, x+w, y+h);
  point(0, 0);
  knobGranRate1 = new RightClickAnimKnob(size, listener, tagGranRate1, 59, bmKnob->getHeight()/59, bmKnob, point);
  knobGranRate1->setDefaultValue(0.5f);
  addView(knobGranRate1);

  x2 = x + 22;
  y2 = y + 35;
  w2 = bmOnOff->getWidth();
  h2 = bmOnOff->getHeight()/2;
  size(x2, y2, x2+w2, y2+h2);
  onoffGranRate1Mode = new COnOffButton(size, listener, tagGranRate1Mode, bmOnOff);
  addView(onoffGranRate1Mode);

  x += 48;
  // Gran smooth0
  size(x, y, x+w, y+h);
  point(0, 0);
  knobGranSmooth0 = new RightClickAnimKnob(size, listener, tagGranSmooth0, 59, bmKnob->getHeight()/59, bmKnob, point);
  knobGranSmooth0->setDefaultValue(0.5f);
  addView(knobGranSmooth0);

  x += 48;
  // Gran smooth1
  size(x, y, x+w, y+h);
  point(0, 0);
  knobGranSmooth1 = new RightClickAnimKnob(size, listener, tagGranSmooth1, 59, bmKnob->getHeight()/59, bmKnob, point);
  knobGranSmooth1->setDefaultValue(0.5f);
  addView(knobGranSmooth1);

  x2 = x + 22;
  y2 = y + 35;
  w2 = bmOnOff->getWidth();
  h2 = bmOnOff->getHeight()/2;
  size(x2, y2, x2+w2, y2+h2);
  onoffGranSmooth1Mode = new COnOffButton(size, listener, tagGranSmooth1Mode, bmOnOff);
  addView(onoffGranSmooth1Mode);
}


DWGSCtrl :: DWGSCtrl(CControlListener *listener, CRect rect, CFrame *frame, CBitmap *bitmap) : GenCtrl(listener, rect, frame, bitmap)
{
  setMode (kOnlyDirtyUpdate);
  CRect size;
  CPoint point;
  int x,y,w,h;
  int x2,y2,w2,h2;

  x = 5;
  y = 25;
  w = bmKnob->getWidth();
  h = bmKnob->getHeight()/59;  

  // DWGS Decay0
  size(x, y, x+w, y+h);
  point(0, 0);
  knobDWGSDecay0 = new RightClickAnimKnob(size, listener, tagDWGSDecay0, 59, bmKnob->getHeight()/59, bmKnob, point);
  knobDWGSDecay0->setDefaultValue(0.5f);
  addView(knobDWGSDecay0);

  x += 48;
  // DWGS Decay1
  size(x, y, x+w, y+h);
  point(0, 0);
  knobDWGSDecay1 = new RightClickAnimKnob(size, listener, tagDWGSDecay1, 59, bmKnob->getHeight()/59, bmKnob, point);
  knobDWGSDecay1->setDefaultValue(0.5f);
  addView(knobDWGSDecay1);

  x2 = x + 22;
  y2 = y + 35;
  w2 = bmOnOff->getWidth();
  h2 = bmOnOff->getHeight()/2;
  size(x2, y2, x2+w2, y2+h2);
  onoffDWGSDecay1Mode = new COnOffButton(size, listener, tagDWGSDecay1Mode, bmOnOff);
  addView(onoffDWGSDecay1Mode);

  x += 48;
  // DWGS Lopass0
  size(x, y, x+w, y+h);
  point(0, 0);
  knobDWGSLopass0 = new RightClickAnimKnob(size, listener, tagDWGSLopass0, 59, bmKnob->getHeight()/59, bmKnob, point);
  knobDWGSLopass0->setDefaultValue(0.5f);
  addView(knobDWGSLopass0);

  x += 48;
  // DWGS Lopass1
  size(x, y, x+w, y+h);
  point(0, 0);
  knobDWGSLopass1 = new RightClickAnimKnob(size, listener, tagDWGSLopass1, 59, bmKnob->getHeight()/59, bmKnob, point);
  knobDWGSLopass1->setDefaultValue(0.5f);
  addView(knobDWGSLopass1);

  x2 = x + 22;
  y2 = y + 35;
  w2 = bmOnOff->getWidth();
  h2 = bmOnOff->getHeight()/2;
  size(x2, y2, x2+w2, y2+h2);
  onoffDWGSLopass1Mode = new COnOffButton(size, listener, tagDWGSLopass1Mode, bmOnOff);
  addView(onoffDWGSLopass1Mode);

  x = 5;
  y = 91;
  // DWGS StringPos0
  size(x, y, x+w, y+h);
  point(0, 0);
  knobDWGSStringPos0 = new RightClickAnimKnob(size, listener, tagDWGSStringPos0, 59, bmKnob->getHeight()/59, bmKnob, point);
  knobDWGSStringPos0->setDefaultValue(2.0/7.0);
  addView(knobDWGSStringPos0);

  x += 48;
  // DWGS StringPos1
  size(x, y, x+w, y+h);
  point(0, 0);
  knobDWGSStringPos1 = new RightClickAnimKnob(size, listener, tagDWGSStringPos1, 59, bmKnob->getHeight()/59, bmKnob, point);
  knobDWGSStringPos1->setDefaultValue(2.0/7.0);
  addView(knobDWGSStringPos1);

  x2 = x + 10;
  y2 = y + 32;
  w2 = bmOnOff->getWidth();
  h2 = bmOnOff->getHeight()/2;
  size(x2, y2, x2+w2, y2+h2);
  onoffDWGSStringPos1Mode = new COnOffButton(size, listener, tagDWGSStringPos1Mode, bmOnOff);
  addView(onoffDWGSStringPos1Mode);
}
