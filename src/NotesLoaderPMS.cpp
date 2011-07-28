#include "global.h"
#include "NotesLoaderPMS.h"
#include "NoteData.h"
#include "GameConstantsAndTypes.h"
#include "RageLog.h"
#include "GameManager.h"
#include "RageFile.h"
#include "SongUtil.h"
#include "StepsUtil.h"
#include "Song.h"
#include "Steps.h"
#include "RageUtil_CharConversions.h"
#include "NoteTypes.h"
#include "NotesLoader.h"

typedef multimap<RString, RString> NameToData_t;
typedef map<int, float> MeasureToTimeSig_t;

/*
 * popn-nine:		11-15,22-25
 * popn-five:   	13-15,21-22
 * 
 * So the magics for these are:
 * popn-nine: nothing >5, with 12, 14, 22 and/or 24
 * popn-five: nothing >5, with 14 and/or 22
*/

enum PmsTrack
{
	PMS_P1_KEY1 = 0,
	PMS_P1_KEY2,
	PMS_P1_KEY3,
	PMS_P1_KEY4,
	PMS_P1_KEY5,
	PMS_P1_KEY6,
	PMS_P1_KEY7,
	PMS_P1_KEY8,
	PMS_P1_KEY9,
	// max 4 simultaneous auto keysounds
	PMS_AUTO_KEYSOUND_1,
	PMS_AUTO_KEYSOUND_2,
	PMS_AUTO_KEYSOUND_3,
	PMS_AUTO_KEYSOUND_4,
	PMS_AUTO_KEYSOUND_5,
	PMS_AUTO_KEYSOUND_6,
	PMS_AUTO_KEYSOUND_7,
	PMS_AUTO_KEYSOUND_LAST,
	NUM_PMS_TRACKS,
};

const int NUM_NON_AUTO_KEYSOUND_TRACKS = PMS_AUTO_KEYSOUND_1;	
const int NUM_AUTO_KEYSOUND_TRACKS = NUM_PMS_TRACKS - NUM_NON_AUTO_KEYSOUND_TRACKS;	

static bool ConvertRawTrackToTapNote( int iRawTrack, PmsTrack &pmsTrackOut, bool &bIsHoldOut )
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
	case 1:  	pmsTrackOut = PMS_AUTO_KEYSOUND_1;	break;
	case 11:	pmsTrackOut = PMS_P1_KEY1;		break;
	case 12:	pmsTrackOut = PMS_P1_KEY2;		break;
	case 13:	pmsTrackOut = PMS_P1_KEY3;		break;
	case 14:	pmsTrackOut = PMS_P1_KEY4;		break;
	case 15:	pmsTrackOut = PMS_P1_KEY5;		break;
	case 22:	pmsTrackOut = PMS_P1_KEY6;		break;
	case 23:	pmsTrackOut = PMS_P1_KEY7;		break;
	case 24:	pmsTrackOut = PMS_P1_KEY8;		break;
	case 25:	pmsTrackOut = PMS_P1_KEY9;		break;
	default:	// unknown track
		return false;
	}
	return true;
}

// Find the largest common substring at the start of both strings.
static RString FindLargestInitialSubstring( const RString &string1, const RString &string2 )
{
	// First see if the whole first string matches an appropriately-sized
	// substring of the second, then keep chopping off the last character of
	// each until they match.
	unsigned i;
	for( i = 0; i < string1.size() && i < string2.size(); ++i )
		if( string1[i] != string2[i] )
			break;

	return string1.substr( 0, i );
}

static StepsType DetermineStepsType( int iPlayer, const NoteData &nd, const RString &sPath )
{
	ASSERT( NUM_PMS_TRACKS == nd.GetNumTracks() );

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
	case 1:
	case 3:
		switch( iNumNonEmptyTracks ) 
		{
			case 5:		return StepsType_popn_five;
			case 9:		return StepsType_popn_nine;
			default:	return StepsType_Invalid;
		}
	default:
		LOG->UserLog( "Song file", sPath, "has an invalid #PLAYER value %d.", iPlayer );
		return StepsType_Invalid;
	}
}

static bool GetTagFromMap( const NameToData_t &mapNameToData, const RString &sName, RString &sOut )
{
	NameToData_t::const_iterator it;
	it = mapNameToData.find( sName );
	if( it == mapNameToData.end() )
		return false;

	sOut = it->second;

	return true;
}

/* Finds the longest common match for the given tag in all files.  If the given tag
 * was found in at least one file, returns true; otherwise returns false. */
static bool GetCommonTagFromMapList( const vector<NameToData_t> &aPMSData, const RString &sName, RString &sOut )
{
	bool bFoundOne = false;
	for( unsigned i=0; i < aPMSData.size(); i++ )
	{
		RString sTag;
		if( !GetTagFromMap( aPMSData[i], sName, sTag ) )
			continue;

		if( !bFoundOne )
		{
			bFoundOne = true;
			sOut = sTag;
		}
		else
		{
			sOut = FindLargestInitialSubstring( sOut, sTag );
		}
	}

	return bFoundOne;
}


