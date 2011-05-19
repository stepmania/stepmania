#include "global.h"
#include "NotesLoaderSMA.h"
#include "BackgroundUtil.h"
#include "GameManager.h"
#include "MsdFile.h"
#include "NoteTypes.h"
#include "NotesLoaderSM.h" // may need this.
#include "PrefsManager.h"
#include "RageFileManager.h"
#include "RageLog.h"
#include "RageUtil.h"
#include "Song.h"
#include "SongManager.h"
#include "Steps.h"
#include "Attack.h"

/**
 * @brief A custom .edit file can only be so big before we have to reject it.
 */
const int MAX_EDIT_STEPS_SIZE_BYTES = 60*1024; // 60 KB

bool SMALoader::LoadFromBGChangesString( BackgroundChange &change, 
					const RString &sBGChangeExpression )
{
	return SMLoader::LoadFromBGChangesString(change, sBGChangeExpression);
}

bool SMALoader::LoadFromDir( const RString &sPath, Song &out )
{
	vector<RString> aFileNames;
	GetApplicableFiles( sPath, aFileNames );
	
	if( aFileNames.size() > 1 )
	{
		LOG->UserLog( "Song", sPath, "has more than one SMA file. Only one SMA file is allowed per song." );
		return false;
	}
	ASSERT( aFileNames.size() == 1 );
	return LoadFromSMAFile( sPath + aFileNames[0], out );
}

float SMALoader::RowToBeat( RString sLine, const int iRowsPerBeat )
{
	if( sLine.find("R") || sLine.find("r") )
	{
		sLine = sLine.Left(sLine.size()-1);
		return StringToFloat( sLine ) / iRowsPerBeat;
	}
	else
	{
		return StringToFloat( sLine );
	}
}

bool SMALoader::ProcessBPMs( TimingData &out, const int iRowsPerBeat, const RString sParam )
{
	vector<RString> arrayBPMChangeExpressions;
	split( sParam, ",", arrayBPMChangeExpressions );
	
	// prepare storage variables for negative BPMs -> Warps.
	float negBeat = -1;
	float negBPM = 1;
	float highspeedBeat = -1;
	bool bNotEmpty = false;
	
	for( unsigned b=0; b<arrayBPMChangeExpressions.size(); b++ )
	{
		vector<RString> arrayBPMChangeValues;
		split( arrayBPMChangeExpressions[b], "=", arrayBPMChangeValues );
		// XXX: Hard to tell which file caused this.
		if( arrayBPMChangeValues.size() != 2 )
		{
			LOG->UserLog( "Song file", "(UNKNOWN)", "has an invalid #BPMs value \"%s\" (must have exactly one '='), ignored.",
				     arrayBPMChangeExpressions[b].c_str() );
			continue;
		}
		
		bNotEmpty = true;
		
		const float fBeat = RowToBeat( arrayBPMChangeValues[0], iRowsPerBeat );
		const float fNewBPM = StringToFloat( arrayBPMChangeValues[1] );
		
		if( fNewBPM < 0.0f )
		{
			out.m_bHasNegativeBpms = true;
			negBeat = fBeat;
			negBPM = fNewBPM;
		}
		else if( fNewBPM > 0.0f )
		{
			// add in a warp.
			if( negBPM < 0 )
			{
				float endBeat = fBeat + (fNewBPM / -negBPM) * (fBeat - negBeat);
				WarpSegment new_seg(negBeat, endBeat - negBeat);
				out.AddWarpSegment( new_seg );
				
				negBeat = -1;
				negBPM = 1;
			}
			// too fast. make it a warp.
			if( fNewBPM > FAST_BPM_WARP )
			{
				highspeedBeat = fBeat;
			}
			else
			{
				// add in a warp.
				if( highspeedBeat > 0 )
				{
					WarpSegment new_seg(highspeedBeat, fBeat - highspeedBeat);
					out.AddWarpSegment( new_seg );
					highspeedBeat = -1;
				}
				{
					BPMSegment new_seg( BeatToNoteRow( fBeat ), fNewBPM );
					out.AddBPMSegment( new_seg );
				}
			}
		}
	}
	
	return bNotEmpty;
}

