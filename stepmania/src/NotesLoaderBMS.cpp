#include "global.h"
#include "NotesLoaderBMS.h"
#include "NoteData.h"
#include "GameConstantsAndTypes.h"
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
	bm-single5: 11-16
	bm-double5: 11-16,21-26
	bm-single7: 11-16,18-19
	bm-double7: 11-16,18-19,21-26,28-29

	So the magics for these are:
	pnm-nine: nothing >5, with 12, 14, 22 and/or 24
	pnm-five: nothing >5, with 14 and/or 22
	bm-*: can't tell difference between bm-single and dance-solo
		18/19 marks bm-single7, 28/29 marks bm-double7
		bm-double uses 21-26. */

enum BmsTrack
{
	BMS_P1_KEY1 = 0,
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
	// max 4 simultaneous auto keysounds
	BMS_AUTO_KEYSOUND_1,
	BMS_AUTO_KEYSOUND_2,
	BMS_AUTO_KEYSOUND_3,
	BMS_AUTO_KEYSOUND_4,
	BMS_AUTO_KEYSOUND_5,
	BMS_AUTO_KEYSOUND_6,
	BMS_AUTO_KEYSOUND_7,
	BMS_AUTO_KEYSOUND_LAST,
	NUM_BMS_TRACKS,
};

const int NUM_NON_AUTO_KEYSOUND_TRACKS = BMS_AUTO_KEYSOUND_1;	
const int NUM_AUTO_KEYSOUND_TRACKS = NUM_BMS_TRACKS - NUM_NON_AUTO_KEYSOUND_TRACKS;	

static bool ConvertRawTrackToTapNote( int iRawTrack, BmsTrack &bmsTrackOut, bool &bIsHoldOut )
{
	if( iRawTrack > 40 )
	{
		bIsHoldOut = true;
		iRawTrack -= 40;
	}
	else
	{
		bIsHoldOut = false;
	}

	switch( iRawTrack )
	{
	case 1:  bmsTrackOut = BMS_AUTO_KEYSOUND_1;	break;
	case 11:	bmsTrackOut = BMS_P1_KEY1;		break;
	case 12:	bmsTrackOut = BMS_P1_KEY2;		break;
	case 13:	bmsTrackOut = BMS_P1_KEY3;		break;
	case 14:	bmsTrackOut = BMS_P1_KEY4;		break;
	case 15:	bmsTrackOut = BMS_P1_KEY5;		break;
	case 16:	bmsTrackOut = BMS_P1_TURN;		break;
	case 18:	bmsTrackOut = BMS_P1_KEY6;		break;
	case 19:	bmsTrackOut = BMS_P1_KEY7;		break;
	case 21:	bmsTrackOut = BMS_P2_KEY1;		break;
	case 22:	bmsTrackOut = BMS_P2_KEY2;		break;
	case 23:	bmsTrackOut = BMS_P2_KEY3;		break;
	case 24:	bmsTrackOut = BMS_P2_KEY4;		break;
	case 25:	bmsTrackOut = BMS_P2_KEY5;		break;
	case 26:	bmsTrackOut = BMS_P2_TURN;		break;
	case 28:	bmsTrackOut = BMS_P2_KEY6;		break;
	case 29:	bmsTrackOut = BMS_P2_KEY7;		break;
	default:	// unknown track
		return false;
	}
	return true;
}


static StepsType DetermineStepsType( int iPlayer, const NoteData &nd )
{
	ASSERT( NUM_BMS_TRACKS == nd.GetNumTracks() );

	bool bTrackHasNote[NUM_NON_AUTO_KEYSOUND_TRACKS];
	ZERO( bTrackHasNote );

	int iLastRow = nd.GetLastRow();
	for( int t=0; t<NUM_NON_AUTO_KEYSOUND_TRACKS; t++ )
	{
		for( int r=0; r<=iLastRow; r++ )
		{
			if( nd.GetTapNote(t, r).type != TapNote::empty )
			{
				bTrackHasNote[t] = true;
				break;				
			}
		}
	}

	int iNumNonEmptyTracks = 0;
	for( int t=0; t<NUM_NON_AUTO_KEYSOUND_TRACKS; t++ )
		if( bTrackHasNote[t] )
			iNumNonEmptyTracks++;

	switch( iPlayer )
	{
	case 1:		// "1 player"		
		/*	Track counts:
			4 - DDR
			5 - PNM 5-key
			6 - DDR Solo, BM 5-key
			8 - BM 7-key
			9 - PNM 9-key */
		switch( iNumNonEmptyTracks ) 
		{
		case 4:		return STEPS_TYPE_DANCE_SINGLE;
		case 5:		return STEPS_TYPE_PNM_FIVE;
		case 6:
			// FIXME: There's no way to distinguish between these types.
			// They use the same tracks.  Assume it's a BM type since they
			// are more common.
			//return STEPS_TYPE_DANCE_SOLO;
			return STEPS_TYPE_BM_SINGLE5;
		case 8:		return STEPS_TYPE_BM_SINGLE7;
		case 9:		return STEPS_TYPE_PNM_NINE;
		default:	return STEPS_TYPE_INVALID;
		}
		break;
	case 2:		// couple/battle
		return STEPS_TYPE_DANCE_COUPLE;
	case 3:		// double
	/*	Track counts:
		8 - DDR Double
		12 - BM Double 5-key
		16 - BM Double 7-key */
		switch( iNumNonEmptyTracks ) 
		{
		case 8:		return STEPS_TYPE_BM_SINGLE7;
		case 12:	return STEPS_TYPE_BM_DOUBLE5;
		case 16:	return STEPS_TYPE_BM_DOUBLE7;
		default:	return STEPS_TYPE_INVALID;
		}
		break;
	default:
		LOG->Warn( "Invalid #PLAYER value %d", iPlayer );
		return STEPS_TYPE_INVALID;
	}
}

