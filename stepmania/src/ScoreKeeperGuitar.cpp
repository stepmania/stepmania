#include "global.h"
#include "ScoreKeeperGuitar.h"
#include "PlayerStageStats.h"
#include "MessageManager.h"


ScoreKeeperGuitar::ScoreKeeperGuitar( PlayerState *pPlayerState, PlayerStageStats *pPlayerStageStats ) : 
	ScoreKeeperNormal(pPlayerState, pPlayerStageStats)
{
	m_fMusicSecondsHeldRemainder = 0;
}

void ScoreKeeperGuitar::AddTapScore( TapNoteScore tns )
{
}

void ScoreKeeperGuitar::AddHoldScore( HoldNoteScore hns )
{
}

void ScoreKeeperGuitar::HandleHoldActiveSeconds( float fMusicSecondsHeld )
{
	int &iScore = m_pPlayerStageStats->m_iScore;
	int &iCurMaxScore = m_pPlayerStageStats->m_iCurMaxScore;

	const float fPointsPerSecond = 25;
	const float fMusicSecondsHeldPlusRemainer = m_fMusicSecondsHeldRemainder + fMusicSecondsHeld;
	const float fPoints = fMusicSecondsHeldPlusRemainer * fPointsPerSecond;
	const int iPoints = (int)floorf( fPoints );
	const float fPointsRemainder = fPoints - iPoints;
	m_fMusicSecondsHeldRemainder = (fPointsRemainder) / fPointsPerSecond;

	if( iPoints != 0 ) 
	{
		iScore += iPoints;
		iCurMaxScore += iPoints;
		MESSAGEMAN->Broadcast( Message_ScoreChangedP1 );
	}
}

void ScoreKeeperGuitar::AddTapRowScore( TapNoteScore tns, const NoteData &nd, int iRow )
{
	// calculate score multiplier
	int iNewCurScoreMultiplier = m_pPlayerStageStats->m_iCurScoreMultiplier;
	iNewCurScoreMultiplier = 1 + (m_pPlayerStageStats->m_iCurCombo / 10);
	iNewCurScoreMultiplier = min( iNewCurScoreMultiplier, 4 );
	if( iNewCurScoreMultiplier != m_pPlayerStageStats->m_iCurScoreMultiplier )
	{
		m_pPlayerStageStats->m_iCurScoreMultiplier = iNewCurScoreMultiplier;
		MESSAGEMAN->Broadcast( Message_ScoreMultiplierChangedP1 );
	}

	int &iScore = m_pPlayerStageStats->m_iScore;

	if( tns != TNS_Miss )
	{
		TapNoteScore scoreOfLastTap;
		int iNumTapsInRow;
		
		GetScoreOfLastTapInRow( nd, iRow, scoreOfLastTap, iNumTapsInRow );
		
		ASSERT( iNumTapsInRow > 0 );

		iScore += 50 * iNumTapsInRow * iNewCurScoreMultiplier;
		MESSAGEMAN->Broadcast( Message_ScoreChangedP1 );
	}
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
