//	CStaticText.cpp - Simple static text control.
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

#include <string.h>
#include "statictext.h"

CStaticText::CStaticText(const CRect &size,
						 CBitmap *background,
						 const long style):
CParamDisplay(size, background, style)
{
	strcpy(Text, "");
}

void CStaticText::setText(char *text)
{
	strcpy(Text, text);
	setDirty(true);
}

void CStaticText::draw(CDrawContext *pContext)
{
	drawText(pContext, Text);
}

void CStaticText::drawText(CDrawContext *pContext, char *string, CBitmap *newBack)
{
	if(style&kNoDrawStyle)
		return;

	CRect recadendrum;
	pContext->setFillColor(kWhiteCColor);
	recadendrum.left = -1;
	recadendrum.right = getWidth() + 1;
	recadendrum.top = -1;
	recadendrum.bottom = getHeight() + 1;
	recadendrum.offset(size.left, size.top);
	pContext->fillRect(recadendrum);

	if(!bTransparencyEnabled)
	{
		pContext->setFillColor(backColor);
		pContext->fillRect(recadendrum);	
	}

	if(!(style&kNoTextStyle)&&string)
	{
		CRect oldClip;
		pContext->getClipRect(oldClip);
		CRect newClip(recadendrum);
		pContext->setClipRect(newClip);
		pContext->setFont(fontID, 0, txtFace);
	
		pContext->setFontColor(fontColor);
		pContext->drawString(string, recadendrum, !bTextTransparencyEnabled, horiTxtAlign);
	}

	setDirty(false);
}
