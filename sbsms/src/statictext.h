//	CStaticText.h - Simple static text control.
//	--------------------------------------------------------------------------
//	Copyright (c) 2005 Niall Moody
//	
//	Permission is hereby granted, free of charge, to any person obtaining a
//	copy of this software and associated documentation files (the "Software"),
//	to deal in the Software without restriction, including without limitation
//	the rights to use, copy, modify, merge, publish, distribute, sublicense,
//	and/or sell copies of the Software, and to permit persons to whom the
//	Software is furnished to do so, subject to the following conditions:
//
//	The above copyright notice and this permission notice shall be included in
//	all copies or substantial portions of the Software.
//
//	THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
//	IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
//	FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL
//	THE AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
//	LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING
//	FROM, OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER
//	DEALINGS IN THE SOFTWARE.
//	--------------------------------------------------------------------------

#ifndef CSTATICTEXT_H_
#define CSTATICTEXT_H_

#include "vstcontrols.h"

///	Simple, un-editable text display.
/*!
	You use it the same way you'd set text in CTextEdit, but you can't edit
	the text by clicking on it.
 */
class CStaticText : public CParamDisplay
{
 public:
	///	Constructor.
	CStaticText(const CRect &size, CBitmap *background = 0, const long style = 0);
	///	Destructor.
	~CStaticText() {};

	///	Sets the widget's text to that of text.
	void setText(char *text);

	///	Draws the widget.
	void draw(CDrawContext *pContext);
 
 protected:
	///	Can't remember why I needed to override this...
	void drawText(CDrawContext *pContext, char *string, CBitmap *newBack = 0);

	char Text[256];
};

#endif