void SMALoader::ProcessStops( TimingData &out, const int iRowsPerBeat, const RString sParam )
{
	vector<RString> arrayFreezeExpressions;
	split( sParam, ",", arrayFreezeExpressions );
	
	// Prepare variables for negative stop conversion.
	float negBeat = -1;
	float negPause = 0;
	
	for( unsigned f=0; f<arrayFreezeExpressions.size(); f++ )
	{
		vector<RString> arrayFreezeValues;
		split( arrayFreezeExpressions[f], "=", arrayFreezeValues );
		if( arrayFreezeValues.size() != 2 )
		{
			// XXX: Hard to tell which file caused this.
			LOG->UserLog( "Song file", "(UNKNOWN)", "has an invalid #STOPS value \"%s\" (must have exactly one '='), ignored.",
				     arrayFreezeExpressions[f].c_str() );
			continue;
		}
		
		const float fFreezeBeat = RowToBeat( arrayFreezeValues[0], iRowsPerBeat );
		const float fFreezeSeconds = StringToFloat( arrayFreezeValues[1] );
		
		// Process the prior stop.
		if( negPause > 0 )
		{
			BPMSegment oldBPM = out.GetBPMSegmentAtRow(BeatToNoteRow(negBeat));
			float fSecondsPerBeat = 60 / oldBPM.GetBPM();
			float fSkipBeats = negPause / fSecondsPerBeat;
			
			if( negBeat + fSkipBeats > fFreezeBeat )
				fSkipBeats = fFreezeBeat - negBeat;
			
			WarpSegment ws( negBeat, fSkipBeats);
			out.AddWarpSegment( ws );
			
			negBeat = -1;
			negPause = 0;
		}
		
		if( fFreezeSeconds < 0.0f )
		{
			negBeat = fFreezeBeat;
			negPause = -fFreezeSeconds;
		}
		else if( fFreezeSeconds > 0.0f )
		{
			StopSegment ss( BeatToNoteRow(fFreezeBeat), fFreezeSeconds );
			out.AddStopSegment( ss );
		}
		
	}
	
	// Process the prior stop if there was one.
	if( negPause > 0 )
	{
		BPMSegment oldBPM = out.GetBPMSegmentAtRow(BeatToNoteRow(negBeat));
		float fSecondsPerBeat = 60 / oldBPM.GetBPM();
		float fSkipBeats = negPause / fSecondsPerBeat;
		
		WarpSegment ws( negBeat, fSkipBeats);
		out.AddWarpSegment( ws );
	}
}

void SMALoader::ProcessDelays( TimingData &out, const int iRowsPerBeat, const RString sParam )
{
	vector<RString> arrayDelayExpressions;
	split( sParam, ",", arrayDelayExpressions );
	
	for( unsigned f=0; f<arrayDelayExpressions.size(); f++ )
	{
		vector<RString> arrayDelayValues;
		split( arrayDelayExpressions[f], "=", arrayDelayValues );
		if( arrayDelayValues.size() != 2 )
		{
			// XXX: Hard to tell which file caused this.
			LOG->UserLog( "Song file", "(UNKNOWN)", "has an invalid #DELAYS value \"%s\" (must have exactly one '='), ignored.",
				     arrayDelayExpressions[f].c_str() );
			continue;
		}
		
		const float fFreezeBeat = RowToBeat( arrayDelayValues[0], iRowsPerBeat );
		const float fFreezeSeconds = StringToFloat( arrayDelayValues[1] );
		
		StopSegment new_seg( BeatToNoteRow(fFreezeBeat), fFreezeSeconds, true );
		// XXX: Remove Negatives Bug?
		new_seg.m_iStartRow = BeatToNoteRow(fFreezeBeat);
		new_seg.m_fStopSeconds = fFreezeSeconds;
		
		// LOG->Trace( "Adding a delay segment: beat: %f, seconds = %f", new_seg.m_fStartBeat, new_seg.m_fStopSeconds );
		
		if(fFreezeSeconds > 0.0f)
			out.AddStopSegment( new_seg );
		else
			LOG->UserLog( "Song file", "(UNKNOWN)", "has an invalid delay at beat %f, length %f.", fFreezeBeat, fFreezeSeconds );
	}
}

