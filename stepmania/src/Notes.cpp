#include "stdafx.h"
/*
-----------------------------------------------------------------------------
 Class: Notes

 Desc: Hold Notes data and metadata.  Does not hold gameplay-time information,
	   like keeping track of which Notes have been stepped on (PlayerSteps does that).

 Copyright (c) 2001-2002 by the person(s) listed below.  All rights reserved.
-----------------------------------------------------------------------------
*/

#include "Notes.h"
#include "Song.h"
#include "Notes.h"
#include "IniFile.h"
#include "math.h"	// for fabs()
#include "RageUtil.h"
#include "RageLog.h"

#include "GameInput.h"


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
	m_DifficultyClass = CLASS_EASY;
	m_iMeter = 0;
	m_iNumTimesPlayed = 0;
	m_iMaxCombo = 0;
	m_iTopScore = 0;
	m_TopGrade = GRADE_NO_DATA;

	m_pNoteData = NULL;
}

Notes::~Notes()
{
	DeleteNoteData();
}

void Notes::WriteToCacheFile( FILE* file )
{
	LOG->WriteLine( "Notes::WriteToCacheFile()" );

	fprintf( file, "%d\n", m_NotesType );
	WriteStringToFile( file, m_sDescription );
	WriteStringToFile( file, m_sCredit );
	fprintf( file, "%d\n", m_DifficultyClass );
	fprintf( file, "%d\n", m_iMeter );
	for( int i=0; i<NUM_RADAR_VALUES; i++ )
		fprintf( file, "%f\n", m_fRadarValues[i] );

	fprintf( file, "%d\n", m_TopGrade );
	fprintf( file, "%d\n", m_iTopScore );
	fprintf( file, "%d\n", m_iMaxCombo );
	fprintf( file, "%d\n", m_iNumTimesPlayed );

	ASSERT( m_pNoteData != NULL );
	m_pNoteData->WriteToCacheFile( file );
}

void Notes::ReadFromCacheFile( FILE* file, bool bLoadNoteData )
{
	LOG->WriteLine( "Notes::ReadFromCacheFile( %d )", bLoadNoteData );

	fscanf( file, "%d\n", &m_NotesType );
	ReadStringFromFile( file, m_sDescription );
	ReadStringFromFile( file, m_sCredit );
	fscanf( file, "%d\n", &m_DifficultyClass );
	fscanf( file, "%d\n", &m_iMeter );
	for( int i=0; i<NUM_RADAR_VALUES; i++ )
		fscanf( file, "%f\n", &m_fRadarValues[i] );

	fscanf( file, "%d\n", &m_TopGrade );
	fscanf( file, "%d\n", &m_iTopScore );
	fscanf( file, "%d\n", &m_iMaxCombo );
	fscanf( file, "%d\n", &m_iNumTimesPlayed );

	if( bLoadNoteData )
	{
		GetNoteData()->ReadFromCacheFile( file );
	}
	else
	{
		DeleteNoteData();
		NoteData::SkipOverDataInCacheFile( file );
	}
}

