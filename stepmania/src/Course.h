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


const int MAX_COURSE_STAGES = 300; // Increased since the user can place all Bemani songs in a single folder


class Course
{
public:
	Course()
	{
		m_iStages = 0;
		m_bRepeat = false;
		m_bRandomize = false;
		m_iLives = 4;
		m_iExtra = 0;
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
	CString		m_asModifiers[MAX_COURSE_STAGES];	// set player and song options from these
	bool		m_bRepeat;	// repeat after last song?
	bool		m_bRandomize;	// play the songs in a random order
	int			m_iLives;	// -1 means use bar life meter
	int			m_iExtra;	// extra stage number...

	void GetPlayerOptions( PlayerOptions* pPO_out );
	void GetSongOptions( SongOptions* pSO_out);

	void LoadFromCRSFile( CString sPath, CArray<Song*,Song*> &apSongs );
	void CreateEndlessCourseFromGroupAndDifficulty( CString sGroupName, Difficulty dc, CArray<Song*,Song*> &apSongsInGroup );

	void AddStage( Song* pSong, CString sDescription, CString sModifiers )
	{
		ASSERT( m_iStages <= MAX_COURSE_STAGES - 1 );
		m_apSongs[m_iStages] = pSong;
		m_asDescriptions[m_iStages] = sDescription;
		m_asModifiers[m_iStages] = sModifiers;
		SongOrdering[m_iStages] = m_iStages;

		m_iStages++;
	}

	void GetSongAndNotesForCurrentStyle( CArray<Song*,Song*>& apSongsOut, CArray<Notes*,Notes*>& apNotesOut, CStringArray& asModifiersOut, bool bShuffled );
	D3DXCOLOR	GetColor();

private:
	int SongOrdering[MAX_COURSE_STAGES];
	void Shuffle();
};


void SortCoursePointerArrayByDifficulty( CArray<Course*,Course*> &apCourses );

