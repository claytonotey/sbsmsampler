#include "wavedisplaysliders.h"
#include "vstsbsms.h"
#include "real.h"

CWaveDisplaySliders::CWaveDisplaySliders(CFrame *frame,
                                         const CRect &size,
                                         CControlListener *listener,
                                         long tag,
                                         long tag_L,
                                         long tag_R,
                                         long tag_S,
                                         CBitmap *handle)
 : CControl(size, listener, tag)
{
  setLoop(loopForward);
  this->frame = frame;
  this->renderSize = size;
  this->pHandle = handle;
  this->Tag_L = tag_L;  
  this->Tag_R = tag_R;
  this->Tag_S = tag_S;

  pthread_mutex_init(&posMutex,NULL);
  if(pHandle) {
    pHandle->remember();
    widthOfSlider  = pHandle->getWidth();
    heightOfSlider = pHandle->getHeight();
  } else {
    widthOfSlider  = 1; 
    heightOfSlider = 1;
  }
  halfWidthOfSlider = widthOfSlider / 2;

  xScale = (float)(size.width()-2*halfWidthOfSlider-1);
  r = NULL;
  offSize = CRect(0,0,renderSize.getWidth(),renderSize.getHeight());
  offContext = new COffscreenContext(frame,renderSize.getWidth(),renderSize.getHeight());
  offContext2 = new COffscreenContext(frame,renderSize.getWidth(),renderSize.getHeight());
  offContext2->setDrawMode(kOrMode);
  CColor col;
  col(255,255,255,255);
  offContext2->setFrameColor(col);
  bReallyDirty = true;
  value_L = 0.0f;
  value_S = 0.0f;
  value_R = 1.0f;
}

void CWaveDisplaySliders :: setLoop(int loop)
{
  this->loop = loop;
  loopLeftBounded = (loop == loopBackward || loop == loopBackwardLoop);
  loopRightBounded = (loop == loopForward || loop == loopForwardLoop);
}

void CWaveDisplaySliders :: setDirty2(bool bReallyDirty)
{
  if(bReallyDirty) this->bReallyDirty = true;
  CControl :: setDirty();
}

void CWaveDisplaySliders :: close() 
{
  if(r) { r->forget(); r = NULL; }
}

CWaveDisplaySliders :: ~CWaveDisplaySliders()
{
  close();
  offContext->forget();
  offContext2->forget();
  if(pHandle) pHandle->forget();
}

void CWaveDisplaySliders :: draw(CDrawContext *context)
{
  if(bReallyDirty) {
    bReallyDirty = false;

    // wave display
    if(r) {
      r->copyFrom(offContext,offSize);
    }

    // left pos
    CRect rectNew;
    rectNew.top = 0;
    rectNew.bottom = rectNew.top + heightOfSlider;	
    
    rectNew.left = lrintf(value_L * xScale);
    if(rectNew.left < 0)
      rectNew.left = 0;
    
    rectNew.right = rectNew.left + widthOfSlider;
    if(rectNew.right > size.width())
      rectNew.right = size.width();
      
    if(pHandle) {
      pHandle->drawTransparent(offContext, rectNew);
    }

     // right pos
    rectNew.top = 0;
    rectNew.bottom = rectNew.top + heightOfSlider;	
    
    rectNew.left = lrintf(value_R * xScale);
    if(rectNew.left < 0)
      rectNew.left = 0;
    
    rectNew.right = rectNew.left + widthOfSlider;
    if(rectNew.right > size.width())
      rectNew.right = size.width();
      
    if(pHandle) {
      pHandle->drawTransparent(offContext, rectNew);
    }

    // start pos
    rectNew.top = 0;
    rectNew.bottom = rectNew.top + heightOfSlider;	
    
    rectNew.left = lrintf(value_S * xScale);
    if(rectNew.left < 0)
      rectNew.left = 0;
    
    rectNew.right = rectNew.left + widthOfSlider;
    if(rectNew.right > size.width())
      rectNew.right = size.width();

    if(pHandle) {
      pHandle->drawTransparent(offContext, rectNew);
    }
  }
  offContext->copyFrom(offContext2, offSize);
  
  pthread_mutex_lock(&posMutex);
  set<int> xSet = this->xSet;
  pthread_mutex_unlock(&posMutex);

  for(set<int>::iterator i = xSet.begin(); i != xSet.end(); ++i) {
    int x = *i;
    CPoint p0(x,0);
    CPoint p1(x,renderSize.getHeight()-1);
    offContext2->moveTo(p0);        
    offContext2->lineTo(p1);
  }

  offContext2->copyFrom(context,renderSize);
}

void CWaveDisplaySliders :: setRenderer(SBSMSDrawRenderer *r)
{ 
  r->remember();
  if(this->r) this->r->forget();
  this->r = r;
  setDirty2(true);
}

SBSMSDrawRenderer *CWaveDisplaySliders :: getCurrentRenderer()
{
  return r;
}

SBSMSDrawRenderer* CWaveDisplaySliders :: getNewRenderer(countType samplesToProcess)
{
  close();
  r = new SBSMSDrawRenderer(samplesToProcess,frame,offSize,halfWidthOfSlider,halfWidthOfSlider);
  r->remember();
  return r;
}

