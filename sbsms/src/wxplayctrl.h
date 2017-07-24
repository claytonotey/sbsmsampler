#ifndef PLAYCTRL_H
#define PLAYCTRL_H

#include "sbsms.h"
#include "wx/wx.h"
#include "wxbtndown.h"
#include "sbsmsplayer.h"
class wxPlayerFrame;

using namespace _sbsms_;

class PlayCtrl : public wxPanel 
{
 public:
  PlayCtrl(wxWindow *parent, sbsmsplayer *player);
  ~PlayCtrl();

  void open();
  void OnPlayPause( wxCommandEvent& WXUNUSED(event) );
  void OnTrack( wxScrollEvent &event);
  void OnRelease( wxScrollEvent &event);
  void OnTimer( wxTimerEvent& WXUNUSED(event) );
  void OnClickTimeDisplay( wxMouseEvent& event );
  wxBitmap	bmPlay,		bmPlayDown;
  wxBitmap	bmPause,	bmPauseDown;

  wxPlayerFrame *parent;

  wxBitmapButton *btnPlayPause;  
  wxSlider *slVolume;
  wxSlider *slSeek;
  wxSlider *slRate;
  wxSlider *slPitch;
  wxStaticText *stVolume;
  wxStaticText *stRate;
  wxStaticText *stPitch;
  wxStaticText *stCurtime;

#ifdef __WXMSW__
  CBtnDownEvt *pPlayEvt;
  CBtnDownEvt *pPauseEvt;
#endif

  DECLARE_EVENT_TABLE()

protected:
  wxString getTime(real time);
  wxString getTimeLeft(real time, real duration);

  bool bWasPlaying;
  bool bPlay;
  sbsmsplayer *player;
  int nTimeDisplayMode;
  void PlayBtnToPauseBtn();
  void PauseBtnToPlayBtn();
  
  void UpdateTime();
  void StartTimer();
  void KillTimer();
  wxTimer *pSecTimer;
};

#endif
