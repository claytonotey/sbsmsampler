#include "wxplayctrl.h"
#include "wxplayer.h"
#include "wxplayerframe.h"

#include "../resource/Play.xpm"
#include "../resource/PlayDown.xpm"
#include "../resource/Pause.xpm"
#include "../resource/PauseDown.xpm"

#define PLAYPAUSE 10000
#define TIMER 10001
#define SEEK 10002
#define RATE 10003
#define PITCH 10004
#define VOLUME 10005

#define RATE_MAX 200
#define RATE_MIN 0
#define RATE_DEFAULT 100
#define PITCH_MIN -120
#define PITCH_DEFAULT 0
#define PITCH_MAX 120

#define SEEK_MAX 8192

BEGIN_EVENT_TABLE(PlayCtrl, wxPanel)
  EVT_BUTTON(PLAYPAUSE,PlayCtrl::OnPlayPause)
  EVT_TIMER(TIMER,PlayCtrl::OnTimer)
  EVT_SCROLL_THUMBTRACK(PlayCtrl::OnTrack)
  EVT_SCROLL_THUMBRELEASE(PlayCtrl::OnRelease)
END_EVENT_TABLE()

PlayCtrl :: PlayCtrl(wxWindow *parent, sbsmsplayer *player)
  : wxPanel( parent, -1, wxDefaultPosition, wxDefaultSize, wxCLIP_CHILDREN| wxTAB_TRAVERSAL )
{
  this->parent = (wxPlayerFrame*)parent;
  this->player = player;
  bmPlay = wxBitmap(Play_xpm);
  bmPlayDown = wxBitmap(PlayDown_xpm);
  bmPause = wxBitmap(Pause_xpm);
  bmPauseDown = wxBitmap(PlayDown_xpm);

  btnPlayPause = new wxBitmapButton(this, PLAYPAUSE, bmPlay,wxDefaultPosition,wxDefaultSize,0);

  int slBorder;
#ifndef __WXMSW__
  btnPlayPause->SetBitmapFocus( bmPlayDown );
  slBorder = 4;
#else
  pPlayEvt = new CBtnDownEvt( btnPlayPause, &bmPlayDown, &bmPlay );
  pPauseEvt = new CBtnDownEvt( btnPlayPause, &bmPauseDown, &bmPause );
  btnPlayPause->PushEventHandler( pPlayEvt );
  slBorder = 0;
#endif
  
  wxSize size;
  slVolume = new wxSlider( this,VOLUME,90,0,100);
  slSeek = new wxSlider( this,SEEK,0,0,SEEK_MAX);
  size = slSeek->GetSize();
  size.SetWidth(2*size.GetWidth());
  slSeek->SetMinSize(size);

  slRate = new wxSlider( this,RATE,RATE_DEFAULT,RATE_MIN,RATE_MAX);
  size = slRate->GetSize();
  size.SetWidth((int)(1.5*size.GetWidth()));
  slRate->SetMinSize(size);

  slPitch = new wxSlider( this,PITCH,PITCH_DEFAULT,PITCH_MIN,PITCH_MAX);
  size = slPitch->GetSize();
  size.SetWidth((int)(1.5*size.GetWidth()));
  slPitch->SetMinSize(size);

  stVolume = new wxStaticText( this, -1, wxT("Volume: 90%"));
  stRate = new wxStaticText( this, -1, wxT("Rate: 100%"));
  stPitch = new wxStaticText( this, -1, wxT("Pitch: 0   "));

  wxGridSizer *hsButtons = new wxGridSizer(1);
  hsButtons->Add( btnPlayPause ,1,wxALIGN_CENTER,1);
  
  stCurtime = new wxStaticText( this, -1, wxT("+000:00:00"));
  stCurtime->Connect(wxEVT_LEFT_DOWN,wxMouseEventHandler(PlayCtrl::OnClickTimeDisplay),NULL,this);
  nTimeDisplayMode = 0;

  wxFlexGridSizer *hsPlay = new wxFlexGridSizer(1,3,0,16);
  hsPlay->SetFlexibleDirection(wxHORIZONTAL);
  hsPlay->SetNonFlexibleGrowMode(wxFLEX_GROWMODE_NONE);
  hsPlay->Add( hsButtons, 1, wxEXPAND|wxALL|wxALIGN_TOP);
  hsPlay->Add( slSeek, 1, wxEXPAND|wxALIGN_CENTER_HORIZONTAL|wxALIGN_BOTTOM|wxTOP,slBorder);
  hsPlay->Add( stCurtime, 1, wxEXPAND|wxALIGN_CENTER_VERTICAL|wxALIGN_RIGHT);
  hsPlay->AddGrowableCol(1,1);

  wxFlexGridSizer *gsControls = new wxFlexGridSizer(3,2,8,8);
  gsControls->Add(stVolume,1,wxEXPAND|wxALIGN_CENTER_VERTICAL);
  gsControls->Add(slVolume,1,wxEXPAND|wxALIGN_CENTER_VERTICAL|wxTOP,slBorder);
  gsControls->Add(stRate,1,wxEXPAND|wxALIGN_CENTER_VERTICAL);
  gsControls->Add(slRate,1,wxEXPAND|wxALIGN_CENTER_VERTICAL|wxTOP,slBorder);
  gsControls->Add(stPitch,1,wxEXPAND|wxALIGN_CENTER_VERTICAL);
  gsControls->Add(slPitch,1,wxEXPAND|wxALIGN_CENTER_VERTICAL|wxTOP,slBorder);

  wxFlexGridSizer *vsPlay = new wxFlexGridSizer(2,1,8,0);
  vsPlay->Add(gsControls,1,wxEXPAND|wxALL,4);
  vsPlay->Add(hsPlay,1,wxEXPAND|wxALL,4);
  vsPlay->SetFlexibleDirection(wxVERTICAL);
  vsPlay->SetNonFlexibleGrowMode(wxFLEX_GROWMODE_ALL);

  SetSizer(vsPlay);

  pSecTimer = NULL;
  UpdateTime();
  StartTimer();
  bPlay = false;
  bWasPlaying = false;
  Layout();
}

