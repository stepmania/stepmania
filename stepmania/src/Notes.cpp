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
	m_NotesType = NOTES_TYPE_DANCE_SINGLE;
	m_DifficultyClass = CLASS_EASY;
	m_iMeter = 0;
	for( int r=0; r<NUM_RADAR_VALUES; r++ )
		m_fRadarValues[r] = 0;

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
	LOG->WriteLine( "Notes::LoadFromBMSFile( '%s' )", sPath );

	this->GetNoteData();	// make sure NoteData is loaded




	int tempNotes[MAX_NOTE_TRACKS][MAX_TAP_NOTE_ROWS];
	for( int t=0; t<MAX_NOTE_TRACKS; t++ )
	{
		for( int i=0; i<MAX_TAP_NOTE_ROWS; i++ )
			tempNotes[t][i] = '0';
	}



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
			LOG->WriteLine( "Notes description found to be '%s'", m_sDescription );

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
					int iColumnNumber;
					char cNoteChar;
					mapBMSTrackToDanceNote( iTrackNum, iColumnNumber, cNoteChar );

					if( iColumnNumber != -1 )
						tempNotes[iColumnNumber][iNoteIndex] = cNoteChar;
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
			if( tempNotes[DANCE_NOTE_PAD1_UPRIGHT][i] != '0' )	// if up+right is a step
			{
				tempNotes[DANCE_NOTE_PAD1_UP][i] = tempNotes[DANCE_NOTE_PAD1_UPRIGHT][i];			// add up
				tempNotes[DANCE_NOTE_PAD1_UPRIGHT][i] = '0';	// subtract up+right
			}
			if( tempNotes[DANCE_NOTE_PAD2_UPRIGHT][i] != '0' )	// if up+right is a step
			{
				tempNotes[DANCE_NOTE_PAD2_UP][i] = tempNotes[DANCE_NOTE_PAD2_UPRIGHT][i];			// add up
				tempNotes[DANCE_NOTE_PAD2_UPRIGHT][i] = '0';	// subtract up+right
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
			m_pNoteData->m_TapNotes[iNoteDataCol][i] = tempNotes[iTempCol][i];
		}
	}

	m_pNoteData->m_iNumTracks = mapDanceNoteToNoteDataColumn.GetCount();
	
	m_pNoteData->Convert2sAnd3sToHoldNotes();


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


	m_DifficultyClass = DifficultyClassFromDescriptionAndMeter( m_sDescription, m_iMeter );

	if( m_iMeter == 0 )
	{
		switch( m_DifficultyClass )
		{
		case CLASS_EASY:	m_iMeter = 3;	break;
		case CLASS_MEDIUM:	m_iMeter = 5;	break;
		case CLASS_HARD:	m_iMeter = 8;	break;
		default:	ASSERT(0);
		}
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

	m_DifficultyClass = DifficultyClassFromDescriptionAndMeter( m_sDescription, m_iMeter );

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

void Notes::LoadFromSMTokens( 
	const CString &sNotesType, 
	const CString &sDescription,
	const CString &sCredit,
	const CString &sDifficultyClass,
	const CString &sMeter,
	const CString &sRadarValues,
	const CString &sNoteDataOut,
	const bool bLoadNoteData
)
{
	LOG->WriteLine( "Notes::LoadFromSMTokens( %d )", bLoadNoteData );

	m_NotesType = StringToNotesType(sNotesType);
	m_sDescription = sDescription;
	m_sCredit = sCredit;
	m_DifficultyClass = StringToDifficultyClass( sDifficultyClass );
	m_iMeter = atoi(sMeter);
	CStringArray saValues;
	split( sRadarValues, ",", saValues, true );
	if( saValues.GetSize() == NUM_RADAR_VALUES )
		for( int r=0; r<NUM_RADAR_VALUES; r++ )
			m_fRadarValues[r] = (float)atof(saValues[r]);

	if( m_iMeter < 1 || m_iMeter > 10 ) 
		m_iMeter = 4;
	if( m_DifficultyClass == CLASS_INVALID )
		m_DifficultyClass = DifficultyClassFromDescriptionAndMeter( sDescription, m_iMeter );
    

	//
	// Load NoteData
	//
	NoteData *pND = GetNoteData();

	pND->m_iNumTracks = NotesTypeToNumTracks( m_NotesType );

	CStringArray asNoteData;
	split( sNoteDataOut, ",", asNoteData, true );
	for( int i=0; i<asNoteData.GetSize(); i+=2 )
	{
		int iNumRowsInMeasure = atoi( asNoteData[i] );

		CString sMeasureString = asNoteData[i+1];
		sMeasureString.TrimLeft();
		sMeasureString.TrimRight();

		CStringArray arrayNoteLines;
		split( sMeasureString, "\n", arrayNoteLines );

		if( arrayNoteLines.GetSize() != iNumRowsInMeasure )
			throw RageException( "Actual number of note rows (%d) doesn't match what tag says (%d).", arrayNoteLines.GetSize(), iNumRowsInMeasure );

		for( int l=0; l<iNumRowsInMeasure; l++ )
		{
			const float fPercentIntoMeasure = l/(float)arrayNoteLines.GetSize();
			const float fBeat = (i + fPercentIntoMeasure) * BEATS_PER_MEASURE;
			const int iIndex = BeatToNoteRow( fBeat );

			CString sNoteLine = arrayNoteLines[l];
			sNoteLine.TrimRight();

			if( pND->m_iNumTracks != sNoteLine.GetLength() )
				throw RageException( "Actual number of note columns (%d) is different from the NotesType (%d).", pND->m_iNumTracks, sNoteLine.GetLength() );

			for( int c=0; c<sNoteLine.GetLength(); c++ )
				pND->m_TapNotes[c][iIndex] = sNoteLine[c];
		}
	}


}

void Notes::WriteSMNotesTag( FILE* fp )
{
	fprintf( fp, "#NOTES:\n" );
	fprintf( fp, "     %s:\n", NotesTypeToString(m_NotesType) );
	fprintf( fp, "     %s:\n", m_sDescription );
	fprintf( fp, "     %s:\n", m_sCredit );
	fprintf( fp, "     %s:\n", DifficultyClassToString(m_DifficultyClass) );
	fprintf( fp, "     %d:\n", m_iMeter );
	
	CStringArray asRadarValues;
	for( int r=0; r<NUM_RADAR_VALUES; r++ )
		asRadarValues.Add( ssprintf("%.1f", m_fRadarValues[r]) );
	fprintf( fp, "     %s:\n", join(",",asRadarValues) );


	//
	// fill in sNoteDataOut 
	//
	NoteData* pND = GetNoteData();

	pND->ConvertHoldNotesTo2sAnd3s();

	float fLastBeat = pND->GetLastBeat();
	int iLastMeasure = int( fLastBeat/BEATS_PER_MEASURE );

	for( int m=0; m<=iLastMeasure; m++ )	// foreach measure
	{
		int iMeasureStartIndex = m * ELEMENTS_PER_MEASURE;
		int iMeasureLastIndex = (m+1) * ELEMENTS_PER_MEASURE - 1;

		// probe to find the smallest note type
		NoteType nt;
		int iNoteIndexSpacing;
		for( nt=(NoteType)0; nt<NUM_NOTE_TYPES; nt=NoteType(nt+1) )
		{
			float fBeatSpacing = NoteTypeToBeat( nt );
			iNoteIndexSpacing = roundf( fBeatSpacing * ELEMENTS_PER_BEAT );

			bool bFoundSmallerNote = false;
			for( int i=iMeasureStartIndex; i<=iMeasureLastIndex; i++ )	// for each index in this measure
			{
				if( i % iNoteIndexSpacing == 0 )
					continue;	// skip
				
				if( !pND->IsRowEmpty(i) )
				{
					bFoundSmallerNote = true;
					break;
				}
				
			}
			if( bFoundSmallerNote )
				continue;	// keep searching
			else
				break;	// stop searching
		}

		if( nt == NUM_NOTE_TYPES )	// we didn't find one
			iNoteIndexSpacing = 1;

		fprintf( fp, "%d, // measure %d\n", ELEMENTS_PER_MEASURE/iNoteIndexSpacing, m+1 );

		for( int i=iMeasureStartIndex; i<=iMeasureLastIndex; i+=iNoteIndexSpacing )
		{
			CString sLineString;
			for( int c=0; c<pND->m_iNumTracks; c++ )
				sLineString += pND->m_TapNotes[c][i];
			fprintf( fp, "%s", sLineString );
			if( i == iMeasureLastIndex )
				fprintf( fp, ",\n" );
			else
				fprintf( fp, "\n" );
		}
	}

	fprintf( fp, ";\n" );

	pND->Convert2sAnd3sToHoldNotes();
}


DifficultyClass Notes::DifficultyClassFromDescriptionAndMeter( CString sDescription, int iMeter )
{
	sDescription.MakeLower();

	const CString sDescriptionParts[NUM_DIFFICULTY_CLASSES][3] = {
		{
			"basic",
            "light",
			"SDFKSJDKFJS",
		},
		{
			"another",
            "trick",
            "standard",
		},
		{
			"ssr",
            "maniac",
            "heavy",
		},
	};

	for( int i=0; i<NUM_DIFFICULTY_CLASSES; i++ )
		for( int j=0; j<3; j++ )
			if( sDescription.Find(sDescriptionParts[i][j]) != -1 )
				return DifficultyClass(i);
	
	// guess difficulty class from meter
	if(		 iMeter <= 3 )	return CLASS_EASY;
	else if( iMeter <= 6 )	return CLASS_MEDIUM;
	else					return CLASS_HARD;
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