void SMALoader::ProcessTickcounts( TimingData &out, const int iRowsPerBeat, const RString sParam )
{
	vector<RString> arrayTickcountExpressions;
	split( sParam, ",", arrayTickcountExpressions );
	
	for( unsigned f=0; f<arrayTickcountExpressions.size(); f++ )
	{
		vector<RString> arrayTickcountValues;
		split( arrayTickcountExpressions[f], "=", arrayTickcountValues );
		if( arrayTickcountValues.size() != 2 )
		{
			// XXX: Hard to tell which file caused this.
			LOG->UserLog( "Song file", "(UNKNOWN)", "has an invalid #TICKCOUNTS value \"%s\" (must have exactly one '='), ignored.",
				     arrayTickcountExpressions[f].c_str() );
			continue;
		}
		
		const float fTickcountBeat = RowToBeat( arrayTickcountValues[0], iRowsPerBeat );
		int iTicks = clamp(atoi( arrayTickcountValues[1] ), 0, ROWS_PER_BEAT);
		
		TickcountSegment new_seg( BeatToNoteRow(fTickcountBeat), iTicks );
		out.AddTickcountSegment( new_seg );
	}
}

void SMALoader::ProcessMultipliers( TimingData &out, const int iRowsPerBeat, const RString sParam )
{
	vector<RString> arrayMultiplierExpressions;
	split( sParam, ",", arrayMultiplierExpressions );
	
	for( unsigned f=0; f<arrayMultiplierExpressions.size(); f++ )
	{
		vector<RString> arrayMultiplierValues;
		split( arrayMultiplierExpressions[f], "=", arrayMultiplierValues );
		if( arrayMultiplierValues.size() != 2 )
		{
			LOG->UserLog( "Song file", "(UNKNOWN)", "has an invalid #MULTIPLIER value \"%s\" (must have exactly one '='), ignored.",
				     arrayMultiplierExpressions[f].c_str() );
			continue;
		}
		const float fComboBeat = RowToBeat( arrayMultiplierValues[0], iRowsPerBeat );
		const int iCombos = StringToInt( arrayMultiplierValues[1] );
		ComboSegment new_seg( BeatToNoteRow( fComboBeat ), iCombos );
		out.AddComboSegment( new_seg );
	}
}

void SMALoader::ProcessBeatsPerMeasure( TimingData &out, const RString sParam )
{
	vector<RString> vs1;
	split( sParam, ",", vs1 );
	
	FOREACH_CONST( RString, vs1, s1 )
	{
		vector<RString> vs2;
		split( *s1, "=", vs2 );
		
		if( vs2.size() < 2 )
		{
			LOG->UserLog( "Song file", "(UNKNOWN)", "has an invalid beats per measure change with %i values.", (int)vs2.size() );
			continue;
		}
		
		const float fBeat = StringToFloat( vs2[0] );
		
		TimeSignatureSegment seg( BeatToNoteRow( fBeat ), StringToInt( vs2[1] ), 4 );
		
		if( fBeat < 0 )
		{
			LOG->UserLog( "Song file", "(UNKNOWN)", "has an invalid time signature change with beat %f.", fBeat );
			continue;
		}
		
		if( seg.m_iNumerator < 1 )
		{
			LOG->UserLog( "Song file", "(UNKNOWN)", "has an invalid time signature change with beat %f, iNumerator %i.", fBeat, seg.m_iNumerator );
			continue;
		}
		
		out.AddTimeSignatureSegment( seg );
	}
}