static float GetBeatsPerMeasure( const MeasureToTimeSig_t &sigs, int iMeasure, const MeasureToTimeSig_t &sigAdjustments )
{
	map<int, float>::const_iterator time_sig = sigs.find( iMeasure );

	float fRet = 4.0f;
	if( time_sig != sigs.end() )
		fRet *= time_sig->second;

	time_sig = sigAdjustments.find( iMeasure );
	if( time_sig != sigAdjustments.end() )
		fRet *= time_sig->second;

	return fRet;
}

static int GetMeasureStartRow( const MeasureToTimeSig_t &sigs, int iMeasureNo, const MeasureToTimeSig_t &sigAdjustments )
{
	int iRowNo = 0;
	for( int i = 0; i < iMeasureNo; ++i )
		iRowNo += BeatToNoteRow( GetBeatsPerMeasure(sigs, i, sigAdjustments) );
	return iRowNo;
}


static void SearchForDifficulty( RString sTag, Steps *pOut )
{
	sTag.MakeLower();

	/* Only match "Light" in parentheses. */
	if( sTag.find( "(light" ) != sTag.npos )
	{
		pOut->SetDifficulty( Difficulty_Easy );
	}
	else if( sTag.find( "another" ) != sTag.npos )
	{
		pOut->SetDifficulty( Difficulty_Hard );
	}
	else if( sTag.find( "(solo)" ) != sTag.npos )
	{
		pOut->SetDescription( "Solo" );
		pOut->SetDifficulty( Difficulty_Edit );
	}

	LOG->Trace( "Tag \"%s\" is %s", sTag.c_str(), DifficultyToString(pOut->GetDifficulty()).c_str() );
}

static bool ReadPMSFile( const RString &sPath, NameToData_t &mapNameToData )
{
	RageFile file;
	if( !file.Open(sPath) )
	{
		LOG->UserLog( "Song file", sPath, "couldn't be opened: %s", file.GetError().c_str() );
		return false;
	}

	while( !file.AtEOF() )
	{
		RString line;
		if( file.GetLine(line) == -1 )
		{
			LOG->UserLog( "Song file", sPath, "had a read error: %s", file.GetError().c_str() );
			return false;
		}

		StripCrnl( line );

		// PMS value names can be separated by a space or a colon.
		size_t iIndexOfSeparator = line.find_first_of( ": " );
		RString value_name = line.substr( 0, iIndexOfSeparator );
		RString value_data;
		if( iIndexOfSeparator != line.npos )
			value_data = line.substr( iIndexOfSeparator+1 );

		value_name.MakeLower();
		mapNameToData.insert( make_pair(value_name, value_data) );
	}

	return true;
}

enum
{
	PMS_TRACK_TIME_SIG = 2,
	PMS_TRACK_BPM = 3,
	PMS_TRACK_BPM_REF = 8,
	PMS_TRACK_STOP = 9
};

/*
 * Time signatures are often abused to tweak sync.  Real time signatures should
 * cause us to adjust the row offsets so one beat remains one beat.  Fake time signatures,
 * like 1.001 or 0.999, should be removed and converted to BPM changes.  This is much
 * more accurate, and prevents the whole song from being shifted off of the beat, causing
 * BeatToNoteType to be wrong.
 *
 * Evaluate each time signature, and guess which time signatures should be converted
 * to BPM changes.  This isn't perfect, but errors aren't fatal.
 */
