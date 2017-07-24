#include "wxconvertdlg.h"

#define PREANALYZE 9000

ConvertDialog :: ConvertDialog(wxWindow *parent) : 
  wxDialog(parent, wxID_ANY, _("Convert"), wxDefaultPosition, wxDefaultSize)
{
  mPreAnalyze = new wxCheckBox(this,PREANALYZE,_("Transient Sharpening"));
  wxBoxSizer *mSizer = new wxBoxSizer(wxVERTICAL);

  wxStdDialogButtonSizer *bs = new wxStdDialogButtonSizer();
  wxButton *b;
  b = new wxButton( this, wxID_OK );
  b->SetDefault();
  bs->AddButton( b );
  bs->AddButton( new wxButton( this, wxID_CANCEL ) );
  bs->AddStretchSpacer();
  bs->Realize();

  mSizer->Add(mPreAnalyze);
  mSizer->Add(bs);
  SetSizer(mSizer);
  mSizer->Fit(this);
}

ConvertDialog :: ~ConvertDialog()
{
}

bool ConvertDialog :: getPreAnalyze()
{
  return mPreAnalyze->GetValue();
}
 