bool Notes::LoadFromBMSFile( const CString &sPath )
{
	LOG->WriteLine( "Notes::LoadFromBMSFile( '%s' )", sPath );

	this->GetNoteData();	// make sure NoteData is loaded

// BMS encoding:
// 4&8panel: Player1  Player2
//    Left		11		21
//    Down		13		23
//    Up		15		25
//    Right		16		26
//	6panel:	 Player1  Player2
//    Left		11		21
//    Left+Up	12		22
//    Down		13		23
//    Up		14		24
//    Up+Right	15		25
//    Right		16		26
//
//	Notice that 15 and 25 have double meanings!  What were they thinking???
//	While reading in, use the 6 panel mapping.  After reading in, detect if only 4 notes
//	are used.  If so, shift the Up+Right column back to the Up column
//
	CMap<int, int, int, int>  mapBMSTrackToDanceNote;
	mapBMSTrackToDanceNote[11] = DANCE_NOTE_PAD1_LEFT;
	mapBMSTrackToDanceNote[12] = DANCE_NOTE_PAD1_UPLEFT;
	mapBMSTrackToDanceNote[13] = DANCE_NOTE_PAD1_DOWN;
	mapBMSTrackToDanceNote[14] = DANCE_NOTE_PAD1_UP;
	mapBMSTrackToDanceNote[15] = DANCE_NOTE_PAD1_UPRIGHT;
	mapBMSTrackToDanceNote[16] = DANCE_NOTE_PAD1_RIGHT;
	mapBMSTrackToDanceNote[21] = DANCE_NOTE_PAD2_LEFT;
	mapBMSTrackToDanceNote[22] = DANCE_NOTE_PAD2_UPLEFT;
	mapBMSTrackToDanceNote[23] = DANCE_NOTE_PAD2_DOWN;
	mapBMSTrackToDanceNote[24] = DANCE_NOTE_PAD2_UP;
	mapBMSTrackToDanceNote[25] = DANCE_NOTE_PAD2_UPRIGHT;
	mapBMSTrackToDanceNote[26] = DANCE_NOTE_PAD2_RIGHT;


	bool tempNotes[MAX_NOTE_TRACKS][MAX_TAP_NOTE_ROWS];
	for( int t=0; t<MAX_NOTE_TRACKS; t++ )
	{
		for( int i=0; i<MAX_TAP_NOTE_ROWS; i++ )
			tempNotes[t][i] = false;
	}



	CStdioFile file;	
	if( !file.Open( sPath, CFile::modeRead|CFile::shareDenyNone ) )
	{
		throw RageException( ssprintf("Failed to open %s.", sPath) );
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
			LOG->WriteLine( "Notes description found to be '%s'", m_sDescription );

			// if there's a 6 in the description, it's probably part of "6panel" or "6-panel"
			if( m_sDescription.Find("6") != -1 )
				m_NotesType = NOTES_TYPE_DANCE_SOLO;
			
		}
		if( -1 != value_name.Find("#playlevel") ) 
		{
			m_iMeter = atoi( value_data );

			// Should we calculate m_DifficultyClass here?
		}
		else if( value_name.Left(1) == "#"  
			 && IsAnInt( value_name.Mid(1,3) )
			 && IsAnInt( value_name.Mid(4,2) ) )	// this is step or offset data.  Looks like "#00705"
		{
			int iMeasureNo	= atoi( value_name.Mid(1,3) );
			int iTrackNum	= atoi( value_name.Mid(4,2) );

			CString sNoteData = value_data;
			CArray<bool, bool&> arrayNotes;

			for( int i=0; i<sNoteData.GetLength(); i+=2 )
			{
				bool bThisIsANote = sNoteData.Mid(i,2) != "00";
				arrayNotes.Add( bThisIsANote );
			}

			const int iNumNotesInThisMeasure = arrayNotes.GetSize();
			//LOG->WriteLine( "%s:%s: iMeasureNo = %d, iNoteNum = %d, iNumNotesInThisMeasure = %d", 
			//	valuename, sNoteData, iMeasureNo, iNoteNum, iNumNotesInThisMeasure );
			for( int j=0; j<iNumNotesInThisMeasure; j++ )
			{
				if( arrayNotes.GetAt(j) == TRUE )
				{
					float fPercentThroughMeasure = (float)j/(float)iNumNotesInThisMeasure;

					const int iNoteIndex = (int) ( (iMeasureNo + fPercentThroughMeasure)
									 * BEATS_PER_MEASURE * ELEMENTS_PER_BEAT );
					const int iColumnNumber = mapBMSTrackToDanceNote[iTrackNum];

					tempNotes[iColumnNumber][iNoteIndex] = true;
				}
			}
		}
	}
	
	if( m_NotesType == NOTES_TYPE_DANCE_SINGLE  || 
		m_NotesType == NOTES_TYPE_DANCE_DOUBLE  || 
		m_NotesType == NOTES_TYPE_DANCE_COUPLE )	// if there are 4 panels, then we have to flip the Up and Up+Right bits
	{
		for( int i=0; i<MAX_TAP_NOTE_ROWS; i++ )	// for each TapNote
		{
			if( tempNotes[DANCE_NOTE_PAD1_UPRIGHT][i] )	// if up+right is a step
			{
				tempNotes[DANCE_NOTE_PAD1_UP][i] = true;			// add up
				tempNotes[DANCE_NOTE_PAD1_UPRIGHT][i] = false;	// subtract up+right
			}
			if( tempNotes[DANCE_NOTE_PAD2_UPRIGHT][i] )	// if up+right is a step
			{
				tempNotes[DANCE_NOTE_PAD2_UP][i] = true;			// add up
				tempNotes[DANCE_NOTE_PAD2_UPRIGHT][i] = false;	// subtract up+right
			}
		}
	}

	// we're done reading in all of the BMS values

	CMap<int, int, int, int>  mapDanceNoteToNoteDataColumn;
	if( m_NotesType == NOTES_TYPE_DANCE_SINGLE )
	{
		mapDanceNoteToNoteDataColumn[DANCE_NOTE_PAD1_LEFT] = 0;
		mapDanceNoteToNoteDataColumn[DANCE_NOTE_PAD1_DOWN] = 1;
		mapDanceNoteToNoteDataColumn[DANCE_NOTE_PAD1_UP] = 2;
		mapDanceNoteToNoteDataColumn[DANCE_NOTE_PAD1_RIGHT] = 3;
	}
	else if( m_NotesType == NOTES_TYPE_DANCE_DOUBLE )
	{
		mapDanceNoteToNoteDataColumn[DANCE_NOTE_PAD1_LEFT] = 0;
		mapDanceNoteToNoteDataColumn[DANCE_NOTE_PAD1_DOWN] = 1;
		mapDanceNoteToNoteDataColumn[DANCE_NOTE_PAD1_UP] = 2;
		mapDanceNoteToNoteDataColumn[DANCE_NOTE_PAD1_RIGHT] = 3;
		mapDanceNoteToNoteDataColumn[DANCE_NOTE_PAD2_LEFT] = 4;
		mapDanceNoteToNoteDataColumn[DANCE_NOTE_PAD2_DOWN] = 5;
		mapDanceNoteToNoteDataColumn[DANCE_NOTE_PAD2_UP] = 6;
		mapDanceNoteToNoteDataColumn[DANCE_NOTE_PAD2_RIGHT] = 7;
	}
	else if( m_NotesType == NOTES_TYPE_DANCE_COUPLE )
	{
		mapDanceNoteToNoteDataColumn[DANCE_NOTE_PAD1_LEFT] = 0;
		mapDanceNoteToNoteDataColumn[DANCE_NOTE_PAD1_DOWN] = 1;
		mapDanceNoteToNoteDataColumn[DANCE_NOTE_PAD1_UP] = 2;
		mapDanceNoteToNoteDataColumn[DANCE_NOTE_PAD1_RIGHT] = 3;
		mapDanceNoteToNoteDataColumn[DANCE_NOTE_PAD2_LEFT] = 4;
		mapDanceNoteToNoteDataColumn[DANCE_NOTE_PAD2_DOWN] = 5;
		mapDanceNoteToNoteDataColumn[DANCE_NOTE_PAD2_UP] = 6;
		mapDanceNoteToNoteDataColumn[DANCE_NOTE_PAD2_RIGHT] = 7;
	}
	else if( m_NotesType == NOTES_TYPE_DANCE_SOLO )
	{
		mapDanceNoteToNoteDataColumn[DANCE_NOTE_PAD1_LEFT] = 0;
		mapDanceNoteToNoteDataColumn[DANCE_NOTE_PAD1_UPLEFT] = 1;
		mapDanceNoteToNoteDataColumn[DANCE_NOTE_PAD1_DOWN] = 2;
		mapDanceNoteToNoteDataColumn[DANCE_NOTE_PAD1_UP] = 3;
		mapDanceNoteToNoteDataColumn[DANCE_NOTE_PAD1_UPRIGHT] = 4;
		mapDanceNoteToNoteDataColumn[DANCE_NOTE_PAD1_RIGHT] = 5;
	}

	// copy the tempData we collected into NoteData
	POSITION pos = mapDanceNoteToNoteDataColumn.GetStartPosition();
	while( pos != NULL )  // iterate over all k/v pairs in map
	{
		int iTempCol;
		int iNoteDataCol;
		mapDanceNoteToNoteDataColumn.GetNextAssoc( pos, iTempCol, iNoteDataCol );

		for( int i=0; i<MAX_TAP_NOTE_ROWS; i++ )	// foreach TapNote element
		{
			if( tempNotes[iTempCol][i] )
				m_pNoteData->m_TapNotes[iNoteDataCol][i] = '1';
		}
	}

	m_pNoteData->m_iNumTracks = mapDanceNoteToNoteDataColumn.GetCount();
	

	// search for runs of 32nd notes of all the same note type, and convert them to freezes
	for( int iCol=0; iCol<mapDanceNoteToNoteDataColumn.GetCount(); iCol++ )
	{
		int iIndexRunStart = -1;	// -1 means we're not in the middle of a run

		for( float fBeat=0; fBeat<MAX_BEATS; fBeat += 1/32.0f )		// foreach 32nd note
		{
			int i = BeatToNoteRow( fBeat );

			if( m_pNoteData->m_TapNotes[iCol][i] == '1' )	// there is a step here
			{
				if( iIndexRunStart == -1 )	// we are not in the middle of a run
				{
					iIndexRunStart = i;
				}
				else	// we are in the middle of a run
				{
					;	// do nothing.  Keep reading in until we hit the end of the run.	
				}
			}
			else	// there isn't a step here
			{
				if( iIndexRunStart == -1 )	// we are not in the middle of a run
				{
					;	// do nothing
				}
				else	// we are in the middle of a run
				{
					// this ends the run

 					if( i - iIndexRunStart < 2 )	// reject runs of length 1
					{
						;	// do nothing
					}
					else	// this run is longer than 1
					{
						// add this HoldNote to the list
						int iIndexRunEnd = i - 1;
						HoldNote hn = { iCol, iIndexRunStart, iIndexRunEnd };
						m_pNoteData->AddHoldNote( hn );

						LOG->WriteLine( "Found a run on track %d: start: %d, end: %d.", hn.m_iTrack, hn.m_iStartIndex, hn.m_iEndIndex );
					}

					iIndexRunStart = -1;	// reset so we can look for more runs
				}
			}
		}
		// done looking for runs


	}

	return true;
}




