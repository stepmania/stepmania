#include "global.h"
#include "NotesLoaderBMS.h"
#include "NotesLoader.h"
#include "NoteData.h"
#include "GameConstantsAndTypes.h"
#include "RageUtil.h"
#include "RageLog.h"
#include "GameManager.h"
#include "RageException.h"
#include "RageFile.h"
#include "StepsUtil.h"
#include "song.h"
#include "Steps.h"

/*	BMS encoding:     tap-hold
	4&8panel:   Player1     Player2
	Left		11-51		21-61
	Down		13-53		23-63
	Up			15-55		25-65
	Right		16-56		26-66

	6panel:	   Player1
	Left		11-51
	Left+Up		12-52
	Down		13-53
	Up			14-54
	Up+Right	15-55
	Right		16-56

	Notice that 15 and 25 have double meanings!  What were they thinking???
	While reading in, use the 6 panel mapping.  After reading in, detect if 
	only 4 notes are used.  If so, shift the Up+Right column back to the Up
	column

	Hey, folks, BMSes are used for things BESIDES DDR steps,
	and so we're borking up BMSes that are for pnm/bm/etc.

	pnm-nine:   11-15,22-25
	pnm-five:   13-15,21-22
	bm-single:  11-16
	bm-double:  11-16,21-26
	bm-single7: 11-16,18-19
	bm-double7: 11-16,18-19,21-26,28-29

	So the magics for these are:
	pnm-nine: nothing >5, with 12, 14, 22 and/or 24
	pnm-five: nothing >5, with 14 and/or 22
	bm-*: can't tell difference between bm-single and dance-solo
		18/19 marks bm-single7, 28/29 marks bm-double7
		bm-double uses 21-26. */

static int iTracks[MAX_NOTE_TRACKS];

enum
{
	BMS_NULL_COLUMN = 0,
	BMS_P1_KEY1,
	BMS_P1_KEY2,
	BMS_P1_KEY3,
	BMS_P1_KEY4,
	BMS_P1_KEY5,
	BMS_P1_TURN,
	BMS_P1_KEY6,
	BMS_P1_KEY7,
	BMS_P2_KEY1,
	BMS_P2_KEY2,
	BMS_P2_KEY3,
	BMS_P2_KEY4,
	BMS_P2_KEY5,
	BMS_P2_TURN,
	BMS_P2_KEY6,
	BMS_P2_KEY7,
};


void BMSLoader::ResetTracksMagic()
{
	for( int i = 0; i<MAX_NOTE_TRACKS; i++ )
		iTracks[i] = 0;
}

void BMSLoader::PushTrackNumForMagic( int iTrackNum )
{
	int ix = (iTrackNum < 20) ? (iTrackNum - 11) : (iTrackNum - 12);
	iTracks[ix]++;
}

StepsType BMSLoader::CheckTracksMagic()
{
	int iTrackCount = 0;
	for (int ix = 0; ix<MAX_NOTE_TRACKS; ix++) {
		if(iTracks[ix] != 0) iTrackCount++;
	}
	/*	Panel counts:
		4 - DDR
		5 - PNM 5-key
		6 - DDR Solo, BM 5-key
		8 - DDR Double. BM 7-key
		9 - PNM 9-key, BM 7-key
		12 - BM Double 5-key
		16 - BM Double 7-key */
	switch(iTrackCount) {
	case 4:
		return STEPS_TYPE_DANCE_SINGLE;
	case 5:
		return STEPS_TYPE_PNM_FIVE;
	case 6:
		/*	No reason to return STEPS_TYPE_BM_SINGLE here...
			...at least, none that I can see.  Same data, no way to distinguish.
			We also don't need to autogen between them, though. */
		return STEPS_TYPE_DANCE_SOLO;
	case 8:
		// Could also be couple or 7-key.
		if (iTracks[7] == 0 && iTracks[8] == 0 && iTracks[1] == 0 && iTracks[3] == 
0)
			// these four tracks are IIDX-related
			return STEPS_TYPE_DANCE_DOUBLE;
		else
			return STEPS_TYPE_IIDX_SINGLE7;
	case 9:
		return STEPS_TYPE_PNM_NINE;
	case 12:
		return STEPS_TYPE_BM_DOUBLE;
	case 16:
		return STEPS_TYPE_IIDX_DOUBLE7;
	default:
		return STEPS_TYPE_INVALID;
	}
}

