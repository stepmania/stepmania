/* Trail - A queue of Songs and Steps that are generated from a Course. */

#ifndef TRAIL_H
#define TRAIL_H

#include "Attack.h"
#include "RadarValues.h"
#include "Difficulty.h"

class Song;
class Steps;
struct lua_State;

struct TrailEntry
{
	TrailEntry(): 
		pSong(NULL), 
		pSteps(NULL),
		bSecret(false),
		iLowMeter(-1),
		iHighMeter(-1),
		dc(Difficulty_Invalid)
	{
	}
	void GetAttackArray( AttackArray &out ) const;

	Song*		pSong;
	Steps*		pSteps;
	RString		Modifiers;
	AttackArray	Attacks;
	bool		bSecret;	// show "???"

	/* These represent the meter and difficulty used by the course to pick the
	 * steps; if you want the real difficulty and meter, look at pSteps. */
	int		iLowMeter;
	int		iHighMeter;
	Difficulty	dc;
	bool operator== ( const TrailEntry &rhs ) const;
	bool operator!= ( const TrailEntry &rhs ) const { return !(*this==rhs); }
	bool ContainsTransformOrTurn() const;
};

class Trail
{
public:
	StepsType		m_StepsType;
	CourseDifficulty	m_CourseDifficulty;
	vector<TrailEntry>	m_vEntries;
	int			m_iSpecifiedMeter;	// == -1 if no meter specified
	mutable bool		m_bRadarValuesCached;
	mutable RadarValues	m_CachedRadarValues;

	Trail()
	{
		Init();
	}
	void Init()
	{
		m_StepsType = StepsType_Invalid;
		m_CourseDifficulty = Difficulty_Invalid;
		m_iSpecifiedMeter = -1;
		m_vEntries.clear();
		m_bRadarValuesCached = false;
	}

	const RadarValues &GetRadarValues() const;
	void SetRadarValues( const RadarValues &rv ); // for pre-populating cache
	int GetMeter() const;
	int GetTotalMeter() const;
	float GetLengthSeconds() const;
	void GetDisplayBpms( DisplayBpms &AddTo ) const;
	bool IsSecret() const;
	bool ContainsSong( const Song *pSong ) const;

	// Lua
	void PushSelf( lua_State *L );
};

#endif

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