static void SetTimeSigAdjustments( const MeasureToTimeSig_t &sigs, Song &out, MeasureToTimeSig_t &sigAdjustmentsOut )
{
	return;
#if 0
	sigAdjustmentsOut.clear();

	MeasureToTimeSig_t::const_iterator it;
	for( it = sigs.begin(); it != sigs.end(); ++it )
	{
		int iMeasure = it->first;
		float fFactor = it->second;

#if 1
		static const float ValidFactors[] =
		{
			0.25f,  /* 1/4 */
			0.5f,   /* 2/4 */
			0.75f,  /* 3/4 */
			0.875f, /* 7/8 */
			1.0f,
			1.5f,   /* 6/4 */
			1.75f   /* 7/4 */
		};

		bool bValidTimeSignature = false;
		for( unsigned i = 0; i < ARRAYLEN(ValidFactors); ++i )
			if( fabsf(fFactor-ValidFactors[i]) < 0.001 )
				bValidTimeSignature = true;

		if( bValidTimeSignature )
			continue;
#else
		/* Alternate approach that I tried first: see if the ratio is sane.  However,
		 * some songs have values like "1.4", which comes out to 7/4 and is not a valid
		 * time signature. */
		/* Convert the factor to a ratio, and reduce it. */
		int iNum = lrintf( fFactor * 1000 ), iDen = 1000;
		int iDiv = gcd( iNum, iDen );
		iNum /= iDiv;
		iDen /= iDiv;

		/* Real time signatures usually come down to 1/2, 3/4, 7/8, etc.  Bogus
		 * signatures that are only there to adjust sync usually look like 99/100. */
		if( iNum <= 8 && iDen <= 8 )
			continue;
#endif

		/* This time signature is bogus.  Convert it to a BPM adjustment for this
		 * measure. */
		LOG->Trace("Converted time signature %f in measure %i to a BPM segment.", fFactor, iMeasure );

		/* Note that this GetMeasureStartRow will automatically include any adjustments
		 * that we've made previously in this loop; as long as we make the timing
		 * adjustment and the BPM adjustment together, everything remains consistent.
		 * Adjust sigAdjustmentsOut first, or fAdjustmentEndBeat will be wrong. */
		sigAdjustmentsOut[iMeasure] = 1.0f / fFactor;
		int iAdjustmentStartRow = GetMeasureStartRow( sigs, iMeasure, sigAdjustmentsOut );
		int iAdjustmentEndRow = GetMeasureStartRow( sigs, iMeasure+1, sigAdjustmentsOut );
		out.m_Timing.MultiplyBPMInBeatRange( iAdjustmentStartRow, iAdjustmentEndRow, 1.0f / fFactor );
	}
#endif
}

static void ReadTimeSigs( const NameToData_t &mapNameToData, MeasureToTimeSig_t &out )
{
	NameToData_t::const_iterator it;
	for( it = mapNameToData.lower_bound("#00000"); it != mapNameToData.end(); ++it )
	{
		const RString &sName = it->first;
		if( sName.size() != 6 || sName[0] != '#' || !IsAnInt(sName.substr(1, 5)) )
			continue;

		// this is step or offset data.  Looks like "#00705"
		const RString &sData = it->second;
		int iMeasureNo	= StringToInt( sName.substr(1, 3) );
		int iPMSTrackNo	= StringToInt( sName.substr(4, 2) );
		if( iPMSTrackNo == PMS_TRACK_TIME_SIG )
			out[iMeasureNo] = StringToFloat( sData );
	}
}

static const int BEATS_PER_MEASURE = 4;
static const int ROWS_PER_MEASURE = ROWS_PER_BEAT * BEATS_PER_MEASURE;

