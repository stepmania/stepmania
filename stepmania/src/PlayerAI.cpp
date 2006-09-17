#include "global.h"
#include "PlayerAI.h"
#include "RageUtil.h"
#include "IniFile.h"
#include "RageException.h"
#include "GameState.h"
#include "PlayerState.h"

#define AI_PATH "Data/AI.ini"

struct TapScoreDistribution
{
	float fPercent[NUM_TapNoteScore];

	TapNoteScore GetTapNoteScore()
	{
		float fRand = randomf(0,1);
		float fCumulativePercent = 0;
		for( int i=0; i<=TNS_W1; i++ )
		{
			fCumulativePercent += fPercent[i];
			if( fRand <= fCumulativePercent+1e-4 ) /* rounding error */
				return (TapNoteScore)i;
		}
		ASSERT_M( 0, ssprintf("%f,%f",fRand,fCumulativePercent) );	// the fCumulativePercents must sum to 1.0, so we should never get here!
		return TNS_W1;
	}

};

static TapScoreDistribution g_Distributions[NUM_SKILL_LEVELS];


void PlayerAI::InitFromDisk()
{
	bool bSuccess;
	
	IniFile ini;
	bSuccess = ini.ReadFile( AI_PATH );
	ASSERT( bSuccess );

	for( int i=0; i<NUM_SKILL_LEVELS; i++ )
	{
		RString sKey = ssprintf("Skill%d", i);
		XNode* pNode = ini.GetChild(sKey);
		if( pNode == NULL )
			RageException::Throw( "AI.ini: \"%s\" doesn't exist.", sKey.c_str() );

		TapScoreDistribution& dist = g_Distributions[i];
		dist.fPercent[TNS_None] = 0;
		bSuccess = pNode->GetAttrValue( "WeightMiss", dist.fPercent[TNS_Miss] );
		ASSERT( bSuccess );
		bSuccess = pNode->GetAttrValue( "WeightW5", dist.fPercent[TNS_W5] );
		ASSERT( bSuccess );
		bSuccess = pNode->GetAttrValue( "WeightW4", dist.fPercent[TNS_W4] );
		ASSERT( bSuccess );
		bSuccess = pNode->GetAttrValue( "WeightW3", dist.fPercent[TNS_W3] );
		ASSERT( bSuccess );
		bSuccess = pNode->GetAttrValue( "WeightW2", dist.fPercent[TNS_W2] );
		ASSERT( bSuccess );
		bSuccess = pNode->GetAttrValue( "WeightW1", dist.fPercent[TNS_W1] );
		ASSERT( bSuccess );
		
		float fSum = 0;
		for( int j=0; j<NUM_TapNoteScore; j++ )
			fSum += dist.fPercent[j];
		for( int j=0; j<NUM_TapNoteScore; j++ )
			dist.fPercent[j] /= fSum;
	}
}


TapNoteScore PlayerAI::GetTapNoteScore( const PlayerState* pPlayerState )
{
	if( pPlayerState->m_PlayerController == PC_AUTOPLAY )
		return TNS_W1;

	int iCpuSkill = pPlayerState->m_iCpuSkill;

	/* If we're in battle mode, reduce the skill based on the number of modifier attacks
	 * against us.  If we're in demonstration or jukebox mode with modifiers attached,
	 * don't make demonstration miss a lot. */
	if( !GAMESTATE->m_bDemonstrationOrJukebox )
	{
		int iSumOfAttackLevels = 
			pPlayerState->m_fSecondsUntilAttacksPhasedOut > 0 ? 
			pPlayerState->m_iLastPositiveSumOfAttackLevels : 
			0;

		ASSERT_M( iCpuSkill>=0 && iCpuSkill<NUM_SKILL_LEVELS, ssprintf("%i", iCpuSkill) );
		ASSERT_M( pPlayerState->m_PlayerController == PC_CPU, ssprintf("%i", pPlayerState->m_PlayerController) );

		iCpuSkill -= iSumOfAttackLevels*3;
		CLAMP( iCpuSkill, 0, NUM_SKILL_LEVELS-1 );
	}

	TapScoreDistribution& distribution = g_Distributions[iCpuSkill];
		
	return distribution.GetTapNoteScore();
}

/*
 * (c) 2003-2004 Chris Danford
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