void DWIcharToNote( char c, InstrumentNumber i, DanceNote &note1Out, DanceNote &note2Out )
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
		default:	throw RageException( "Encountered invalid DWI note characer '%c'", c );			break;
	}

	switch( i )
	{
	case INSTRUMENT_1:
		break;
	case INSTRUMENT_2:
		note1Out <<= 6;
		if( note2Out != DANCE_NOTE_NONE )
			note2Out <<= 6;
		break;
	default:
		ASSERT( false );
	}
}



bool Notes::LoadFromDWITokens( 
	const CString &sMode, const CString &sDescription,
	const int &iNumFeet,
	const CString &sStepData1, const CString &sStepData2 )
{
	LOG->WriteLine( "Notes::LoadFromDWITokens()" );

	if(		 sMode == "#SINGLE" )	m_NotesType = NOTES_TYPE_DANCE_SINGLE;
	else if( sMode == "#DOUBLE" )	m_NotesType = NOTES_TYPE_DANCE_DOUBLE;
	else if( sMode == "#COUPLE" )	m_NotesType = NOTES_TYPE_DANCE_COUPLE;
	else if( sMode == "#SOLO" )		m_NotesType = NOTES_TYPE_DANCE_SOLO;
	else					LOG->WriteLine( "Unrecognized DWI mode '%s'", sMode );


	CMap<int, int, int, int>  mapDanceNoteToNoteDataColumn;
	if( m_NotesType == NOTES_TYPE_DANCE_SINGLE )
	{
		mapDanceNoteToNoteDataColumn[DANCE_NOTE_PAD1_LEFT] = 0;
		mapDanceNoteToNoteDataColumn[DANCE_NOTE_PAD1_DOWN] = 1;
		mapDanceNoteToNoteDataColumn[DANCE_NOTE_PAD1_UP] = 2;
		mapDanceNoteToNoteDataColumn[DANCE_NOTE_PAD1_RIGHT] = 3;
	}
	else if( m_NotesType == NOTES_TYPE_DANCE_DOUBLE  ||  m_NotesType == NOTES_TYPE_DANCE_COUPLE )
	{
		mapDanceNoteToNoteDataColumn[DANCE_NOTE_PAD1_LEFT] = 0;
		mapDanceNoteToNoteDataColumn[DANCE_NOTE_PAD1_DOWN] = 1;
		mapDanceNoteToNoteDataColumn[DANCE_NOTE_PAD1_UP] = 2;
		mapDanceNoteToNoteDataColumn[DANCE_NOTE_PAD1_RIGHT] = 3;
		mapDanceNoteToNoteDataColumn[DANCE_NOTE_PAD2_LEFT] = 4;
		mapDanceNoteToNoteDataColumn[DANCE_NOTE_PAD2_DOWN] = 5;
		mapDanceNoteToNoteDataColumn[DANCE_NOTE_PAD2_UP] = 6;
		mapDanceNoteToNoteDataColumn[DANCE_NOTE_PAD2_RIGHT] = 7;
	}
	else if( m_NotesType == NOTES_TYPE_DANCE_SOLO )
	{
		mapDanceNoteToNoteDataColumn[DANCE_NOTE_PAD1_LEFT] = 0;
		mapDanceNoteToNoteDataColumn[DANCE_NOTE_PAD1_UPLEFT] = 1;
		mapDanceNoteToNoteDataColumn[DANCE_NOTE_PAD1_DOWN] = 2;
		mapDanceNoteToNoteDataColumn[DANCE_NOTE_PAD1_UP] = 3;
		mapDanceNoteToNoteDataColumn[DANCE_NOTE_PAD1_UPRIGHT] = 4;
		mapDanceNoteToNoteDataColumn[DANCE_NOTE_PAD1_RIGHT] = 5;
	}
	else
		ASSERT(0);

	m_sDescription = sDescription;

	m_iMeter = iNumFeet;

	if( m_sDescription == "BASIC" )			m_DifficultyClass = CLASS_EASY;
	else if( m_sDescription == "ANOTHER" )	m_DifficultyClass = CLASS_MEDIUM;
	else if( m_sDescription == "MANIAC" )	m_DifficultyClass = CLASS_HARD;
	else if( m_sDescription == "SMANIAC" )	m_DifficultyClass = CLASS_HARD;
	else
	{
		// guess difficulty class from m_iMeter
		if(		 m_iMeter <= 3 )	m_DifficultyClass = CLASS_EASY;
		else if( m_iMeter <= 6 )	m_DifficultyClass = CLASS_MEDIUM;
		else						m_DifficultyClass = CLASS_HARD;
	}

	m_sCredit = "";



	ASSERT( m_pNoteData == NULL );	// if not, then we're loading this Notes twice
	m_pNoteData = new NoteData;
	m_pNoteData->m_iNumTracks = mapDanceNoteToNoteDataColumn.GetCount();

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
				DWIcharToNote( holdChar, (InstrumentNumber)pad, note1, note2 );

				if( note1 != DANCE_NOTE_NONE )
				{
					int iCol1 = mapDanceNoteToNoteDataColumn[note1];
					m_pNoteData->m_TapNotes[iCol1][iIndex] = '2';
				}
				if( note2 != DANCE_NOTE_NONE )
				{
					int iCol2 = mapDanceNoteToNoteDataColumn[note2];
					m_pNoteData->m_TapNotes[iCol2][iIndex] = '2';
				}

				}
				break;
			default:	// this is a note character
				{
				int iIndex = BeatToNoteRow( (float)fCurrentBeat );

				DanceNote note1, note2;
				DWIcharToNote( c, (InstrumentNumber)pad, note1, note2 );

				if( note1 != DANCE_NOTE_NONE )
				{
					int iCol1 = mapDanceNoteToNoteDataColumn[note1];
					m_pNoteData->m_TapNotes[iCol1][iIndex] = '1';
				}
				if( note2 != DANCE_NOTE_NONE )
				{
					int iCol2 = mapDanceNoteToNoteDataColumn[note2];
					m_pNoteData->m_TapNotes[iCol2][iIndex] = '1';
				}

				fCurrentBeat += fCurrentIncrementer;
				}
				break;
			}
		}
	}

	m_pNoteData->Convert2sAnd3sToHoldNotes();	// this will expand the HoldNote begin markers we wrote into actual HoldNotes

	ASSERT( m_pNoteData->m_iNumTracks > 0 );

	return true;
}

