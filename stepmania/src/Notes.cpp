#include "stdafx.h"
/*
-----------------------------------------------------------------------------
 Class: Notes

 Desc: See header.

 Copyright (c) 2001-2002 by the person(s) listed below.  All rights reserved.
	Chris Danford
	Glenn Maynard
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


#define COLOR_BEGINNER	THEME->GetMetricC("Notes","ColorBeginner")
#define COLOR_EASY		THEME->GetMetricC("Notes","ColorEasy")
#define COLOR_MEDIUM	THEME->GetMetricC("Notes","ColorMedium")
#define COLOR_HARD		THEME->GetMetricC("Notes","ColorHard")
#define COLOR_CHALLENGE	THEME->GetMetricC("Notes","ColorChallenge")

Notes::Notes()
{
	/* FIXME: should we init this to NOTES_TYPE_INVALID? 
	 * I have a feeling that it's the right thing to do but that
	 * it'd trip obscure asserts all over the place, so I'll wait
	 * until after b6 to do this. -glenn */
	m_NotesType = NOTES_TYPE_DANCE_SINGLE;
	m_Difficulty = DIFFICULTY_INVALID;
	m_iMeter = 0;
	for(int i = 0; i < NUM_RADAR_VALUES; ++i)
		m_fRadarValues[i] = -1; /* unknown */

	m_iNumTimesPlayed = 0;
	notes = NULL;
	notes_comp = NULL;
	parent = NULL;

	for( int p=0; p<NUM_PLAYERS; p++ )
	{
		m_MemCardScores[p].grade = GRADE_NO_DATA;
		m_MemCardScores[p].iScore = 0;
	}
}

Notes::~Notes()
{
	delete notes;
	delete notes_comp;
}

void Notes::SetNoteData( NoteData* pNewNoteData )
{
	ASSERT( pNewNoteData->m_iNumTracks == GameManager::NotesTypeToNumTracks(m_NotesType) );

	DeAutogen();

	delete notes_comp;
	notes_comp = NULL;

	delete notes;
	notes = new NoteData(*pNewNoteData);
}

void Notes::GetNoteData( NoteData* pNoteDataOut ) const
{
	ASSERT(this);
	ASSERT(pNoteDataOut);
	Decompress();
	pNoteDataOut->m_iNumTracks = notes->m_iNumTracks;
	*pNoteDataOut = *notes;
}

void Notes::SetSMNoteData( const CString &out )
{
	delete notes;
	notes = NULL;

	if(!notes_comp)
		notes_comp = new CString;

	*notes_comp = out;
}

CString Notes::GetSMNoteData() const
{
	if(!notes_comp)
	{
		if(!notes) return ""; /* no data is no data */
		notes_comp = new CString(NoteDataUtil::GetSMNoteDataString(*notes));
	}

	return *notes_comp;
}

RageColor Notes::GetColor() const
{
	switch( GetDifficulty() )
	{
	case DIFFICULTY_BEGINNER:	return COLOR_BEGINNER;
	case DIFFICULTY_EASY:		return COLOR_EASY;
	case DIFFICULTY_MEDIUM:		return COLOR_MEDIUM;
	case DIFFICULTY_HARD:		return COLOR_HARD;
	case DIFFICULTY_CHALLENGE:	return COLOR_CHALLENGE;
	default:	ASSERT(0);	return COLOR_BEGINNER;
	}
}

void Notes::TidyUpData()
{
	if( GetDifficulty() == DIFFICULTY_INVALID )
	{
		CString sDescription = GetDescription();
		sDescription.MakeLower();
		if( sDescription == "beginner" )		SetDifficulty(DIFFICULTY_BEGINNER);
		else if( sDescription == "easy" )		SetDifficulty(DIFFICULTY_EASY);
		else if( sDescription == "basic" )		SetDifficulty(DIFFICULTY_EASY);
		else if( sDescription == "light" )		SetDifficulty(DIFFICULTY_EASY);
		else if( sDescription == "medium" )		SetDifficulty(DIFFICULTY_MEDIUM);
		else if( sDescription == "another" )	SetDifficulty(DIFFICULTY_MEDIUM);
		else if( sDescription == "trick" )		SetDifficulty(DIFFICULTY_MEDIUM);
		else if( sDescription == "standard" )	SetDifficulty(DIFFICULTY_MEDIUM);
		else if( sDescription == "hard" )		SetDifficulty(DIFFICULTY_HARD);
		else if( sDescription == "ssr" )		SetDifficulty(DIFFICULTY_HARD);
		else if( sDescription == "maniac" )		SetDifficulty(DIFFICULTY_HARD);
		else if( sDescription == "heavy" )		SetDifficulty(DIFFICULTY_HARD);
		else if( sDescription == "smaniac" )	SetDifficulty(DIFFICULTY_CHALLENGE);
		else if( sDescription == "challenge" )	SetDifficulty(DIFFICULTY_CHALLENGE);
	}
	
	if( GetDifficulty() == DIFFICULTY_INVALID )
	{
		if(		 GetMeter() == 1 )	SetDifficulty(DIFFICULTY_BEGINNER);
		else if( GetMeter() <= 3 )	SetDifficulty(DIFFICULTY_EASY);
		else if( GetMeter() <= 6 )	SetDifficulty(DIFFICULTY_MEDIUM);
		else						SetDifficulty(DIFFICULTY_HARD);
	}


	if( GetMeter() < 1 || GetMeter() > 10 ) // meter is invalid
	{
		// guess meter from difficulty class
		switch( GetDifficulty() )
		{
		case DIFFICULTY_BEGINNER:	SetMeter(1);	break;
		case DIFFICULTY_EASY:		SetMeter(3);	break;
		case DIFFICULTY_MEDIUM:		SetMeter(5);	break;
		case DIFFICULTY_HARD:		SetMeter(8);	break;
		case DIFFICULTY_CHALLENGE:	SetMeter(8);	break;
		case DIFFICULTY_INVALID:	SetMeter(5);	break;
		default:	ASSERT(0);
		}
	}
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
		fScore1 += pNotes1->GetRadarValues()[r];
		fScore2 += pNotes2->GetRadarValues()[r];
	}

	return fScore1 < fScore2;
}