void CWaveDisplaySliders::mouse(CDrawContext *pContext, CPoint &where, long button)
{
  CRect rect;
  long delta;
  long actualPos;
  float oldVal;

  if(!bMouseEnabled) return;
  bool bLR = false;

  if(button == -1) button = pContext->getMouseButtons();
  if(!((button & kLButton)||(button & kRButton))) return;
  if(listener&&(button&(kAlt|kShift|kControl|kApple))) {
    if(listener->controlModifierClicked(pContext, this, button) != 0) return;
  }
  if((button & kControl) || (button & kRButton)) {
    bLR = true;
  }
  float dv = (float)widthOfSlider / xScale;

  // start pos  
  if(!bLR) {
    delta = size.left;
    actualPos = lrintf(value_S * xScale) + size.left;  
    rect.left = actualPos;
    rect.top = size.top;
    rect.right = rect.left + widthOfSlider;
    rect.bottom = rect.top  + heightOfSlider;  
    if(where.isInside(rect)) {
      setTag(Tag_S);
      delta += where.h - actualPos;
      oldVal = value_S;
      beginEdit();      
      while(1) {
	      button = pContext->getMouseButtons();
	      if(!(button & kLButton)) break;	  
        value_S = (float)(where.h - delta)/xScale;	  
        if(loopLeftBounded && value_S < value_L) {
          value_S = value_L;
        } else if(loopRightBounded && value_S > value_R) {
          value_S = value_R;
        }
        if(oldVal != value_S) {
          setDirty2(true);
          listener->valueChanged(pContext, this);	  
        }
        oldVal = value_S;
	      pContext->getMouseLocation(where);	  
	      doIdleStuff();
	    }      
      endEdit();
    }
  } else {
    // left pos
    delta = size.left;
    actualPos = lrintf(value_L * xScale) + size.left;  
    rect.left = actualPos;
    rect.top = size.top;
    rect.right = rect.left + widthOfSlider;
    rect.bottom = rect.top  + heightOfSlider;  
    if(where.isInside(rect)) {
      bool bSAttached = false;
      delta += where.h - actualPos;
      oldVal = value_L;
      // begin of edit parameter
      beginEdit();      
      while(1) {
	      button = pContext->getMouseButtons();
	      if(!((button & kLButton)||(button & kRButton))) break;
	      value_L = (float)(where.h - delta)/xScale;	  
        if(value_L < 0.0f) {
	        value_L = 0.0f;
        } else if(value_L > value_R - dv) {
	        value_L = value_R - dv;
        } else if(loopLeftBounded && value_L > value_S) {
          bSAttached = true;
        }
        if(bSAttached) {
          value_S = value_L;
        }
        if(oldVal != value_L) { 
          setDirty2(true);
          setTag(Tag_L);
          listener->valueChanged(pContext, this);
          if(bSAttached) {
            setTag(Tag_S);
            listener->valueChanged(pContext, this);
          }
        }
        oldVal = value_L;
	      pContext->getMouseLocation(where);	  
	      doIdleStuff();
	    }   
      endEdit();
    }
 
    // right pos
    delta = size.left;
    actualPos = lrintf(value_R * xScale) + size.left;  
    rect.left = actualPos;
    rect.top = size.top;
    rect.right = rect.left + widthOfSlider;
    rect.bottom = rect.top  + heightOfSlider;  
    if(where.isInside(rect)) {
      bool bSAttached = false;
      delta += where.h - actualPos;
      oldVal = value_R;
      beginEdit();
      while(1) {
	      button = pContext->getMouseButtons();
	      if(!((button & kLButton)||(button & kRButton))) break;
	      value_R = (float)(where.h - delta)/xScale;
        if(value_R < value_L + dv) {
	        value_R = value_L + dv;
        } else if(value_R > 1.0f) {
	        value_R = 1.0f;
        } else if(loopRightBounded && value_R < value_S) {
          bSAttached = true;
        }
        if(bSAttached) {
          value_S = value_R;
        }
        if(oldVal != value_R) {
          setDirty2(true);
          setTag(Tag_R);
          listener->valueChanged(pContext, this);
          if(bSAttached) {
            setTag(Tag_S);
            listener->valueChanged(pContext, this);
          } 
        }
        oldVal = value_R;
	      pContext->getMouseLocation(where);
	      doIdleStuff();
	    }      
      endEdit();
    }
  }
}

void CWaveDisplaySliders :: setCurrPos(const set<float> &pos) 
{
  pthread_mutex_lock(&posMutex);
  set<int> newXSet;
  for(set<float>::const_iterator i = pos.begin(); i != pos.end(); ++i) {
    float value =  *i;
    int x = lrintf(value * xScale) + halfWidthOfSlider + 1;
    newXSet.insert(x);
  }
  if(!(xSet == newXSet)) {
    setDirty2(false);
    xSet = newXSet;
  }
  pthread_mutex_unlock(&posMutex);
}


void CWaveDisplaySliders :: setLeftPos(float value) 
{
  if(value_L != value) {
    value_L = value;
    setDirty2(true);
  }
}

void CWaveDisplaySliders :: setRightPos(float value) 
{ 
  if(value_R != value) {
    value_R = value;
    setDirty2(true);
  }
}

void CWaveDisplaySliders :: setStartPos(float value) 
{ 
  if(value_S != value) {
    value_S = value;
    setDirty2(true);
  }
}

float CWaveDisplaySliders :: getLeftPos()
{
    return value_L;
}

float CWaveDisplaySliders :: getRightPos()
{
    return value_R;
}

float CWaveDisplaySliders :: getStartPos()
{
    return value_S;
}