bool Notes::LoadFromNotesFile( const CString &sPath )
{
	LOG->WriteLine( "Notes::LoadFromNotesFile( '%s' )", sPath );

	CStdioFile file;	
	if( !file.Open( sPath, CFile::modeRead|CFile::shareDenyNone ) )
		throw RageException( "Error opening DWI file '%s'.", sPath );

	// read the whole file into a sFileText
	CString sFileText;
	CString buffer;
	while( file.ReadString(buffer) )
		sFileText += buffer + "\n";
	file.Close();

	// strip comments out of sFileText
	while( sFileText.Find("//") != -1 )
	{
		int iIndexCommentStart = sFileText.Find("//");
		int iIndexCommentEnd = sFileText.Find("\n", iIndexCommentStart);
		if( iIndexCommentEnd == -1 )	// comment doesn't have an end?
			sFileText.Delete( iIndexCommentStart, 2 );
		else
			sFileText.Delete( iIndexCommentStart, iIndexCommentEnd-iIndexCommentStart );
	}

	// split sFileText into strings containing each value expression
	CStringArray arrayValueStrings;
	split( sFileText, ";", arrayValueStrings );


	// for each value expression string, parse it into a value name and data
	for( int i=0; i < arrayValueStrings.GetSize(); i++ )
	{
		CString sValueString = arrayValueStrings[i];

		// split the value string into tokens
		CStringArray arrayValueTokens;
		split( sValueString, ":", arrayValueTokens, false );

		if( arrayValueTokens.GetSize() == 0 )
			continue;

		CString sValueName = arrayValueTokens.GetAt( 0 );
		sValueName.TrimLeft();
		sValueName.TrimRight();

		// handle the data
		if( sValueName == "#TYPE" )
			m_NotesType = StringToNotesType( arrayValueTokens[1] );

		else if( sValueName == "#DESCRIPTION" )
			m_sDescription = arrayValueTokens[1];

		else if( sValueName == "#CREDIT" )
			m_sCredit = arrayValueTokens[1];

		else if( sValueName == "#METER" )
			m_iMeter = atoi( arrayValueTokens[1] );

		else if( sValueName == "#NOTES" )
		{
			arrayValueTokens.RemoveAt( 0 );

			ASSERT( m_pNoteData == NULL );	// if not, then we're loading this NoteData twice
			m_pNoteData = new NoteData;

			m_pNoteData->SetFromMeasureStrings( arrayValueTokens );
		}

		else
			LOG->WriteLine( "Unexpected value named '%s'", sValueName );
	}

	
	if     ( stricmp(m_sDescription, "BASIC") == 0 )	m_DifficultyClass = CLASS_EASY;
	else if( stricmp(m_sDescription, "TRICK") == 0 )	m_DifficultyClass = CLASS_MEDIUM;
	else if( stricmp(m_sDescription, "STANDARD") == 0 )	m_DifficultyClass = CLASS_MEDIUM;
	else if( stricmp(m_sDescription, "ANOTHER") == 0 )	m_DifficultyClass = CLASS_MEDIUM;
	else if( stricmp(m_sDescription, "SSR") == 0 )		m_DifficultyClass = CLASS_HARD;
	else if( stricmp(m_sDescription, "MANIAC") == 0 )	m_DifficultyClass = CLASS_HARD;
	else if( stricmp(m_sDescription, "HEAVY") == 0 )	m_DifficultyClass = CLASS_HARD;
	else if( stricmp(m_sDescription, "SMANIAC") == 0 )	m_DifficultyClass = CLASS_HARD;
	else
	{
		// guess difficulty class from m_iMeter
		if(		 m_iMeter <= 3 )	m_DifficultyClass = CLASS_EASY;
		else if( m_iMeter <= 6 )	m_DifficultyClass = CLASS_MEDIUM;
		else						m_DifficultyClass = CLASS_HARD;
	}

	return true;
}



