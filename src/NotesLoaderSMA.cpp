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

/**
 * @brief A custom .edit file can only be so big before we have to reject it.
 */
const int MAX_EDIT_STEPS_SIZE_BYTES = 60*1024; // 60 KB

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
	// we're loading from disk, so this is by definition already saved:
	out.SetSavedToDisk( true );
	
	Trim( sStepsType );
	Trim( sDescription );
	Trim( sDifficulty );
	Trim( sNoteData );
	
	//	LOG->Trace( "Steps::LoadFromSMTokens()" );
	
	// insert stepstype hacks from GameManager.cpp here? -aj
	out.m_StepsType = GAMEMAN->StringToStepsType( sStepsType );
	out.SetDescription( sDescription );
	out.SetCredit( sDescription ); // this is often used for both.
	out.SetDifficulty( StringToDifficulty(sDifficulty) );
	
	sDescription.MakeLower();
	
	// Handle hacks that originated back when StepMania didn't have
	// Difficulty_Challenge. (At least v1.64, possibly v3.0 final...)
	if( out.GetDifficulty() == Difficulty_Hard )
	{
		// HACK: SMANIAC used to be Difficulty_Hard with a special description.
		if( sDescription == "smaniac" ) 
			out.SetDifficulty( Difficulty_Challenge );
		
		// HACK: CHALLENGE used to be Difficulty_Hard with a special description.
		if( sDescription == "challenge" ) 
			out.SetDifficulty( Difficulty_Challenge );
	}
	
	out.SetMeter( StringToInt(sMeter) );
	vector<RString> saValues;
	split( sRadarValues, ",", saValues, true );
	int categories = NUM_RadarCategory - 1; // Fakes aren't counted in the radar values.
	if( saValues.size() == (unsigned)categories * NUM_PLAYERS )
	{
		RadarValues v[NUM_PLAYERS];
		FOREACH_PlayerNumber( pn )
		{
			// Can't use the foreach anymore due to flexible radar lines.
			for( RadarCategory rc = (RadarCategory)0; rc < categories; 
			    enum_add<RadarCategory>( rc, 1 ) )
			{
				v[pn][rc] = StringToFloat( saValues[pn*categories + rc] );
			}
		}
		out.SetCachedRadarValues( v );
	}
	
	out.SetSMNoteData( sNoteData );
	
	out.TidyUpData();
}

