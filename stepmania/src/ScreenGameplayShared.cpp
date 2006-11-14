#include "global.h"
#include "ScreenGameplayShared.h"
#include "GameState.h"
#include "Player.h"
#include "NoteDataUtil.h"
#include "NoteDataWithScoring.h"
#include "ActiveAttackList.h"
#include "ScoreDisplayNormal.h"
#include "ScoreKeeperShared.h"
#include "song.h"

REGISTER_SCREEN_CLASS( ScreenGameplayShared );

void ScreenGameplayShared::FillPlayerInfo( vector<PlayerInfo> &vPlayerInfoOut )
{
	PlayerNumber mpn = GAMESTATE->m_MasterPlayerNumber;
	vPlayerInfoOut.resize( NUM_PLAYERS );
	PlayerInfo &mpi = vPlayerInfoOut[mpn];
	FOREACH_PlayerNumber( pn )
	{
		PlayerInfo &pi = vPlayerInfoOut[pn];
		pi.m_pn = pn;
		PlayerState *pPlayerState = pi.GetPlayerState();
		PlayerStageStats *pPlayerStageStats = pi.GetPlayerStageStats();
		
		pi.m_pPrimaryScoreDisplay = new ScoreDisplayNormal;
		pi.m_pPrimaryScoreDisplay->Init( pPlayerState, pPlayerStageStats );
		pi.m_pPrimaryScoreKeeper = new ScoreKeeperShared( pPlayerState, pPlayerStageStats );
		pi.m_pPlayer = new Player( mpi.m_NoteData, pn == mpn, true );
	}
}

PlayerInfo &ScreenGameplayShared::GetPlayerInfoForInput( const InputEventPlus& iep )
{
	return m_vPlayerInfo[GAMESTATE->m_MasterPlayerNumber];
}

void ScreenGameplayShared::SaveStats()
{
	vector<NoteData> vParts;
	PlayerNumber mpn = GAMESTATE->m_MasterPlayerNumber;
	float fMusicLen = GAMESTATE->m_pCurSong->m_fMusicLengthSeconds;
	
	NoteDataUtil::SplitCompositeNoteData( m_vPlayerInfo[mpn].m_pPlayer->GetNoteData(), vParts );
	for( size_t i = 0; i < min(vParts.size(), m_vPlayerInfo.size()); ++i )
	{
		PlayerInfo &pi = m_vPlayerInfo[i];
		
		if( !pi.IsEnabled() )
			continue;
		NoteData &nd = vParts[i];
		RadarValues rv;
		PlayerStageStats &pss = *pi.GetPlayerStageStats();
		
		NoteDataUtil::CalculateRadarValues( nd, fMusicLen, rv );
		pss.m_radarPossible += rv;
		
		NoteDataWithScoring::GetActualRadarValues( nd, pss, fMusicLen, rv );
		pss.m_radarActual += rv;
	}
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

