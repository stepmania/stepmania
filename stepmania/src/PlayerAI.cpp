#include "global.h"
/*
-----------------------------------------------------------------------------
 Class: PlayerAI

 Desc: See header.

 Copyright (c) 2001-2002 by the person(s) listed below.  All rights reserved.
	Chris Danford
-----------------------------------------------------------------------------
*/

#include "PlayerAI.h"
#include "RageUtil.h"
#include "IniFile.h"
#include "RageException.h"
#include "GameState.h"
#include <math.h>
#include "arch/arch.h"

#define AI_PATH BASE_PATH "Data" SLASH "AI.ini"

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
	ini.SetPath( AI_PATH );
	ini.ReadFile();

	for( int i=0; i<NUM_SKILL_LEVELS; i++ )
	{
		CString sKey = ssprintf("Skill%d", i);
		if( ini.GetKey(sKey)==NULL )
			RageException::Throw( "AI.ini: '%s' doesn't exist.", sKey.c_str() );

		TapScoreDistribution& dist = g_Distributions[i];
		dist.fPercent[TNS_NONE] = 0;
		ini.GetValueF( sKey, "MissWeight", dist.fPercent[TNS_MISS] );
		ini.GetValueF( sKey, "BooWeight", dist.fPercent[TNS_BOO] );
		ini.GetValueF( sKey, "GoodWeight", dist.fPercent[TNS_GOOD] );
		ini.GetValueF( sKey, "GreatWeight", dist.fPercent[TNS_GREAT] );
		ini.GetValueF( sKey, "PerfectWeight", dist.fPercent[TNS_PERFECT] );
		ini.GetValueF( sKey, "MarvelousWeight", dist.fPercent[TNS_MARVELOUS] );
		
		float fSum = 0;
		int j;
		for( j=0; j<NUM_TAP_NOTE_SCORES; j++ )
			fSum += dist.fPercent[j];
		for( j=0; j<NUM_TAP_NOTE_SCORES; j++ )
			dist.fPercent[j] /= fSum;
	}
}


TapNoteScore PlayerAI::GetTapNoteScore( int iCpuSkill, int iSumOfAttackLevels )
{
	// shouldn't call this unless we're CPU controlled
	ASSERT( iCpuSkill>=0 && iCpuSkill<NUM_SKILL_LEVELS );

	iCpuSkill -= iSumOfAttackLevels;
	CLAMP( iCpuSkill, 0, NUM_SKILL_LEVELS-1 );

	TapScoreDistribution& distribution = g_Distributions[iCpuSkill];
		
	return distribution.GetTapNoteScore();
}