PlayCtrl::~PlayCtrl()
{
  KillTimer();

#ifdef __WXMSW__
  btnPlayPause->PopEventHandler();
  delete pPlayEvt;
  delete pPauseEvt;
#endif
}

void PlayCtrl::OnTrack(wxScrollEvent &event)
{
  if(event.GetId() == SEEK) {
    if(player->isPlaying()) {
      bWasPlaying = true;
      KillTimer();
      player->pause();
    }
  } else if(event.GetId() == VOLUME) {
    int intval = slVolume->GetValue();
    real val = (real)intval*.01f;
    player->setVolume(val);
    stVolume->SetLabel( wxString::Format( wxT("Volume: %d"),intval) + wxT("%") );
  } else if(event.GetId() == RATE) {
    int intval = slRate->GetValue();
    real val = (real)intval*.01f;
    player->setRate(val);
    stRate->SetLabel( wxString::Format( wxT("Rate: %d"),intval) + wxT("%") );
  } else if(event.GetId() == PITCH) {
    int intval = slPitch->GetValue();
    real val = 0.1f * (real)intval;
    real pitch = pow(2.0f,val/12.0f);
    player->setPitch(pitch);
    stPitch->SetLabel( wxString::Format( wxT("Pitch: %s%.3g"),intval>0?wxT("+"):wxT(""),val) );
  }
}

void PlayCtrl::OnRelease(wxScrollEvent &event)
{
  if(event.GetId() == SEEK) {
    real fPos = (real)slSeek->GetValue()/SEEK_MAX;
    player->setPos(fPos);
    if(bWasPlaying) {
      player->play();
      StartTimer();
      bWasPlaying = false;
    }
  }
}

void PlayCtrl::OnTimer( wxTimerEvent& WXUNUSED(event) )
{
  if( player->isPlaying() ) {
    UpdateTime();
  } else if( player->isPlayedToEnd() ){
    parent->OnPause();
    player->setPos(0.0f);
    UpdateTime();
    PauseBtnToPlayBtn();
    bPlay = false;
  }
}

void PlayCtrl::StartTimer()
{
  if ( pSecTimer == NULL ) {
    pSecTimer = new wxTimer(this, TIMER );
    pSecTimer->Start( 100, false );
  }
}

void PlayCtrl::KillTimer()
{
  if(pSecTimer) {
    pSecTimer->Stop();
    delete pSecTimer;
    pSecTimer = NULL;
  }
}

void PlayCtrl::PlayBtnToPauseBtn()
{
#ifndef __WXMSW__
  btnPlayPause->SetBitmapLabel( bmPause );
  btnPlayPause->SetBitmapFocus( bmPauseDown );
#else
  btnPlayPause->SetBitmapLabel( bmPause );
  btnPlayPause->PopEventHandler();
  btnPlayPause->PushEventHandler( pPauseEvt );
#endif
  btnPlayPause->Refresh();
}

void PlayCtrl::PauseBtnToPlayBtn()
{
#ifndef __WXMSW__
  btnPlayPause->SetBitmapLabel( bmPlay );
  btnPlayPause->SetBitmapFocus( bmPlayDown );
#else
  btnPlayPause->SetBitmapLabel( bmPlay );
  btnPlayPause->PopEventHandler();
  btnPlayPause->PushEventHandler( pPlayEvt );
#endif
  btnPlayPause->Refresh();
}

void PlayCtrl::open()
{
  UpdateTime();
}

void PlayCtrl::OnPlayPause( wxCommandEvent& WXUNUSED(event) )	
{	
  if(bPlay) {
    if(player->pause()) {
      parent->OnPause();
      bPlay = false;
      PauseBtnToPlayBtn();
      KillTimer();
    }
  } else {
    if(player->play()) {
      parent->OnPlay();
      bPlay = true;
      PlayBtnToPauseBtn();
      StartTimer();
    }
  }
}

void PlayCtrl::OnClickTimeDisplay(wxMouseEvent & WXUNUSED(event))
{
  ++nTimeDisplayMode %= 2;
  UpdateTime();
}

wxString PlayCtrl::getTime(real time)
{
  int h = (int)(time/3600.0f);
  time -= h*3600.0f;
  int m = (int)(time/60.0f);
  time -= m*60.0f;
  int s = (int)time;
  time -= s;
  int cs = (int)(time*100.0f);
  return wxString::Format( wxT("%.3d:%.2d:%.2d"),m,s,cs);
}

wxString PlayCtrl::getTimeLeft(real time, real duration)
{
  return getTime(duration-time);
}

void PlayCtrl::UpdateTime()
{
  real duration = player->getDuration(); 
  real time = player->getTime();
  real fPos = duration?SEEK_MAX*time/duration : 0;
	    
#ifdef __WXGTK__
  if ( fPos < 2.0f )
    fPos = 2.0f;
#endif
  
  slSeek->SetValue( (int)fPos );
  if(nTimeDisplayMode == 0) {
    stCurtime->SetLabel(wxT("+") + getTime(time));
  } else {
    stCurtime->SetLabel(wxT("-") + getTimeLeft(time,duration));
  }
}
