#ifndef Trail_H
#define Trail_H

#include "Attack.h"

class Song;
class Steps;

struct TrailEntry
{
	TrailEntry(): 
		pSong(NULL), 
		pNotes(NULL),
		bMystery(false),
		iLowMeter(-1),
		iHighMeter(-1)
	{
	}
	void GetAttackArray( AttackArray &out ) const;

	Song*		pSong;
	Steps*		pNotes;
	CString		Modifiers;
	AttackArray Attacks;
	bool		bMystery;
	int			iLowMeter;
	int			iHighMeter;
};

class Trail
{
public:
	StepsType			m_st;
	CourseDifficulty	m_cd;
	vector<TrailEntry>  m_vEntries;

	Trail()
	{
		m_st = STEPS_TYPE_INVALID;
		m_cd = COURSE_DIFFICULTY_INVALID;
	}

	RadarValues GetRadarValues() const;
	float GetAverageMeter() const;
	int GetTotalMeter() const;
	float GetLengthSeconds() const;
	void GetDisplayBpms( DisplayBpms &AddTo );
};


#endif