bool SMALoader::LoadFromDir( const RString &sPath, Song &out )
{
	vector<RString> aFileNames;
	GetApplicableFiles( sPath, aFileNames );
	
	if( aFileNames.size() > 1 )
	{
		LOG->UserLog( "Song", sPath, "has more than one SMA file. There can be only one!" );
		return false;
	}
	ASSERT( aFileNames.size() == 1 );
	return LoadFromSMAFile( sPath + aFileNames[0], out );
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
	
	out.m_Timing.m_sFile = sPath;
	LoadTimingFromSMAFile( msd, out.m_Timing );
	
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
			vector<RString> vs1;
			split( sParams[1], ",", vs1 );
			FOREACH_CONST( RString, vs1, s )
			{
				vector<RString> vs2;
				split( *s, "=", vs2 );
				if( vs2.size() >= 2 )
				{
					InstrumentTrack it = StringToInstrumentTrack( vs2[0] );
					if( it != InstrumentTrack_Invalid )
						out.m_sInstrumentTrackFile[it] = vs2[1];
				}
			}
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
			;		}
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
			BackgroundLayer iLayer = BACKGROUND_LAYER_1;
			if( sscanf(sValueName, "BGCHANGES%d", &*ConvertValue<int>(&iLayer)) == 1 )
				enum_add(iLayer, -1);	// #BGCHANGES2 = BACKGROUND_LAYER_2
			
			bool bValid = iLayer>=0 && iLayer<NUM_BackgroundLayer;
			if( !bValid )
			{
				LOG->UserLog( "Song file", sPath, "has a #BGCHANGES tag \"%s\" that is out of range.", sValueName.c_str() );
			}
			else
			{
				vector<RString> aBGChangeExpressions;
				split( sParams[1], ",", aBGChangeExpressions );
				
				for( unsigned b=0; b<aBGChangeExpressions.size(); b++ )
				{
					BackgroundChange change;
					if( LoadFromBGChangesString( change, aBGChangeExpressions[b] ) )
						out.AddBackgroundChange( iLayer, change );
				}
			}
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
		
		else if( sValueName=="KEYSOUNDS" )
		{
			split( sParams[1], ",", out.m_vsKeysoundFile );
		}
		
		// Attacks loaded from file
		else if( sValueName=="ATTACKS" )
		{
			// Build the RString vector here so we can write it to file again later
			for( unsigned s=1; s < sParams.params.size(); ++s )
				out.m_sAttackString.push_back( sParams[s] );
			
			Attack attack;
			float end = -9999;
			
			for( unsigned j=1; j < sParams.params.size(); ++j )
			{
				vector<RString> sBits;
				split( sParams[j], "=", sBits, false );
				
				// Need an identifer and a value for this to work
				if( sBits.size() < 2 )
					continue;
				
				TrimLeft( sBits[0] );
				TrimRight( sBits[0] );
				
				if( !sBits[0].CompareNoCase("TIME") )
					attack.fStartSecond = strtof( sBits[1], NULL );
				else if( !sBits[0].CompareNoCase("LEN") )
					attack.fSecsRemaining = strtof( sBits[1], NULL );
				else if( !sBits[0].CompareNoCase("END") )
					end = strtof( sBits[1], NULL );
				else if( !sBits[0].CompareNoCase("MODS") )
				{
					attack.sModifiers = sBits[1];
					
					if( end != -9999 )
					{
						attack.fSecsRemaining = end - attack.fStartSecond;
						end = -9999;
					}
					
					if( attack.fSecsRemaining < 0.0f )
						attack.fSecsRemaining = 0.0f;
					
					out.m_Attacks.push_back( attack );
				}
			}
		}
		
		else if( sValueName=="NOTES" || sValueName=="NOTES2" )
		{
			if( iNumParams < 7 )
			{
				LOG->UserLog( "Song file", sPath, "has %d fields in a #NOTES tag, but should have at least 7.", iNumParams );
				continue;
			}
			
			Steps* pNewNotes = new Steps;
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
		/*
		 * We used to check for timing data in this section. That has
		 * since been moved to a dedicated function.
		 */
		else if( sValueName=="OFFSET" || sValueName=="BPMS" || sValueName=="STOPS" || sValueName=="FREEZES" || sValueName=="DELAYS" || sValueName=="TIMESIGNATURES" || sValueName=="LEADTRACK" || sValueName=="TICKCOUNTS" )
			;
		else
			LOG->UserLog( "Song file", sPath, "has an unexpected value named \"%s\".", sValueName.c_str() );
	}
	
	return true;
}

void SMALoader::GetApplicableFiles( const RString &sPath, vector<RString> &out )
{
	GetDirListing( sPath + RString("*.sma"), out );
}

bool SMALoader::LoadTimingFromFile( const RString &fn, TimingData &out )
{
	MsdFile msd;
	if( !msd.ReadFile( fn, true ) )  // unescape
	{
		LOG->UserLog( "Song file", fn, "couldn't be loaded: %s", msd.GetError().c_str() );
		return false;
	}
	
	out.m_sFile = fn;
	LoadTimingFromSMAFile( msd, out );
	return true;
}

