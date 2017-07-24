#ifndef TEMPOCTRL_H_
#define TEMPOCTRL_H_

#include "vstcontrols.h"
#include "statictext.h"
#include "rightclickcontrols.h"
#include <utility>
#include <string>
#include <vector>

using namespace std;

class TempoCtrl : public CControlListener, public CViewContainer
{
  public:
  TempoCtrl(const CRect &size,
            const CRect &sizeKnob,
            CControlListener *listener,
            CFrame *parent,
            //ParamInterpretor *paramInterp,
            long tag, 
            long subPixmaps,
            long heightOfOneImage,
            CBitmap *background,
            CPoint &offset,
            vector< pair<float, string> > *optionItems = NULL,
            CBitmap *optionBmp = NULL,
            long tagMode = 0);

	~TempoCtrl();
  void setDefaultValue(float val);
  void valueChanged (CDrawContext *pContext, CControl *pControl);
  int getMode();
  void setMode(int mode);
  void drawValue();
  float getValue() const;
  void setValue(float val);
  void draw(CDrawContext *pContext);
  void setText(const char *text);

  vector< pair<float,string> > *options;
  CControlListener *listener;
  RightClickAnimKnob *knob;
  COnOffButton *optionBtn;
  COptionMenu *optionText;
  CStaticText *text;
  float values[2];
  long tagKnob, tagMode;
};

#endif
