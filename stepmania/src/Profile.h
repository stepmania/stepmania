#ifndef Profile_H
#define Profile_H
/*
-----------------------------------------------------------------------------
 Class: Profile

 Desc: Player data that persists between sessions.  Can be stored on a local
	disk or on a memory card.

 Copyright (c) 2001-2002 by the person(s) listed below.  All rights reserved.
	Chris Danford
-----------------------------------------------------------------------------
*/

#include "GameConstantsAndTypes.h"
#include "Style.h"

struct Profile
{
	Profile() { Init(); }
	void Init()
	{
		m_sName = "";
		m_sLastUsedHighScoreName = "";
		m_bUsingProfileDefaultModifiers = false;
		m_sDefaultModifiers = "";
		m_iTotalPlays = 0;
		m_iTotalPlaySeconds = 0;
		m_iTotalGameplaySeconds = 0;
		m_iCurrentCombo = 0;
		m_fWeightPounds = 0;
		m_fCaloriesBurned = 0;

		int i;
		for( i=0; i<NUM_PLAY_MODES; i++ )
			m_iNumSongsPlayedByPlayMode[i] = 0;
		for( i=0; i<NUM_STYLES; i++ )
			m_iNumSongsPlayedByStyle[i] = 0;
		for( i=0; i<NUM_DIFFICULTIES; i++ )
			m_iNumSongsPlayedByDifficulty[i] = 0;
		for( i=0; i<MAX_METER+1; i++ )
			m_iNumSongsPlayedByMeter[i] = 0;
	}

	CString GetDisplayName();
	CString GetDisplayCaloriesBurned();
	int GetTotalNumSongsPlayed();

	bool LoadFromIni( CString sIniPath );
	bool SaveToIni( CString sIniPath );
	CString m_sName;
	CString m_sLastUsedHighScoreName;
	bool m_bUsingProfileDefaultModifiers;
	CString m_sDefaultModifiers;
	int m_iTotalPlays;
	int m_iTotalPlaySeconds;
	int m_iTotalGameplaySeconds;
	int m_iCurrentCombo;
	float m_fWeightPounds;
	int m_fCaloriesBurned;
	int m_iNumSongsPlayedByPlayMode[NUM_PLAY_MODES];
	int m_iNumSongsPlayedByStyle[NUM_STYLES];
	int m_iNumSongsPlayedByDifficulty[NUM_DIFFICULTIES];
	int m_iNumSongsPlayedByMeter[MAX_METER+1];
};


#endif
