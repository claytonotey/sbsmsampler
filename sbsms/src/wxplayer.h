#ifndef WXPLAYER_H
#define WXPLAYER_H

#include "wx/wx.h"

class wxPlayerApp  : public wxApp
{
 public:
  wxPlayerApp() {}
  ~wxPlayerApp() {};
  bool OnInit();
  int OnExit();
};


DECLARE_APP(wxPlayerApp);

#endif