float BeatToSeconds(float fromBeat, RString toSomething)
{
	return 0;
}

void SMALoader::ProcessSpeeds( TimingData &out, const int iRowsPerBeat, const RString sParam )
{
	vector<RString> vs1;
	split( sParam, ",", vs1 );
	
	FOREACH_CONST( RString, vs1, s1 )
	{
		vector<RString> vs2;
		split( *s1, "=", vs2 );
		
		if( RowToBeat(vs2[0], iRowsPerBeat) == 0 && vs2.size() == 2 ) // First one always seems to have 2.
		{
			vs2.push_back("0");
		}
		
		if( vs2.size() < 3 )
		{
			LOG->UserLog( "Song file", "(UNKNOWN)", "has an speed change with %i values.", (int)vs2.size() );
			continue;
		}
		
		const float fBeat = RowToBeat( vs2[0], iRowsPerBeat );
		
		unsigned short tmp = ( (vs2[2].find("s") || vs2[2].find("S") )
				      ? 1 : 0);
		
		SpeedSegment seg( fBeat, StringToFloat( vs2[1] ), StringToFloat( vs2[2] ), tmp);
		
		if( fBeat < 0 )
		{
			LOG->UserLog( "Song file", "(UNKNOWN)", "has an speed change with beat %f.", fBeat );
			continue;
		}
		
		if( seg.m_fWait < 0 )
		{
			LOG->UserLog( "Song file", "(UNKNOWN)", "has an speed change with beat %f, fWait %f.", fBeat, seg.m_fWait );
			continue;
		}
		
		out.AddSpeedSegment( seg );
	}
}

void SMALoader::ProcessFakes( TimingData &out, const int iRowsPerBeat, const RString sParam )
{
	vector<RString> arrayFakeExpressions;
	split( sParam, ",", arrayFakeExpressions );
	
	for( unsigned b=0; b<arrayFakeExpressions.size(); b++ )
	{
		vector<RString> arrayFakeValues;
		split( arrayFakeExpressions[b], "=", arrayFakeValues );
		// XXX: Hard to tell which file caused this.
		if( arrayFakeValues.size() != 2 )
		{
			LOG->UserLog( "Song file", "(UNKNOWN)", "has an invalid #FAKES value \"%s\" (must have exactly one '='), ignored.",
				     arrayFakeExpressions[b].c_str() );
			continue;
		}
		
		const float fBeat = RowToBeat( arrayFakeValues[0], iRowsPerBeat );
		const float fNewBeat = StringToFloat( arrayFakeValues[1] );
		
		if(fNewBeat > 0)
			out.AddFakeSegment( FakeSegment(fBeat, fNewBeat) );
		else
		{
			LOG->UserLog( "Song file", "(UNKNOWN)", "has an invalid Fake at beat %f, BPM %f.", fBeat, fNewBeat );
		}
	}
}


void SMALoader::LoadFromSMATokens(
				  RString sStepsType,
				  RString sDescription,
				  RString sDifficulty,
				  RString sMeter,
				  RString sRadarValues,
				  RString sNoteData,
				  Steps &out
)
{
	SMLoader::LoadFromSMTokens( sStepsType, sDescription,
				    sDifficulty, sMeter, sRadarValues,
				    sNoteData, out );
}

void SMALoader::TidyUpData( Song &song, bool bFromCache )
{
	SMLoader::TidyUpData( song, bFromCache );
}