void BMSLoader::mapBMSTrackToDanceNote( int iBMSTrack, int &iDanceColOut, TapNote &tapNoteOut )
{
	if( iBMSTrack > 40 )
	{
		tapNoteOut = TAP_ORIGINAL_HOLD_HEAD;
		iBMSTrack -= 40;
	}
	else
	{
		tapNoteOut = TAP_ORIGINAL_TAP;
	}

	switch( iBMSTrack )
	{
	case 11:	iDanceColOut = BMS_P1_KEY1;				break;
	case 12:	iDanceColOut = BMS_P1_KEY2;				break;
	case 13:	iDanceColOut = BMS_P1_KEY3;				break;
	case 14:	iDanceColOut = BMS_P1_KEY4;				break;
	case 15:	iDanceColOut = BMS_P1_KEY5;				break;
	case 16:	iDanceColOut = BMS_P1_TURN;				break;
	case 18:	iDanceColOut = BMS_P1_KEY6;				break;
	case 19:	iDanceColOut = BMS_P1_KEY7;				break;
	case 21:	iDanceColOut = BMS_P2_KEY1;				break;
	case 22:	iDanceColOut = BMS_P2_KEY2;				break;
	case 23:	iDanceColOut = BMS_P2_KEY3;				break;
	case 24:	iDanceColOut = BMS_P2_KEY4;				break;
	case 25:	iDanceColOut = BMS_P2_KEY5;				break;
	case 26:	iDanceColOut = BMS_P2_TURN;				break;
	case 28:	iDanceColOut = BMS_P2_KEY6;				break;
	case 29:	iDanceColOut = BMS_P2_KEY7;				break;
	default:	iDanceColOut = -1;						break;
	}
}