static bool LoadFromPMSFile( const RString &sPath, const NameToData_t &mapNameToData, Steps &out,
			     const MeasureToTimeSig_t &sigAdjustments, const map<RString,int> &idToKeySoundIndex )
{
	LOG->Trace( "Steps::LoadFromPMSFile( '%s' )", sPath.c_str() );

	out.m_StepsType = StepsType_Invalid;

	// PMS player code.  Fill in below and use to determine StepsType.
	int iPlayer = -1;
	RString sData;
	if( GetTagFromMap( mapNameToData, "#player", sData ) )
		iPlayer = StringToInt(sData);
	if( GetTagFromMap( mapNameToData, "#playlevel", sData ) )
		out.SetMeter( StringToInt(sData) );

	NoteData ndNotes;
	ndNotes.SetNumTracks( NUM_PMS_TRACKS );

	/* Read time signatures.  Note that these can differ across files in the same
	 * song. */
	MeasureToTimeSig_t mapMeasureToTimeSig;
	ReadTimeSigs( mapNameToData, mapMeasureToTimeSig );

	int iHoldStarts[NUM_PMS_TRACKS];
	int iHoldPrevs[NUM_PMS_TRACKS];

	for( int i = 0; i < NUM_PMS_TRACKS; ++i )
	{
		iHoldStarts[i] = -1;
		iHoldPrevs[i] = -1;
	}

	NameToData_t::const_iterator it;
	for( it = mapNameToData.lower_bound("#00000"); it != mapNameToData.end(); ++it )
	{
		const RString &sName = it->first;
		if( sName.size() != 6 || sName[0] != '#' || !IsAnInt( sName.substr(1, 5) ) )
			 continue;

		// this is step or offset data.  Looks like "#00705"
		int iMeasureNo = StringToInt( sName.substr(1,3) );
		int iRawTrackNum = StringToInt( sName.substr(4,2) );
		int iRowNo = GetMeasureStartRow( mapMeasureToTimeSig, iMeasureNo, sigAdjustments );
		float fBeatsPerMeasure = GetBeatsPerMeasure( mapMeasureToTimeSig, iMeasureNo, sigAdjustments );
		const RString &sNoteData = it->second;

		vector<TapNote> vTapNotes;
		for( size_t i=0; i+1<sNoteData.size(); i+=2 )
		{
			RString sNoteId = sNoteData.substr( i, 2 );
			if( sNoteId != "00" )
			{
				vTapNotes.push_back( TAP_ORIGINAL_TAP );
				map<RString,int>::const_iterator rInt = idToKeySoundIndex.find( sNoteId );
				if( rInt != idToKeySoundIndex.end() )
					vTapNotes.back().iKeysoundIndex = rInt->second;
			}
			else
			{
				vTapNotes.push_back( TAP_EMPTY );
			}
		}

		const unsigned iNumNotesInThisMeasure = vTapNotes.size();
		for( unsigned j=0; j<iNumNotesInThisMeasure; j++ )
		{
			float fPercentThroughMeasure = (float)j/(float)iNumNotesInThisMeasure;

			int row = iRowNo + lrintf( fPercentThroughMeasure * fBeatsPerMeasure * ROWS_PER_BEAT );

			// some PMS files seem to have funky alignment, causing us to write gigantic cache files.
			// Try to correct for this by quantizing.
			row = Quantize( row, ROWS_PER_MEASURE/64 );

			PmsTrack pmsTrack;
			bool bIsHold;
			if( ConvertRawTrackToTapNote(iRawTrackNum, pmsTrack, bIsHold) )
			{
				TapNote &tn = vTapNotes[j];
				if( tn.type != TapNote::empty )
				{
					if( pmsTrack == PMS_AUTO_KEYSOUND_1 )
					{
						// shift the auto keysound as far right as possible
						int iLastEmptyTrack = -1;
						if( ndNotes.GetTapLastEmptyTrack(row, iLastEmptyTrack)  &&
						    iLastEmptyTrack >= PMS_AUTO_KEYSOUND_1 )
						{
							tn.type = TapNote::autoKeysound;
							pmsTrack = (PmsTrack)iLastEmptyTrack;
						}
						else
						{
							// no room for this note.  Drop it.
							continue;
						}
					}
					else if( bIsHold )
					{
						if( iHoldStarts[pmsTrack] == -1 )
						{
							// Start of a hold.
							iHoldStarts[pmsTrack] = row;
							iHoldPrevs[pmsTrack] = row;
						}
						else
						{
							// We're continuing a hold.
							iHoldPrevs[pmsTrack] = row;
						}
						continue;
					}
				}
				if( iHoldStarts[pmsTrack] != -1 )
				{
					// This is ending a hold.
					const int iBegin = iHoldStarts[pmsTrack];
					const int iEnd = iHoldPrevs[pmsTrack];
					
					if( iBegin < iEnd )
						ndNotes.AddHoldNote( pmsTrack, iBegin, iEnd, TAP_ORIGINAL_HOLD_HEAD );
					else
						ndNotes.SetTapNote( pmsTrack, iBegin, TAP_ORIGINAL_TAP );
					iHoldStarts[pmsTrack] = -1;
					iHoldPrevs[pmsTrack] = -1;
				}
				// Don't bother inserting empty taps.
				if( tn.type != TapNote::empty )
					ndNotes.SetTapNote( pmsTrack, row, tn );
			}
		}
	}

	// We're done reading in all of the PMS values. Time to check for any unfinished holds.
	for( int iTrack = 0; iTrack < NUM_PMS_TRACKS; ++iTrack )
	{
		const int iBegin = iHoldStarts[iTrack];
		const int iEnd = iHoldPrevs[iTrack];

		if( iBegin == -1 )
			continue;
		if( iBegin < iEnd )
			ndNotes.AddHoldNote( iTrack, iBegin, iEnd, TAP_ORIGINAL_HOLD_HEAD );
		else
			ndNotes.SetTapNote( iTrack, iBegin, TAP_ORIGINAL_TAP );
	}		

	out.m_StepsType = DetermineStepsType( iPlayer, ndNotes, sPath );

	if( out.m_StepsType == StepsType_Invalid )
	{
		LOG->UserLog( "Song file", sPath, "has an unknown steps type" );
		return false;
	}


	// shift all of the autokeysound tracks onto the main tracks
	for( int t=PMS_AUTO_KEYSOUND_1; t<PMS_AUTO_KEYSOUND_1+NUM_AUTO_KEYSOUND_TRACKS; t++ )
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
				LOG->UserLog( "Song file", sPath, "has no room to shift the autokeysound tracks." );
			}
		}
	}


	int iNumNewTracks = GAMEMAN->GetStepsTypeInfo( out.m_StepsType ).iNumTracks;
	vector<int> iTransformNewToOld;
	iTransformNewToOld.resize( iNumNewTracks, -1 );

	switch( out.m_StepsType )
	{
	case StepsType_popn_five:
		iTransformNewToOld[0] = PMS_P1_KEY3;
		iTransformNewToOld[1] = PMS_P1_KEY4;
		iTransformNewToOld[2] = PMS_P1_KEY5;
		iTransformNewToOld[3] = PMS_P1_KEY6;
		iTransformNewToOld[4] = PMS_P1_KEY7;
		break;
	case StepsType_popn_nine:
		iTransformNewToOld[0] = PMS_P1_KEY1; // lwhite
		iTransformNewToOld[1] = PMS_P1_KEY2; // lyellow
		iTransformNewToOld[2] = PMS_P1_KEY3; // lgreen
		iTransformNewToOld[3] = PMS_P1_KEY4; // lblue
		iTransformNewToOld[4] = PMS_P1_KEY5; // red
		iTransformNewToOld[5] = PMS_P1_KEY6; // rblue
		iTransformNewToOld[6] = PMS_P1_KEY7; // rgreen
		iTransformNewToOld[7] = PMS_P1_KEY8; // ryellow
		iTransformNewToOld[8] = PMS_P1_KEY9; // rwhite
		break;
	default:
		ASSERT(0);
	}

	NoteData noteData2;
	noteData2.SetNumTracks( iNumNewTracks );
	noteData2.LoadTransformed( ndNotes, iNumNewTracks, &*iTransformNewToOld.begin() );

	out.SetNoteData( noteData2 );

	out.TidyUpData();

	out.SetSavedToDisk( true );	// we're loading from disk, so this is by definintion already saved

	return true;
}

