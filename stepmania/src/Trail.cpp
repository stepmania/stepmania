#include "global.h"
#include "Trail.h"
#include "Foreach.h"
#include "Steps.h"
#include "song.h"
#include "PlayerOptions.h"
#include "NoteData.h"
#include "NoteDataUtil.h"

void TrailEntry::GetAttackArray( AttackArray &out ) const
{
	if( !Modifiers.empty() )
	{
		Attack a;
		a.fStartSecond = 0;
		a.fSecsRemaining = 10000; /* whole song */
		a.level = ATTACK_LEVEL_1;
		a.sModifier = Modifiers;
		a.bGlobal = true;

		out.push_back( a );
	}

	out.insert( out.end(), Attacks.begin(), Attacks.end() );
}

bool TrailEntry::operator== ( const TrailEntry &rhs ) const
{
#define EQUAL(a) (a==rhs.a)
	return 
		EQUAL(pSong) &&
		EQUAL(pSteps) &&
		EQUAL(Modifiers) &&
		EQUAL(Attacks) &&
		EQUAL(bMystery) &&
		EQUAL(iLowMeter) &&
		EQUAL(iHighMeter) &&
		EQUAL(dc);
}

bool TrailEntry::ContainsTransformOrTurn() const
{
	PlayerOptions po;
	po.FromString( Modifiers );
	if( po.ContainsTransformOrTurn() )
		return true;
	if( Attacks.ContainsTransformOrTurn() )
		return true;
	return false;
}

RadarValues Trail::GetRadarValues() const
{
	if( IsMystery() )
	{
		// Don't calculate RadarValues for a non-fixed Course.  They values are 
		// worthless because they'll change every time this Trail is 
		// regenerated.
		return RadarValues();
	}
	else if( m_bRadarValuesCached )
	{
		return m_CachedRadarValues;
	}
	else
	{
		m_bRadarValuesCached = true;
		m_CachedRadarValues.Zero();

		FOREACH_CONST( TrailEntry, m_vEntries, e )
		{
			const Steps *pSteps = e->pSteps;
			ASSERT( pSteps );
			/* Hack: don't calculate for autogen entries; it makes writing Catalog.xml
			 * take way too long.  (Tournamix 4 Sample.crs takes me ~10s.) */
			if( !pSteps->IsAutogen() && e->ContainsTransformOrTurn() )
			{
				NoteData nd;
				pSteps->GetNoteData( &nd );
				RadarValues rv_orig;
				NoteDataUtil::GetRadarValues( nd, e->pSong->m_fMusicLengthSeconds, rv_orig );
				PlayerOptions po;
				po.FromString( e->Modifiers );
				if( po.ContainsTransformOrTurn() )
					NoteDataUtil::TransformNoteData( nd, po, pSteps->m_StepsType );
				NoteDataUtil::TransformNoteData( nd, e->Attacks, pSteps->m_StepsType, e->pSong );
				RadarValues rv;
				NoteDataUtil::GetRadarValues( nd, e->pSong->m_fMusicLengthSeconds, rv );
				m_CachedRadarValues += rv;
			}
			else
			{
				m_CachedRadarValues += pSteps->GetRadarValues();			
			}
		}

		return m_CachedRadarValues;
	}
}

int Trail::GetMeter() const
{
	if( m_iSpecifiedMeter != -1 )
		return m_iSpecifiedMeter;

	if( m_vEntries.empty() )
		return 0;

	float fMeter = GetTotalMeter() / (float)m_vEntries.size();

	return (int)roundf( fMeter );
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

bool Trail::IsMystery() const
{
	FOREACH_CONST( TrailEntry, m_vEntries, e )
	{
		if( e->bMystery )
			return true;
	}
	return false;
}

bool Trail::ContainsSong( Song* pSong ) const
{
	FOREACH_CONST( TrailEntry, m_vEntries, e )
	{
		if( e->pSong == pSong )
			return true;
	}
	return false;
}

/*
 * (c) 2001-2004 Chris Danford, Glenn Maynard
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