bool BMSLoader::LoadFromBMSFile( const CString &sPath, Steps &out )
{
	LOG->Trace( "Steps::LoadFromBMSFile( '%s' )", sPath.c_str() );

	out.m_StepsType = STEPS_TYPE_INVALID;

	NoteData* pNoteData = new NoteData;
	pNoteData->SetNumTracks( MAX_NOTE_TRACKS );
	ResetTracksMagic();

	RageFile file;
	if( !file.Open(sPath) )
		RageException::Throw( "Failed to open \"%s\" for reading: %s", sPath.c_str(), file.GetError().c_str() );
	while( !file.AtEOF() )
	{
		CString line;
		if( file.GetLine( line ) == -1 )
		{
			LOG->Warn( "Error reading \"%s\": %s", sPath.c_str(), file.GetError().c_str() );
			delete pNoteData;
			return false;
		}

		StripCrnl(line);
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
			value_name = line.substr( 0, iIndexOfSeparator );
			value_data = line;	// the rest
			value_data.erase(0,iIndexOfSeparator+1);
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
				out.m_StepsType = STEPS_TYPE_DANCE_SINGLE;
				/*	if the mode should be solo, then we'll update m_DanceStyle below when 
					we read in step data */
				break;
			case 2:		// couple/battle
				out.m_StepsType = STEPS_TYPE_DANCE_COUPLE;
				break;
			case 3:		// double
				// Fix it if we find that.
				out.m_StepsType = STEPS_TYPE_DANCE_DOUBLE;
				break;
			default:
				LOG->Warn( "Invalid #PLAYER in \"%s\": \"%s\"", sPath.c_str(), value_data.c_str() );
				delete pNoteData;
				return false;
			}
		}
		if( -1 != value_name.Find("#title") )
		{
			value_data.MakeLower();

			// extract the Steps description (looks like 'Music <BASIC>')
			int iPosOpenBracket = value_data.Find( "<" );
			if( iPosOpenBracket == -1 )
				iPosOpenBracket = value_data.Find( "(" );
			int iPosCloseBracket = value_data.Find( ">" );
			if( iPosCloseBracket == -1 )
				iPosCloseBracket = value_data.Find( ")" );

			if( iPosOpenBracket != -1  &&  iPosCloseBracket != -1 )
				value_data = value_data.substr( iPosOpenBracket+1, 
				iPosCloseBracket-iPosOpenBracket-1 );
			LOG->Trace( "Steps description found to be '%s'", value_data.c_str() );

			out.SetDescription(value_data);

			// if there's a 6 in the description, it's probably part of "6panel" or "6-panel"
			if( value_data.Find("6") != -1 )
				out.m_StepsType = STEPS_TYPE_DANCE_SOLO;
		}
		if( -1 != value_name.Find("#playlevel") )
		{
			out.SetMeter(atoi(value_data));
		}
		else if( value_name.size() >= 6 && value_name[0] == '#'
			 && IsAnInt( value_name.substr(1,3) )
			 && IsAnInt( value_name.substr(4,2) ) )	// this is step or offset data.  Looks like "#00705"
		{
			int iMeasureNo	= atoi( value_name.substr(1,3).c_str() );
			int iTrackNum	= atoi( value_name.substr(4,2).c_str() );

			/*	fix for Pop N' and such, including "if there are six panels, then we have Solo" - check here,
				then put the correct step type later */
			PushTrackNumForMagic(iTrackNum);

			CString &sNoteData = value_data;
			vector<bool> arrayNotes;

			for( int i=0; i+1<sNoteData.GetLength(); i+=2 )
			{
				bool bThisIsANote = sNoteData.substr(i,2) != "00";
				arrayNotes.push_back( bThisIsANote );
			}

			const unsigned iNumNotesInThisMeasure = arrayNotes.size();

			for( unsigned j=0; j<iNumNotesInThisMeasure; j++ )
			{
				if( arrayNotes[j] )
				{
					float fPercentThroughMeasure = (float)j/(float)iNumNotesInThisMeasure;

					const int iNoteIndex = (int) ( (iMeasureNo + fPercentThroughMeasure)
									 * BEATS_PER_MEASURE * ROWS_PER_BEAT );
					int iColumnNumber;
					TapNote tapNoteOut;

					mapBMSTrackToDanceNote( iTrackNum, iColumnNumber, tapNoteOut );

					if( iColumnNumber != -1 )
						pNoteData->SetTapNote(iColumnNumber, iNoteIndex, tapNoteOut);
				}
			}
		}
	}

	// dance-couple is the only one we should retain unchanged.
	if( out.m_StepsType != STEPS_TYPE_DANCE_COUPLE)
	{
		out.m_StepsType = CheckTracksMagic();
	}

	// we're done reading in all of the BMS values
	if( out.m_StepsType == STEPS_TYPE_INVALID )
	{
		LOG->Warn( "Couldn't determine note type of file '%s'", sPath.c_str() );
		delete pNoteData;
		return false;
	}

	int iNumNewTracks = GameManager::StepsTypeToNumTracks( out.m_StepsType );
	int iTransformNewToOld[MAX_NOTE_TRACKS];

	int i;
	for( i = 0; i < MAX_NOTE_TRACKS; ++i)
		iTransformNewToOld[i] = -1;

	switch( out.m_StepsType )
	{
	// fix PNM &c.
	case STEPS_TYPE_DANCE_SINGLE:
		iTransformNewToOld[0] = BMS_P1_KEY1;
		iTransformNewToOld[1] = BMS_P1_KEY3;
		iTransformNewToOld[2] = BMS_P1_KEY5;
		iTransformNewToOld[3] = BMS_P1_TURN;
		break;
	case STEPS_TYPE_DANCE_DOUBLE:
	case STEPS_TYPE_DANCE_COUPLE:
		iTransformNewToOld[0] = BMS_P1_KEY1;
		iTransformNewToOld[1] = BMS_P1_KEY3;
		iTransformNewToOld[2] = BMS_P1_KEY5;
		iTransformNewToOld[3] = BMS_P1_TURN;
		iTransformNewToOld[4] = BMS_P2_KEY1;
		iTransformNewToOld[5] = BMS_P2_KEY3;
		iTransformNewToOld[6] = BMS_P2_KEY5;
		iTransformNewToOld[7] = BMS_P2_TURN;
		break;
	case STEPS_TYPE_DANCE_SOLO:
	case STEPS_TYPE_BM_SINGLE:
		// Hey! Why the hell are these exactly the same? :-)
		iTransformNewToOld[0] = BMS_P1_KEY1;
		iTransformNewToOld[1] = BMS_P1_KEY2;
		iTransformNewToOld[2] = BMS_P1_KEY3;
		iTransformNewToOld[3] = BMS_P1_KEY4;
		iTransformNewToOld[4] = BMS_P1_KEY5;
		iTransformNewToOld[5] = BMS_P1_TURN;
		break;
	case STEPS_TYPE_PNM_FIVE:
		iTransformNewToOld[0] = BMS_P1_KEY3;
		iTransformNewToOld[1] = BMS_P1_KEY4;
		iTransformNewToOld[2] = BMS_P1_KEY5;
		// fix these columns!
		iTransformNewToOld[3] = BMS_P2_KEY2;
		iTransformNewToOld[4] = BMS_P2_KEY3;
		break;
	case STEPS_TYPE_PNM_NINE:
		iTransformNewToOld[0] = BMS_P1_KEY1; // lwhite
		iTransformNewToOld[1] = BMS_P1_KEY2; // lyellow
		iTransformNewToOld[2] = BMS_P1_KEY3; // lgreen
		iTransformNewToOld[3] = BMS_P1_KEY4; // lblue
		iTransformNewToOld[4] = BMS_P1_KEY5; // red
		// fix these columns!
		iTransformNewToOld[5] = BMS_P2_KEY2; // rblue
		iTransformNewToOld[6] = BMS_P2_KEY3; // rgreen
		iTransformNewToOld[7] = BMS_P2_KEY4; // ryellow
		iTransformNewToOld[8] = BMS_P2_KEY5; // rwhite
		break;
	case STEPS_TYPE_BM_DOUBLE:
		iTransformNewToOld[0] = BMS_P1_KEY1;
		iTransformNewToOld[1] = BMS_P1_KEY2;
		iTransformNewToOld[2] = BMS_P1_KEY3;
		iTransformNewToOld[3] = BMS_P1_KEY4;
		iTransformNewToOld[4] = BMS_P1_KEY5;
		iTransformNewToOld[5] = BMS_P1_TURN;
		iTransformNewToOld[6] = BMS_P2_KEY1;
		iTransformNewToOld[7] = BMS_P2_KEY2;
		iTransformNewToOld[8] = BMS_P2_KEY3;
		iTransformNewToOld[9] = BMS_P2_KEY4;
		iTransformNewToOld[10] = BMS_P2_KEY5;
		iTransformNewToOld[11] = BMS_P2_TURN;
		break;
	case STEPS_TYPE_IIDX_SINGLE7:
		iTransformNewToOld[0] = BMS_P1_KEY1;
		iTransformNewToOld[1] = BMS_P1_KEY2;
		iTransformNewToOld[2] = BMS_P1_KEY3;
		iTransformNewToOld[3] = BMS_P1_KEY4;
		iTransformNewToOld[4] = BMS_P1_KEY5;
		iTransformNewToOld[5] = BMS_P1_KEY6;
		iTransformNewToOld[6] = BMS_P1_KEY7;
		iTransformNewToOld[7] = BMS_P1_TURN;
		break;
	case STEPS_TYPE_IIDX_DOUBLE7:
		iTransformNewToOld[0] = BMS_P1_KEY1;
		iTransformNewToOld[1] = BMS_P1_KEY2;
		iTransformNewToOld[2] = BMS_P1_KEY3;
		iTransformNewToOld[3] = BMS_P1_KEY4;
		iTransformNewToOld[4] = BMS_P1_KEY5;
		iTransformNewToOld[5] = BMS_P1_KEY6;
		iTransformNewToOld[6] = BMS_P1_KEY7;
		iTransformNewToOld[7] = BMS_P1_TURN;
		iTransformNewToOld[8] = BMS_P2_KEY1;
		iTransformNewToOld[9] = BMS_P2_KEY2;
		iTransformNewToOld[10] = BMS_P2_KEY3;
		iTransformNewToOld[11] = BMS_P2_KEY4;
		iTransformNewToOld[12] = BMS_P2_KEY5;
		iTransformNewToOld[13] = BMS_P2_KEY6;
		iTransformNewToOld[14] = BMS_P2_KEY7;
		iTransformNewToOld[15] = BMS_P2_TURN;
		break;
	default:
		ASSERT(0);
	}

	NoteData* pNoteData2 = new NoteData;
	pNoteData2->SetNumTracks( iNumNewTracks );
	pNoteData2->LoadTransformed( pNoteData, iNumNewTracks, iTransformNewToOld );

	out.SetNoteData(pNoteData2);

	delete pNoteData;
	delete pNoteData2;

	out.TidyUpData();

	return true;
}