static void ReadGlobalTags( const RString &sPath, const NameToData_t &mapNameToData, Song &out, MeasureToTimeSig_t &sigAdjustmentsOut, map<RString,int> &idToKeySoundIndexOut )
{
	RString sData;
	if( GetTagFromMap(mapNameToData, "#title", sData) )
		NotesLoader::GetMainAndSubTitlesFromFullTitle( sData, out.m_sMainTitle, out.m_sSubTitle );

	GetTagFromMap( mapNameToData, "#artist", out.m_sArtist );
	GetTagFromMap( mapNameToData, "#genre", out.m_sGenre );
	GetTagFromMap( mapNameToData, "#backbmp", out.m_sBackgroundFile );
	GetTagFromMap( mapNameToData, "#wav", out.m_sMusicFile );

	if( GetTagFromMap(mapNameToData, "#bpm", sData) )
	{
		const float fBPM = StringToFloat( sData );

		if( fBPM > 0.0f )
		{
			out.m_SongTiming.AddSegment( SEGMENT_BPM, new BPMSegment(0, fBPM) );
			LOG->Trace( "Inserting new BPM change at beat %f, BPM %f", NoteRowToBeat(0), fBPM );
		}
		else
		{
			LOG->UserLog( "Song file", out.GetSongDir(), "has an invalid BPM change at beat %f, BPM %f.",
				      NoteRowToBeat(0), fBPM );
		}
	}

	NameToData_t::const_iterator it;
	for( it = mapNameToData.lower_bound("#wav"); it != mapNameToData.end(); ++it )
	{
		const RString &sName = it->first;

		if( sName.size() != 6 || sName.Left(4) != "#wav" )
			continue;

		// this is keysound file name.  Looks like "#WAV1A"
		RString nData = it->second;
		RString sWavID = sName.Right(2);
		RString dir = out.GetSongDir();
		if (dir.empty())
			dir = Dirname(sPath);

		/* Due to bugs in some programs, many PMS files have a "WAV" extension
		 * on files in the PMS for files that actually have some other extension.
		 * Do a search.  Don't do a wildcard search; if sData is "song.wav",
		 * we might also have "song.png", which we shouldn't match. */
		if( !IsAFile(dir+nData) )
		{
			const char *exts[] = { "oga", "ogg", "wav", "mp3", NULL }; // XXX: stop duplicating these everywhere
			for( unsigned i = 0; exts[i] != NULL; ++i )
			{
				RString fn = SetExtension( nData, exts[i] );
				if( IsAFile(dir+fn) )
				{
					nData = fn;
					break;
				}
			}
		}
		if( !IsAFile(dir+nData) )
			LOG->UserLog( "Song file", dir, "references key \"%s\" that can't be found", nData.c_str() );

		sWavID.MakeUpper();		// HACK: undo the MakeLower()
		out.m_vsKeysoundFile.push_back( nData );
		idToKeySoundIndexOut[ sWavID ] = out.m_vsKeysoundFile.size()-1;
		LOG->Trace( "Inserting keysound index %u '%s'", unsigned(out.m_vsKeysoundFile.size()-1), sWavID.c_str() );
	}

	/* Time signature tags affect all other global timing tags, so read them first. */
	MeasureToTimeSig_t mapMeasureToTimeSig;
	ReadTimeSigs( mapNameToData, mapMeasureToTimeSig );

	for( it = mapNameToData.lower_bound("#00000"); it != mapNameToData.end(); ++it )
	{
		const RString &sName = it->first;
		if( sName.size() != 6 || sName[0] != '#' || !IsAnInt( sName.substr(1,5) ) )
			 continue;
		// this is step or offset data.  Looks like "#00705"
		int iMeasureNo	= StringToInt( sName.substr(1, 3) );
		int iPMSTrackNo	= StringToInt( sName.substr(4, 2) );
		int iStepIndex = GetMeasureStartRow( mapMeasureToTimeSig, iMeasureNo, sigAdjustmentsOut );
		float fBeatsPerMeasure = GetBeatsPerMeasure( mapMeasureToTimeSig, iMeasureNo, sigAdjustmentsOut );
		int iRowsPerMeasure = BeatToNoteRow( fBeatsPerMeasure );

		RString nData = it->second;
		int totalPairs = nData.size() / 2;
		for( int i = 0; i < totalPairs; ++i )
		{
			RString sPair = nData.substr( i*2, 2 );

			int iVal = 0;
			if( sscanf( sPair, "%x", &iVal ) == 0 || iVal == 0 )
				continue;

			int iRow = iStepIndex + (i * iRowsPerMeasure) / totalPairs;
			float fBeat = NoteRowToBeat( iRow );

			switch( iPMSTrackNo )
			{
				case PMS_TRACK_BPM:
					if( iVal > 0 )
					{
						out.m_SongTiming.SetBPMAtBeat( fBeat, (float) iVal );
						LOG->Trace( "Inserting new BPM change at beat %f, BPM %i", fBeat, iVal );
					}
					else
					{
						LOG->UserLog( "Song file", out.GetSongDir(), "has an invalid BPM change at beat %f, BPM %d.",
								  fBeat, iVal );
					}
					break;

				case PMS_TRACK_BPM_REF:
				{
					RString sTagToLookFor = ssprintf( "#bpm%02x", iVal );
					RString sBPM;
					if( GetTagFromMap( mapNameToData, sTagToLookFor, sBPM ) )
					{
						float fBPM = StringToFloat( sBPM );

						if( fBPM > 0.0f )
						{
							BPMSegment * newSeg = new BPMSegment( fBeat, fBPM );
							out.m_SongTiming.AddSegment( SEGMENT_BPM, newSeg );
							LOG->Trace( "Inserting new BPM change at beat %f, BPM %f", fBeat, newSeg->GetBPM() );
						}
						else
						{
							LOG->UserLog( "Song file", out.GetSongDir(), "has an invalid BPM change at beat %f, BPM %f",
									  fBeat, fBPM );
						}
					}
					else
					{
						LOG->UserLog( "Song file", out.GetSongDir(), "has tag \"%s\" which cannot be found.", sTagToLookFor.c_str() );
					}
					break;
				}
				case PMS_TRACK_STOP:
				{
					RString sTagToLookFor = ssprintf( "#stop%02x", iVal );
					RString sBeats;
					if( GetTagFromMap( mapNameToData, sTagToLookFor, sBeats ) )
					{
						// find the BPM at the time of this freeze
						float fBPS = out.m_SongTiming.GetBPMAtBeat(fBeat) / 60.0f;
						float fBeats = StringToFloat( sBeats ) / 48.0f;
						float fFreezeSecs = fBeats / fBPS;

						StopSegment * newSeg = new StopSegment( fBeat, fFreezeSecs );
						out.m_SongTiming.AddSegment( SEGMENT_STOP, newSeg );
						LOG->Trace( "Inserting new Freeze at beat %f, secs %f", fBeat, newSeg->GetPause() );
					}
					else
					{
						LOG->UserLog( "Song file", out.GetSongDir(), "has tag \"%s\" which cannot be found.", sTagToLookFor.c_str() );
					}
					break;
				}
			}
		}

		switch( iPMSTrackNo )
		{
			case PMS_TRACK_BPM_REF:
			{
				// XXX: offset
				int iBPMNo;
				sscanf( nData, "%x", &iBPMNo );	// data is in hexadecimal

				RString sBPM;
				RString sTagToLookFor = ssprintf( "#bpm%02x", iBPMNo );
				if( GetTagFromMap( mapNameToData, sTagToLookFor, sBPM ) )
				{
					float fBPM = StringToFloat( sBPM );

					if( fBPM > 0.0f )
					{
						BPMSegment * newSeg = new BPMSegment( iStepIndex, fBPM );
						out.m_SongTiming.AddSegment( SEGMENT_BPM, newSeg );
						LOG->Trace("Inserting new BPM change at beat %f, BPM %f",
								   newSeg->GetBeat(),
								   newSeg->GetBPM() );
				
					}
					else
					{
						LOG->UserLog( "Song file", out.GetSongDir(), "has an invalid BPM change at beat %f, BPM %f.",
								  NoteRowToBeat(iStepIndex), fBPM );
					}
				}
				else
				{
					LOG->UserLog( "Song file", out.GetSongDir(), "has tag \"%s\" which cannot be found.", sTagToLookFor.c_str() );
				}

				break;
			}
		}
	}

	/* Now that we're done reading BPMs, factor out weird time signatures. */
	SetTimeSigAdjustments( mapMeasureToTimeSig, out, sigAdjustmentsOut );
}

