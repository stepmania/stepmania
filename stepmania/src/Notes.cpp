#include "stdafx.h"
/*
-----------------------------------------------------------------------------
 Class: Notes

 Desc: See header.

 Copyright (c) 2001-2002 by the person(s) listed below.  All rights reserved.
	Chris Danford
-----------------------------------------------------------------------------
*/

#include "Notes.h"
#include "Song.h"
#include "Notes.h"
#include "IniFile.h"
#include "math.h"	// for fabs()
#include "RageUtil.h"
#include "RageLog.h"
#include "NoteData.h"
#include "GameInput.h"
#include "RageException.h"
#include "MsdFile.h"
#include "GameManager.h"
#include "ThemeManager.h"


#define COLOR_EASY		THEME->GetMetricC("Notes","ColorEasy")
#define COLOR_MEDIUM	THEME->GetMetricC("Notes","ColorMedium")
#define COLOR_HARD		THEME->GetMetricC("Notes","ColorHard")
#define COLOR_S_HARD	THEME->GetMetricC("Notes","ColorSHard")
#define COLOR_CHALLENGE	THEME->GetMetricC("Notes","ColorChallenge")
#define COLOR_BATTLE	THEME->GetMetricC("Notes","ColorBattle")


Notes::Notes()
{
	/* FIXME: should we init this to NOTES_TYPE_INVALID? 
	 * I have a feeling that it's the right thing to do but that
	 * it'd trip obscure asserts all over the place, so I'll wait
	 * until after b6 to do this. -glenn */
	m_NotesType = NOTES_TYPE_DANCE_SINGLE;
	m_bAutoGen = false;
	m_Difficulty = CLASS_INVALID;
	m_iMeter = 0;
	for(int i = 0; i < NUM_RADAR_VALUES; ++i)
		m_fRadarValues[i] = -1; /* unknown */

	m_iNumTimesPlayed = 0;
	m_iMaxCombo = 0;
	m_iTopScore = 0;
	m_TopGrade = GRADE_NO_DATA;
}

Notes::~Notes()
{
}

void Notes::SetNoteData( NoteData* pNewNoteData )
{
	ASSERT( pNewNoteData->m_iNumTracks == GameManager::NotesTypeToNumTracks(m_NotesType) );
	m_sSMNoteData = NoteDataUtil::GetSMNoteDataString(*pNewNoteData);
}

void Notes::GetNoteData( NoteData* pNoteDataOut ) const
{
	pNoteDataOut->m_iNumTracks = GameManager::NotesTypeToNumTracks( m_NotesType );
	NoteDataUtil::LoadFromSMNoteDataString( *pNoteDataOut, m_sSMNoteData );
}

void Notes::SetSMNoteData( const CString &out )
{
	m_sSMNoteData = out;
}

CString Notes::GetSMNoteData() const
{
	return m_sSMNoteData;
}

// Color is a function of Difficulty and Intended Style
NotesDisplayType Notes::GetNotesDisplayType() const
{
	CString sDescription = m_sDescription;
	sDescription.MakeLower();

	if( -1 != sDescription.Find("battle") )			return NOTES_DISPLAY_BATTLE;
	else if( -1 != sDescription.Find("couple") )	return NOTES_DISPLAY_BATTLE;
	else if( -1 != sDescription.Find("smaniac") )	return NOTES_DISPLAY_S_HARD;
	else if( -1 != sDescription.Find("challenge") )	return NOTES_DISPLAY_CHALLENGE;

	switch( m_Difficulty )
	{
	case DIFFICULTY_EASY:	return NOTES_DISPLAY_EASY;
	case DIFFICULTY_MEDIUM:	return NOTES_DISPLAY_MEDIUM;
	case DIFFICULTY_HARD:	return NOTES_DISPLAY_HARD;
	default:	ASSERT(0);	return NOTES_DISPLAY_EASY;
	}
}

RageColor Notes::GetColor() const
{
	switch( GetNotesDisplayType() )
	{
	case NOTES_DISPLAY_EASY:		return COLOR_EASY;
	case NOTES_DISPLAY_MEDIUM:		return COLOR_MEDIUM;
	case NOTES_DISPLAY_HARD:		return COLOR_HARD;
	case NOTES_DISPLAY_S_HARD:		return COLOR_S_HARD;
	case NOTES_DISPLAY_CHALLENGE:	return COLOR_CHALLENGE;
	case NOTES_DISPLAY_BATTLE:		return COLOR_BATTLE;
	default:	return COLOR_EASY;
	}
}

void Notes::TidyUpData()
{
	if( m_Difficulty == CLASS_INVALID )
		m_Difficulty = DifficultyFromDescriptionAndMeter( m_sDescription, m_iMeter );

	if( m_iMeter < 1 || m_iMeter > 10 ) 
	{
		switch( m_Difficulty )
		{
		case DIFFICULTY_EASY:	m_iMeter = 3;	break;
		case DIFFICULTY_MEDIUM:	m_iMeter = 5;	break;
		case DIFFICULTY_HARD:	m_iMeter = 8;	break;
		default:	ASSERT(0);
		}
	}
}

Difficulty Notes::DifficultyFromDescriptionAndMeter( CString sDescription, int iMeter )
{
	sDescription.MakeLower();

	const int DESCRIPTIONS_PER_CLASS = 4;
	const CString sDescriptionParts[NUM_DIFFICULTIES][DESCRIPTIONS_PER_CLASS] = {
		{
			"easy",
			"basic",
            "light",
			"GARBAGE",	// don't worry - this will never match because the compare string is all lowercase
		},
		{
			"medium",
			"another",
            "trick",
            "standard",
		},
		{
			"hard",
			"ssr",
            "maniac",
            "heavy",
		},
	};

	for( int i=0; i<NUM_DIFFICULTIES; i++ )
		for( int j=0; j<DESCRIPTIONS_PER_CLASS; j++ )
			if( sDescription.Find(sDescriptionParts[i][j]) != -1 )
				return (Difficulty)i;
	
	// guess difficulty class from meter
	if(		 iMeter <= 3 )	return DIFFICULTY_EASY;
	else if( iMeter <= 6 )	return DIFFICULTY_MEDIUM;
	else					return DIFFICULTY_HARD;
}


//////////////////////////////////////////////
//
//////////////////////////////////////////////

bool CompareNotesPointersByRadarValues(const Notes* pNotes1, const Notes* pNotes2)
{
	float fScore1 = 0;
	float fScore2 = 0;
	
	for( int r=0; r<NUM_RADAR_VALUES; r++ )
	{
		fScore1 += pNotes1->m_fRadarValues[r];
		fScore2 += pNotes2->m_fRadarValues[r];
	}

	return fScore1 < fScore2;
}

bool CompareNotesPointersByMeter(const Notes *pNotes1, const Notes* pNotes2)
{
	if( pNotes1->m_iMeter < pNotes2->m_iMeter )
		return true;
	if( pNotes1->m_iMeter > pNotes2->m_iMeter )
		return false;
	return CompareNotesPointersByRadarValues( pNotes1, pNotes2 );
}

bool CompareNotesPointersByDifficulty(const Notes *pNotes1, const Notes *pNotes2)
{
	if( pNotes1->m_Difficulty < pNotes2->m_Difficulty )
		return true;
	if( pNotes1->m_Difficulty > pNotes2->m_Difficulty )
		return false;
	return CompareNotesPointersByMeter( pNotes1, pNotes2 );
}

void SortNotesArrayByDifficulty( CArray<Notes*,Notes*> &arraySteps )
{
	sort( arraySteps.begin(), arraySteps.end(), CompareNotesPointersByDifficulty );
}