bool BMSLoader::LoadFromBMSFile( const CString &sPath, Steps &out, const map<CString,int> &mapWavIdToKeysoundIndex )
{
	LOG->Trace( "Steps::LoadFromBMSFile( '%s' )", sPath.c_str() );

	out.m_StepsType = STEPS_TYPE_INVALID;

	// BMS player code.  Fill in below and use to determine StepsType.
	int iPlayer = -1;

	NoteData ndNotes;
	ndNotes.SetNumTracks( NUM_BMS_TRACKS );

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

		if( -1 != value_name.Find("#player") )
		{
			iPlayer = atoi(value_data);
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
		else if( value_name.size() == 6 && value_name[0] == '#'
			 && IsAnInt( value_name.substr(1,3) )
			 && IsAnInt( value_name.substr(4,2) ) )	// this is step or offset data.  Looks like "#00705"
		{
			int iMeasureNo		= atoi( value_name.substr(1,3).c_str() );
			int iRawTrackNum	= atoi( value_name.substr(4,2).c_str() );

			CString &sNoteData = value_data;
			vector<TapNote> vTapNotes;

			for( int i=0; i+1<sNoteData.GetLength(); i+=2 )
			{
				CString sNoteId = sNoteData.substr(i,2);
				if( sNoteId != "00" )
				{
					TapNote tn = TAP_ORIGINAL_TAP;
					map<CString,int>::const_iterator it = mapWavIdToKeysoundIndex.find(sNoteId);
					if( it != mapWavIdToKeysoundIndex.end() )
					{
						tn.bKeysound = true;
						tn.iKeysoundIndex = it->second;
					}
					vTapNotes.push_back( tn );
				}
				else
				{
					vTapNotes.push_back( TAP_EMPTY );
				}
			}

			const unsigned uNumNotesInThisMeasure = vTapNotes.size();

			for( unsigned j=0; j<uNumNotesInThisMeasure; j++ )
			{
				if( vTapNotes[j].type != TapNote::empty )
				{
					float fPercentThroughMeasure = (float)j/(float)uNumNotesInThisMeasure;

					int row = (int) ( (iMeasureNo + fPercentThroughMeasure)
						* BEATS_PER_MEASURE * ROWS_PER_BEAT );

					// some BMS files seem to have funky alignment, causing us to write gigantic cache files.
					// Try to correct for this by quantizing.
					row = Quantize( row, ROWS_PER_MEASURE/64 );
						
					BmsTrack bmsTrack;
					bool bIsHold;
					if( ConvertRawTrackToTapNote(iRawTrackNum, bmsTrack, bIsHold) )
					{
						TapNote tn = vTapNotes[j];

						if( bmsTrack == BMS_AUTO_KEYSOUND_1 )
						{
							tn.type = TapNote::autoKeysound;

							// shift the auto keysound as far right as possible
							int iLastEmptyTrack = -1;
							if( ndNotes.GetTapLastEmptyTrack(row,iLastEmptyTrack)  &&
								iLastEmptyTrack >= BMS_AUTO_KEYSOUND_1 )
							{
								bmsTrack = (BmsTrack)iLastEmptyTrack;
							}
							else
							{
								// no room for this note.  Drop it.
								continue;
							}
						}
						else if( bIsHold )
						{
							tn.type = TapNote::hold_head;
						}
						else
						{
							tn.type = TapNote::tap;
						}

						ndNotes.SetTapNote( bmsTrack, row, tn );
					}
				}
			}
		}
	}

	// we're done reading in all of the BMS values

	out.m_StepsType = DetermineStepsType( iPlayer, ndNotes );

	if( out.m_StepsType == STEPS_TYPE_INVALID )
	{
		LOG->Warn( "Couldn't determine note type of file '%s'", sPath.c_str() );
		return false;
	}


	// shift all of the autokeysound tracks onto the main tracks
	for( int t=BMS_AUTO_KEYSOUND_1; t<BMS_AUTO_KEYSOUND_1+NUM_AUTO_KEYSOUND_TRACKS; t++ )
	{
		FOREACH_NONEMPTY_ROW_IN_TRACK( ndNotes, t, row )
		{
			TapNote tn = ndNotes.GetTapNote( t, row );
			int iEmptyTrack;
			if( ndNotes.GetTapFirstEmptyTrack(row, iEmptyTrack) )
			{
				ndNotes.SetTapNote( iEmptyTrack, row, tn );
				ndNotes.SetTapNote( t, row, TAP_EMPTY );
			}
			else
			{
				LOG->Warn( "No room to shift." );
			}
		}
	}


	int iNumNewTracks = GameManager::StepsTypeToNumTracks( out.m_StepsType );
	vector<int> iTransformNewToOld;
	iTransformNewToOld.resize( iNumNewTracks, -1 );

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
	case STEPS_TYPE_BM_SINGLE5:
		// Hey! Why are these exactly the same? :-)
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
	case STEPS_TYPE_BM_DOUBLE5:
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
	case STEPS_TYPE_BM_SINGLE7:
		iTransformNewToOld[0] = BMS_P1_KEY1;
		iTransformNewToOld[1] = BMS_P1_KEY2;
		iTransformNewToOld[2] = BMS_P1_KEY3;
		iTransformNewToOld[3] = BMS_P1_KEY4;
		iTransformNewToOld[4] = BMS_P1_KEY5;
		iTransformNewToOld[5] = BMS_P1_KEY6;
		iTransformNewToOld[6] = BMS_P1_KEY7;
		iTransformNewToOld[7] = BMS_P1_TURN;
		break;
	case STEPS_TYPE_BM_DOUBLE7:
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

	NoteData noteData2;
	noteData2.SetNumTracks( iNumNewTracks );
	noteData2.LoadTransformed( ndNotes, iNumNewTracks, &*iTransformNewToOld.begin() );

	out.SetNoteData( noteData2 );

	out.TidyUpData();

	return true;
}

