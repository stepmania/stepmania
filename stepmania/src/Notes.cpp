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




Notes::Notes()
{
	/* FIXME: should we init this to NOTES_TYPE_INVALID? 
	 * I have a feeling that it's the right thing to do but that
	 * it'd trip obscure asserts all over the place, so I'll wait
	 * until after b6 to do this. -glenn */
	m_NotesType = NOTES_TYPE_DANCE_SINGLE;
	m_DifficultyClass = CLASS_INVALID;
	m_iMeter = 0;
	ZeroMemory(m_fRadarValues, sizeof(m_fRadarValues));

	m_iNumTimesPlayed = 0;
	m_iMaxCombo = 0;
	m_iTopScore = 0;
	m_TopGrade = GRADE_NO_DATA;
}

Notes::~Notes()
{
}

void Notes::WriteSMNotesTag( FILE* fp )
{
	fprintf( fp, "\n//---------------%s - %s----------------\n",
		GameManager::NotesTypeToString(m_NotesType), m_sDescription );
	fprintf( fp, "#NOTES:\n" );
	fprintf( fp, "     %s:\n", GameManager::NotesTypeToString(m_NotesType) );
	fprintf( fp, "     %s:\n", m_sDescription );
	fprintf( fp, "     %s:\n", DifficultyClassToString(m_DifficultyClass) );
	fprintf( fp, "     %d:\n", m_iMeter );
	
	CStringArray asRadarValues;
	for( int r=0; r<NUM_RADAR_VALUES; r++ )
		asRadarValues.Add( ssprintf("%.2f", m_fRadarValues[r]) );
	fprintf( fp, "     %s:\n", join(",",asRadarValues) );

	fprintf( fp, "%s;\n", m_sSMNoteData );
}

struct DWICharLookup {
	char c;
	bool bCol[6];	
};

char NotesToDWIChar( bool bCol1, bool bCol2, bool bCol3, bool bCol4, bool bCol5, bool bCol6 )
{
	const DWICharLookup lookup[] = {
		{ '0', 0, 0, 0, 0, 0, 0 },
		{ '1', 0, 1, 1, 0, 0, 0 },
		{ '2', 0, 0, 1, 0, 0, 0 },
		{ '3', 0, 0, 1, 0, 1, 0 },
		{ '4', 0, 1, 0, 0, 0, 0 },
		{ '6', 0, 0, 0, 0, 1, 0 },
		{ '7', 0, 1, 0, 1, 0, 0 },
		{ '8', 0, 0, 0, 1, 0, 0 },
		{ '9', 0, 0, 0, 1, 1, 0 },
		{ 'A', 0, 0, 1, 1, 0, 0 },
		{ 'B', 0, 1, 0, 0, 1, 0 },
		{ 'C', 0, 1, 0, 0, 0, 0 },
		{ 'D', 0, 0, 0, 0, 1, 0 },
		{ 'E', 1, 1, 0, 0, 0, 0 },
		{ 'F', 0, 1, 1, 0, 0, 0 },
		{ 'G', 0, 1, 0, 1, 0, 0 },
		{ 'H', 0, 1, 0, 0, 0, 1 },
		{ 'I', 1, 0, 0, 0, 1, 0 },
		{ 'J', 0, 0, 1, 0, 1, 0 },
		{ 'K', 0, 0, 0, 1, 1, 0 },
		{ 'L', 0, 0, 0, 0, 1, 1 },
		{ 'M', 0, 1, 0, 0, 1, 0 },
	};
	const int iNumLookups = sizeof(lookup) / sizeof(DWICharLookup);

	for( int i=0; i<iNumLookups; i++ )
	{
		const DWICharLookup& l = lookup[i];
		if( l.bCol[0]==bCol1 && l.bCol[1]==bCol2 && l.bCol[2]==bCol3 && l.bCol[3]==bCol4 && l.bCol[4]==bCol5 && l.bCol[5]==bCol6 )
			return l.c;
	}
	ASSERT(0);
	return '0';
}

char NotesToDWIChar( bool bCol1, bool bCol2, bool bCol3, bool bCol4 )
{
	return NotesToDWIChar( 0, bCol1, bCol2, bCol3, bCol4, 0 );
}

CString NotesToDWIString( char cNoteCol1, char cNoteCol2, char cNoteCol3, char cNoteCol4, char cNoteCol5, char cNoteCol6 )
{
	char cShow = NotesToDWIChar( cNoteCol1!='0', cNoteCol2!='0', cNoteCol3!='0', cNoteCol4!='0', cNoteCol5!='0', cNoteCol6!='0' );
	char cHold = NotesToDWIChar( cNoteCol1=='2', cNoteCol2=='2', cNoteCol3=='2', cNoteCol4=='2', cNoteCol5=='2', cNoteCol6=='2' );
	
	if( cHold != '0' )
		return ssprintf( "%c!%c", cShow, cHold );
	else
		return cShow;
}

