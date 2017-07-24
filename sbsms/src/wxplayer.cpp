#include "wxplayer.h"
#include "wxplayerframe.h"
#include "wx/filename.h"

IMPLEMENT_APP(wxPlayerApp)
  
bool wxPlayerApp::OnInit()
{
#ifdef _WIN32
	int status = pthread_win32_process_attach_np();
#endif

  sbsmsplayer *player = new sbsmsplayer();
  wxImage::AddHandler(new wxPNGHandler);
  wxPlayerFrame *pMain = new wxPlayerFrame(player);
  SetTopWindow( pMain );
  pMain->Show();
  if(argc > 1) {
    wxFileName path(argv[1]);    
    pMain->open(path);
  }
  return true;
}

int wxPlayerApp::OnExit()
{
#ifdef _WIN32
	int status = pthread_win32_process_detach_np();
#endif
  return 0;
}
