#include "global.h"

#include "ScreenReloadSongs.h"
#include "SongManager.h"
#include "UnlockSystem.h"
#include "ScreenManager.h"
#include "RageTimer.h"
#include "RageLog.h"
#include "ThemeManager.h"

#include "arch/LoadingWindow/LoadingWindow.h"

static const int DrawFrameRate = 20;
class ScreenReloadSongsLoadingWindow: public LoadingWindow
{
	RageTimer m_LastDraw;
	BitmapText &m_BitmapText;

public:
	ScreenReloadSongsLoadingWindow( BitmapText &bt ):
		m_BitmapText(bt)
	{
	}

	void SetText( CString str )
	{
		m_BitmapText.SetText( str );
		Paint();
	}

	void Paint()
	{
		/* We load songs much faster than we draw frames.  Cap the draw rate, so we don't
		 * slow down the reload. */
		if( m_LastDraw.PeekDeltaTime() < 1.0f/DrawFrameRate )
			return;
		m_LastDraw.GetDeltaTime();

		SCREENMAN->Draw();
	}
};

/* This could be cleaned up: show progress, for example.  Let's not use
 * this for the initial load, since we don't want to start up the display
 * until we finish loading songs; that way, people can continue to use their
 * computer while songs load. */
ScreenReloadSongs::ScreenReloadSongs( CString sClassName ): Screen(sClassName)
{
	m_iUpdates = 0;

	m_Loading.LoadFromFont( THEME->GetPathF("Common", "normal") );
	m_Loading.SetXY( CENTER_X, CENTER_Y );
	this->AddChild( &m_Loading );

	m_LoadingWindow = new ScreenReloadSongsLoadingWindow( m_Loading );
}

ScreenReloadSongs::~ScreenReloadSongs()
{
	delete m_LoadingWindow;
}


void ScreenReloadSongs::Update( float fDeltaTime )
{
	Screen::Update( fDeltaTime );

	/* Start the reload on the second update.  On the first, 0, SCREENMAN->Draw won't draw. */
	++m_iUpdates;
	if( m_iUpdates != 2 )
		return;
	ASSERT( !IsFirstUpdate() );

	SONGMAN->Reload( m_LoadingWindow );
	UNLOCKMAN->UpdateSongs();
	SCREENMAN->SetNewScreen( "ScreenTitleMenu" );
}