void Notes::SaveToSMDir( CString sSongDir )
{
	LOG->WriteLine( "Notes::Save( '%s' )", sSongDir );

	CString sNewNotesFilePath = sSongDir + ssprintf("%s-%s.notes", NotesTypeToString(m_NotesType), m_sDescription);

	CStdioFile file;	
	if( !file.Open( sNewNotesFilePath, CFile::modeWrite | CFile::modeCreate ) )
		throw RageException( "Error opening Notes file '%s' for writing.", sNewNotesFilePath );

	file.WriteString( ssprintf("#TYPE:%s;\n", NotesTypeToString(m_NotesType)) );
	file.WriteString( ssprintf("#DESCRIPTION:%s;\n", m_sDescription) );
	file.WriteString( ssprintf("#METER:%d;\n", m_iMeter) );
	file.WriteString( ssprintf("#CREDIT:%s;\n", m_sCredit) );


	file.WriteString( "#NOTES:\n" );
	CStringArray sMeasureStrings;
	GetNoteData()->GetMeasureStrings( sMeasureStrings );
	file.WriteString( join(":\n", sMeasureStrings) );
	file.WriteString( ";" );

	
	file.Close();
}

bool Notes::IsNoteDataLoaded()
{
	return m_pNoteData != NULL;
}