void BMSLoader::GetApplicableFiles( CString sPath, CStringArray &out )
{
	GetDirListing( sPath + CString("*.bms"), out );
	GetDirListing( sPath + CString("*.bme"), out );
}

bool BMSLoader::LoadFromDir( CString sDir, Song &out )
{
	LOG->Trace( "Song::LoadFromBMSDir(%s)", sDir.c_str() );

	ASSERT( out.m_vsKeysoundFile.empty() );

	CStringArray arrayBMSFileNames;
	GetApplicableFiles( sDir, arrayBMSFileNames );

	/* We should have at least one; if we had none, we shouldn't have been
	 * called to begin with. */
	ASSERT( arrayBMSFileNames.size() );

	// This maps from a BMS wav ID (e.g. "1A") to an entry in the Song's 
	// keysound vector.  Fill this in below while parsing the song data.
	map<CString,int> mapWavIdToKeysoundIndex;

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
		else if( value_name.size() == 6 && value_name.Left(4) == "#wav" )	// this is keysound file name.  Looks like "#WAV1A"
		{
			CString sWavID = value_name.Right(2);
			sWavID.MakeUpper();		// HACK: undo the MakeLower()
			out.m_vsKeysoundFile.push_back( value_data );
			mapWavIdToKeysoundIndex[ sWavID ] = out.m_vsKeysoundFile.size()-1;
			LOG->Trace( "Inserting keysound index %lu '%s'", out.m_vsKeysoundFile.size()-1, sWavID.c_str() );
		}
		else if( value_name.size() == 6 && value_name[0] == '#'
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
					RageFile file;
					if( !file.Open(sPath) )
						RageException::Throw( "Failed to open %s for reading: %s", sPath.c_str(), file.GetError().c_str() );
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
					RageFile file;
					if( !file.Open(sPath) )
						RageException::Throw( "Failed to open %s for reading: %s", sPath.c_str(), file.GetError().c_str() );
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

	for( unsigned i=0; i<out.m_Timing.m_BPMSegments.size(); i++ )
		LOG->Trace( "There is a BPM change at beat %f, BPM %f, index %d",
					out.m_Timing.m_BPMSegments[i].m_fStartBeat, out.m_Timing.m_BPMSegments[i].m_fBPM, i );


	// Now that we've parsed the keysound data, load the Steps from the rest 
	// of the .bms files.
	for( unsigned i=0; i<arrayBMSFileNames.size(); i++ )
	{
		Steps* pNewNotes = new Steps;

		const bool ok = LoadFromBMSFile( 
			out.GetSongDir() + arrayBMSFileNames[i], 
			*pNewNotes,
			mapWavIdToKeysoundIndex );
		if( ok )
			out.AddSteps( pNewNotes );
		else
			delete pNewNotes;
	}

	SlideDuplicateDifficulties( out );



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
