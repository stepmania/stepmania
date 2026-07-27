#include "global.h"
#include "ScoreKeeperShared.h"
#include "RageLog.h"
#include "GameState.h"
#include "PlayerState.h"

#include <vector>


/* In Routine, we have two Players, but the master one handles all of the scoring.  The other
 * one will just receive misses for everything, and shouldn't do anything. */
ScoreKeeperShared::ScoreKeeperShared( PlayerState *pPlayerState, PlayerStageStats *pPlayerStageStats ) :
	ScoreKeeperNormal( pPlayerState, pPlayerStageStats )
{
}

void ScoreKeeperShared::Load(
	const std::vector<Song*> &apSongs,
	const std::vector<Steps*> &apSteps,
	const std::vector<AttackArray> &asModifiers )
{
	if( m_pPlayerState->m_PlayerNumber != GAMESTATE->GetMasterPlayerNumber() )
		return;
	ScoreKeeperNormal::Load( apSongs, apSteps, asModifiers );
}

// These ScoreKeepers don't get to draw.
void ScoreKeeperShared::DrawPrimitives()
{
	if( m_pPlayerState->m_PlayerNumber != GAMESTATE->GetMasterPlayerNumber() )
		return;
	ScoreKeeperNormal::DrawPrimitives();
}

void ScoreKeeperShared::Update( float fDelta )
{
	if( m_pPlayerState->m_PlayerNumber != GAMESTATE->GetMasterPlayerNumber() )
		return;
	ScoreKeeperNormal::Update( fDelta );
}

void ScoreKeeperShared::OnNextSong( int iSongInCourseIndex, const Steps* pSteps, const NoteData* pNoteData )
{
	if( m_pPlayerState->m_PlayerNumber != GAMESTATE->GetMasterPlayerNumber() )
		return;
	ScoreKeeperNormal::OnNextSong( iSongInCourseIndex, pSteps, pNoteData );
}

void ScoreKeeperShared::HandleTapScore( const TapNote &tn )
{
	if( m_pPlayerState->m_PlayerNumber != GAMESTATE->GetMasterPlayerNumber() )
		return;
	ScoreKeeperNormal::HandleTapScore( tn );
}

void ScoreKeeperShared::HandleTapRowScore( const NoteData &nd, int iRow )
{
	if( m_pPlayerState->m_PlayerNumber != GAMESTATE->GetMasterPlayerNumber() )
		return;
	ScoreKeeperNormal::HandleTapRowScore( nd, iRow );
}

void ScoreKeeperShared::HandleHoldScore( const TapNote &tn )
{
	if( m_pPlayerState->m_PlayerNumber != GAMESTATE->GetMasterPlayerNumber() )
		return;
	ScoreKeeperNormal::HandleHoldScore( tn );
}

void ScoreKeeperShared::HandleHoldActiveSeconds( float fMusicSecondsHeld )
{
	if( m_pPlayerState->m_PlayerNumber != GAMESTATE->GetMasterPlayerNumber() )
		return;
	ScoreKeeperNormal::HandleHoldActiveSeconds( fMusicSecondsHeld );
}

void ScoreKeeperShared::HandleHoldCheckpointScore( const NoteData &nd, int iRow, int iNumHoldsHeldThisRow, int iNumHoldsMissedThisRow )
{
	if( m_pPlayerState->m_PlayerNumber != GAMESTATE->GetMasterPlayerNumber() )
		return;
	ScoreKeeperNormal::HandleHoldCheckpointScore( nd, iRow, iNumHoldsHeldThisRow,  iNumHoldsMissedThisRow );
}

void ScoreKeeperShared::HandleTapScoreNone()
{
	if( m_pPlayerState->m_PlayerNumber != GAMESTATE->GetMasterPlayerNumber() )
		return;
	ScoreKeeperNormal::HandleTapScoreNone();
}

/*
 * (c) 2006-2010 Steve Checkoway, Glenn Maynard
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