bool SMALoader::LoadFromSMAFile( const RString &sPath, Song &out )
{
	LOG->Trace( "Song::LoadFromSMAFile(%s)", sPath.c_str() );
	
	MsdFile msd;
	if( !msd.ReadFile( sPath, true ) )  // unescape
	{
		LOG->UserLog( "Song file", sPath, "couldn't be opened: %s", msd.GetError().c_str() );
		return false;
	}
	
	out.m_SongTiming.m_sFile = sPath; // songs still have their fallback timing.
	
	int state = SMA_GETTING_SONG_INFO;
	Steps* pNewNotes = NULL;
	TimingData stepsTiming;
	int iRowsPerBeat = -1; // Start with an invalid value: needed for checking.
	
	for( unsigned i=0; i<msd.GetNumValues(); i++ )
	{
		int iNumParams = msd.GetNumParams(i);
		const MsdFile::value_t &sParams = msd.GetValue(i);
		RString sValueName = sParams[0];
		sValueName.MakeUpper();
		
		// handle the data
		/* Don't use GetMainAndSubTitlesFromFullTitle; that's only for heuristically
		 * splitting other formats that *don't* natively support #SUBTITLE. */
		if( sValueName=="TITLE" )
			out.m_sMainTitle = sParams[1];
		
		else if( sValueName=="SUBTITLE" )
			out.m_sSubTitle = sParams[1];
		
		else if( sValueName=="ARTIST" )
			out.m_sArtist = sParams[1];
		
		else if( sValueName=="TITLETRANSLIT" )
			out.m_sMainTitleTranslit = sParams[1];
		
		else if( sValueName=="SUBTITLETRANSLIT" )
			out.m_sSubTitleTranslit = sParams[1];
		
		else if( sValueName=="ARTISTTRANSLIT" )
			out.m_sArtistTranslit = sParams[1];
		
		else if( sValueName=="GENRE" )
			out.m_sGenre = sParams[1];
		
		else if( sValueName=="CREDIT" )
			out.m_sCredit = sParams[1];
		
		else if( sValueName=="BANNER" )
			out.m_sBannerFile = sParams[1];
		
		else if( sValueName=="BACKGROUND" )
			out.m_sBackgroundFile = sParams[1];
		
		// Save "#LYRICS" for later, so we can add an internal lyrics tag.
		else if( sValueName=="LYRICSPATH" )
			out.m_sLyricsFile = sParams[1];
		
		else if( sValueName=="CDTITLE" )
			out.m_sCDTitleFile = sParams[1];
		
		else if( sValueName=="MUSIC" )
			out.m_sMusicFile = sParams[1];
		
		else if( sValueName=="INSTRUMENTTRACK" )
		{
			SMLoader::ProcessInstrumentTracks( out, sParams[1] );
		}
		
		else if( sValueName=="MUSICLENGTH" )
		{
			continue;
		}
		
		else if( sValueName=="LASTBEATHINT" )
			out.m_fSpecifiedLastBeat = StringToFloat( sParams[1] );
		
		else if( sValueName=="MUSICBYTES" )
			; /* ignore */
		
		/* We calculate these.  Some SMs in circulation have bogus values for
		 * these, so make sure we always calculate it ourself. */
		else if( sValueName=="FIRSTBEAT" )
		{
			;
		}
		else if( sValueName=="LASTBEAT" )
		{
			;
		}
		else if( sValueName=="SONGFILENAME" )
		{
			;
		}
		else if( sValueName=="HASMUSIC" )
		{
			;
		}
		else if( sValueName=="HASBANNER" )
		{
			;
		}
		
		else if( sValueName=="SAMPLESTART" )
			out.m_fMusicSampleStartSeconds = HHMMSSToSeconds( sParams[1] );
		
		else if( sValueName=="SAMPLELENGTH" )
			out.m_fMusicSampleLengthSeconds = HHMMSSToSeconds( sParams[1] );
		
		// SamplePath is used when the song has a separate preview clip. -aj
		//else if( sValueName=="SAMPLEPATH" )
		//out.m_sMusicSamplePath = sParams[1];
		
		else if( sValueName=="LISTSORT" )
		{
			;
		}
		
		else if( sValueName=="DISPLAYBPM" )
		{
			// #DISPLAYBPM:[xxx][xxx:xxx]|[*]; 
			if( sParams[1] == "*" )
				out.m_DisplayBPMType = DISPLAY_BPM_RANDOM;
			else 
			{
				out.m_DisplayBPMType = DISPLAY_BPM_SPECIFIED;
				out.m_fSpecifiedBPMMin = StringToFloat( sParams[1] );
				if( sParams[2].empty() )
					out.m_fSpecifiedBPMMax = out.m_fSpecifiedBPMMin;
				else
					out.m_fSpecifiedBPMMax = StringToFloat( sParams[2] );
			}
		}
		
		else if( sValueName=="SMAVERSION" )
		{
			; // ignore it.
		}
		
		else if( sValueName=="ROWSPERBEAT" )
		{
			/* This value is used to help translate the timings
			 * the SMA format uses. Starting with the second
			 * appearance, it delimits NoteData. Right now, this
			 * value doesn't seem to be editable in SMA. When it
			 * becomes so, make adjustments to this code. */
			if( iRowsPerBeat < 0 )
			{
				vector<RString> arrayBeatChangeExpressions;
				split( sParams[1], ",", arrayBeatChangeExpressions );
				
				vector<RString> arrayBeatChangeValues;
				split( arrayBeatChangeExpressions[0], "=", arrayBeatChangeValues );
				iRowsPerBeat = StringToInt(arrayBeatChangeValues[1]);
			}
			else
			{
				state = SMA_GETTING_STEP_INFO;
				pNewNotes = new Steps;
			}
		}
		
		else if( sValueName=="BEATSPERMEASURE" )
		{
			TimingData &timing = (state == SMA_GETTING_STEP_INFO 
					      ? pNewNotes->m_Timing : out.m_SongTiming);
			ProcessBeatsPerMeasure( timing, sParams[1] );
		}
		
		else if( sValueName=="SELECTABLE" )
		{
			if(sParams[1].EqualsNoCase("YES"))
				out.m_SelectionDisplay = out.SHOW_ALWAYS;
			else if(sParams[1].EqualsNoCase("NO"))
				out.m_SelectionDisplay = out.SHOW_NEVER;
			// ROULETTE from 3.9. It was removed since UnlockManager can serve
			// the same purpose somehow. This, of course, assumes you're using
			// unlocks. -aj
			else if(sParams[1].EqualsNoCase("ROULETTE"))
				out.m_SelectionDisplay = out.SHOW_ALWAYS;
			/* The following two cases are just fixes to make sure simfiles that
			 * used 3.9+ features are not excluded here */
			else if(sParams[1].EqualsNoCase("ES") || sParams[1].EqualsNoCase("OMES"))
				out.m_SelectionDisplay = out.SHOW_ALWAYS;
			else if( StringToInt(sParams[1]) > 0 )
				out.m_SelectionDisplay = out.SHOW_ALWAYS;
			else
				LOG->UserLog( "Song file", sPath, "has an unknown #SELECTABLE value, \"%s\"; ignored.", sParams[1].c_str() );
		}
		
		else if( sValueName.Left(strlen("BGCHANGES"))=="BGCHANGES" || sValueName=="ANIMATIONS" )
		{
			SMLoader::ProcessBGChanges( out, sValueName, sPath, sParams[1]);
		}
		
		else if( sValueName=="FGCHANGES" )
		{
			vector<RString> aFGChangeExpressions;
			split( sParams[1], ",", aFGChangeExpressions );
			
			for( unsigned b=0; b<aFGChangeExpressions.size(); b++ )
			{
				BackgroundChange change;
				if( LoadFromBGChangesString( change, aFGChangeExpressions[b] ) )
					out.AddForegroundChange( change );
			}
		}
		
		else if( sValueName=="OFFSET" )
		{
			TimingData &timing = (state == SMA_GETTING_STEP_INFO 
					      ? pNewNotes->m_Timing : out.m_SongTiming);
			timing.m_fBeat0OffsetInSeconds = StringToFloat( sParams[1] );
		}
		
		else if( sValueName=="BPMS" )
		{
			TimingData &timing = (state == SMA_GETTING_STEP_INFO 
					      ? pNewNotes->m_Timing : out.m_SongTiming);
			ProcessBPMs( timing, iRowsPerBeat, sParams[1] );
		}
		
		else if( sValueName=="STOPS" || sValueName=="FREEZES" )
		{
			TimingData &timing = (state == SMA_GETTING_STEP_INFO 
					      ? pNewNotes->m_Timing : out.m_SongTiming);
			ProcessStops( timing, iRowsPerBeat, sParams[1] );
		}
		
		else if( sValueName=="DELAYS" )
		{
			TimingData &timing = (state == SMA_GETTING_STEP_INFO 
					      ? pNewNotes->m_Timing : out.m_SongTiming);
			ProcessDelays( timing, iRowsPerBeat, sParams[1] );
		}
		
		else if( sValueName=="TICKCOUNT" )
		{
			TimingData &timing = (state == SMA_GETTING_STEP_INFO 
					      ? pNewNotes->m_Timing : out.m_SongTiming);
			ProcessTickcounts( timing, iRowsPerBeat, sParams[1] );
		}
		
		else if( sValueName=="SPEED" )
		{
			TimingData &timing = (state == SMA_GETTING_STEP_INFO 
					      ? pNewNotes->m_Timing : out.m_SongTiming);
			ProcessSpeeds( timing, iRowsPerBeat, sParams[1] );
		}
		
		else if( sValueName=="MULTIPLIER" )
		{
			ProcessMultipliers( pNewNotes->m_Timing, iRowsPerBeat, sParams[1] );
		}
		
		else if( sValueName=="FAKES" )
		{
			TimingData &timing = (state == SMA_GETTING_STEP_INFO 
					      ? pNewNotes->m_Timing : out.m_SongTiming);
			ProcessFakes( timing, iRowsPerBeat, sParams[1] );
		}
		
		else if( sValueName=="METERTYPE" )
		{
			; // We don't use this...yet.
		}
		
		else if( sValueName=="KEYSOUNDS" )
		{
			split( sParams[1], ",", out.m_vsKeysoundFile );
		}
		
		// Attacks loaded from file
		else if( sValueName=="ATTACKS" )
		{
			SMLoader::ProcessAttacks( out, sParams );
		}
		
		else if( sValueName=="NOTES" || sValueName=="NOTES2" )
		{
			if( iNumParams < 7 )
			{
				LOG->UserLog( "Song file", sPath, "has %d fields in a #NOTES tag, but should have at least 7.", iNumParams );
				continue;
			}
			
			LoadFromSMATokens( 
					 sParams[1], 
					 sParams[2], 
					 sParams[3], 
					 sParams[4], 
					 sParams[5], 
					 sParams[6],
					 *pNewNotes );
			
			out.AddSteps( pNewNotes );
		}
		else if( sValueName=="TIMESIGNATURES" || sValueName=="LEADTRACK"  )
			;
		else
			LOG->UserLog( "Song file", sPath, "has an unexpected value named \"%s\".", sValueName.c_str() );
	}
	TidyUpData(out, false);
	out.TidyUpData();
	return true;
}