void SMALoader::LoadTimingFromSMAFile( const MsdFile &msd, TimingData &out )
{
	out.m_fBeat0OffsetInSeconds = 0;
	out.m_BPMSegments.clear();
	out.m_StopSegments.clear();
	out.m_WarpSegments.clear();
	out.m_vTimeSignatureSegments.clear();
	
	vector<WarpSegment> arrayWarpsFromNegativeBPMs;
	//vector<WarpSegment> arrayWarpsFromNegativeStops;
	int rowsPerMeasure = 0;
	bool encountered = false;
	
	for( unsigned i=0; i<msd.GetNumValues(); i++ )
	{
		const MsdFile::value_t &sParams = msd.GetValue(i);
		RString sValueName = sParams[0];
		sValueName.MakeUpper();
		
		if( sValueName=="ROWSPERBEAT")
		{
			if( encountered )
			{
				break;
			}
			encountered = true;
			rowsPerMeasure = StringToInt( sParams[1] );
		}
		else if( sValueName=="BEATSPERMEASURE" )
		{
			TimeSignatureSegment new_seg;
			new_seg.m_iStartRow = 0;
			new_seg.m_iNumerator = StringToInt( sParams[1] );
			new_seg.m_iDenominator = 4;
			out.AddTimeSignatureSegment( new_seg );
		}
		else if( sValueName=="OFFSET" )
		{
			out.m_fBeat0OffsetInSeconds = StringToFloat( sParams[1] );
		}
		else if( sValueName=="STOPS" )
		{
			vector<RString> arrayFreezeExpressions;
			split( sParams[1], ",", arrayFreezeExpressions );
			
			for( unsigned f=0; f<arrayFreezeExpressions.size(); f++ )
			{
				vector<RString> arrayFreezeValues;
				split( arrayFreezeExpressions[f], "=", arrayFreezeValues );
				if( arrayFreezeValues.size() != 2 )
				{
					// XXX: Hard to tell which file caused this.
					LOG->UserLog( "Song file", "(UNKNOWN)", "has an invalid #%s value \"%s\" (must have exactly one '='), ignored.",
						     sValueName.c_str(), arrayFreezeExpressions[f].c_str() );
					continue;
				}
				
				float fFreezeBeat = 0;
				RString beat = arrayFreezeValues[0];
				if( beat.Right(0).MakeUpper() == "R" )
				{
					beat = beat.Left(beat.size()-1);
					fFreezeBeat = StringToFloat( beat ) / rowsPerMeasure;
				}
				else
				{
					fFreezeBeat = StringToFloat(beat);
				}
				
				//float fFreezeBeat = StringToFloat( arrayBPMChangeValues[0] );
				const float fFreezeSeconds = StringToFloat( arrayFreezeValues[1] );
				StopSegment new_seg( BeatToNoteRow(fFreezeBeat), fFreezeSeconds );
				// XXX: Remove Negatives Bug?
				new_seg.m_iStartRow = BeatToNoteRow(fFreezeBeat);
				new_seg.m_fStopSeconds = fFreezeSeconds;
				
				if(fFreezeSeconds > 0.0f)
				{
					// LOG->Trace( "Adding a freeze segment: beat: %f, seconds = %f", new_seg.m_fStartBeat, new_seg.m_fStopSeconds );
					out.AddStopSegment( new_seg );
				}
				else
				{
					// negative stops (hi JS!) -aj
					if( PREFSMAN->m_bQuirksMode )
					{
						// LOG->Trace( "Adding a negative freeze segment: beat: %f, seconds = %f", new_seg.m_fStartBeat, new_seg.m_fStopSeconds );
						out.AddStopSegment( new_seg );
					}
					else
						LOG->UserLog( "Song file", "(UNKNOWN)", "has an invalid stop at beat %f, length %f.", fFreezeBeat, fFreezeSeconds );
				}
			}
		}
		
		else if( sValueName=="BPMS" )
		{
			vector<RString> arrayBPMChangeExpressions;
			split( sParams[1], ",", arrayBPMChangeExpressions );
			
			for( unsigned b=0; b<arrayBPMChangeExpressions.size(); b++ )
			{
				vector<RString> arrayBPMChangeValues;
				split( arrayBPMChangeExpressions[b], "=", arrayBPMChangeValues );
				// XXX: Hard to tell which file caused this.
				if( arrayBPMChangeValues.size() != 2 )
				{
					LOG->UserLog( "Song file", "(UNKNOWN)", "has an invalid #%s value \"%s\" (must have exactly one '='), ignored.",
						     sValueName.c_str(), arrayBPMChangeExpressions[b].c_str() );
					continue;
				}
				
				float fBeat = 0;
				RString beat = arrayBPMChangeValues[0];
				if( beat.Right(0).MakeUpper() == "R" )
				{
					beat = beat.Left(beat.size()-1);
					fBeat = StringToFloat( beat ) / rowsPerMeasure;
				}
				else
				{
					fBeat = StringToFloat(beat);
				}
				
				//float fBeat = StringToFloat( arrayBPMChangeValues[0] );
				const float fNewBPM = StringToFloat( arrayBPMChangeValues[1] );
				// XXX: Remove Negatives Bug?
				BPMSegment new_seg;
				new_seg.m_iStartRow = BeatToNoteRow(fBeat);
				new_seg.SetBPM( fNewBPM );
				
				// convert negative BPMs into Warp segments
				if( fNewBPM < 0.0f )
				{
					vector<RString> arrayNextBPMChangeValues;
					// get next bpm in sequence
					if((b+1) < arrayBPMChangeExpressions.size())
					{
						split( arrayBPMChangeExpressions[b+1], "=", arrayNextBPMChangeValues );
						const float fNextPositiveBeat = StringToFloat( arrayNextBPMChangeValues[0] );
						const float fNextPositiveBPM  = StringToFloat( arrayNextBPMChangeValues[1] );
						
						// tJumpPos = (tPosBPS-abs(negBPS)) + (gPosBPMPosition - fNegPosition)
						float fDeltaBeat = ((fNextPositiveBPM/60.0f)-abs(fNewBPM/60.0f)) + (fNextPositiveBeat-fBeat);
						//float fWarpLengthBeats = fNextPositiveBeat + fDeltaBeat;
						WarpSegment wsTemp(BeatToNoteRow(fBeat),fDeltaBeat);
						arrayWarpsFromNegativeBPMs.push_back(wsTemp);
						
						/*
						 LOG->Trace( ssprintf("==NotesLoSM negbpm==\nfnextposbeat = %f, fnextposbpm = %f,\nfdelta = %f, fwarpto = %f",
						 fNextPositiveBeat,
						 fNextPositiveBPM,
						 fDeltaBeat,
						 fWarpToBeat
						 ) );
						 */
						/*
						 LOG->Trace( ssprintf("==Negative/Subtractive BPM in NotesLoader==\nNegBPM has noterow = %i, BPM = %f\nNextBPM @ noterow %i\nDelta value = %i noterows\nThis warp will have us end up at noterow %i",
						 BeatToNoteRow(fBeat), fNewBPM,
						 BeatToNoteRow(fNextPositiveBeat),
						 BeatToNoteRow(fDeltaBeat),
						 BeatToNoteRow(fWarpToBeat))
						 );
						 */
						//float fDeltaBeat = ((fNextPositiveBPM/60.0f)-abs(fNewBPM/60.0f)) + (fNextPositiveBeat-fBeat);
						/*
						 LOG->Trace( ssprintf("==NotesLoader Delta as NoteRows==\nfDeltaBeat = %f (beat)\nfDeltaBeat = (NextBPMSeg %f - abs(fBPS %f)) + (nextStartRow %i - thisRow %i)",
						 fDeltaBeat,(fNextPositiveBPM/60.0f),abs(fNewBPM/60.0f),BeatToNoteRow(fNextPositiveBeat),BeatToNoteRow(fBeat))
						 );
						 */
						
						out.AddBPMSegment( new_seg );
						
						continue;
					}
					else
					{
						// last BPM is a negative one? ugh. -aj (MAX_NOTE_ROW exists btw)
						out.AddBPMSegment( new_seg );
					}
				}
				
				if(fNewBPM > 0.0f)
					out.AddBPMSegment( new_seg );
				else
				{
					out.m_bHasNegativeBpms = true;
					// only add Negative BPMs in quirks mode -aj
					if( PREFSMAN->m_bQuirksMode )
						out.AddBPMSegment( new_seg );
					else
						LOG->UserLog( "Song file", "(UNKNOWN)", "has an invalid BPM change at beat %f, BPM %f.", fBeat, fNewBPM );
				}
			}
		}
		
		
		// Note: Even though it is possible to have Negative BPMs and Stops in
		// a song along with Warps, we should not support files that contain
		// both styles of warp tricks (Negatives vs. #WARPS).
		// If Warps have been populated from Negative BPMs, then go through that
		// instead of using the data in the Warps tag. This should be above,
		// but it breaks compiling so...
		if(arrayWarpsFromNegativeBPMs.size() > 0)
		{
			// zomg we already have some warps...
			for( unsigned j=0; j<arrayWarpsFromNegativeBPMs.size(); j++ )
			{
				out.AddWarpSegment( arrayWarpsFromNegativeBPMs[j] );
			}
		}
		// warp sorting will need to take place.
		//sort(out.m_WarpSegments.begin(), out.m_WarpSegments.end());
	}
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
			
			Steps* pNewNotes = new Steps;
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

bool SMALoader::LoadFromBGChangesString( BackgroundChange &change, 
					const RString &sBGChangeExpression )
{
	return SMLoader::LoadFromBGChangesString(change, sBGChangeExpression);
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
