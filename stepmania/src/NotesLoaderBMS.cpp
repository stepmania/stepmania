#include "stdafx.h"

#include "NotesLoaderBMS.h"
#include "NotesLoader.h"
#include "NoteData.h"
#include "GameConstantsAndTypes.h"
#include "RageUtil.h"
#include "RageLog.h"
#include "GameManager.h"
#include "RageException.h"

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
void BMSLoader::mapBMSTrackToDanceNote( int iBMSTrack, int &iDanceColOut, char &cNoteCharOut )
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


bool BMSLoader::LoadFromBMSFile( const CString &sPath, Notes &out, Notes &out2 )
{
	LOG->Trace( "Notes::LoadFromBMSFile( '%s' )", sPath );

	out.m_NotesType = NOTES_TYPE_INVALID;
	out2.m_NotesType = NOTES_TYPE_INVALID;

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
				out.m_NotesType = NOTES_TYPE_DANCE_SINGLE;
				// if the mode should be solo, then we'll update m_DanceStyle below when we read in step data
				break;
			case 2:		// couple/battle
				out.m_NotesType = NOTES_TYPE_DANCE_COUPLE_1;
				break;
			case 3:		// double
				out.m_NotesType = NOTES_TYPE_DANCE_DOUBLE;
				break;
			}
		}
		if( -1 != value_name.Find("#title") )
		{
			out.m_sDescription = value_data;
			
			// extract the Notes description (looks like 'Music <BASIC>')
			int iPosOpenBracket = out.m_sDescription.Find( "<" );
			if( iPosOpenBracket == -1 )
				iPosOpenBracket = out.m_sDescription.Find( "(" );
			int iPosCloseBracket = out.m_sDescription.Find( ">" );
			if( iPosCloseBracket == -1 )
				iPosCloseBracket = out.m_sDescription.Find( ")" );

			if( iPosOpenBracket != -1  &&  iPosCloseBracket != -1 )
				out.m_sDescription = out.m_sDescription.Mid( iPosOpenBracket+1, iPosCloseBracket-iPosOpenBracket-1 );
			out.m_sDescription.MakeLower();
			LOG->Trace( "Notes description found to be '%s'", out.m_sDescription );

			// if there's a 6 in the description, it's probably part of "6panel" or "6-panel"
			if( out.m_sDescription.Find("6") != -1 )
				out.m_NotesType = NOTES_TYPE_DANCE_SOLO;
			
		}
		if( -1 != value_name.Find("#playlevel") ) 
		{
			out.m_iMeter = atoi( value_data );
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
	
	if( out.m_NotesType == NOTES_TYPE_DANCE_SINGLE  || 
		out.m_NotesType == NOTES_TYPE_DANCE_DOUBLE  || 
		out.m_NotesType == NOTES_TYPE_DANCE_COUPLE_1)	// if there are 4 panels, then the Up+Right track really contains the notes for Up
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
	int iNumNewTracks = GameManager::NotesTypeToNumTracks( out.m_NotesType );
	int iTransformNewToOld[MAX_NOTE_TRACKS];

	int i;
	for( i = 0; i < MAX_NOTE_TRACKS; ++i)
		iTransformNewToOld[i] = -1;

	switch( out.m_NotesType )
	{
	case NOTES_TYPE_DANCE_SINGLE:
	case NOTES_TYPE_DANCE_COUPLE_1:
		iTransformNewToOld[0] = DANCE_NOTE_PAD1_LEFT;
		iTransformNewToOld[1] = DANCE_NOTE_PAD1_DOWN;
		iTransformNewToOld[2] = DANCE_NOTE_PAD1_UP;
		iTransformNewToOld[3] = DANCE_NOTE_PAD1_RIGHT;
		break;
	case NOTES_TYPE_DANCE_DOUBLE:
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

	out.SetNoteData(pNoteData2);

	if(out.m_NotesType == NOTES_TYPE_DANCE_COUPLE_1) {
		/* Couples.  Set up a second note pattern for the 2p side. */
		out2 = out;
		out2.m_NotesType = NOTES_TYPE_DANCE_COUPLE_2;

		for( i = 0; i < MAX_NOTE_TRACKS; ++i)
			iTransformNewToOld[i] = -1;

		iTransformNewToOld[0] = DANCE_NOTE_PAD2_LEFT;
		iTransformNewToOld[1] = DANCE_NOTE_PAD2_DOWN;
		iTransformNewToOld[2] = DANCE_NOTE_PAD2_UP;
		iTransformNewToOld[3] = DANCE_NOTE_PAD2_RIGHT;

		pNoteData2->LoadTransformed( pNoteData, iNumNewTracks, iTransformNewToOld );
		out2.SetNoteData(pNoteData2);
		out2.TidyUpData();
	}

	delete pNoteData;
	delete pNoteData2;

	out.TidyUpData();

	return true;
}

bool BMSLoader::LoadFromBMSDir( CString sDir, Song &out )
{
	LOG->Trace( "Song::LoadFromBMSDir(%s)", sDir );

	CStringArray arrayBMSFileNames;
	GetDirListing( sDir + CString("*.bms"), arrayBMSFileNames );

	if( arrayBMSFileNames.GetSize() == 0 )
		throw RageException( "Couldn't find any BMS files in '%s'", sDir );

	// load the Notes from the rest of the BMS files
	for( int i=0; i<arrayBMSFileNames.GetSize(); i++ ) 
	{
		Notes* pNewNotes = new Notes;
		Notes* pNewNotes2 = new Notes;

		LoadFromBMSFile( out.m_sSongDir + arrayBMSFileNames[i],
			*pNewNotes, *pNewNotes2 );
		/* Add either note pattern that actually loaded. */
		if(pNewNotes->m_NotesType != NOTES_TYPE_INVALID)
			out.m_apNotes.Add( pNewNotes );
		else
			delete pNewNotes;
		if(pNewNotes2->m_NotesType != NOTES_TYPE_INVALID)
			out.m_apNotes.Add( pNewNotes2 );
		else
			delete pNewNotes2;
	}

	CString sPath = out.m_sSongDir + arrayBMSFileNames[0];

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


		// handle the data
		if( value_name == "#title" ) 
		{
			// strip Notes type out of description leaving only song title - looks like 'B4U <BASIC>'
			int iIndex = value_data.ReverseFind('<');
			if( iIndex == -1 )
				iIndex = value_data.ReverseFind('(');
			if( iIndex != -1 )
			{
				value_data = value_data.Left( iIndex );
				out.GetMainAndSubTitlesFromFullTitle( value_data, out.m_sMainTitle, out.m_sSubTitle );
			}
			else
				out.m_sMainTitle = value_data;
		}
		else if( value_name == "#artist" ) 
		{
			out.m_sArtist = value_data;
		}
		else if( value_name == "#bpm" ) 
		{
			BPMSegment newSeg( 0, (float)atof(value_data) );
			out.AddBPMSegment( newSeg );
		
			LOG->Trace( "Inserting new BPM change at beat %f, BPM %f", newSeg.m_fStartBeat, newSeg.m_fBPM );
		}
		else if( value_name == "#backbmp" ) 
		{
			out.m_sBackgroundFile = value_data;
		}
		else if( value_name == "#wav" ) 
		{
			out.m_sMusicFile = value_data;
		}
		else if( value_name.Left(1) == "#"  
			 && IsAnInt( value_name.Mid(1,3) )
			 && IsAnInt( value_name.Mid(4,2) ) )	// this is step or offset data.  Looks like "#00705"
		{
			int iMeasureNo	= atoi( value_name.Mid(1,3) );
			int iBMSTrackNo	= atoi( value_name.Mid(4,2) );

			CString sNoteData = value_data;
			CArray<int, int> arrayNotes;

			for( int i=0; i<sNoteData.GetLength(); i+=2 )
			{
				CString sNote = sNoteData.Mid(i,2);
				int iNote;
				sscanf( sNote, "%x", &iNote );	// data is in hexadecimal
				arrayNotes.Add( iNote );
			}

			const int iNumNotesInThisMeasure = arrayNotes.GetSize();
			//LOG->Trace( "%s:%s: iMeasureNo = %d, iBMSTrackNo = %d, iNumNotesInThisMeasure = %d", 
			//	valuename, sNoteData, iMeasureNo, iBMSTrackNo, iNumNotesInThisMeasure );
			for( int j=0; j<iNumNotesInThisMeasure; j++ )
			{
				if( arrayNotes[j] == 0 )
					continue;

				float fPercentThroughMeasure = (float)j/(float)iNumNotesInThisMeasure;

				// index is in quarter beats starting at beat 0
				int iStepIndex = (int) ( (iMeasureNo + fPercentThroughMeasure)
								 * BEATS_PER_MEASURE * ROWS_PER_BEAT );

				switch( iBMSTrackNo )
				{
				case 1:	{ // background music track
					float fBeatOffset = fBeatOffset = NoteRowToBeat( (float)iStepIndex );
					if( fBeatOffset > 10 )	// some BPMs's play the music again at the end.  Why?  Who knows...
						break;
					float fBPS;
					fBPS = out.m_BPMSegments[0].m_fBPM/60.0f;
					out.m_fBeat0OffsetInSeconds = fBeatOffset / fBPS;
					//LOG->Trace( "Found offset to be index %d, beat %f", iStepIndex, NoteRowToBeat(iStepIndex) );
					break;
				}
				case 3:	{ // bpm change
					BPMSegment newSeg( NoteRowToBeat(iStepIndex), (float)arrayNotes[j] );
					out.AddBPMSegment( newSeg );
					LOG->Trace( "Inserting new BPM change at beat %f, BPM %f", newSeg.m_fStartBeat, newSeg.m_fBPM );
					break;
				}

				// Let me just take a moment to express how frustrated I am with the new, 
				// poorly-designed changes to the BMS format.
				//
				//
				// AAAAAAAAAAAAAAAAAAAAAAAAAAAAAaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaahhhhhhhhhhhhhhhhhhhhhhhh!!!!!!!!!!!!!!!
				//
				// Thank you.

				case 8:	{ // indirect bpm
					// This is a very inefficient way to parse, but it doesn't matter much
					// because this is only parsed on the first run after the song is installed.
					CString sTagToLookFor = ssprintf( "#BPM%02x", arrayNotes[j] );
					float fBPM = -1;


					// open the song file again and and look for this tag's value
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

						if( 0==stricmp(value_name, sTagToLookFor) )
						{
							fBPM = (float)atof( value_data );
							break;
						}
					}

					if( fBPM == -1 )	// we didn't find the line we were looking for
					{
						LOG->Trace( "WARNING:  Couldn't find tag '%s' in '%s'.", sTagToLookFor, sPath );
					}
					else
					{
						BPMSegment newSeg( NoteRowToBeat(iStepIndex), fBPM );
						out.AddBPMSegment( newSeg );
						LOG->Trace( "Inserting new BPM change at beat %f, BPM %f", newSeg.m_fStartBeat, newSeg.m_fBPM );
					}

					file.Close();
					break;
				}
				case 9:	{ // stop
					// This is a very inefficient way to parse, but it doesn't 
					// matter much because this is only parsed on the first run after the song is installed.
					CString sTagToLookFor = ssprintf( "#STOP%02x", arrayNotes[j] );
					float fFreezeStartBeat = NoteRowToBeat(iStepIndex);
					float fFreezeSecs = -1;


					// open the song file again and and look for this tag's value
					CStdioFile file;	
					if( !file.Open( sPath, CFile::modeRead|CFile::shareDenyNone ) )
						throw RageException( "Failed to open %s.", sPath );

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

						if( 0==stricmp(value_name, sTagToLookFor) )
						{
							// find the BPM at the time of this freeze
							float fBPM = -1;
							for( int i=0; i<out.m_BPMSegments.GetSize()-1; i++ )
							{
								if( out.m_BPMSegments[i].m_fStartBeat <= fFreezeStartBeat && 
									out.m_BPMSegments[i+1].m_fStartBeat > fFreezeStartBeat )
								{
									fBPM = out.m_BPMSegments[i].m_fBPM;
									break;
								}
							}
							// the BPM segment of this beat is the last BPM segment
							if( fBPM == -1 )
								fBPM = out.m_BPMSegments[out.m_BPMSegments.GetSize()-1].m_fBPM;

							fFreezeSecs = (float)atof(value_data)/(fBPM*0.81f);	// I have no idea what units these are in, so I experimented until finding this factor.
							break;
						}
					}

					if( fFreezeSecs == -1 )	// we didn't find the line we were looking for
					{
						LOG->Trace( "WARNING:  Couldn't find tag '%s' in '%s'.", sTagToLookFor, sPath );
					}
					else
					{
						StopSegment newSeg( fFreezeStartBeat, fFreezeSecs );
						out.AddStopSegment( newSeg );
						LOG->Trace( "Inserting new Freeze at beat %f, secs %f", newSeg.m_fStartBeat, newSeg.m_fStopSeconds );
					}

					file.Close();
					break;
				}
				}
			}
		}
	}

	for( i=0; i<out.m_BPMSegments.GetSize(); i++ )
		LOG->Trace( "There is a BPM change at beat %f, BPM %f, index %d", 
					out.m_BPMSegments[i].m_fStartBeat, out.m_BPMSegments[i].m_fBPM, i );

	file.Close();

	return TRUE;
}

