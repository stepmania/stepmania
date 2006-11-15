#include "global.h"
#include "ScoreKeeperGuitar.h"
#include "PlayerStageStats.h"
#include "MessageManager.h"


ScoreKeeperGuitar::ScoreKeeperGuitar( PlayerState *pPlayerState, PlayerStageStats *pPlayerStageStats ) : 
	ScoreKeeperNormal(pPlayerState, pPlayerStageStats)
{
}

void ScoreKeeperGuitar::AddTapScore( TapNoteScore tns )
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
		iScore += 50 * iNewCurScoreMultiplier;
		MESSAGEMAN->Broadcast( Message_ScoreChangedP1 );
	}
}

void ScoreKeeperGuitar::AddHoldScore( HoldNoteScore hns )
{
	if( hns == HNS_Held )
		AddTapScore( TNS_W1 );
	else if ( hns == HNS_LetGo )
		AddTapScore( TNS_W4 ); // required for subtractive score display to work properly
}

void ScoreKeeperGuitar::AddTapRowScore( TapNoteScore score )
{

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
