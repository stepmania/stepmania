#pragma once
/*
-----------------------------------------------------------------------------
 Class: Notes

 Desc: Holds note information for a Song.  A Song may have one or more Notess.
	A Notes can be played by one or more Styles.  See StyleDef.h for more info on
	Styles.

 Copyright (c) 2001-2002 by the person(s) listed below.  All rights reserved.
	Chris Danford
-----------------------------------------------------------------------------
*/

#include "GameConstantsAndTypes.h"
#include "Grade.h"
#include "NoteData.h"


struct Notes
{
public:
	Notes();
	~Notes();

	// Loading
	bool LoadFromNotesFile( const CString &sPath );
	bool LoadFromBMSFile( const CString &sPath );
	bool LoadFromDWITokens( const CString &sMode, const CString &sDescription,
							const CString &sNumFeet,
							const CString &sStepData1, const CString &sStepData2 );
	void ReadFromCacheFile( FILE* file, bool bReadNoteData );

	// for saving
	void SaveToSMDir( CString sSongDir );
	void WriteToCacheFile( FILE* file );

public:
	CString			m_sIntendedGame;
	CString			m_sIntendedStyle;
	CString			m_sDescription;			// This text is displayed next to thte number of feet when a song is selected
	CString			m_sCredit;				// name of the person who created these Notes
	DifficultyClass m_DifficultyClass;		// this is inferred from m_sDescription
	int				m_iMeter;				// difficulty from 1-10
	float			m_fRadarValues[NUM_RADAR_VALUES];	// between 0.0-1.2 starting from 12-o'clock rotating clockwise
	
	// Statistics
	Grade m_TopGrade;
	int m_iTopScore;
	int m_iMaxCombo;
	int m_iNumTimesPlayed;

	bool		IsNoteDataLoaded();
	NoteData*	GetNoteData();
	void		SetNoteData( NoteData* pNewNoteData );
	void		DeleteNoteData();

protected:
	NoteData*	m_pNoteData;
};


void SortNotesArrayByDifficultyClass( CArray<Notes*,Notes*> &arrayNotess );
