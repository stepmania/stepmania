#pragma once
/*
-----------------------------------------------------------------------------
 Course: Course

 Desc: A queue of songs and notes.

 Copyright (c) 2001-2002 by the person(s) listed below.  All rights reserved.
	Chris Danford
-----------------------------------------------------------------------------
*/

#include "GameConstantsAndTypes.h"
class Song;
struct Notes;

const int MAX_COURSE_STAGES = 100;

class Course
{
public:
	Course()
	{
		m_iStages = 0;
		m_bRepeat = false;
		for( int i=0; i<MAX_COURSE_STAGES; i++ )
			m_apSongs[i] = NULL;
	}

	CString		m_sName;
	CString		m_sBannerPath;
	CString		m_sCDTitlePath;
	int			m_iStages;
	Song*		m_apSongs[MAX_COURSE_STAGES];
	DifficultyClass m_aDifficultyClasses[MAX_COURSE_STAGES];
	bool		m_bRepeat;	// repeat after last song?
	PlayerOptions	m_PlayerOptions;

	void LoadFromCRSFile( CString sPath, CArray<Song*,Song*> &apSongs );

	void AddStage( Song* pSong, DifficultyClass dc )
	{
		ASSERT( m_iStages <= MAX_COURSE_STAGES );
		m_apSongs[m_iStages] = pSong;
		m_aDifficultyClasses[m_iStages] = dc;
		m_iStages++;
	}

};
