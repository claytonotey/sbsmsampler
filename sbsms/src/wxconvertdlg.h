#ifndef CONVERTDLG_H
#define CONVERTDLG_H

#include "wx/wx.h"

class ConvertDialog : public wxDialog 
{
 public:
  ConvertDialog(wxWindow *parent);
  ~ConvertDialog();

  wxCheckBox *mPreAnalyze;
  bool getPreAnalyze();
};

#endif
