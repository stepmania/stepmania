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


typedef int DanceNote;
enum {
	DANCE_NOTE_NONE = 0,
	DANCE_NOTE_PAD1_LEFT,
	DANCE_NOTE_PAD1_UPLEFT,
	DANCE_NOTE_PAD1_DOWN,
	DANCE_NOTE_PAD1_UP,
	DANCE_NOTE_PAD1_UPRIGHT,
	DANCE_NOTE_PAD1_RIGHT,
	DANCE_NOTE_PAD2_LEFT,
	DANCE_NOTE_PAD2_UPLEFT,
	DANCE_NOTE_PAD2_DOWN,
	DANCE_NOTE_PAD2_UP,
	DANCE_NOTE_PAD2_UPRIGHT,
	DANCE_NOTE_PAD2_RIGHT
};



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



// BMS encoding:     tap-hold
// 4&8panel:   Player1     Player2
//    Left		11-51		21-61
//    Down		13-53		23-63
//    Up		15-55		25-65
//    Right		16-56		26-66
//	6panel:	   Player1
//    Left		11-51
//    Left+Up	12-52
//    Down		13-53
//    Up		14-54
//    Up+Right	15-55
//    Right		16-56
//
//	Notice that 15 and 25 have double meanings!  What were they thinking???
//	While reading in, use the 6 panel mapping.  After reading in, detect if only 4 notes
//	are used.  If so, shift the Up+Right column back to the Up column
//
void mapBMSTrackToDanceNote( int iBMSTrack, int &iDanceColOut, char &cNoteCharOut )
{
	if( iBMSTrack > 40 )
	{
		cNoteCharOut = '2';
		iBMSTrack -= 40;
	}
	else
	{
		cNoteCharOut = '1';
	}

	switch( iBMSTrack )
	{
	case 11:	iDanceColOut = DANCE_NOTE_PAD1_LEFT;	break;
	case 12:	iDanceColOut = DANCE_NOTE_PAD1_UPLEFT;	break;
	case 13:	iDanceColOut = DANCE_NOTE_PAD1_DOWN;	break;
	case 14:	iDanceColOut = DANCE_NOTE_PAD1_UP;		break;
	case 15:	iDanceColOut = DANCE_NOTE_PAD1_UPRIGHT;	break;
	case 16:	iDanceColOut = DANCE_NOTE_PAD1_RIGHT;	break;
	case 21:	iDanceColOut = DANCE_NOTE_PAD2_LEFT;	break;
	case 22:	iDanceColOut = DANCE_NOTE_PAD2_UPLEFT;	break;
	case 23:	iDanceColOut = DANCE_NOTE_PAD2_DOWN;	break;
	case 24:	iDanceColOut = DANCE_NOTE_PAD2_UP;		break;
	case 25:	iDanceColOut = DANCE_NOTE_PAD2_UPRIGHT;	break;
	case 26:	iDanceColOut = DANCE_NOTE_PAD2_RIGHT;	break;
	default:	iDanceColOut = -1;						break;
	}
}