static void SlideDuplicateDifficulties( Song &p )
{
	/* PMS files have to guess the Difficulty from the meter; this is inaccurate,
	* and often leads to duplicates.  Slide duplicate difficulties upwards.  We
	* only do this with PMS files, since a very common bug was having *all*
	* difficulties slid upwards due to (for example) having two beginner steps.
	* We do a second pass in Song::TidyUpData to eliminate any remaining duplicates
	* after this. */
	FOREACH_ENUM( StepsType,st )
	{
		FOREACH_ENUM( Difficulty, dc )
		{
			if( dc == Difficulty_Edit )
				continue;

			vector<Steps*> vSteps;
			SongUtil::GetSteps( &p, vSteps, st, dc );

			StepsUtil::SortNotesArrayByDifficulty( vSteps );
			for( unsigned k=1; k<vSteps.size(); k++ )
			{
				Steps* pSteps = vSteps[k];

				Difficulty dc2 = min( (Difficulty)(dc+1), Difficulty_Challenge );
				pSteps->SetDifficulty( dc2 );
			}
		}
	}
}

void PMSLoader::GetApplicableFiles( const RString &sPath, vector<RString> &out )
{
	GetDirListing( sPath + RString("*.pms"), out );
}

bool PMSLoader::LoadNoteDataFromSimfile(const RString &cachePath, Steps &out)
{
	Song dummy;
	// TODO: Simplify this copy/paste from LoadFromDir.
	
	vector<NameToData_t> BMSData;
	BMSData.push_back(NameToData_t());
	ReadPMSFile(cachePath, BMSData.back());
	
	RString commonSubstring;
	GetCommonTagFromMapList( BMSData, "#title", commonSubstring );
	
	Steps *copy = dummy.CreateSteps();
	
	copy->SetDifficulty( Difficulty_Medium );
	RString sTag;
	if( GetTagFromMap( BMSData[0], "#title", sTag ) && sTag.size() != commonSubstring.size() )
	{
		sTag = sTag.substr( commonSubstring.size(), sTag.size() - commonSubstring.size() );
		sTag.MakeLower();
		
		if( sTag.find('l') != sTag.npos )
		{
			unsigned lPos = sTag.find('l');
			if( lPos > 2 && sTag.substr(lPos-2,4) == "solo" )
			{
				copy->SetDifficulty( Difficulty_Edit );
			}
			else
			{
				copy->SetDifficulty( Difficulty_Easy );
			}
		}
		else if( sTag.find('a') != sTag.npos )
			copy->SetDifficulty( Difficulty_Hard );
		else if( sTag.find('b') != sTag.npos )
			copy->SetDifficulty( Difficulty_Beginner );
	}
	if( commonSubstring == "" )
	{
		copy->SetDifficulty(Difficulty_Medium);
		RString unused;
		if (GetTagFromMap(BMSData[0], "#title#", sTag))
			SearchForDifficulty(unused, copy);
	}
	MeasureToTimeSig_t sigAdjustments;
	map<RString,int> idToKeysoundIndex;
	ReadGlobalTags( cachePath, BMSData[0], dummy, sigAdjustments, idToKeysoundIndex );
	
	const bool ok = LoadFromPMSFile( cachePath, BMSData[0], *copy, sigAdjustments, idToKeysoundIndex );
	if( ok )
	{
		out.SetNoteData(copy->GetNoteData());
	}
	return ok;
	
}

