#include "global.h"
#include "ScreenSongOptions.h"
#include "ScreenManager.h"
#include "RageLog.h"
#include "GameState.h"
#include "ThemeManager.h"
#include "PrefsManager.h"

#define PREV_SCREEN		THEME->GetMetric ("ScreenSongOptions","PrevScreen")
#define NEXT_SCREEN		THEME->GetMetric ("ScreenSongOptions","NextScreen")

REGISTER_SCREEN_CLASS( ScreenSongOptions );
ScreenSongOptions::ScreenSongOptions( CString sClassName ) :
	ScreenOptionsMaster( sClassName )
{
}


void ScreenSongOptions::Init()
{
	ScreenOptionsMaster::Init();

	/* Hack: If we're coming in from "press start for more options", we need a different
	 * fade in. */
	if(PREFSMAN->m_ShowSongOptions == PrefsManager::ASK)
	{
		m_In.Load( THEME->GetPathB("ScreenSongOptions","option in") );
		m_In.StartTransitioning();
	}
}

void ScreenSongOptions::ExportOptions( int row, const vector<PlayerNumber> &vpns )
{
	const SongOptions::FailType ft = GAMESTATE->m_SongOptions.m_FailType;

	ScreenOptionsMaster::ExportOptions( row, vpns );

	if( ft != GAMESTATE->m_SongOptions.m_FailType )
		GAMESTATE->m_bChangedFailTypeOnScreenSongOptions = true;
}

void ScreenSongOptions::GoToPrevScreen()
{
	if( SCREENMAN->IsStackedScreen(this) )
	{
		SCREENMAN->PopTopScreen( SM_BackFromSongOptions );
	}
	else
	{
		SCREENMAN->DeletePreparedScreens();
		SCREENMAN->SetNewScreen( PREV_SCREEN );
	}
}

void ScreenSongOptions::GoToNextScreen()
{
	if( GAMESTATE->m_bEditing )
		SCREENMAN->PopTopScreen( SM_BackFromSongOptions );
	else
		SCREENMAN->SetNewScreen( NEXT_SCREEN );
}

/*
 * (c) 2001-2004 Chris Danford
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
