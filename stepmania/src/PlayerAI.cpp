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
#include <math.h>


struct TapScoreDistribution
{
	float fCumulativeProbability[NUM_TAP_NOTE_SCORES];
	TapNoteScore GetTapNoteScore( int iDifficultyExponent )
	{
		float fRand = randomf(0,1);
		ASSERT( iDifficultyExponent >= 1 );
		fRand = powf(fRand, iDifficultyExponent);
		for( int i=TNS_MISS; i<=TNS_MARVELOUS; i++ )
			if( fRand <= fCumulativeProbability[i] )
				return (TapNoteScore)i;
		ASSERT(0);	// the last probability must be 1.0, so we should never get here!
		return TNS_MARVELOUS;
	}
};

TapScoreDistribution TAP_SCORE_DISTRIBUTIONS[NUM_PLAYER_CONTROLLERS] = 
{
	// HUMAN
	{
		0.00f,	// TNS_NONE 
		1.00f,	// TNS_MISS	// don't ever hit automatically when human
		0.00f,	// TNS_BOO 
		0.00f,	// TNS_GOOD 
		0.00f,	// TNS_GREAT 
		0.00f,	// TNS_PERFECT 
		0.00f,	// TNS_MARVELOUS 
	},
	// CPU_EASY
	{
		0.00f,	// TNS_NONE 
		0.02f,	// TNS_MISS
		0.06f,	// TNS_BOO 
		0.10f,	// TNS_GOOD 
		0.55f,	// TNS_GREAT 
		0.80f,	// TNS_PERFECT 
		1.00f,	// TNS_MARVELOUS 
	},
	// CPU_MEDIUM
	{
		0.00f,	// TNS_NONE 
		0.01f,	// TNS_MISS
		0.02f,	// TNS_BOO 
		0.05f,	// TNS_GOOD 
		0.45f,	// TNS_GREAT 
		0.70f,	// TNS_PERFECT 
		1.00f,	// TNS_MARVELOUS 
	},
	// CPU_HARD
	{
		0.00f,	// TNS_NONE 
		0.005f,	// TNS_MISS
		0.007f,	// TNS_BOO 
		0.01f,	// TNS_GOOD 
		0.10f,	// TNS_GREAT 
		0.60f,	// TNS_PERFECT 
		1.00f,	// TNS_MARVELOUS 
	},
	// CPU_AUTOPLAY
	{
		0.00f,	// TNS_NONE 
		0.00f,	// TNS_MISS
		0.00f,	// TNS_BOO 
		0.00f,	// TNS_GOOD 
		0.00f,	// TNS_GREAT 
		0.00f,	// TNS_PERFECT 
		1.00f,	// TNS_MARVELOUS 
	},
};


TapNoteScore PlayerAI::GetTapNoteScore( PlayerController pc, int iSumOfAttackLevels )
{
	TapNoteScore tns = TAP_SCORE_DISTRIBUTIONS[pc].GetTapNoteScore( iSumOfAttackLevels+1 );
	ASSERT( tns != TNS_NONE );	// sanity check on PlayerAI's result
	return tns;
}