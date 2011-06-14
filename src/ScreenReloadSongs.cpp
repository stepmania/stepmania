#include "global.h"
#include "ScreenReloadSongs.h"
#include "ScreenManager.h"
#include "SongManager.h"
#include "RageTimer.h"
#include "RageLog.h"
#include "ThemeManager.h"
#include "ScreenDimensions.h"
#include "arch/LoadingWindow/LoadingWindow.h"
#include "InGameLoadingWindow.h"

/* This could be cleaned up: show progress, for example.  Let's not use
 * this for the initial load, since we don't want to start up the display
 * until we finish loading songs; that way, people can continue to use their
 * computer while songs load. */
REGISTER_SCREEN_CLASS( ScreenReloadSongs );

ScreenReloadSongs::ScreenReloadSongs() {}

void ScreenReloadSongs::Init()
{
	Screen::Init();

	loadWin=new InGameLoadingWindow( );

	AddChild( loadWin );

	loadWin->SetXY( SCREEN_CENTER_X, SCREEN_CENTER_Y );
	
	pLoadingWindow=loadWin;

	m_loadingThread.SetName("Song reload work thread");
	m_loadingThread.Create(loadingThreadProc,this);
}

ScreenReloadSongs::~ScreenReloadSongs()
{
	m_loadingThread.Wait();
	RemoveChild(loadWin);
	delete loadWin;
}

int ScreenReloadSongs::loadingThreadProc(void *thisAsVoidPtr)
{
	SONGMAN->Reload( false );
	SCREENMAN->PostMessageToTopScreen( SM_GoToNextScreen, 0 );
	return 0;
}

/*
 * (c) 2003-2004 Glenn Maynard
 * All rights reserved.
 * 
 * Permission is hereby granted, free of charge, to any person obtaining a
 * copy of this software and associated documentation files (the
 * "Software"), to deal in the Software without restriction, including
 * without limitation the rights to use, copy, modify, merge, publish,
 * distribute, and/or sell copies of the Software, and to permit persons to
 * whom the Software is furnished to do so, provided that the above
 * copyright notice(s) and this permission notice appear in all copies of
 * the Software and that both the above copyright notice(s) and this
 * permission notice appear in supporting documentation.
 * 
 * THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS
 * OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF
 * MERCHANTABILITY, FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT OF
 * THIRD PARTY RIGHTS. IN NO EVENT SHALL THE COPYRIGHT HOLDER OR HOLDERS
 * INCLUDED IN THIS NOTICE BE LIABLE FOR ANY CLAIM, OR ANY SPECIAL INDIRECT
 * OR CONSEQUENTIAL DAMAGES, OR ANY DAMAGES WHATSOEVER RESULTING FROM LOSS
 * OF USE, DATA OR PROFITS, WHETHER IN AN ACTION OF CONTRACT, NEGLIGENCE OR
 * OTHER TORTIOUS ACTION, ARISING OUT OF OR IN CONNECTION WITH THE USE OR
 * PERFORMANCE OF THIS SOFTWARE.
 */