bool Notes::LoadFromBMSFile( const CString &sPath )
{
	LOG->Trace( "Notes::LoadFromBMSFile( '%s' )", sPath );

	NoteData* pNoteData = new NoteData;
	pNoteData->m_iNumTracks = MAX_NOTE_TRACKS;

	CStdioFile file;	
	if( !file.Open( sPath, CFile::modeRead|CFile::shareDenyNone ) )
	{
		throw RageException( "Failed to open %s.", sPath );
		return false;
	}

	CString line;
	while( file.ReadString(line) )	// foreach line
	{
		CString value_name;		// fill these in
		CString value_data;	

		// BMS value names can be separated by a space or a colon.
		int iIndexOfFirstColon = line.Find( ":" );
		int iIndexOfFirstSpace = line.Find( " " );

		if( iIndexOfFirstColon == -1 )
			iIndexOfFirstColon = 10000;
		if( iIndexOfFirstSpace == -1 )
			iIndexOfFirstSpace = 10000;
		
		int iIndexOfSeparator = min( iIndexOfFirstSpace, iIndexOfFirstColon );

		if( iIndexOfSeparator != 10000 )
		{
			value_name = line.Mid( 0, iIndexOfSeparator );
			value_data = line;	// the rest
			value_data.Delete(0,iIndexOfSeparator+1);
		}
		else	// no separator
		{
			value_name = line;
		}

		value_name.MakeLower();

		if( -1 != value_name.Find("#player") ) 
		{
			switch( atoi(value_data) )
			{
			case 1:		// 4 or 6 single
				m_NotesType = NOTES_TYPE_DANCE_SINGLE;
				// if the mode should be solo, then we'll update m_DanceStyle below when we read in step data
				break;
			case 2:		// couple/battle
				m_NotesType = NOTES_TYPE_DANCE_COUPLE;
				break;
			case 3:		// double
				m_NotesType = NOTES_TYPE_DANCE_DOUBLE;
				break;
			}
		}
		if( -1 != value_name.Find("#title") )
		{
			m_sDescription = value_data;
			
			// extract the Notes description (looks like 'Music <BASIC>')
			int iPosOpenBracket = m_sDescription.Find( "<" );
			if( iPosOpenBracket == -1 )
				iPosOpenBracket = m_sDescription.Find( "(" );
			int iPosCloseBracket = m_sDescription.Find( ">" );
			if( iPosCloseBracket == -1 )
				iPosCloseBracket = m_sDescription.Find( ")" );

			if( iPosOpenBracket != -1  &&  iPosCloseBracket != -1 )
				m_sDescription = m_sDescription.Mid( iPosOpenBracket+1, iPosCloseBracket-iPosOpenBracket-1 );
			m_sDescription.MakeLower();
			LOG->Trace( "Notes description found to be '%s'", m_sDescription );

			// if there's a 6 in the description, it's probably part of "6panel" or "6-panel"
			if( m_sDescription.Find("6") != -1 )
				m_NotesType = NOTES_TYPE_DANCE_SOLO;
			
		}
		if( -1 != value_name.Find("#playlevel") ) 
		{
			m_iMeter = atoi( value_data );
		}
		else if( value_name.Left(1) == "#"  
			 && IsAnInt( value_name.Mid(1,3) )
			 && IsAnInt( value_name.Mid(4,2) ) )	// this is step or offset data.  Looks like "#00705"
		{
			int iMeasureNo	= atoi( value_name.Mid(1,3) );
			int iTrackNum	= atoi( value_name.Mid(4,2) );

			CString &sNoteData = value_data;
			CArray<bool, bool&> arrayNotes;

			for( int i=0; i<sNoteData.GetLength(); i+=2 )
			{
				bool bThisIsANote = sNoteData.Mid(i,2) != "00";
				arrayNotes.Add( bThisIsANote );
			}

			const int iNumNotesInThisMeasure = arrayNotes.GetSize();
			//LOG->Trace( "%s:%s: iMeasureNo = %d, iNoteNum = %d, iNumNotesInThisMeasure = %d", 
			//	valuename, sNoteData, iMeasureNo, iNoteNum, iNumNotesInThisMeasure );
			for( int j=0; j<iNumNotesInThisMeasure; j++ )
			{
				if( arrayNotes.GetAt(j) == TRUE )
				{
					float fPercentThroughMeasure = (float)j/(float)iNumNotesInThisMeasure;

					const int iNoteIndex = (int) ( (iMeasureNo + fPercentThroughMeasure)
									 * BEATS_PER_MEASURE * ROWS_PER_BEAT );
					int iColumnNumber;
					char cNoteChar;
					mapBMSTrackToDanceNote( iTrackNum, iColumnNumber, cNoteChar );

					if( iColumnNumber != -1 )
						pNoteData->m_TapNotes[iColumnNumber][iNoteIndex] = cNoteChar;
				}
			}
		}
	}
	
	if( m_NotesType == NOTES_TYPE_DANCE_SINGLE  || 
		m_NotesType == NOTES_TYPE_DANCE_DOUBLE  || 
		m_NotesType == NOTES_TYPE_DANCE_COUPLE )	// if there are 4 panels, then the Up+Right track really contains the notes for Up
	{
		for( int i=0; i<MAX_TAP_NOTE_ROWS; i++ )	// for each TapNote
		{
			memcpy( 
				pNoteData->m_TapNotes[DANCE_NOTE_PAD1_UP], 
				pNoteData->m_TapNotes[DANCE_NOTE_PAD1_UPRIGHT],
				MAX_TAP_NOTE_ROWS*sizeof(pNoteData->m_TapNotes[0][0]) 
				);
			memcpy( 
				pNoteData->m_TapNotes[DANCE_NOTE_PAD2_UP], 
				pNoteData->m_TapNotes[DANCE_NOTE_PAD2_UPRIGHT],
				MAX_TAP_NOTE_ROWS*sizeof(pNoteData->m_TapNotes[0][0]) 
				);
		}
	}

	// we're done reading in all of the BMS values
	int iNumNewTracks = GameManager::NotesTypeToNumTracks( m_NotesType );
	int iTransformNewToOld[MAX_NOTE_TRACKS];

	switch( m_NotesType )
	{
	case NOTES_TYPE_DANCE_SINGLE:
		iTransformNewToOld[0] = DANCE_NOTE_PAD1_LEFT;
		iTransformNewToOld[1] = DANCE_NOTE_PAD1_DOWN;
		iTransformNewToOld[2] = DANCE_NOTE_PAD1_UP;
		iTransformNewToOld[3] = DANCE_NOTE_PAD1_RIGHT;
		break;
	case NOTES_TYPE_DANCE_DOUBLE:
	case NOTES_TYPE_DANCE_COUPLE:
		iTransformNewToOld[0] = DANCE_NOTE_PAD1_LEFT;
		iTransformNewToOld[1] = DANCE_NOTE_PAD1_DOWN;
		iTransformNewToOld[2] = DANCE_NOTE_PAD1_UP;
		iTransformNewToOld[3] = DANCE_NOTE_PAD1_RIGHT;
		iTransformNewToOld[4] = DANCE_NOTE_PAD2_LEFT;
		iTransformNewToOld[5] = DANCE_NOTE_PAD2_DOWN;
		iTransformNewToOld[6] = DANCE_NOTE_PAD2_UP;
		iTransformNewToOld[7] = DANCE_NOTE_PAD2_RIGHT;
		break;
	case NOTES_TYPE_DANCE_SOLO:
		iTransformNewToOld[0] = DANCE_NOTE_PAD1_LEFT;
		iTransformNewToOld[1] = DANCE_NOTE_PAD1_UPLEFT;
		iTransformNewToOld[2] = DANCE_NOTE_PAD1_DOWN;
		iTransformNewToOld[3] = DANCE_NOTE_PAD1_UP;
		iTransformNewToOld[4] = DANCE_NOTE_PAD1_UPRIGHT;
		iTransformNewToOld[5] = DANCE_NOTE_PAD1_RIGHT;
		break;
	default:
		throw RageException( "Invalid NotesType." );
	}

	NoteData* pNoteData2 = new NoteData;
	pNoteData2->m_iNumTracks = iNumNewTracks;
	pNoteData2->LoadTransformed( pNoteData, iNumNewTracks, iTransformNewToOld );

	m_sSMNoteData = pNoteData2->GetSMNoteDataString();

	delete pNoteData;
	delete pNoteData2;

	TidyUpData();

	return true;
}