void SMALoader::GetApplicableFiles( const RString &sPath, vector<RString> &out )
{
	GetDirListing( sPath + RString("*.sma"), out );
}

bool SMALoader::LoadEditFromFile( RString sEditFilePath, ProfileSlot slot, bool bAddStepsToSong )
{
	LOG->Trace( "SMALoader::LoadEditFromFile(%s)", sEditFilePath.c_str() );
	
	int iBytes = FILEMAN->GetFileSizeInBytes( sEditFilePath );
	if( iBytes > MAX_EDIT_STEPS_SIZE_BYTES )
	{
		LOG->UserLog( "Edit file", sEditFilePath, "is unreasonably large. It won't be loaded." );
		return false;
	}
	
	MsdFile msd;
	if( !msd.ReadFile( sEditFilePath, true ) ) // unescape
	{
		LOG->UserLog( "Edit file", sEditFilePath, "couldn't be opened: %s", msd.GetError().c_str() );
		return false;
	}
	
	return LoadEditFromMsd( msd, sEditFilePath, slot, bAddStepsToSong );
}

bool SMALoader::LoadEditFromBuffer( const RString &sBuffer, const RString &sEditFilePath, ProfileSlot slot )
{
	MsdFile msd;
	msd.ReadFromString( sBuffer, true ); // unescape
	return LoadEditFromMsd( msd, sEditFilePath, slot, true );
}

