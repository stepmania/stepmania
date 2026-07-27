#ifndef TRAIL_H
#define TRAIL_H

#include "Attack.h"
#include "RadarValues.h"
#include "Difficulty.h"

#include <vector>


class Song;
class Steps;
struct lua_State;

/** @brief One such Song and
 * <a class="el" href="class_steps.html">Step</a> in the entire Trail. */
struct TrailEntry
{
	TrailEntry():
		pSong(nullptr),
		pSteps(nullptr),
		Modifiers(""),
		Attacks(),
		bSecret(false),
		iLowMeter(-1),
		iHighMeter(-1),
		dc(Difficulty_Invalid)
	{
	}
	void GetAttackArray( AttackArray &out ) const;

	/** @brief The Song involved in the entry. */
	Song*		pSong;
	/** @brief The <a class="el" href="class_steps.html">Step</a> involved in the entry. */
	Steps*		pSteps;
	/** @brief The Modifiers applied for the whole Song. */
	RString		Modifiers;
	/** @brief The Attacks that will take place durring the Song. */
	AttackArray	Attacks;
	/**
	 * @brief Is this Song and its Step meant to be a secret?
	 * If so, it will show text such as "???" to indicate that it's a mystery. */
	bool		bSecret;

	/* These represent the meter and difficulty used by the course to pick the
	 * steps; if you want the real difficulty and meter, look at pSteps. */
	int		iLowMeter;
	int		iHighMeter;
	Difficulty	dc;
	bool operator== ( const TrailEntry &rhs ) const;
	bool operator!= ( const TrailEntry &rhs ) const { return !(*this==rhs); }
	bool ContainsTransformOrTurn() const;

	// Lua
	void PushSelf( lua_State *L );
};

/** @brief A queue of Songs and Steps that are generated from a Course. */
class Trail
{
public:
	StepsType		m_StepsType;
	CourseType		m_CourseType;
	CourseDifficulty	m_CourseDifficulty;
	std::vector<TrailEntry>	m_vEntries;
	int			m_iSpecifiedMeter;	// == -1 if no meter specified
	mutable bool		m_bRadarValuesCached;
	mutable RadarValues	m_CachedRadarValues;

	/**
	 * @brief Set up the Trail with default values.
	 *
	 * This used to call Init(), which is still available. */
	Trail(): m_StepsType(StepsType_Invalid),
		m_CourseType(CourseType_Invalid),
		m_CourseDifficulty(Difficulty_Invalid),
		m_vEntries(), m_iSpecifiedMeter(-1),
		m_bRadarValuesCached(false), m_CachedRadarValues() {}
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

/**
 * @file
 * @author Chris Danford, Glenn Maynard (c) 2001-2004
 * @section LICENSE
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