void DWIcharToNote( char c, GameController i, DanceNote &note1Out, DanceNote &note2Out )
{
	switch( c )
		{
		case '0':	note1Out = DANCE_NOTE_NONE;			note2Out = DANCE_NOTE_NONE;			break;
		case '1':	note1Out = DANCE_NOTE_PAD1_DOWN;	note2Out = DANCE_NOTE_PAD1_LEFT;	break;
		case '2':	note1Out = DANCE_NOTE_PAD1_DOWN;	note2Out = DANCE_NOTE_NONE;			break;
		case '3':	note1Out = DANCE_NOTE_PAD1_DOWN;	note2Out = DANCE_NOTE_PAD1_RIGHT;	break;
		case '4':	note1Out = DANCE_NOTE_PAD1_LEFT;	note2Out = DANCE_NOTE_NONE;			break;
		case '5':	note1Out = DANCE_NOTE_NONE;			note2Out = DANCE_NOTE_NONE;			break;
		case '6':	note1Out = DANCE_NOTE_PAD1_RIGHT;	note2Out = DANCE_NOTE_NONE;			break;
		case '7':	note1Out = DANCE_NOTE_PAD1_UP;		note2Out = DANCE_NOTE_PAD1_LEFT;	break;
		case '8':	note1Out = DANCE_NOTE_PAD1_UP;		note2Out = DANCE_NOTE_NONE;			break;
		case '9':	note1Out = DANCE_NOTE_PAD1_UP;		note2Out = DANCE_NOTE_PAD1_RIGHT;	break;
		case 'A':	note1Out = DANCE_NOTE_PAD1_UP;		note2Out = DANCE_NOTE_PAD1_DOWN;	break;
		case 'B':	note1Out = DANCE_NOTE_PAD1_LEFT;	note2Out = DANCE_NOTE_PAD1_RIGHT;	break;
		case 'C':	note1Out = DANCE_NOTE_PAD1_UPLEFT;	note2Out = DANCE_NOTE_NONE;			break;
		case 'D':	note1Out = DANCE_NOTE_PAD1_UPRIGHT;	note2Out = DANCE_NOTE_NONE;			break;
		case 'E':	note1Out = DANCE_NOTE_PAD1_LEFT;	note2Out = DANCE_NOTE_PAD1_UPLEFT;	break;
		case 'F':	note1Out = DANCE_NOTE_PAD1_UPLEFT;	note2Out = DANCE_NOTE_PAD1_DOWN;	break;
		case 'G':	note1Out = DANCE_NOTE_PAD1_UPLEFT;	note2Out = DANCE_NOTE_PAD1_UP;		break;
		case 'H':	note1Out = DANCE_NOTE_PAD1_UPLEFT;	note2Out = DANCE_NOTE_PAD1_RIGHT;	break;
		case 'I':	note1Out = DANCE_NOTE_PAD1_LEFT;	note2Out = DANCE_NOTE_PAD1_UPRIGHT;	break;
		case 'J':	note1Out = DANCE_NOTE_PAD1_DOWN;	note2Out = DANCE_NOTE_PAD1_UPRIGHT;	break;
		case 'K':	note1Out = DANCE_NOTE_PAD1_UP;		note2Out = DANCE_NOTE_PAD1_UPRIGHT;	break;
		case 'L':	note1Out = DANCE_NOTE_PAD1_UPRIGHT;	note2Out = DANCE_NOTE_PAD1_RIGHT;	break;
		case 'M':	note1Out = DANCE_NOTE_PAD1_UPLEFT;	note2Out = DANCE_NOTE_PAD1_UPRIGHT;	break;
		default:	
			LOG->Warn( "Encountered invalid DWI note characer '%c'", c );
			note1Out = DANCE_NOTE_NONE;			note2Out = DANCE_NOTE_NONE;			break;
	}

	switch( i )
	{
	case GAME_CONTROLLER_1:
		break;
	case GAME_CONTROLLER_2:
		if( note1Out != DANCE_NOTE_NONE )
			note1Out += 6;
		if( note2Out != DANCE_NOTE_NONE )
			note2Out += 6;
		break;
	default:
		ASSERT( false );
	}
}



