#ifndef SONGOPTIONS_H
#define SONGOPTIONS_H
/*
-----------------------------------------------------------------------------
 Class: SongOptions

 Desc: Per-song options that are not saved between sessions.  These are options
	that should probably be disabled in a coin-operated machine.

 Copyright (c) 2001-2002 by the person(s) listed below.  All rights reserved.
	Chris Danford
-----------------------------------------------------------------------------
*/


struct SongOptions
{
	enum LifeType { LIFE_BAR=0, LIFE_BATTERY, NUM_LIFE_TYPES };
	LifeType m_LifeType;
	enum DrainType { DRAIN_NORMAL, DRAIN_NO_RECOVER, DRAIN_SUDDEN_DEATH };
	DrainType m_DrainType;	// only used with LifeBar
	int m_iBatteryLives;
	enum FailType { FAIL_ARCADE=0, FAIL_END_OF_SONG, FAIL_OFF };
	FailType m_FailType;
	float m_fMusicRate;
	bool m_bAssistTick, m_bAutoSync;

	SongOptions() { Init(); };
	void Init() 
	{
		m_LifeType = LIFE_BAR;
		m_DrainType = DRAIN_NORMAL;
		m_iBatteryLives = 4;
		m_FailType = FAIL_ARCADE;
		m_bAssistTick = false;
		m_fMusicRate = 1.0f;
		m_bAutoSync = false;
	};
	CString GetString();
	void FromString( CString sOptions );
};

#endif
