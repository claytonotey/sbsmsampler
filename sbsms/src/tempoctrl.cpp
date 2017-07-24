#include "tempoctrl.h"
#include "vstcontrols.h"
#include <algorithm>

using namespace std;

//----------------------------------------------------------------------------
TempoCtrl::TempoCtrl(const CRect &sizeThis,
                     const CRect &sizeKnob,
                     CControlListener *listener,
                     //ParamInterpretor *paramInterp,
                     CFrame *parent,
                     long tagKnob, 
                     long subPixmaps,
                     long heightOfOneImage,
                     CBitmap *background,
                     CPoint &offset,
                     vector< pair<float,string> > *optionItems,
                     CBitmap *optionBmp,
                     long tagMode
                     ): CViewContainer(sizeThis,parent,0)
{
  CViewContainer::setMode (kOnlyDirtyUpdate);
  CPoint point(0,0);
  this->listener = listener;
  this->tagKnob = tagKnob;
  this->tagMode = tagMode;
  this->options = optionItems;


  CRect size;
  int x,y,h,w;
  x = 0;
  y = 0;
  h = sizeKnob.height();
  w = sizeKnob.width();
  size(x, y, x+w, y+h);

  knob = new RightClickAnimKnob(size, this, tagKnob, subPixmaps, heightOfOneImage, background, point);
  addView(knob);

  x = 29;
  y = 33;
  w = optionBmp->getWidth();
  h = optionBmp->getHeight()/2;
  size(x, y, x+w, y+h);
  optionBtn = new COnOffButton(size, this, tagMode, optionBmp);
  addView(optionBtn);

  x = 0;
  y = 33;
  h = 12;
  w = 30;
  size(x, y, x+w, y+h);

  text = new CStaticText(size);
  text->setTransparency(false);
  addView(text);
	text->setFont(kNormalFontSmall);
  text->setFontColor(kWhiteCColor);
  setTransparency(true);

  setMode(0);
}

TempoCtrl :: ~TempoCtrl()
{

}

void TempoCtrl::setDefaultValue(float val)
{
  values[0] = val;
  values[1] = val;
  knob->setDefaultValue(val);
}

void TempoCtrl::valueChanged(CDrawContext *pContext, CControl *control)
{
  long tag = control->getTag();
  float value;
  if(tag == tagKnob) {
    value = knob->getValue();
    setValue(value);
    listener->valueChanged(pContext,knob);
  } else if(tag == tagMode) {
    value = values[getMode()];
    setValue(value);
    listener->valueChanged(pContext,optionBtn);
  }
}

float TempoCtrl::getValue() const
{
  return knob->getValue();
}

void TempoCtrl::draw(CDrawContext *pContext)
{
  CViewContainer::draw(pContext);
  drawValue();
}

void TempoCtrl::setValue(float value)
{
  values[getMode()] = value;
  knob->setValue(value);
  drawValue();
}

int TempoCtrl::getMode()
{
  return optionBtn->getValue();
}

void TempoCtrl::setMode(int mode)
{
  optionBtn->setValue(mode);
  drawValue();
}

void TempoCtrl::setText(const char *s)
{
  if(getMode()==0) {
    text->setText((char*)s);
  }
}

void TempoCtrl::drawValue()
{
  float value = getValue();
  char s[6];
  if(getMode() == 0) {
    sprintf(s,"%.4g",value);
  } else {
    unsigned long k = lrintf(value * (options->size()-1));
    k = min(k,options->size()-1);
    k = max(k,0UL);
    sprintf(s,"%.4s",options->at(k).second.c_str());
  }
  text->setText(s);
}