bool Notes::LoadFromDWITokens( 
	CString sMode, 
	CString sDescription,
	CString sNumFeet,
	CString sStepData1, 
	CString sStepData2 )
{
	LOG->Trace( "Notes::LoadFromDWITokens()" );

	sStepData1.Replace( "\n", "" );
	sStepData1.Replace( " ", "" );
	sStepData2.Replace( "\n", "" );
	sStepData2.Replace( " ", "" );

	if(		 sMode == "SINGLE" )	m_NotesType = NOTES_TYPE_DANCE_SINGLE;
	else if( sMode == "DOUBLE" )	m_NotesType = NOTES_TYPE_DANCE_DOUBLE;
	else if( sMode == "COUPLE" )	m_NotesType = NOTES_TYPE_DANCE_COUPLE;
	else if( sMode == "SOLO" )		m_NotesType = NOTES_TYPE_DANCE_SOLO;
	else	
	{
		ASSERT(0);	// Unrecognized DWI notes format
		m_NotesType = NOTES_TYPE_DANCE_SINGLE;
	}


	CMap<int, int, int, int>  mapDanceNoteToNoteDataColumn;
	switch( m_NotesType )
	{
	case NOTES_TYPE_DANCE_SINGLE:
		mapDanceNoteToNoteDataColumn[DANCE_NOTE_PAD1_LEFT] = 0;
		mapDanceNoteToNoteDataColumn[DANCE_NOTE_PAD1_DOWN] = 1;
		mapDanceNoteToNoteDataColumn[DANCE_NOTE_PAD1_UP] = 2;
		mapDanceNoteToNoteDataColumn[DANCE_NOTE_PAD1_RIGHT] = 3;
		break;
	case NOTES_TYPE_DANCE_DOUBLE:
	case NOTES_TYPE_DANCE_COUPLE:
		mapDanceNoteToNoteDataColumn[DANCE_NOTE_PAD1_LEFT] = 0;
		mapDanceNoteToNoteDataColumn[DANCE_NOTE_PAD1_DOWN] = 1;
		mapDanceNoteToNoteDataColumn[DANCE_NOTE_PAD1_UP] = 2;
		mapDanceNoteToNoteDataColumn[DANCE_NOTE_PAD1_RIGHT] = 3;
		mapDanceNoteToNoteDataColumn[DANCE_NOTE_PAD2_LEFT] = 4;
		mapDanceNoteToNoteDataColumn[DANCE_NOTE_PAD2_DOWN] = 5;
		mapDanceNoteToNoteDataColumn[DANCE_NOTE_PAD2_UP] = 6;
		mapDanceNoteToNoteDataColumn[DANCE_NOTE_PAD2_RIGHT] = 7;
		break;
	case NOTES_TYPE_DANCE_SOLO:
		mapDanceNoteToNoteDataColumn[DANCE_NOTE_PAD1_LEFT] = 0;
		mapDanceNoteToNoteDataColumn[DANCE_NOTE_PAD1_UPLEFT] = 1;
		mapDanceNoteToNoteDataColumn[DANCE_NOTE_PAD1_DOWN] = 2;
		mapDanceNoteToNoteDataColumn[DANCE_NOTE_PAD1_UP] = 3;
		mapDanceNoteToNoteDataColumn[DANCE_NOTE_PAD1_UPRIGHT] = 4;
		mapDanceNoteToNoteDataColumn[DANCE_NOTE_PAD1_RIGHT] = 5;
		break;
	default:
		ASSERT(0);
	}

	m_sDescription = sDescription;

	m_iMeter = atoi( sNumFeet );

	//m_DifficultyClass = DifficultyClassFromDescriptionAndMeter( m_sDescription, m_iMeter );

	NoteData* pNoteData = new NoteData;
	ASSERT( pNoteData );
	pNoteData->m_iNumTracks = mapDanceNoteToNoteDataColumn.GetCount();

	for( int pad=0; pad<2; pad++ )		// foreach pad
	{
		CString sStepData;
		switch( pad )
		{
		case 0:
			sStepData = sStepData1;
			break;
		case 1:
			if( sStepData2 == "" )	// no data
				continue;	// skip
			sStepData = sStepData2;
			break;
		default:
			ASSERT( false );
		}

		double fCurrentBeat = 0;
		double fCurrentIncrementer = 1.0/8 * BEATS_PER_MEASURE;

		for( int i=0; i<sStepData.GetLength(); )
		{
			char c = sStepData[i++];

			switch( c )
			{
			// begins a series
			case '(':
				fCurrentIncrementer = 1.0/16 * BEATS_PER_MEASURE;
				break;
			case '[':
				fCurrentIncrementer = 1.0/24 * BEATS_PER_MEASURE;
				break;
			case '{':
				fCurrentIncrementer = 1.0/64 * BEATS_PER_MEASURE;
				break;
			case '<':
				fCurrentIncrementer = 1.0/192 * BEATS_PER_MEASURE;
				break;

			// ends a series
			case ')':
			case ']':
			case '}':
			case '>':
				fCurrentIncrementer = 1.0/8 * BEATS_PER_MEASURE;
				break;

			case ' ':
				break;	// do nothing!
			
			case '!':		// hold start
				{
					// rewind and get the last step we inserted
					double fLastStepBeat = fCurrentBeat - fCurrentIncrementer;
					int iIndex = BeatToNoteRow( (float)fLastStepBeat );

					char holdChar = sStepData[i++];
					
					DanceNote note1, note2;
					DWIcharToNote( holdChar, (GameController)pad, note1, note2 );

					if( note1 != DANCE_NOTE_NONE )
					{
						int iCol1 = mapDanceNoteToNoteDataColumn[note1];
						pNoteData->m_TapNotes[iCol1][iIndex] = '2';
					}
					if( note2 != DANCE_NOTE_NONE )
					{
						int iCol2 = mapDanceNoteToNoteDataColumn[note2];
						pNoteData->m_TapNotes[iCol2][iIndex] = '2';
					}
				}
				break;
			default:	// this is a note character
				{
					int iIndex = BeatToNoteRow( (float)fCurrentBeat );

					DanceNote note1, note2;
					DWIcharToNote( c, (GameController)pad, note1, note2 );

					if( note1 != DANCE_NOTE_NONE )
					{
						int iCol1 = mapDanceNoteToNoteDataColumn[note1];
						pNoteData->m_TapNotes[iCol1][iIndex] = '1';
					}
					if( note2 != DANCE_NOTE_NONE )
					{
						int iCol2 = mapDanceNoteToNoteDataColumn[note2];
						pNoteData->m_TapNotes[iCol2][iIndex] = '1';
					}

					fCurrentBeat += fCurrentIncrementer;
				}
				break;
			}
		}
	}

	// this will expand the HoldNote begin markers we wrote into actual HoldNotes
	pNoteData->Convert2sAnd3sToHoldNotes();

	ASSERT( pNoteData->m_iNumTracks > 0 );

	m_sSMNoteData = pNoteData->GetSMNoteDataString();

	delete pNoteData;

	TidyUpData();

	return true;
}