bool CompareNotesPointersByMeter(const Notes *pNotes1, const Notes* pNotes2)
{
	if( pNotes1->GetMeter() < pNotes2->GetMeter() )
		return true;
	if( pNotes1->GetMeter() > pNotes2->GetMeter() )
		return false;
	return CompareNotesPointersByRadarValues( pNotes1, pNotes2 );
}

bool CompareNotesPointersByDifficulty(const Notes *pNotes1, const Notes *pNotes2)
{
	if( pNotes1->GetDifficulty() < pNotes2->GetDifficulty() )
		return true;
	if( pNotes1->GetDifficulty() > pNotes2->GetDifficulty() )
		return false;
	return CompareNotesPointersByMeter( pNotes1, pNotes2 );
}

void SortNotesArrayByDifficulty( vector<Notes*> &arraySteps )
{
	sort( arraySteps.begin(), arraySteps.end(), CompareNotesPointersByDifficulty );
}

void Notes::Decompress() const
{
	if(notes) return;

	if(parent)
	{
		NoteData pdata;
		parent->GetNoteData(&pdata);

		notes = new NoteData;
		notes->m_iNumTracks = GameManager::NotesTypeToNumTracks(m_NotesType);
		if(pdata.m_iNumTracks == notes->m_iNumTracks)
			notes->CopyRange( &pdata, 0, pdata.GetLastRow(), 0 );
		else
			notes->LoadTransformedSlidingWindow( &pdata, notes->m_iNumTracks );

		return;
	}

	if(!notes_comp) return; /* no data is no data */

	notes = new NoteData;
	notes->m_iNumTracks = GameManager::NotesTypeToNumTracks(m_NotesType);

	NoteDataUtil::LoadFromSMNoteDataString(*notes, *notes_comp);
}

void Notes::Compress() const
{
	if(!notes_comp)
	{
		if(!notes) return; /* no data is no data */
		notes_comp = new CString(NoteDataUtil::GetSMNoteDataString(*notes));
	}

	delete notes;
	notes = NULL;
}

/* Copy our parent's data.  This is done when we're being changed from autogen
 * to normal. (needed?) */
void Notes::DeAutogen()
{
	if(!parent)
		return; /* OK */

	m_iMeter		= Real()->m_iMeter;
	m_sDescription	= Real()->m_sDescription;
	m_Difficulty	= Real()->m_Difficulty;
	for(int i = 0; i < NUM_RADAR_VALUES; ++i)
		m_fRadarValues[i] = Real()->m_fRadarValues[i];
	
	delete notes;
	notes = NULL;

	if(!notes_comp)
		notes_comp = new CString;

	*notes_comp = parent->GetSMNoteData();

	parent = NULL;
}

void Notes::AutogenFrom( Notes *parent_, NotesType ntTo )
{
	parent = parent_;
	m_NotesType = ntTo;
}

const Notes *Notes::Real() const
{
	if(parent) return parent;
	return this;
}

bool Notes::IsAutogen() const
{
	return parent != NULL;
}

void Notes::SetDescription(CString desc)
{
	DeAutogen();
	m_sDescription = desc;
}

void Notes::SetDifficulty(Difficulty d)
{
	DeAutogen();
	m_Difficulty = d;
}

void Notes::SetMeter(int meter)
{
	DeAutogen();
	m_iMeter = meter;
}

void Notes::SetRadarValue(int r, float val)
{
	DeAutogen();
	ASSERT(r < NUM_RADAR_VALUES);
	m_fRadarValues[r] = val;
}

bool Notes::AddMemCardScore( PlayerNumber pn, Grade grade, int iScore )	// return true if new high score
{
	if( iScore > m_MemCardScores[pn].iScore )
	{
		m_MemCardScores[pn].iScore = iScore;
		m_MemCardScores[pn].grade = grade;
		return true;
	}
	return false;
}
