#include "global.h"
#include "ScreenGameplayMultiplayer.h"
#include "GameState.h"


REGISTER_SCREEN_CLASS_NEW( ScreenGameplayMultiplayer );

void ScreenGameplayMultiplayer::Init()
{
	ScreenGameplay::Init();
}

void ScreenGameplayMultiplayer::FillPlayerInfo( vector<PlayerInfo> &vPlayerInfoOut )
{
	vPlayerInfoOut.resize( NUM_MultiPlayer+1 );
	FOREACH_MultiPlayer( p )
		vPlayerInfoOut[p].Load( PLAYER_INVALID, p, false );
	PlayerInfo &pi = vPlayerInfoOut.back();
	pi.LoadDummyP1();	// dummy autoplay NoteField
};

void ScreenGameplayMultiplayer::LoadNextSong()
{
	ScreenGameplay::LoadNextSong();

	ASSERT( !m_vPlayerInfo.empty() );
	int iIndex = m_vPlayerInfo.size()-1;
	PlayerInfo &pi = m_vPlayerInfo[iIndex];
	//pi.LoadDummyP1();	// dummy autoplay NoteField
	pi.m_PlayerStateDummy = *GAMESTATE->m_pPlayerState[PLAYER_1];
	pi.m_PlayerStateDummy.m_PlayerController = PC_AUTOPLAY;
}


/*
 * (c) 2005 Chris Danford
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
