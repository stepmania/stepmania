#pragma once
/*
-----------------------------------------------------------------------------
 Class: Course

 Desc: A queue of songs and notes.

 Copyright (c) 2001-2002 by the person(s) listed below.  All rights reserved.
	Chris Danford
-----------------------------------------------------------------------------
*/

#include "GameConstantsAndTypes.h"
struct PlayerOptions;
struct SongOptions;
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
		m_iLives = 4;
		for( int i=0; i<MAX_COURSE_STAGES; i++ )
			m_apSongs[i] = NULL;
	}

	CString		m_sName;
	CString		m_sBannerPath;
	CString		m_sCDTitlePath;
	int			m_iStages;
	Song*		m_apSongs[MAX_COURSE_STAGES];
	CString		m_asDescriptions[MAX_COURSE_STAGES];
	Notes*		GetNotesForStage( int iStage );
	bool		m_bRepeat;	// repeat after last song?
	int			m_iLives;	// -1 means use bar life meter
	CString		m_sModifiers;	// contains player options and song options

	void GetPlayerOptions( PlayerOptions* pPO_out );
	void GetSongOptions( SongOptions* pSO_out);

	void LoadFromCRSFile( CString sPath, CArray<Song*,Song*> &apSongs );
	void CreateFromGroupAndDifficultyClass( CString sGroupName, DifficultyClass dc, CArray<Song*,Song*> &apSongsInGroup );

	void AddStage( Song* pSong, CString sDescription )
	{
		ASSERT( m_iStages <= MAX_COURSE_STAGES );
		m_apSongs[m_iStages] = pSong;
		m_asDescriptions[m_iStages] = sDescription;
		m_iStages++;
	}

	void GetSongAndNotesForCurrentStyle( CArray<Song*,Song*>& apSongsOut, CArray<Notes*,Notes*> apNotesOut[NUM_PLAYERS] );
	D3DXCOLOR	GetColor();
};


void SortCoursePointerArrayByDifficulty( CArray<Course*,Course*> &apCourses );

