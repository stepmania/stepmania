/* Trail - A queue of Songs and Steps that are generated from a Course. */

#ifndef TRAIL_H
#define TRAIL_H

#include "Attack.h"
#include "RadarValues.h"

class Song;
class Steps;

struct TrailEntry
{
	TrailEntry(): 
		pSong(NULL), 
		pSteps(NULL),
		bMystery(false),
		iLowMeter(-1),
		iHighMeter(-1),
		dc(DIFFICULTY_INVALID)
	{
	}
	void GetAttackArray( AttackArray &out ) const;

	Song*		pSong;
	Steps*		pSteps;
	CString		Modifiers;
	AttackArray Attacks;
	bool		bMystery;

	/* These represent the meter and difficulty used by the course to pick the
	 * steps; if you want the real difficulty and meter, look at pSteps. */
	int			iLowMeter;
	int			iHighMeter;
	Difficulty	dc;
	bool operator== ( const TrailEntry &rhs ) const;
	bool operator!= ( const TrailEntry &rhs ) const { return !(*this==rhs); }
	bool ContainsTransformOrTurn() const;
};

class Trail
{
public:
	StepsType			m_StepsType;
	CourseDifficulty	m_CourseDifficulty;
	vector<TrailEntry>  m_vEntries;
	int					m_iSpecifiedMeter;	// == -1 if no meter specified
	mutable bool		m_bRadarValuesCached;
	mutable RadarValues	m_CachedRadarValues;

	Trail()
	{
		Init();
	}
	void Init()
	{
		m_StepsType = STEPS_TYPE_INVALID;
		m_CourseDifficulty = DIFFICULTY_INVALID;
		m_iSpecifiedMeter = -1;
		m_vEntries.clear();
		m_bRadarValuesCached = false;
	}

	RadarValues GetRadarValues() const;
	int GetMeter() const;
	int GetTotalMeter() const;
	float GetLengthSeconds() const;
	void GetDisplayBpms( DisplayBpms &AddTo );
	bool IsMystery() const;
	bool ContainsSong( Song* pSong ) const;
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
