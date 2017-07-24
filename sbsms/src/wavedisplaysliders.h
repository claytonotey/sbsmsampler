#ifndef CWAVEDISPLAYSLIDERS_H_
#define CWAVEDISPLAYSLIDERS_H_

#include "vstcontrols.h"
#include "render.h"
#include "pthread.h"
#include <set>
using namespace _sbsms_;

class Drawer;

class CWaveDisplaySliders : public CControl
{
 public:
	CWaveDisplaySliders(CFrame *frame,
                         const CRect &size,
                         CControlListener *listener,
                         long tag,
                         long tag_L,
                         long tag_R,
                         long tag_S,
                         CBitmap *handle);
	~CWaveDisplaySliders();

	void draw(CDrawContext *pContext);
	void mouse(CDrawContext *pContext, CPoint &where, long button);

	void setLeftPos(float val);
	float getLeftPos();
  void setRightPos(float val);
	float getRightPos();
	void setStartPos(float val);
	float getStartPos();
  void setCurrPos(const set<float> &pos);
  void setDirty2(const bool bReallyDirty);
  void close();
  void setLoop(int loop);
  
  void setRenderer(SBSMSDrawRenderer *r);
  SBSMSDrawRenderer *getCurrentRenderer();
  SBSMSDrawRenderer *getNewRenderer(countType samplesToProcess);


 protected:
  bool bReallyDirty;
  pthread_mutex_t posMutex;
  set<int> xSet;
  float xScale;
	float value_L;
	long Tag_L;  
	float value_R;
	long Tag_R;
	float value_S;
	long Tag_S;
  int loop;
  bool loopLeftBounded;
  bool loopRightBounded;
  CFrame *frame;
  COffscreenContext *offContext;
  COffscreenContext *offContext2;
  SBSMSDrawRenderer *r;
	CBitmap *pHandle;
	CRect renderSize;
  CRect offSize;
  Drawer *drawer;
  pthread_t drawThread;
  pthread_mutex_t drawMutex;

	int widthOfSlider;
  int halfWidthOfSlider;
  int heightOfSlider;
};

#endif
