#include "global.h"
/*
-----------------------------------------------------------------------------
 Class: Trail

 Desc: See header.

 Copyright (c) 2001-2002 by the person(s) listed below.  All rights reserved.
	Chris Danford
-----------------------------------------------------------------------------
*/

#include "Trail.h"
#include "Foreach.h"
#include "Steps.h"
#include "song.h"

void TrailEntry::GetAttackArray( AttackArray &out ) const
{
	if( !Modifiers.empty() )
	{
		Attack a;
		a.fStartSecond = 0;
		a.fSecsRemaining = 10000; /* whole song */
		a.level = ATTACK_LEVEL_1;
		a.sModifier = Modifiers;

		out.push_back( a );
	}

	out.insert( out.end(), Attacks.begin(), Attacks.end() );
}

RadarValues Trail::GetRadarValues() const
{
	RadarValues rv;

	FOREACH_CONST( TrailEntry, m_vEntries, e )
	{
		const Steps *pSteps = e->pSteps;
		ASSERT( pSteps );
		rv += pSteps->GetRadarValues();
	}

	return rv;
}

float Trail::GetAverageMeter() const
{
	if( m_vEntries.empty() )
		return 0;

	return GetTotalMeter() / (float)m_vEntries.size();
}

int Trail::GetTotalMeter() const
{
	int iTotalMeter = 0;
	FOREACH_CONST( TrailEntry, m_vEntries, e )
	{
		iTotalMeter += e->pSteps->GetMeter();
	}

	return iTotalMeter;
}

float Trail::GetLengthSeconds() const
{
	float fSecs = 0;
	FOREACH_CONST( TrailEntry, m_vEntries, e )
	{
		fSecs += e->pSong->m_fMusicLengthSeconds;
	}
	return fSecs;
}

void Trail::GetDisplayBpms( DisplayBpms &AddTo )
{
	FOREACH_CONST( TrailEntry, m_vEntries, e )
	{
		if( e->bMystery )
		{
			AddTo.Add( -1 );
			continue;
		}

		Song *pSong = e->pSong;
		ASSERT( pSong );
		switch( pSong->m_DisplayBPMType )
		{
		case Song::DISPLAY_ACTUAL:
		case Song::DISPLAY_SPECIFIED:
			{
				pSong->GetDisplayBpms( AddTo );
			}
			break;
		case Song::DISPLAY_RANDOM:
			AddTo.Add( -1 );
			break;
		default:
			ASSERT(0);
		}
	}
}

