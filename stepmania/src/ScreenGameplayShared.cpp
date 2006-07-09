#include "global.h"
#include "ScreenGameplayShared.h"
#include "GameState.h"
#include "Player.h"

REGISTER_SCREEN_CLASS( ScreenGameplayShared );

void ScreenGameplayShared::FillPlayerInfo( vector<PlayerInfo> &vPlayerInfoOut )
{
	PlayerNumber mpn = GAMESTATE->m_MasterPlayerNumber;
	
	vPlayerInfoOut.resize( NUM_PLAYERS );
	FOREACH_PlayerNumber( pn )
		vPlayerInfoOut[pn].Load( pn, MultiPlayer_INVALID, true );
	FOREACH_PlayerNumber( pn )
	{
		if( pn != mpn )
			vPlayerInfoOut[pn].m_pPlayer->SetSharedNoteField( vPlayerInfoOut[mpn].m_pPlayer );
	}
}

PlayerInfo &ScreenGameplayShared::GetPlayerInfoForInput( const InputEventPlus& iep )
{
	const float fPositionSeconds = GAMESTATE->m_fMusicSeconds - iep.DeviceI.ts.Ago();
	const float fSongBeat = GAMESTATE->m_pCurSong->GetBeatFromElapsedTime( fPositionSeconds );
	const int row = BeatToNoteRow( fSongBeat );
	const int col = iep.StyleI.col;
	int distance = INT_MAX;
	size_t index = 0;
	
	for( size_t i = 0; i < m_vPlayerInfo.size(); ++i )
	{
		if( !m_vPlayerInfo[i].IsEnabled() )
			continue;
		int d = m_vPlayerInfo[i].m_pPlayer->GetClosestNoteDistance( col, row );
		if( d == -1 || d >= distance)
			continue;
		distance = d;
		index = i;
	}
	return m_vPlayerInfo[index];
}

/*
 * (c) 2006 Steve Checkoway
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