void Notes::LoadFromSMTokens( 
	CString sNotesType, 
	CString sDescription,
	CString sDifficultyClass,
	CString sMeter,
	CString sRadarValues,
	CString sNoteData
)
{
	sNotesType.TrimLeft(); 
	sNotesType.TrimRight(); 
	sDescription.TrimLeft(); 
	sDescription.TrimRight(); 
	sDifficultyClass.TrimLeft(); 
	sDifficultyClass.TrimRight(); 


//	LOG->Trace( "Notes::LoadFromSMTokens()" );

	m_NotesType = GameManager::StringToNotesType(sNotesType);
	m_sDescription = sDescription;
	m_DifficultyClass = StringToDifficultyClass( sDifficultyClass );
	m_iMeter = atoi(sMeter);
	CStringArray saValues;
	split( sRadarValues, ",", saValues, true );
	if( saValues.GetSize() == NUM_RADAR_VALUES )
		for( int r=0; r<NUM_RADAR_VALUES; r++ )
			m_fRadarValues[r] = (float)atof(saValues[r]);
    
	m_sSMNoteData = sNoteData;

	TidyUpData();
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

bool Notes::LoadFromKSFFile( const CString &sPath )
{
	LOG->Trace( "Notes::LoadFromKSFFile( '%s' )", sPath );

	MsdFile msd;
	bool bResult = msd.ReadFile( sPath );
	if( !bResult )
		throw RageException( "Error opening file '%s'.", sPath );

	int iTickCount = -1;	// this is the value we read for TICKCOUNT
	CString iStep;			// this is the value we read for STEP

	for( int i=0; i<msd.m_iNumValues; i++ )
	{
		int iNumParams = msd.m_iNumParams[i];
		CString* sParams = msd.m_sValuesAndParams[i];
		CString sValueName = sParams[0];

		// handle the data
		if( 0==stricmp(sValueName,"TICKCOUNT") )
			iTickCount = atoi(sParams[1]);

		else if( 0==stricmp(sValueName,"STEP") )
			iStep = sParams[1];
		else if( 0==stricmp(sValueName,"DIFFICULTY") )
			m_iMeter = atoi(sParams[1]);
	}

	if( iTickCount == -1 )
	{
		iTickCount = 2;
		LOG->Warn( "%s:\nTICKCOUNT not found; defaulting to %i", sPath, iTickCount );
	}

	NoteData notedata;	// read it into here

	CStringArray asRows;
	iStep.TrimLeft();
	split( iStep, "\n", asRows, true );

	int iHoldStartRow[13];
	for( int t=0; t<13; t++ )
		iHoldStartRow[t] = -1;

	for( int r=0; r<asRows.GetSize(); r++ )
	{
		CString& sRowString = asRows[r];
		ASSERT( sRowString.GetLength() == 13 );		// why 13 notes per row.  Beats me!
		
		/* All 2s indicates the end of the song. */
		if( sRowString == "2222222222222" )
			break;

		// the length of a note in a row depends on TICKCOUNT
		float fBeatThisRow = r/(float)iTickCount;
		int row = BeatToNoteRow(fBeatThisRow);
		for( int t=0; t<13; t++ )
		{
			if( sRowString[t] == '4' )
			{
				/* Remember when each hold starts; ignore the middle. */
				if( iHoldStartRow[t] == -1 )
					iHoldStartRow[t] = r;

				continue;
			}

			if( iHoldStartRow[t] != -1 )	// this ends the hold
			{
				HoldNote hn = {
					t, /* button */
					iHoldStartRow[t]/(float)iTickCount, /* start */
					(r-1)/(float)iTickCount /* end */
				};
				notedata.AddHoldNote( hn );
				iHoldStartRow[t] = -1;
			}

			notedata.m_TapNotes[t][row] = sRowString[t];
		}
	}

	CString sDir, sFName, sExt;
	splitrelpath( sPath, sDir, sFName, sExt );
	sFName.MakeLower();
	
	m_sDescription = sFName;
	if( sFName.Find("crazy")!=-1 )
	{
		m_DifficultyClass = CLASS_HARD;
		if(!m_iMeter) m_iMeter = 8;
	}
	else if( sFName.Find("hard")!=-1 )
	{
		m_DifficultyClass = CLASS_MEDIUM;
		if(!m_iMeter) m_iMeter = 5;
	}
	else if( sFName.Find("easy")!=-1 )
	{
		m_DifficultyClass = CLASS_EASY;
		if(!m_iMeter) m_iMeter = 2;
	}
	else
	{
		m_DifficultyClass = CLASS_MEDIUM;
		if(!m_iMeter) m_iMeter = 5;
	}

	if( sFName.Find("double") != -1 )
	{
		notedata.m_iNumTracks = 10;
		m_NotesType = NOTES_TYPE_PUMP_DOUBLE;
	}
	else
	{
		notedata.m_iNumTracks = 5;
		m_NotesType = NOTES_TYPE_PUMP_SINGLE;
	}

	m_sSMNoteData = notedata.GetSMNoteDataString();

	return true;
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

