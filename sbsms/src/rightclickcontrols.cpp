//	RightClickControls.cpp - Sliders & knobs which will centre on a right click.
//	--------------------------------------------------------------------------
//	Copyright (c) 2004 Niall Moody
//	
//	Permission is hereby granted, free of charge, to any person obtaining a
//	copy of this software and associated documentation files (the "Software"),
//	to deal in the Software without restriction, including without limitation
//	the rights to use, copy, modify, merge, publish, distribute, sublicense,
//	and/or sell copies of the Software, and to permit persons to whom the
//	Software is furnished to do so, subject to the following conditions:

//	The above copyright notice and this permission notice shall be included in
//	all copies or substantial portions of the Software.

//	THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
//	IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
//	FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL
//	THE AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
//	LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING
//	FROM, OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER
//	DEALINGS IN THE SOFTWARE.
//	--------------------------------------------------------------------------

#include "RightClickControls.h"

//----------------------------------------------------------------------------
RightClickAnimKnob::RightClickAnimKnob(const CRect &size,
									   CControlListener *listener,
									   long tag, 
									   long subPixmaps,
									   long heightOfOneImage,
									   CBitmap *background,
									   CPoint &offset):
CAnimKnob(size, listener, tag, subPixmaps, heightOfOneImage, background, offset)
{
	
}

RightClickAnimKnob::~RightClickAnimKnob()
{
	
}

bool RightClickAnimKnob::checkDefaultValue(CDrawContext *pContext,
										   long button)
{
	if((button==(kControl|kLButton))||(button==kRButton))
	{
		// begin of edit parameter
		beginEdit();
	
		value = getDefaultValue();
		if(isDirty()&&listener)
			listener->valueChanged(pContext, this);

		// end of edit parameter
		endEdit();
		return true;
	}
	return false;
}

void RightClickAnimKnob::mouse(CDrawContext *pContext, CPoint &where, long button)
{
	if(!bMouseEnabled)
		return;

	if(button == -1)
		button = pContext->getMouseButtons();
	if((!(button&kLButton))&&(!(button&kRButton)))
		return;

	if(listener&&(button&(kAlt|kShift|kControl|kApple)))
	{
		if(listener->controlModifierClicked(pContext, this, button)!=0)
			return;
	}

	// check if default value wanted
	if(checkDefaultValue(pContext, button))
		return;

	float old = oldValue;
	CPoint firstPoint;
	bool  modeLinear = false;
	float fEntryState = value;
	float middle = (vmax - vmin) * 0.5f;
	float range = 200.f;
	float coef = (vmax - vmin)/range;
	long  oldButton = button;

	long mode    = kCircularMode;
	long newMode = getFrame()->getKnobMode();
	if(kLinearMode == newMode)
	{
		if(!(button & kAlt))
			mode = newMode;
	}
	else if(button & kAlt) 
		mode = kLinearMode;

	if((mode == kLinearMode)&&(button & kLButton))
	{
		if(button & kShift)
			range *= zoomFactor;
		firstPoint = where;
		modeLinear = true;
		coef = (vmax - vmin) / range;
	}
	else
	{
		CPoint where2(where);
		where2.offset(-size.left, -size.top);
		old = valueFromPoint(where2);
	}

	CPoint oldWhere(-1, -1);

	// begin of edit parameter
	beginEdit();
	do
	{
		button = pContext->getMouseButtons();
		if(where != oldWhere)
		{
			oldWhere = where;
			if(modeLinear)
			{
				long diff = (firstPoint.v - where.v) + (where.h - firstPoint.h);
				if(button != oldButton)
				{
					range = 200.f;
					if(button & kShift)
						range *= zoomFactor;
	
					float coef2 = (vmax - vmin) / range;
					fEntryState += diff * (coef - coef2);
					coef = coef2;
					oldButton = button;
				}
				value = fEntryState + diff * coef;
				bounceValue();
			}
			else
			{
				where.offset(-size.left, -size.top);
				value = valueFromPoint(where);
				if((old-value) > middle)
					value = vmax;
				else if((value-old) > middle)
					value = vmin;
				else
					old = value;
			}
			if(isDirty()&&listener)
				listener->valueChanged(pContext, this);
		}
		getMouseLocation(pContext, where);
		doIdleStuff();

	} while(button&kLButton);

	// end of edit parameter
	endEdit();
}