void BMSLoader::GetApplicableFiles( CString sPath, CStringArray &out )
{
	GetDirListing( sPath + CString("*.bms"), out );
}

bool BMSLoader::LoadFromDir( CString sDir, Song &out )
{
	LOG->Trace( "Song::LoadFromBMSDir(%s)", sDir.c_str() );

	CStringArray arrayBMSFileNames;
	GetApplicableFiles( sDir, arrayBMSFileNames );

	/* We should have at least one; if we had none, we shouldn't have been
	 * called to begin with. */
	ASSERT( arrayBMSFileNames.size() );

	// load the Steps from the rest of the BMS files
	unsigned i;
	for( i=0; i<arrayBMSFileNames.size(); i++ )
	{
		Steps* pNewNotes = new Steps;

		const bool ok = LoadFromBMSFile( out.GetSongDir() + arrayBMSFileNames[i], 
			*pNewNotes );
		if( ok )
			out.AddSteps( pNewNotes );
		else
			delete pNewNotes;
	}

	SlideDuplicateDifficulties( out );

	CString sPath = out.GetSongDir() + arrayBMSFileNames[0];

	RageFile file;
	if( !file.Open(sPath) )
		RageException::Throw( "Failed to open \"%s\" for reading: %s", sPath.c_str(), file.GetError().c_str() );
	while( !file.AtEOF() )
	{
		CString line;
		if( file.GetLine( line ) == -1 )
		{
			LOG->Warn( "Error reading \"%s\": %s", sPath.c_str(), file.GetError().c_str() );
			return false;
		}

		StripCrnl(line);
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
			value_name = line.substr( 0, iIndexOfSeparator );
			value_data = line;	// the rest
			value_data.erase(0,iIndexOfSeparator+1);
		}
		else	// no separator
		{
			value_name = line;
		}


		value_name.MakeLower();


		// handle the data
		if( value_name == "#title" )
		{
			// strip Steps type out of description leaving only song title - looks like 'B4U <BASIC>'
			size_t iIndex = value_data.find_last_of('<');
			if( iIndex == value_data.npos )
				iIndex = value_data.find_last_of('(');
			if( iIndex != value_data.npos )
			{
				value_data = value_data.Left( iIndex );
				GetMainAndSubTitlesFromFullTitle( value_data, out.m_sMainTitle, 
					out.m_sSubTitle );
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
			BPMSegment newSeg( 0, strtof(value_data, NULL) );
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
		else if( value_name.size() >= 6 && value_name[0] == '#'
			 && IsAnInt( value_name.substr(1,3) )
			 && IsAnInt( value_name.substr(4,2) ) )	// this is step or offset data.  Looks like "#00705"
		{
			int iMeasureNo	= atoi( value_name.substr(1,3).c_str() );
			int iBMSTrackNo	= atoi( value_name.substr(4,2).c_str() );

			CString sNoteData = value_data;
			vector<int> arrayNotes;

			for( int i=0; i+1<sNoteData.GetLength(); i+=2 )
			{
				CString sNote = sNoteData.substr(i,2);
				int iNote;
				sscanf( sNote, "%x", &iNote );	// data is in hexadecimal
				arrayNotes.push_back( iNote );
			}

			const unsigned iNumNotesInThisMeasure = arrayNotes.size();
			for( unsigned j=0; j<iNumNotesInThisMeasure; j++ )
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
					fBPS = out.m_Timing.m_BPMSegments[0].m_fBPM/60.0f;
					out.m_Timing.m_fBeat0OffsetInSeconds = fBeatOffset / fBPS;
					break;
				}
				case 3:	{ // bpm change
					BPMSegment newSeg( NoteRowToBeat(iStepIndex), (float)arrayNotes[j] );
					out.AddBPMSegment( newSeg );
					LOG->Trace( "Inserting new BPM change at beat %f, BPM %f", newSeg.m_fStartBeat, newSeg.m_fBPM );
					break;
				}

				case 8:	{ // indirect bpm
					/*	This is a very inefficient way to parse, but it doesn't matter much
						because this is only parsed on the first run after the song is installed. */
					CString sTagToLookFor = ssprintf( "#BPM%02x", arrayNotes[j] );
					float fBPM = -1;


					// open the song file again and and look for this tag's value
					/* I don't like this. I think we should just seek back to the beginning
					 * rather than open the file again. However, I'm not changing the logic,
					 * only the implementation. -- Steve
					 */
					RageFile file(sPath); //Why doesn't VC6 bitch here but it does with int??

					if (!file.IsOpen())
						RageException::Throw( "Failed to open %s for reading.", sPath.c_str() );
					while (!file.AtEOF())
					{
						CString line;
						file.GetLine( line );
						StripCrnl(line);
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
							value_name = line.substr( 0, iIndexOfSeparator );
							value_data = line;	// the rest
							value_data.erase(0,iIndexOfSeparator+1);
						}
						else	// no separator
							value_name = line;

						if( 0==stricmp(value_name, sTagToLookFor) )
						{
							fBPM = strtof( value_data, NULL );
							break;
						}
					}

					if( fBPM == -1 )	// we didn't find the line we were looking for
						LOG->Warn( "Couldn't find tag '%s' in '%s'.", sTagToLookFor.c_str(), sPath.c_str() );
					else
					{
						BPMSegment newSeg( NoteRowToBeat(iStepIndex), fBPM );
						out.AddBPMSegment( newSeg );
						LOG->Trace( "Inserting new BPM change at beat %f, BPM %f", newSeg.m_fStartBeat, newSeg.m_fBPM );
					}

					break;
				}
				case 9:	{ // stop
					/*	This is a very inefficient way to parse, but it doesn't
						matter much because this is only parsed on the first run after the song is installed. */
					CString sTagToLookFor = ssprintf( "#STOP%02x", arrayNotes[j] );
					float fFreezeStartBeat = NoteRowToBeat(iStepIndex);
					float fFreezeSecs = -1;


					// open the song file again and and look for this tag's value
					RageFile file(sPath); //Why doesn't VC6 bitch here but it does with int??

					if (!file.IsOpen())
                        RageException::Throw( "Failed to open %s for reading.", sPath.c_str() );
					while (!file.AtEOF())
					{
						CString line;
						file.GetLine( line );
						StripCrnl(line);
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
							value_name = line.substr( 0, iIndexOfSeparator );
							value_data = line;	// the rest
							value_data.erase(0,iIndexOfSeparator+1);
						}
						else	// no separator
						{
							value_name = line;
						}

						if( 0==stricmp(value_name, sTagToLookFor) )
						{
							// find the BPM at the time of this freeze
							float fBPM = -1;
							for( unsigned i=0; i<out.m_Timing.m_BPMSegments.size()-1; i++ )
							{
								if( out.m_Timing.m_BPMSegments[i].m_fStartBeat <= fFreezeStartBeat &&
									out.m_Timing.m_BPMSegments[i+1].m_fStartBeat > fFreezeStartBeat )
								{
									fBPM = out.m_Timing.m_BPMSegments[i].m_fBPM;
									break;
								}
							}
							// the BPM segment of this beat is the last BPM segment
							if( fBPM == -1 )
								fBPM = out.m_Timing.m_BPMSegments[out.m_Timing.m_BPMSegments.size()-1].m_fBPM;

							fFreezeSecs = strtof(value_data,NULL)/(fBPM*0.81f);	// I have no idea what units these are in, so I experimented until finding this factor.
							break;
						}
					}

					if( fFreezeSecs == -1 )	// we didn't find the line we were looking for
					{
						LOG->Warn( "Couldn't find tag '%s' in '%s'.", sTagToLookFor.c_str(), sPath.c_str() );
					}
					else
					{
						StopSegment newSeg( fFreezeStartBeat, fFreezeSecs );
						out.AddStopSegment( newSeg );
						LOG->Trace( "Inserting new Freeze at beat %f, secs %f", newSeg.m_fStartBeat, newSeg.m_fStopSeconds );
					}

					break;
				}
				}
			}
		}
	}

	for( i=0; i<out.m_Timing.m_BPMSegments.size(); i++ )
		LOG->Trace( "There is a BPM change at beat %f, BPM %f, index %d",
					out.m_Timing.m_BPMSegments[i].m_fStartBeat, out.m_Timing.m_BPMSegments[i].m_fBPM, i );

	return true;
}


