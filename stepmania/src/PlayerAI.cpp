#include "global.h"
#include "PlayerAI.h"
#include "RageUtil.h"
#include "IniFile.h"
#include "RageException.h"
#include "GameState.h"
#include "arch/arch.h"

#define AI_PATH "Data/AI.ini"

struct TapScoreDistribution
{
	float fPercent[NUM_TAP_NOTE_SCORES];

	TapNoteScore GetTapNoteScore()
	{
		float fRand = randomf(0,1);
		float fCumulativePercent = 0;
		for( int i=0; i<=TNS_MARVELOUS; i++ )
		{
			fCumulativePercent += fPercent[i];
			if( fRand <= fCumulativePercent+1e-4 ) /* rounding error */
				return (TapNoteScore)i;
		}
		ASSERT(0);	// the fCumulativePercents must sum to 1.0, so we should never get here!
		return TNS_MARVELOUS;
	}

};

TapScoreDistribution g_Distributions[NUM_SKILL_LEVELS];


void PlayerAI::InitFromDisk()
{
	IniFile ini;
	ini.ReadFile( AI_PATH );

	for( int i=0; i<NUM_SKILL_LEVELS; i++ )
	{
		CString sKey = ssprintf("Skill%d", i);
		if( ini.GetKey(sKey)==NULL )
			RageException::Throw( "AI.ini: '%s' doesn't exist.", sKey.c_str() );

		TapScoreDistribution& dist = g_Distributions[i];
		dist.fPercent[TNS_NONE] = 0;
		ini.GetValue ( sKey, "MissWeight", dist.fPercent[TNS_MISS] );
		ini.GetValue ( sKey, "BooWeight", dist.fPercent[TNS_BOO] );
		ini.GetValue ( sKey, "GoodWeight", dist.fPercent[TNS_GOOD] );
		ini.GetValue ( sKey, "GreatWeight", dist.fPercent[TNS_GREAT] );
		ini.GetValue ( sKey, "PerfectWeight", dist.fPercent[TNS_PERFECT] );
		ini.GetValue ( sKey, "MarvelousWeight", dist.fPercent[TNS_MARVELOUS] );
		
		float fSum = 0;
		int j;
		for( j=0; j<NUM_TAP_NOTE_SCORES; j++ )
			fSum += dist.fPercent[j];
		for( j=0; j<NUM_TAP_NOTE_SCORES; j++ )
			dist.fPercent[j] /= fSum;
	}
}


TapNoteScore PlayerAI::GetTapNoteScore( PlayerNumber pn )
{

	int iCpuSkill = GAMESTATE->m_iCpuSkill[pn];
	int iSumOfAttackLevels = 
		GAMESTATE->m_fSecondsUntilAttacksPhasedOut[pn] > 0 ? 
		GAMESTATE->m_iLastPositiveSumOfAttackLevels[pn] : 
		0;

	ASSERT_M( iCpuSkill>=0 && iCpuSkill<NUM_SKILL_LEVELS, ssprintf("%i", iCpuSkill) );
	ASSERT_M( GAMESTATE->m_PlayerController[pn] == PC_CPU, ssprintf("%i", GAMESTATE->m_PlayerController[pn]) );

	iCpuSkill -= iSumOfAttackLevels*3;
	CLAMP( iCpuSkill, 0, NUM_SKILL_LEVELS-1 );

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