//----------------------------------------------------------------------------
RightClickSlider::RightClickSlider(const CRect &size,
								   CControlListener *listener,
								   long tag, 
								   long    iMinPos,
								   long    iMaxPos,
								   CBitmap *handle,
								   CBitmap *background,
								   CPoint  &offset,
								   const long style):
CSlider(size, listener, tag, iMinPos, iMaxPos, handle, background, offset, style)
{
	
}

RightClickSlider::~RightClickSlider()
{
	
}

bool RightClickSlider::checkDefaultValue(CDrawContext *pContext,
										   long button)
{
	if((button==(kControl|kLButton))||(button==kRButton))
	{
		// begin of edit parameter
		beginEdit();
	
		value = getDefaultValue();
		if(isDirty()&&listener)
			listener->valueChanged(pContext, this);

		// end of edit parameter
		endEdit();
		return true;
	}
	return false;
}

bool RightClickSlider::attached(CView *parent)
{
	return CControl::attached(parent);
}

bool RightClickSlider::removed(CView *parent)
{
	return CControl::removed(parent);
}

void RightClickSlider::draw(CDrawContext *context)
{
	float fValue;
	CDrawContext* drawContext = context;

	if(pOScreen&&bTransparencyEnabled)
		pOScreen->copyTo(context, size);

	if((style&kLeft)||(style&kTop))
		fValue = value;
	else 
		fValue = 1.f - value;

	// (re)draw background
	CRect rect(0, 0, widthControl, heightControl);
	if(!pOScreen)
		rect.offset(size.left, size.top);
	if(pBackground)
	{
		if(bTransparencyEnabled)
			pBackground->drawTransparent(drawContext, rect, offset);
		else
			pBackground->draw(drawContext, rect, offset);
	}

	// calc new coords of slider
	CRect rectNew;
	if(style & kHorizontal)
	{
		rectNew.top    = offsetHandle.v;
		rectNew.bottom = rectNew.top + heightOfSlider;	

		rectNew.left   = offsetHandle.h + (int)(fValue * rangeHandle);
		rectNew.left   = (rectNew.left < minTmp) ? minTmp : rectNew.left;

		rectNew.right  = rectNew.left + widthOfSlider;
		rectNew.right  = (rectNew.right > maxTmp) ? maxTmp : rectNew.right;
	}
	else
	{
		rectNew.left   = offsetHandle.h;
		rectNew.right  = rectNew.left + widthOfSlider;	

		rectNew.top    = offsetHandle.v + static_cast<int>(fValue * rangeHandle);
		rectNew.top    = (rectNew.top < minTmp) ? minTmp : rectNew.top;

		rectNew.bottom = rectNew.top + heightOfSlider;
		rectNew.bottom = (rectNew.bottom > maxTmp) ? maxTmp : rectNew.bottom;
	}
	rectNew.offset(size.left, size.top);

	// draw slider at new position
	if(pHandle)
	{
		if(bDrawTransparentEnabled)
			pHandle->drawTransparent(drawContext, rectNew);
		else 
			pHandle->draw(drawContext, rectNew);
	}
	
	setDirty(false);
}

//----------------------------------------------------------------------------
RightClickHSlider::RightClickHSlider(const CRect &size,
									 CControlListener *listener,
									 long tag, 
									 long iMinPos,
									 long iMaxPos,
									 CBitmap *handle,
									 CBitmap *background,
									 CPoint &offset,
									 const long style):
RightClickSlider(size, listener, tag, iMinPos, iMaxPos, handle, background, offset, style|kHorizontal)
{
	
}

//----------------------------------------------------------------------------
RightClickVSlider::RightClickVSlider(const CRect &size,
									 CControlListener *listener,
									 long tag, 
									 long iMinPos,
									 long iMaxPos,
									 CBitmap *handle,
									 CBitmap *background,
									 CPoint &offset,
									 const long style):
RightClickSlider(size, listener, tag, iMinPos, iMaxPos, handle, background, offset, style|kVertical)
{
	
}