CString NotesToDWIString( char cNoteCol1, char cNoteCol2, char cNoteCol3, char cNoteCol4 )
{
	return NotesToDWIString( '0', cNoteCol1, cNoteCol2, cNoteCol3, cNoteCol4, '0' );
}

void Notes::WriteDWINotesTag( FILE* fp )
{
	LOG->Trace( "Notes::WriteDWINotesTag" );

	switch( m_NotesType )
	{
	case NOTES_TYPE_DANCE_SINGLE:	fprintf( fp, "#SINGLE:" );	break;
	case NOTES_TYPE_DANCE_COUPLE:	fprintf( fp, "#COUPLE:" );	break;
	case NOTES_TYPE_DANCE_DOUBLE:	fprintf( fp, "#DOUBLE:" );	break;
	case NOTES_TYPE_DANCE_SOLO:		fprintf( fp, "#SOLO:" );	break;
	default:	return;	// not a type supported by DWI
	}

	switch( m_DifficultyClass )
	{
	case CLASS_EASY:	fprintf( fp, "BASIC:" );	break;
	case CLASS_MEDIUM:	fprintf( fp, "ANOTHER:" );	break;
	case CLASS_HARD:	fprintf( fp, "MANIAC:" );	break;
	default:	ASSERT(0);	return;
	}

	fprintf( fp, "%d:\n", m_iMeter );

	NoteData notedata;
	this->GetNoteData( &notedata );
	notedata.ConvertHoldNotesTo2sAnd3s();

	const int iNumPads = (m_NotesType==NOTES_TYPE_DANCE_COUPLE || m_NotesType==NOTES_TYPE_DANCE_DOUBLE) ? 2 : 1;
	const int iLastMeasure = int( notedata.GetLastBeat()/BEATS_PER_MEASURE );

	for( int pad=0; pad<iNumPads; pad++ )
	{
		if( pad == 1 )	// 2nd pad
			fprintf( fp, ":\n" );

		for( int m=0; m<=iLastMeasure; m++ )	// foreach measure
		{
			NoteType nt = notedata.GetSmallestNoteTypeForMeasure( m );

			double fCurrentIncrementer;
			switch( nt )
			{
			case NOTE_TYPE_4TH:
			case NOTE_TYPE_8TH:	
				fCurrentIncrementer = 1.0/8 * BEATS_PER_MEASURE;
				break;
			case NOTE_TYPE_12TH:
				fprintf( fp, "[" );
				fCurrentIncrementer = 1.0/24 * BEATS_PER_MEASURE;
				break;
			case NOTE_TYPE_16TH:
				fprintf( fp, "(" );
				fCurrentIncrementer = 1.0/16 * BEATS_PER_MEASURE;
				break;
			default:
				ASSERT(0);
				// fall though
			case NOTE_TYPE_INVALID:
				fprintf( fp, "<" );
				fCurrentIncrementer = 1.0/192 * BEATS_PER_MEASURE;
				break;
			}

			double fFirstBeatInMeasure = m * BEATS_PER_MEASURE;
			double fLastBeatInMeasure = (m+1) * BEATS_PER_MEASURE;

			for( double b=fFirstBeatInMeasure; b<fLastBeatInMeasure; b+=fCurrentIncrementer )
			{
				int row = BeatToNoteRow( (float)b );

				switch( m_NotesType )
				{
				case NOTES_TYPE_DANCE_SINGLE:
				case NOTES_TYPE_DANCE_COUPLE:
				case NOTES_TYPE_DANCE_DOUBLE:
					fprintf( fp, NotesToDWIString( notedata.m_TapNotes[pad*4+0][row], notedata.m_TapNotes[pad*4+1][row], notedata.m_TapNotes[pad*4+2][row], notedata.m_TapNotes[pad*4+3][row] ) );
					break;
				case NOTES_TYPE_DANCE_SOLO:
					fprintf( fp, NotesToDWIString( notedata.m_TapNotes[0][row], notedata.m_TapNotes[1][row], notedata.m_TapNotes[2][row], notedata.m_TapNotes[3][row], notedata.m_TapNotes[4][row], notedata.m_TapNotes[5][row] ) );
					break;
				default:	return;	// not a type supported by DWI
				}
			}

			switch( nt )
			{
			case NOTE_TYPE_4TH:
			case NOTE_TYPE_8TH:	
				break;
			case NOTE_TYPE_12TH:
				fprintf( fp, "]" );
				break;
			case NOTE_TYPE_16TH:
				fprintf( fp, ")" );
				break;
			default:
				ASSERT(0);
				// fall though
			case NOTE_TYPE_INVALID:
				fprintf( fp, ">" );
				break;
			}
			fprintf( fp, "\n" );
		}
	}

	fprintf( fp, ";\n" );
}

