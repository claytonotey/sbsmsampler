//	RightClickControls.h - Sliders & knobs which will centre on a right click.
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

#ifndef RIGHTCLICKCONTROLS_H_
#define RIGHTCLICKCONTROLS_H_

#include "vstcontrols.h"

//----------------------------------------------------------------------------
///	A subclass of CAnimKnob, which will go to it's default value when the user right-clicks on it.
class RightClickAnimKnob : public CAnimKnob
{
  public:
	///	Constructor.
	RightClickAnimKnob(const CRect &size,
					   CControlListener *listener,
					   long tag, 
					   long subPixmaps,
					   long heightOfOneImage,
					   CBitmap *background,
					   CPoint &offset);
	///	Desctructor.
	~RightClickAnimKnob();

	///	Was this new with VSTGUI v3?
	bool checkDefaultValue(CDrawContext *pContext, long button);

	///	Handles mouse movement in the widget.
	void mouse(CDrawContext *pContext, CPoint &where, long button=-1);
};

//----------------------------------------------------------------------------
///	A subclass of CSlider, which will go to it's default value when the user right-clicks on it.
/*!
	Also fixes the stupid attached/removed behaviour of the standard VSTGUI
	control, which could lead to crashes/hangs on windows, if you have a lot
	of CSliders on screen at once.
 */
class RightClickSlider : public CSlider
{
  public:
	///	Constructor.
	RightClickSlider(const CRect &size,
					 CControlListener *listener,
					 long tag, 
					 long    iMinPos,
					 long    iMaxPos,
					 CBitmap *handle,
					 CBitmap *background,
					 CPoint  &offset,
					 const long style = kLeft|kHorizontal);
	///	Destructor.
	virtual ~RightClickSlider();

	///	Was this new with VSTGUI v3?
	virtual bool checkDefaultValue(CDrawContext *pContext, long button);

	///	Called when the widget is attached to it's parent frame.
	virtual bool attached(CView *parent);
	///	Called when the widget is removed from it's parent frame.
	virtual bool removed(CView *parent);
	///	Draws the widget.
	virtual void draw(CDrawContext *context);
};

//----------------------------------------------------------------------------
///	A subclass of RightClickSlider, which has it's style set to being horizontal by default.
class RightClickHSlider : public RightClickSlider
{
  public:
	///	Constructor.
	RightClickHSlider(const CRect &size,
					  CControlListener *listener,
					  long tag, 
                      long    iMinPos,
                      long    iMaxPos,
                      CBitmap *handle,
                      CBitmap *background,
                      CPoint  &offset,
                      const long style = kRight);
};

//----------------------------------------------------------------------------
///	A subclass of RightClickSlider, which has it's style set to being vertical by default.
class RightClickVSlider : public RightClickSlider
{
  public:
	///	Constructor.
	RightClickVSlider(const CRect &size,
					  CControlListener *listener,
					  long tag, 
                      long    iMinPos,
                      long    iMaxPos,
                      CBitmap *handle,
                      CBitmap *background,
                      CPoint  &offset,
                      const long style = kBottom);
};

#endif
