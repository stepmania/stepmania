#include "global.h"
#include "Trail.h"
#include "GameState.h"
#include "Steps.h"
#include "Song.h"
#include "PlayerOptions.h"
#include "NoteData.h"
#include "NoteDataUtil.h"
#include "CommonMetrics.h"

#include <cmath>
#include <numeric>
#include <vector>


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

// TrailEntry lua start
#include "LuaBinding.h"

/** @brief Allow Lua to have access to the TrailEntry. */
class LunaTrailEntry: public Luna<TrailEntry>
{
public:
	static int GetSong( T* p, lua_State *L )
	{
		if( p->pSong )
			p->pSong->PushSelf(L);
		else
			lua_pushnil(L);
		return 1;
	}
	static int GetSteps( T* p, lua_State *L )
	{
		if( p->pSteps )
			p->pSteps->PushSelf(L);
		else
			lua_pushnil(L);
		return 1;
	}
	DEFINE_METHOD( IsSecret, bSecret );
	DEFINE_METHOD( GetNormalModifiers, Modifiers );

	LunaTrailEntry()
	{
		ADD_METHOD( GetSong );
		ADD_METHOD( GetSteps );
		ADD_METHOD( IsSecret );
		ADD_METHOD( GetNormalModifiers );
	}
};

LUA_REGISTER_CLASS( TrailEntry )
// TrailEntry lua end

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

		for (TrailEntry const &e : m_vEntries)
		{
			const Steps *pSteps = e.pSteps;
			ASSERT( pSteps != nullptr );
			// Hack: don't calculate for autogen entries
			if( !pSteps->IsAutogen() && e.ContainsTransformOrTurn() )
			{
				NoteData nd;
				pSteps->GetNoteData( nd );
				RadarValues rv_orig;
				GAMESTATE->SetProcessedTimingData(const_cast<TimingData *>(pSteps->GetTimingData()));
				NoteDataUtil::CalculateRadarValues( nd, e.pSong->m_fMusicLengthSeconds, rv_orig );
				PlayerOptions po;
				po.FromString( e.Modifiers );
				if( po.ContainsTransformOrTurn() )
				{
					NoteDataUtil::TransformNoteData(nd, *(pSteps->GetTimingData()), po, pSteps->m_StepsType);
				}
				NoteDataUtil::TransformNoteData(nd, *(pSteps->GetTimingData()), e.Attacks, pSteps->m_StepsType, e.pSong);
				RadarValues transformed_rv;
				NoteDataUtil::CalculateRadarValues( nd, e.pSong->m_fMusicLengthSeconds, transformed_rv );
				GAMESTATE->SetProcessedTimingData(nullptr);
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

	return std::lrint( fMeter );
}

int Trail::GetTotalMeter() const
{
	return std::accumulate(m_vEntries.begin(), m_vEntries.end(), 0, [](int total, TrailEntry const &e) {
		return total + e.pSteps->GetMeter();
	});
}

float Trail::GetLengthSeconds() const
{
	return std::accumulate(m_vEntries.begin(), m_vEntries.end(), 0.f, [](float total, TrailEntry const &e) {
		return total + e.pSong->m_fMusicLengthSeconds;
	});
}

void Trail::GetDisplayBpms( DisplayBpms &AddTo ) const
{
	for (TrailEntry const &e : m_vEntries)
	{
		if( e.bSecret )
		{
			AddTo.Add( -1 );
			continue;
		}

		Song *pSong = e.pSong;
		ASSERT( pSong != nullptr );
		switch( pSong->m_DisplayBPMType )
		{
		case DISPLAY_BPM_ACTUAL:
		case DISPLAY_BPM_SPECIFIED:
			pSong->GetDisplayBpms( AddTo );
			break;
		case DISPLAY_BPM_RANDOM:
			AddTo.Add( -1 );
			break;
		DEFAULT_FAIL( pSong->m_DisplayBPMType );
		}
	}
}

bool Trail::IsSecret() const
{
	return std::any_of(m_vEntries.begin(), m_vEntries.end(), [](TrailEntry const &e) {
		return e.bSecret;
	});
}

bool Trail::ContainsSong( const Song *pSong ) const
{
	return std::any_of(m_vEntries.begin(), m_vEntries.end(), [&](TrailEntry const &e) {
		return e.pSong == pSong;
	});
}

// lua start
#include "LuaBinding.h"

/** @brief Allow Lua to have access to the Trail. */
class LunaTrail: public Luna<Trail>
{
public:
	static int GetDifficulty( T* p, lua_State *L )		{ LuaHelpers::Push(L, p->m_CourseDifficulty ); return 1; }
	static int GetMeter( T* p, lua_State *L )		{ LuaHelpers::Push(L, p->GetMeter() ); return 1; }
	static int GetTotalMeter( T* p, lua_State *L )		{ LuaHelpers::Push(L, p->GetTotalMeter() ); return 1; }
	static int GetStepsType( T* p, lua_State *L )	{ LuaHelpers::Push(L, p->m_StepsType ); return 1; }
	static int GetRadarValues( T* p, lua_State *L )
	{
		RadarValues &rv = const_cast<RadarValues &>(p->GetRadarValues());
		rv.PushSelf(L);
		return 1;
	}
	static int GetArtists( T* p, lua_State *L )
	{
		std::vector<RString> asArtists, asAltArtists;
		for (TrailEntry const &e : p->m_vEntries)
		{
			if( e.bSecret )
			{
				asArtists.push_back( "???" );
				asAltArtists.push_back( "???" );
			}
			else
			{
				asArtists.push_back( e.pSong->GetDisplayArtist() );
				asAltArtists.push_back( e.pSong->GetTranslitArtist() );
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
	static int GetTrailEntry( T* p, lua_State *L )	{ TrailEntry &te = p->m_vEntries[IArg(1)]; te.PushSelf(L); return 1; }
	static int GetTrailEntries( T* p, lua_State *L )
	{
		std::vector<TrailEntry*> v;
		for( unsigned i = 0; i < p->m_vEntries.size(); ++i )
		{
			v.push_back(&p->m_vEntries[i]);
		}
		LuaHelpers::CreateTableFromArray<TrailEntry*>( v, L );
		return 1;
	}
	DEFINE_METHOD( GetLengthSeconds, GetLengthSeconds() )
	DEFINE_METHOD( IsSecret, IsSecret() )
	static int ContainsSong( T* p, lua_State *L )
	{
		const Song *pS = Luna<Song>::check(L,1);
		lua_pushboolean(L, p->ContainsSong(pS));
		return 1;
	}

	LunaTrail()
	{
		ADD_METHOD( GetDifficulty );
		ADD_METHOD( GetMeter );
		ADD_METHOD( GetTotalMeter );
		ADD_METHOD( GetStepsType );
		ADD_METHOD( GetRadarValues );
		ADD_METHOD( GetArtists );
		ADD_METHOD( GetTrailEntry );
		ADD_METHOD( GetTrailEntries );
		ADD_METHOD( GetLengthSeconds );
		ADD_METHOD( IsSecret );
		ADD_METHOD( ContainsSong );
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
