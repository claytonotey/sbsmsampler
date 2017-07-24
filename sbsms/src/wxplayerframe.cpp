#include "wxplayerframe.h"
#include <wx/progdlg.h>
#include <wx/filename.h>
#include "wxconvertdlg.h"
#include "convert.h"

#include <algorithm>
using namespace std;

BEGIN_EVENT_TABLE(wxPlayerFrame, wxFrame)
  EVT_MENU(wxID_OPEN,wxPlayerFrame::OnOpen)
  EVT_MENU(wxID_SAVE,wxPlayerFrame::OnSave)
  EVT_MENU(wxID_EXIT,wxPlayerFrame::OnExit)
  EVT_SIZE(wxPlayerFrame::OnSize)
END_EVENT_TABLE()

bool progressCB(real progress, const char *msg, void *data)
{
  wxProgressDialog *progressDlg = (wxProgressDialog*)data;
  return progressDlg->Update(min(100,(int)(100.0*progress)));
}

void wxPlayerFrame::OnSize(wxSizeEvent & evt)
{
  playCtrl->Layout();
  Layout();
}

bool wxPlayerFrame :: open(const wxFileName &path)
{
  if(player->open(path.GetFullPath().fn_str())) {
    openFileName = path.GetFullPath();
    playCtrl->open();
    return true;
  } else {
    return false;
  }
}

void wxPlayerFrame::OnOpen(wxCommandEvent& WXUNUSED(event))
{
  wxFileDialog dlg(this, wxT("Choose a file"), wxT("."), wxT(""), wxT("All filetypes (*.sbsms;*.mp3;*.wav;*.aif;*.aiff;*.pcm)|*.sbsms;*.mp3;*.wav;*.aif;*.aiff;*.pcm"),wxOPEN);
  if(dlg.ShowModal() == wxID_OK) {
    wxString pathStr = dlg.GetPath();
    wxFileName path(pathStr);
    
    player->close();
    if(path.GetExt().Cmp(wxT("sbsms")) == 0) {
      open(path);
    } else {
      ConvertDialog convertDlg(this);
      if(convertDlg.ShowModal() == wxID_OK) {
        bool bPreAnalyze = convertDlg.getPreAnalyze();
	
        wxFileName sbsmsPath(path.GetPath(wxPATH_GET_VOLUME|wxPATH_GET_SEPARATOR) + path.GetName() + wxT(".sbsms"));
        wxProgressDialog progress(wxT("Progress"),wxT("Converting to ") + sbsmsPath.GetFullName(),100,NULL,wxPD_AUTO_HIDE | wxPD_APP_MODAL | wxPD_ELAPSED_TIME | wxPD_ESTIMATED_TIME);
        wxSize size = progress.GetSize();
        size.SetWidth(min(400,size.GetWidth()*2));
        progress.SetSize(size);
        bool status = sbsms_convert(path.GetFullPath().fn_str(),sbsmsPath.GetFullPath().fn_str(),true,false,bPreAnalyze,progressCB,&progress);
        if(status) {
          if(player->open(sbsmsPath.GetFullPath().fn_str())) {
            openFileName = sbsmsPath.GetFullPath();
          }
        } else {
          wxMessageDialog msg(this,wxT("Conversion Failed"),wxT(""),wxOK);
        }
      }
    }
  }
}

void wxPlayerFrame::OnSave(wxCommandEvent& WXUNUSED(event))
{
  if(openFileName.Cmp(wxT("")) != 0) {
    wxFileDialog dlg(this, wxT("Choose a file"), wxT("."), wxT(""), wxT("*.wav"),wxSAVE|wxOVERWRITE_PROMPT);
    if(dlg.ShowModal() == wxID_OK) {
      wxString pathStr = dlg.GetPath();
      wxFileName path(pathStr);
      
      wxProgressDialog progress(wxT("Progress"),wxT("Converting to ") + path.GetFullName());
      real rate = player->getRate();
      real pitch = player->getPitch(); 
      real volume = player->getVolume();
      bool status = sbsms_convert(openFileName.fn_str(),path.GetFullPath().fn_str(),false,true,false,progressCB,&progress,rate,rate,pitch,pitch,volume);
      if(!status) {
        wxMessageDialog msg(this,wxT("Conversion Failed"),wxT(""),wxOK);
      }
    }
  }
}

void wxPlayerFrame::OnExit(wxCommandEvent& WXUNUSED(event))
{
  Close();
}

wxPlayerFrame::wxPlayerFrame(sbsmsplayer *player) 
  : wxFrame( (wxFrame*)NULL, -1, wxT("SBSMS"))
{
  this->player = player;  
  playCtrl = new PlayCtrl(this,player);
  openFileName = wxT("");
  
  wxMenu* file_menu = new wxMenu;
  file_menu->Append( wxID_OPEN, _("Open") );
  file_menu->AppendSeparator();
  file_menu->Append( wxID_SAVE, _("Save") );
  file_menu->AppendSeparator();
  file_menu->Append( wxID_EXIT, _("Exit") );

  wxMenuBar *menu_bar = new wxMenuBar;
  menu_bar->Append( file_menu,	_("&File") );
  
  SetMenuBar( menu_bar );
#ifdef __WXMSW__
  wxIcon ic(wxICON(SBSMSLogo));
  SetIcon(ic);
#endif 
  wxBoxSizer *hs = new wxBoxSizer( wxHORIZONTAL );
  playCtrl->Show(true);
  hs->Add(playCtrl,1,wxALL|wxEXPAND);

  SetSizer(hs);
  hs->Fit(this);
  Layout();
  wxSize size = GetSize();
  SetMinSize(size);
  size.SetWidth(1600);
  SetMaxSize(size);
}

void wxPlayerFrame :: OnPause()
{
  GetMenuBar()->Enable(wxID_OPEN,true);
  GetMenuBar()->Enable(wxID_SAVE,true);
  GetMenuBar()->Enable(wxID_EXIT,true);
}

void wxPlayerFrame :: OnPlay()
{
  GetMenuBar()->Enable(wxID_OPEN,false);
  GetMenuBar()->Enable(wxID_SAVE,false);
  GetMenuBar()->Enable(wxID_EXIT,false);
}

wxPlayerFrame::~wxPlayerFrame() 
{
}
