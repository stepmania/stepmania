#include "global.h"
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
#include "song.h"
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



Notes::Notes()
{
	/* FIXME: should we init this to NOTES_TYPE_INVALID? 
	 * I have a feeling that it's the right thing to do but that
	 * it'd trip obscure asserts all over the place, so I'll wait
	 * until after b6 to do this. -glenn */
	m_NotesType = NOTES_TYPE_DANCE_SINGLE;
	m_Difficulty = DIFFICULTY_INVALID;
	m_iMeter = 0;
	for(int i = 0; i < NUM_RADAR_CATEGORIES; ++i)
		m_fRadarValues[i] = -1; /* unknown */

	notes = NULL;
	notes_comp = NULL;
	parent = NULL;

	for( int m=0; m<NUM_MEMORY_CARDS; m++ )
	{
		m_MemCardScores[m].iNumTimesPlayed = 0;
		m_MemCardScores[m].grade = GRADE_NO_DATA;
		m_MemCardScores[m].iScore = 0;
	}
}

Notes::~Notes()
{
	delete notes;
	delete notes_comp;
}

void Notes::SetNoteData( NoteData* pNewNoteData )
{
	ASSERT( pNewNoteData->GetNumTracks() == GameManager::NotesTypeToNumTracks(m_NotesType) );

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
	pNoteDataOut->SetNumTracks( notes->GetNumTracks() );
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

void Notes::TidyUpData()
{
	if( GetDifficulty() == DIFFICULTY_INVALID )
		SetDifficulty(StringToDifficulty(GetDescription()));
	
	if( GetDifficulty() == DIFFICULTY_INVALID )
	{
		if(		 GetMeter() == 1 )	SetDifficulty(DIFFICULTY_BEGINNER);
		else if( GetMeter() <= 3 )	SetDifficulty(DIFFICULTY_EASY);
		else if( GetMeter() <= 6 )	SetDifficulty(DIFFICULTY_MEDIUM);
		else						SetDifficulty(DIFFICULTY_HARD);
	}
	// Meter is overflowing (invalid), but some files (especially maniac/smaniac steps) are purposefully set higher than 10.
	// See: BMR's Gravity; we probably should keep those as difficult as we can represent.
	/* Why? If the data file says a meter of 72, we should keep it as 72; if
	 * individual bits of code (eg. scoring, feet) have maximums, they should
	 * enforce it internally.  Doing it here will make us lose the difficulty
	 * completely if the song is edited and written. -glenn */
/*	if( GetMeter() >10 ) {
			if( GetDifficulty() == DIFFICULTY_HARD || GetDifficulty() == DIFFICULTY_CHALLENGE)
				SetMeter(10);
			else
				SetMeter(0);
	} */
	if( GetMeter() < 1) // meter is invalid
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

void Notes::Decompress() const
{
	if(notes)
	{
		return;	// already decompressed
	}
	else if(parent)
	{
		// get autogen notes
		NoteData pdata;
		parent->GetNoteData(&pdata);

		notes = new NoteData;
		notes->SetNumTracks( GameManager::NotesTypeToNumTracks(m_NotesType) );
		if(pdata.GetNumTracks() == notes->GetNumTracks())
			notes->CopyRange( &pdata, 0, pdata.GetLastRow(), 0 );
		else
			notes->LoadTransformedSlidingWindow( &pdata, notes->GetNumTracks() );

		return;
	}
	else if(!notes_comp)
	{
		return; /* there is no data */
	}
	else
	{
		// load from compressed
		notes = new NoteData;
		notes->SetNumTracks( GameManager::NotesTypeToNumTracks(m_NotesType) );

		NoteDataUtil::LoadFromSMNoteDataString(*notes, *notes_comp);
	}
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

	Decompress();	// fills in notes with sliding window transform

	m_iMeter		= Real()->m_iMeter;
	m_sDescription	= Real()->m_sDescription;
	m_Difficulty	= Real()->m_Difficulty;
	for(int i = 0; i < NUM_RADAR_CATEGORIES; ++i)
		m_fRadarValues[i] = Real()->m_fRadarValues[i];

	parent = NULL;

	Compress();
}

void Notes::AutogenFrom( Notes *parent_, NotesType ntTo )
{
	parent = parent_;
	m_NotesType = ntTo;
}

void Notes::CopyFrom( Notes* pSource, NotesType ntTo )	// pSource does not have to be of the same NotesType!
{
	m_NotesType = ntTo;
	NoteData noteData;
	pSource->GetNoteData( &noteData );
	noteData.SetNumTracks( GameManager::NotesTypeToNumTracks(ntTo) ); 
	this->SetNoteData( &noteData );
	this->SetDescription( "Copied from "+pSource->GetDescription() );
	this->SetDifficulty( pSource->GetDifficulty() );
	this->SetMeter( pSource->GetMeter() );

	const float* radarValues = pSource->GetRadarValues();
	for( int r=0; r<NUM_RADAR_CATEGORIES; r++ )
		this->SetRadarValue( (RadarCategory)r, radarValues[r] );
}

void Notes::CreateBlank( NotesType ntTo )
{
	m_NotesType = ntTo;
	NoteData noteData;
	noteData.SetNumTracks( GameManager::NotesTypeToNumTracks(ntTo) );
	this->SetNoteData( &noteData );
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
	ASSERT(r < NUM_RADAR_CATEGORIES);
	m_fRadarValues[r] = val;
}

void Notes::AddScore( PlayerNumber pn, Grade grade, float iScore, bool& bNewRecordOut )
{
	bNewRecordOut = false;

	m_MemCardScores[MEMORY_CARD_MACHINE].iNumTimesPlayed++;
	m_MemCardScores[pn].iNumTimesPlayed++;

	if( iScore > m_MemCardScores[pn].iScore )
	{
		m_MemCardScores[pn].iScore = iScore;
		m_MemCardScores[pn].grade = grade;
		bNewRecordOut = true;
	}

	if( iScore > m_MemCardScores[MEMORY_CARD_MACHINE].iScore )
	{
		m_MemCardScores[MEMORY_CARD_MACHINE].iScore = iScore;
		m_MemCardScores[MEMORY_CARD_MACHINE].grade = grade;
	}
}


//
// Sorting stuff
//

bool CompareNotesPointersByRadarValues(const Notes* pNotes1, const Notes* pNotes2)
{
	float fScore1 = 0;
	float fScore2 = 0;
	
	for( int r=0; r<NUM_RADAR_CATEGORIES; r++ )
	{
		fScore1 += pNotes1->GetRadarValues()[r];
		fScore2 += pNotes2->GetRadarValues()[r];
	}

	return fScore1 < fScore2;
}

bool CompareNotesPointersByMeter(const Notes *pNotes1, const Notes* pNotes2)
{
	return pNotes1->GetMeter() < pNotes2->GetMeter();
}

bool CompareNotesPointersByDifficulty(const Notes *pNotes1, const Notes *pNotes2)
{
	return pNotes1->GetDifficulty() < pNotes2->GetDifficulty();
}

void SortNotesArrayByDifficulty( vector<Notes*> &arraySteps )
{
	/* Sort in reverse order of priority. */
	stable_sort( arraySteps.begin(), arraySteps.end(), CompareNotesPointersByRadarValues );
	stable_sort( arraySteps.begin(), arraySteps.end(), CompareNotesPointersByMeter );
	stable_sort( arraySteps.begin(), arraySteps.end(), CompareNotesPointersByDifficulty );
}
