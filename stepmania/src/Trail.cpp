#include "global.h"
#include "Trail.h"
#include "Foreach.h"
#include "Steps.h"
#include "song.h"
#include "PlayerOptions.h"
#include "NoteData.h"
#include "NoteDataUtil.h"
#include "CommonMetrics.h"

void TrailEntry::GetAttackArray( AttackArray &out ) const
{
	if( !Modifiers.empty() )
		out.push_back( Attack::FromGlobalCourseModifier( Modifiers ) );

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
		EQUAL(bSecret) &&
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

void Trail::SetRadarValues( const RadarValues &rv )
{
	m_CachedRadarValues = rv;
	m_bRadarValuesCached = true;
}

const RadarValues &Trail::GetRadarValues() const
{
	if( m_bRadarValuesCached )
	{
		return m_CachedRadarValues;
	}
	if( IsSecret() )
	{
		// Don't calculate RadarValues for a non-fixed Course.  They values are 
		// worthless because they'll change every time this Trail is 
		// regenerated.
		m_CachedRadarValues = RadarValues();
		return m_CachedRadarValues;
	}
	else
	{
		RadarValues rv;
		rv.Zero();

		FOREACH_CONST( TrailEntry, m_vEntries, e )
		{
			const Steps *pSteps = e->pSteps;
			ASSERT( pSteps );
			/* Hack: don't calculate for autogen entries; it makes writing Catalog.xml
			 * take way too long.  (Tournamix 4 Sample.crs takes me ~10s.) */
			if( !pSteps->IsAutogen() && e->ContainsTransformOrTurn() )
			{
				NoteData nd;
				pSteps->GetNoteData( nd );
				RadarValues rv_orig;
				NoteDataUtil::CalculateRadarValues( nd, e->pSong->m_fMusicLengthSeconds, rv_orig );
				PlayerOptions po;
				po.FromString( e->Modifiers );
				if( po.ContainsTransformOrTurn() )
					NoteDataUtil::TransformNoteData( nd, po, pSteps->m_StepsType );
				NoteDataUtil::TransformNoteData( nd, e->Attacks, pSteps->m_StepsType, e->pSong );
				RadarValues transformed_rv;
				NoteDataUtil::CalculateRadarValues( nd, e->pSong->m_fMusicLengthSeconds, transformed_rv );
				rv += transformed_rv;
			}
			else
			{
				rv += pSteps->GetRadarValues( PLAYER_1 );			
			}
		}

		/* Hack: SetRadarValues is non-const (a const setter doesn't
		 * make sense), but it only modifies a mutable value.  Just
		 * cast away const. */
		const_cast<Trail*>(this)->SetRadarValues( rv );

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

void Trail::GetDisplayBpms( DisplayBpms &AddTo ) const
{
	FOREACH_CONST( TrailEntry, m_vEntries, e )
	{
		if( e->bSecret )
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

bool Trail::IsSecret() const
{
	FOREACH_CONST( TrailEntry, m_vEntries, e )
	{
		if( e->bSecret )
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

// lua start
#include "LuaBinding.h"

class LunaTrail: public Luna<Trail>
{
public:
	static int GetDifficulty( T* p, lua_State *L )		{ LuaHelpers::Push(L, p->m_CourseDifficulty ); return 1; }
	static int GetStepsType( T* p, lua_State *L )	{ lua_pushnumber(L, p->m_StepsType ); return 1; }
	static int GetRadarValues( T* p, lua_State *L )
	{
		RadarValues &rv = const_cast<RadarValues &>(p->GetRadarValues());
		rv.PushSelf(L);
		return 1;
	}
	static int GetArtists( T* p, lua_State *L )
	{
		vector<RString> asArtists, asAltArtists;
		FOREACH_CONST( TrailEntry, p->m_vEntries, e )
		{
			if( e->bSecret )
			{
				asArtists.push_back( "???" );
				asAltArtists.push_back( "???" );
			}
			else
			{
				asArtists.push_back( e->pSong->GetDisplayArtist() );
				asAltArtists.push_back( e->pSong->GetTranslitArtist() );
			}
		}

		if( (int) asArtists.size() > CommonMetrics::MAX_COURSE_ENTRIES_BEFORE_VARIOUS )
		{
			asArtists.clear();
			asAltArtists.clear();
			asArtists.push_back( "Various Artists" );
			asAltArtists.push_back( "Various Artists" );
		}

		LuaHelpers::CreateTableFromArray( asArtists, L );
		LuaHelpers::CreateTableFromArray( asAltArtists, L );
		return 2;
	}

	LunaTrail()
	{
		ADD_METHOD( GetDifficulty );
		ADD_METHOD( GetStepsType );
		ADD_METHOD( GetRadarValues );
		ADD_METHOD( GetArtists );
	}
};

LUA_REGISTER_CLASS( Trail )
// lua end

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