bool PMSLoader::LoadFromDir( const RString &sDir, Song &out )
{
	LOG->Trace( "Song::LoadFromPMSDir(%s)", sDir.c_str() );

	ASSERT( out.m_vsKeysoundFile.empty() );

	vector<RString> arrayPMSFileNames;
	GetApplicableFiles( sDir, arrayPMSFileNames );

	/* We should have at least one; if we had none, we shouldn't have been
	 * called to begin with. */
	ASSERT( arrayPMSFileNames.size() );

	/* Read all PMS files. */
	vector<NameToData_t> aPMSData;
	for( unsigned i=0; i<arrayPMSFileNames.size(); i++ )
	{
		aPMSData.push_back( NameToData_t() );
		ReadPMSFile( out.GetSongDir() + arrayPMSFileNames[i], aPMSData.back() );
	}

	RString commonSubstring;
	GetCommonTagFromMapList( aPMSData, "#title", commonSubstring );

	if( commonSubstring == "" )
	{
		// All bets are off; the titles don't match at all.
		// At this rate we're lucky if we even get the title right.
		LOG->UserLog( "Song", sDir, "has PMS files with inconsistent titles." );
	}

	/* Create a Steps for each. */
	vector<Steps*> apSteps;
	for( unsigned i=0; i<arrayPMSFileNames.size(); i++ )
		apSteps.push_back( out.CreateSteps() );

	// Now, with our fancy little substring, trim the titles and
	// figure out where each goes.
	for( unsigned i=0; i<aPMSData.size(); i++ )
	{
		Steps *pSteps = apSteps[i];
		pSteps->SetDifficulty( Difficulty_Medium );
		RString sTag;
		if( GetTagFromMap( aPMSData[i], "#title", sTag ) && sTag.size() != commonSubstring.size() )
		{
			sTag = sTag.substr( commonSubstring.size(), sTag.size() - commonSubstring.size() );
			sTag.MakeLower();

			// XXX: We should do this with filenames too, I have plenty of examples.
			//      however, filenames will be trickier, as stuff at the beginning AND
			//      end change per-file, so we'll need a fancier FindLargestInitialSubstring()

			// XXX: This matches (double), but I haven't seen it used. Again, MORE EXAMPLES NEEDED
			if( sTag.find('l') != sTag.npos )
			{
				unsigned lPos = sTag.find('l');
				if( lPos > 2 && sTag.substr(lPos-2,4) == "solo" )
				{
					// (solo) -- an edit, apparently (Thanks Glenn!)
					pSteps->SetDifficulty( Difficulty_Edit );
				}
				else
				{
					// Any of [L7] [L14] (LIGHT7) (LIGHT14) (LIGHT) [L] <LIGHT7> <L7>... you get the idea.
					pSteps->SetDifficulty( Difficulty_Easy );
				}
			}
			// [A] <A> (A) [ANOTHER] <ANOTHER> (ANOTHER) (ANOTHER7) Another (DP ANOTHER) (Another) -ANOTHER- [A7] [A14] etc etc etc
			else if( sTag.find('a') != sTag.npos )
				pSteps->SetDifficulty( Difficulty_Hard );
			// XXX: Can also match (double), but should match [B] or [B7]
			else if( sTag.find('b') != sTag.npos )
				pSteps->SetDifficulty( Difficulty_Beginner );
			// Other tags I've seen here include (5KEYS) (10KEYS) (7keys) (14keys) (dp) [MIX] [14] (14 Keys Mix)
			// XXX: I'm sure [MIX] means something... anyone know?
		}
	}

	if( commonSubstring == "" )
	{
		// As said before, all bets are off.
		// From here on in, it's nothing but guesswork.

		/* Try to figure out the difficulty of each file. */
		for( unsigned i=0; i<arrayPMSFileNames.size(); i++ )
		{
			// XXX: Is this really effective if Common Substring parsing failed?
			Steps *pSteps = apSteps[i];
			pSteps->SetDifficulty( Difficulty_Medium );
			RString sTag;
			if( GetTagFromMap( aPMSData[i], "#title", sTag ) )
				SearchForDifficulty( sTag, pSteps );
		}
	}

	/* Prefer to read global tags from a Difficulty_Medium file.  These tend to
	 * have the least cruft in the #TITLE tag, so it's more likely to get a clean
	 * title. */
	int iMainDataIndex = 0;
	for( unsigned i=1; i<apSteps.size(); i++ )
		if( apSteps[i]->GetDifficulty() == Difficulty_Medium )
			iMainDataIndex = i;

	MeasureToTimeSig_t sigAdjustments;
	map<RString,int> idToKeysoundIndex;
	ReadGlobalTags( sDir, aPMSData[iMainDataIndex], out, sigAdjustments, idToKeysoundIndex );
	out.m_sSongFileName = out.GetSongDir() + arrayPMSFileNames[iMainDataIndex];

	// Override what that global tag said about the title if we have a good substring.
	// Prevents clobbering and catches "MySong (7keys)" / "MySong (Another) (7keys)"
	// Also catches "MySong (7keys)" / "MySong (14keys)"
	if( commonSubstring != "" )
		NotesLoader::GetMainAndSubTitlesFromFullTitle( commonSubstring, out.m_sMainTitle, out.m_sSubTitle );

	// Now that we've parsed the keysound data, load the Steps from the rest 
	// of the .pms files.
	for( unsigned i=0; i<arrayPMSFileNames.size(); i++ )
	{
		Steps* pNewNotes = apSteps[i];
		const bool ok = LoadFromPMSFile( out.GetSongDir() + arrayPMSFileNames[i], aPMSData[i], *pNewNotes, sigAdjustments, idToKeysoundIndex );
		if( ok )
		{
			pNewNotes->SetFilename(out.GetSongDir() + arrayPMSFileNames[i]);
			out.AddSteps( pNewNotes );
		}
		else
			delete pNewNotes;
	}

	SlideDuplicateDifficulties( out );

	ConvertString( out.m_sMainTitle, "utf-8,japanese" );
	ConvertString( out.m_sArtist, "utf-8,japanese" );
	ConvertString( out.m_sGenre, "utf-8,japanese" );

	out.TidyUpData(false, true);
	return true;
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