NoteData* Notes::GetNoteData()
{
	if( m_pNoteData != NULL )
		return m_pNoteData;

	m_pNoteData = new NoteData;
		
	return m_pNoteData;
}

void Notes::SetNoteData( NoteData* pNewNoteData )
{
	NoteData* pNoteData = GetNoteData();
	pNoteData->CopyAll( pNewNoteData );
}

void Notes::DeleteNoteData()
{
	SAFE_DELETE( m_pNoteData );
}


//////////////////////////////////////////////
//
//////////////////////////////////////////////


int CompareNotesPointersByMeter(const void *arg1, const void *arg2)
{
	Notes* pNotes1 = *(Notes**)arg1;
	Notes* pNotes2 = *(Notes**)arg2;

	int iScore1 = pNotes1->m_iMeter;
	int iScore2 = pNotes2->m_iMeter;

	if( iScore1 < iScore2 )
		return -1;
	else if( iScore1 == iScore2 )
		return 0;
	else
		return 1;
}

int CompareNotesPointersByDifficultyClass(const void *arg1, const void *arg2)
{
	Notes* pNotes1 = *(Notes**)arg1;
	Notes* pNotes2 = *(Notes**)arg2;

	DifficultyClass class1 = pNotes1->m_DifficultyClass;
	DifficultyClass class2 = pNotes2->m_DifficultyClass;

	if( class1 < class2 )
		return -1;
	else if( class1 == class2 )
		return CompareNotesPointersByMeter( arg1, arg2 );
	else
		return 1;
}

void SortNotesArrayByDifficultyClass( CArray<Notes*,Notes*> &arraySteps )
{
	qsort( arraySteps.GetData(), arraySteps.GetSize(), sizeof(Notes*), CompareNotesPointersByDifficultyClass );
}

