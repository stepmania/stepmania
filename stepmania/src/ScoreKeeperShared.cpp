#include "global.h"
#include "ScoreKeeperShared.h"
#include "ScoreKeeperNormal.h"
#include "GameState.h"
#include "StatsManager.h"
#include "NoteDataUtil.h"
#include "NoteData.h"

ScoreKeeperShared::ScoreKeeperShared() : ScoreKeeper( NULL, NULL )
{
	FOREACH_PlayerNumber( pn )
	{
		PlayerState *pPS = GAMESTATE->m_pPlayerState[pn];
		PlayerStageStats *pPSS = &STATSMAN->m_CurStageStats.m_player[pn];
		
		m_pScoreKeepers[pn] = new ScoreKeeperNormal( pPS, pPSS );
	}
}

ScoreKeeperShared::~ScoreKeeperShared()
{
	FOREACH_PlayerNumber( pn )
		delete m_pScoreKeepers[pn];
}

#define CALL( fun ) FOREACH_PlayerNumber( pn ) m_pScoreKeepers[pn]->fun
void ScoreKeeperShared::Load( const vector<Song *> &apSongs, const vector<Steps *> &apSteps,
			      const vector<AttackArray> &asModifiers )
{
	CALL( Load(apSongs, apSteps, asModifiers) );
}

void ScoreKeeperShared::DrawPrimitives()
{
	CALL( DrawPrimitives() );
}

void ScoreKeeperShared::Update( float fDelta )
{
	CALL( Update(fDelta) );
}

void ScoreKeeperShared::OnNextSong( int iSongInCourseIndex, const Steps* pSteps, const NoteData* pNoteData )
{
	vector<NoteData> vParts;
	
	NoteDataUtil::SplitCompositeNoteData( *pNoteData, vParts );
	CALL( OnNextSong(iSongInCourseIndex, pSteps, &vParts[pn]) );
}

void ScoreKeeperShared::HandleTapScore( const TapNote &tn )
{
	DEBUG_ASSERT( tn.pn != PLAYER_INVALID );
	if( tn.pn == PLAYER_INVALID )
		CALL( HandleTapScore(tn) );
	else
		m_pScoreKeepers[tn.pn]->HandleTapScore( tn );
}

void ScoreKeeperShared::HandleTapRowScore( const NoteData &nd, int row )
{
	CALL( HandleTapRowScore(nd, row) );
}

void ScoreKeeperShared::HandleHoldScore( const TapNote &tn )
{
	DEBUG_ASSERT( tn.pn != PLAYER_INVALID );
	if( tn.pn == PLAYER_INVALID )
		CALL( HandleHoldScore(tn) );
	else
		m_pScoreKeepers[tn.pn]->HandleHoldScore( tn );
}
#undef CALL

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