void BMSLoader::SlideDuplicateDifficulties( Song &p )
{
	/* BMS files have to guess the Difficulty from the meter; this is inaccurate,
	 * and often leads to duplicates.  Slide duplicate difficulties upwards.  We
	 * only do this with BMS files, since a very common bug was having *all*
	 * difficulties slid upwards due to (for example) having two beginner steps.
	 * We do a second pass in Song::TidyUpData to eliminate any remaining duplicates
	 * after this. */
	for( int i=0; i<NUM_STEPS_TYPES; i++ )
	{
		StepsType st = (StepsType)i;

		for( unsigned j=0; j<=DIFFICULTY_CHALLENGE; j++ ) // not DIFFICULTY_EDIT
		{
			Difficulty dc = (Difficulty)j;

			vector<Steps*> vSteps;
			p.GetSteps( vSteps, st, dc );

			StepsUtil::SortNotesArrayByDifficulty( vSteps );
			for( unsigned k=1; k<vSteps.size(); k++ )
			{
				Steps* pSteps = vSteps[k];
			
				Difficulty dc2 = min( (Difficulty)(dc+1), DIFFICULTY_CHALLENGE );
				pSteps->SetDifficulty( dc2 );
			}
		}
	}
}

/*
 * (c) 2001-2004 Chris Danford, Glenn Maynard
 * All rights reserved.
 * 
 * Permission is hereby granted, free of charge, to any person obtaining a
 * copy of this software and associated documentation files (the
 * "Software"), to deal in the Software without restriction, including
 * without limitation the rights to use, copy, modify, merge, publish,
 * distribute, and/or sell copies of the Software, and to permit persons to
 * whom the Software is furnished to do so, provided that the above
 * copyright notice(s) and this permission notice appear in all copies of
 * the Software and that both the above copyright notice(s) and this
 * permission notice appear in supporting documentation.
 * 
 * THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS
 * OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF
 * MERCHANTABILITY, FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT OF
 * THIRD PARTY RIGHTS. IN NO EVENT SHALL THE COPYRIGHT HOLDER OR HOLDERS
 * INCLUDED IN THIS NOTICE BE LIABLE FOR ANY CLAIM, OR ANY SPECIAL INDIRECT
 * OR CONSEQUENTIAL DAMAGES, OR ANY DAMAGES WHATSOEVER RESULTING FROM LOSS
 * OF USE, DATA OR PROFITS, WHETHER IN AN ACTION OF CONTRACT, NEGLIGENCE OR
 * OTHER TORTIOUS ACTION, ARISING OUT OF OR IN CONNECTION WITH THE USE OR
 * PERFORMANCE OF THIS SOFTWARE.
 */