bool SMALoader::LoadEditFromMsd( const MsdFile &msd, const RString &sEditFilePath, ProfileSlot slot, bool bAddStepsToSong )
{
	Song* pSong = NULL;
	
	for( unsigned i=0; i<msd.GetNumValues(); i++ )
	{
		int iNumParams = msd.GetNumParams(i);
		const MsdFile::value_t &sParams = msd.GetValue(i);
		RString sValueName = sParams[0];
		sValueName.MakeUpper();
		
		// handle the data
		if( sValueName=="SONG" )
		{
			if( pSong )
			{
				LOG->UserLog( "Edit file", sEditFilePath, "has more than one #SONG tag." );
				return false;
			}
			
			RString sSongFullTitle = sParams[1];
			sSongFullTitle.Replace( '\\', '/' );
			
			pSong = SONGMAN->FindSong( sSongFullTitle );
			if( pSong == NULL )
			{
				LOG->UserLog( "Edit file", sEditFilePath, "requires a song \"%s\" that isn't present.", sSongFullTitle.c_str() );
				return false;
			}
			
			if( pSong->GetNumStepsLoadedFromProfile(slot) >= MAX_EDITS_PER_SONG_PER_PROFILE )
			{
				LOG->UserLog( "Song file", sSongFullTitle, "already has the maximum number of edits allowed for ProfileSlotP%d.", slot+1 );
				return false;
			}
		}
		
		else if( sValueName=="NOTES" )
		{
			if( pSong == NULL )
			{
				LOG->UserLog( "Edit file", sEditFilePath, "doesn't have a #SONG tag preceeding the first #NOTES tag." );
				return false;
			}
			
			if( iNumParams < 7 )
			{
				LOG->UserLog( "Edit file", sEditFilePath, "has %d fields in a #NOTES tag, but should have at least 7.", iNumParams );
				continue;
			}
			
			if( !bAddStepsToSong )
				return true;
			
			Steps* pNewNotes = pSong->CreateSteps();
			LoadFromSMATokens( 
					 sParams[1], sParams[2], sParams[3], sParams[4], sParams[5], sParams[6],
					 *pNewNotes);
			
			pNewNotes->SetLoadedFromProfile( slot );
			pNewNotes->SetDifficulty( Difficulty_Edit );
			pNewNotes->SetFilename( sEditFilePath );
			
			if( pSong->IsEditAlreadyLoaded(pNewNotes) )
			{
				LOG->UserLog( "Edit file", sEditFilePath, "is a duplicate of another edit that was already loaded." );
				SAFE_DELETE( pNewNotes );
				return false;
			}
			
			pSong->AddSteps( pNewNotes );
			return true; // Only allow one Steps per edit file!
		}
		else
		{
			LOG->UserLog( "Edit file", sEditFilePath, "has an unexpected value \"%s\".", sValueName.c_str() );
		}
	}
	
	return true;
}

/**
 * @file
 * @author Aldo Fregoso, Jason Felds (c) 2009-2011
 * @section LICENSE
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