void Notes::SetNoteData( NoteData* pNewNoteData )
{
	ASSERT( pNewNoteData->m_iNumTracks == GameManager::NotesTypeToNumTracks(m_NotesType) );
	m_sSMNoteData = pNewNoteData->GetSMNoteDataString();
}

void Notes::GetNoteData( NoteData* pNoteDataOut )
{
	pNoteDataOut->m_iNumTracks = GameManager::NotesTypeToNumTracks( m_NotesType );
	pNoteDataOut->LoadFromSMNoteDataString( m_sSMNoteData );
}

void Notes::TidyUpData()
{
	if( m_DifficultyClass == CLASS_INVALID )
		m_DifficultyClass = DifficultyClassFromDescriptionAndMeter( m_sDescription, m_iMeter );

	if( m_iMeter < 1 || m_iMeter > 10 ) 
	{
		switch( m_DifficultyClass )
		{
		case CLASS_EASY:	m_iMeter = 3;	break;
		case CLASS_MEDIUM:	m_iMeter = 5;	break;
		case CLASS_HARD:	m_iMeter = 8;	break;
		default:	ASSERT(0);
		}
	}
}

DifficultyClass Notes::DifficultyClassFromDescriptionAndMeter( CString sDescription, int iMeter )
{
	sDescription.MakeLower();

	const int DESCRIPTIONS_PER_CLASS = 4;
	const CString sDescriptionParts[NUM_DIFFICULTY_CLASSES][DESCRIPTIONS_PER_CLASS] = {
		{
			"easy",
			"basic",
            "light",
			"GARBAGE",	// but don't worry - this will never match because the compare string is all lowercase
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

	for( int i=0; i<NUM_DIFFICULTY_CLASSES; i++ )
		for( int j=0; j<DESCRIPTIONS_PER_CLASS; j++ )
			if( sDescription.Find(sDescriptionParts[i][j]) != -1 )
				return DifficultyClass(i);
	
	// guess difficulty class from meter
	if(		 iMeter <= 3 )	return CLASS_EASY;
	else if( iMeter <= 6 )	return CLASS_MEDIUM;
	else					return CLASS_HARD;
}


//////////////////////////////////////////////
//
//////////////////////////////////////////////

int CompareNotesPointersByRadarValues(const void *arg1, const void *arg2)
{
	Notes* pNotes1 = *(Notes**)arg1;
	Notes* pNotes2 = *(Notes**)arg2;

	float fScore1 = 0;
	float fScore2 = 0;
	
	for( int r=0; r<NUM_RADAR_VALUES; r++ )
	{
		fScore1 += pNotes1->m_fRadarValues[r];
		fScore2 += pNotes2->m_fRadarValues[r];
	}

	if( fScore1 < fScore2 )
		return -1;
	else if( fScore1 == fScore2 )
		return 0;
	else
		return 1;
}

int CompareNotesPointersByMeter(const void *arg1, const void *arg2)
{
	Notes* pNotes1 = *(Notes**)arg1;
	Notes* pNotes2 = *(Notes**)arg2;

	int iScore1 = pNotes1->m_iMeter;
	int iScore2 = pNotes2->m_iMeter;

	if( iScore1 < iScore2 )
		return -1;
	else if( iScore1 == iScore2 )
		return CompareNotesPointersByRadarValues( arg1, arg2 );
	else
		return 1;
}

int CompareNotesPointersByDifficulty(Notes* pNotes1, Notes* pNotes2)
{
	DifficultyClass class1 = pNotes1->m_DifficultyClass;
	DifficultyClass class2 = pNotes2->m_DifficultyClass;

	if( class1 < class2 )
		return -1;
	else if( class1 == class2 )
		return CompareNotesPointersByMeter( &pNotes1, &pNotes2 );
	else
		return 1;
}

int CompareNotesPointersByDifficulty2(const void *arg1, const void *arg2)
{
	Notes* pNotes1 = *(Notes**)arg1;
	Notes* pNotes2 = *(Notes**)arg2;

	return CompareNotesPointersByDifficulty( pNotes1, pNotes2 );
}

void SortNotesArrayByDifficulty( CArray<Notes*,Notes*> &arraySteps )
{
	qsort( arraySteps.GetData(), arraySteps.GetSize(), sizeof(Notes*), CompareNotesPointersByDifficulty2 );
}

