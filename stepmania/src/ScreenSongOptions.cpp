#include "global.h"
#include "ScreenSongOptions.h"
#include "RageLog.h"
#include "GameState.h"
#include "ThemeManager.h"
#include "PrefsManager.h"

REGISTER_SCREEN_CLASS( ScreenSongOptions );

void ScreenSongOptions::Init()
{
	ScreenOptionsMaster::Init();

	/* Hack: If we're coming in from "press start for more options", we need a different
	 * fade in. */
	if( PREFSMAN->m_ShowSongOptions == Maybe_ASK )
	{
		m_In.Load( THEME->GetPathB("ScreenSongOptions","option in") );
		m_In.StartTransitioning();
	}
}

void ScreenSongOptions::ExportOptions( int iRow, const vector<PlayerNumber> &vpns )
{
	const SongOptions::FailType ft = GAMESTATE->m_SongOptions.GetPreferred().m_FailType;

	ScreenOptionsMaster::ExportOptions( iRow, vpns );

	if( ft != GAMESTATE->m_SongOptions.GetPreferred().m_FailType )
		GAMESTATE->m_bChangedFailTypeOnScreenSongOptions = true;
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
