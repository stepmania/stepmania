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


struct TapScoreDistribution
{
	float fCumulativeProbability[NUM_TAP_NOTE_SCORES];

	TapNoteScore GetTapNoteScore()
	{
		float fRand = randomf(0,1);
		for( int i=TNS_MISS; i<=TNS_MARVELOUS; i++ )
			if( fRand <= fCumulativeProbability[i] )
				return (TapNoteScore)i;
		ASSERT(0);	// the last probability must be 1.0, so we should never get here!
		return TNS_MARVELOUS;
	}

};

TapScoreDistribution g_Distributions[NUM_SKILL_LEVELS];


void PlayerAI::InitFromDisk()
{
	IniFile ini;
	ini.SetPath( "AI.ini" );
	ini.ReadFile();

	for( int i=0; i<NUM_SKILL_LEVELS; i++ )
	{
		CString sKey = ssprintf("Skill%d", i);
		if( ini.GetKey(sKey)==NULL )
			RageException::Throw( "AI.ini: '%s' doesn't exist.", sKey.GetString() );

		TapScoreDistribution& dist = g_Distributions[i];
		dist.fCumulativeProbability[TNS_NONE] = 0;
		ini.GetValueF( sKey, "MissCum", dist.fCumulativeProbability[TNS_MISS] );
		ini.GetValueF( sKey, "BooCum", dist.fCumulativeProbability[TNS_BOO] );
		ini.GetValueF( sKey, "GoodCum", dist.fCumulativeProbability[TNS_GOOD] );
		ini.GetValueF( sKey, "GreatCum", dist.fCumulativeProbability[TNS_GREAT] );
		ini.GetValueF( sKey, "PerfectCum", dist.fCumulativeProbability[TNS_PERFECT] );
		dist.fCumulativeProbability[TNS_MARVELOUS] = 1;
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
