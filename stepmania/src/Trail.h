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
		iHighMeter(-1)
	{
	}
	void GetAttackArray( AttackArray &out ) const;

	Song*		pSong;
	Steps*		pSteps;
	CString		Modifiers;
	AttackArray Attacks;
	bool		bMystery;
	int			iLowMeter;
	int			iHighMeter;
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
