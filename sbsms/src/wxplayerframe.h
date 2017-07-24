#ifndef WXPLAYERFRAME_H
#define WXPLAYERFRAME_H

#include "wx/wx.h"
#include "sbsmsplayer.h"
#include "wxplayctrl.h"
#include "wx/filename.h"

class wxPlayerFrame : public wxFrame 
{
 public:
  wxPlayerFrame(sbsmsplayer *player);
  ~wxPlayerFrame();

  bool open(const wxFileName &path);
  void OnPause();
  void OnPlay();
  void OnOpen(wxCommandEvent& WXUNUSED(event));
  void OnSave(wxCommandEvent& WXUNUSED(event));
  void OnExit(wxCommandEvent& WXUNUSED(event));
  void OnSize(wxSizeEvent & evt);
  DECLARE_EVENT_TABLE()
 protected:
  wxString openFileName;
  sbsmsplayer *player;
  PlayCtrl *playCtrl;
};

#endif
