#ifndef Trail_H
#define Trail_H
/*
-----------------------------------------------------------------------------
 Class: TrialUtil

 Desc: 

 Copyright (c) 2001-2002 by the person(s) listed below.  All rights reserved.
	Chris Danford
-----------------------------------------------------------------------------
*/

#include "Attack.h"

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
};

class Trail
{
public:
	StepsType			m_StepsType;
	CourseDifficulty	m_CourseDifficulty;
	vector<TrailEntry>  m_vEntries;

	Trail()
	{
		m_StepsType = STEPS_TYPE_INVALID;
		m_CourseDifficulty = COURSE_DIFFICULTY_INVALID;
	}

	RadarValues GetRadarValues() const;
	float GetAverageMeter() const;
	int GetTotalMeter() const;
	float GetLengthSeconds() const;
	void GetDisplayBpms( DisplayBpms &AddTo );
};


#endif
