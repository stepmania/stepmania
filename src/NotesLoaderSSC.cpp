#include "global.h"
#include "NotesLoaderSSC.h"
#include "BackgroundUtil.h"
#include "GameManager.h"
#include "MsdFile.h" // No JSON here.
#include "NoteTypes.h"
#include "NotesLoaderSM.h" // For programming shortcuts.
#include "RageFileManager.h"
#include "RageLog.h"
#include "RageUtil.h"
#include "Song.h"
#include "SongManager.h"
#include "Steps.h"
#include "Attack.h"
#include "PrefsManager.h"

void SSCLoader::ProcessWarps( TimingData &out, const RString sParam, const float fVersion )
{
	vector<RString> arrayWarpExpressions;
	split( sParam, ",", arrayWarpExpressions );
	
	for( unsigned b=0; b<arrayWarpExpressions.size(); b++ )
	{
		vector<RString> arrayWarpValues;
		split( arrayWarpExpressions[b], "=", arrayWarpValues );
		if( arrayWarpValues.size() != 2 )
		{
			LOG->UserLog("Song file",
				     this->GetSongTitle(),
				     "has an invalid #WARPS value \"%s\" (must have exactly one '='), ignored.",
				     arrayWarpExpressions[b].c_str() );
			continue;
		}
		
		const float fBeat = StringToFloat( arrayWarpValues[0] );
		const float fNewBeat = StringToFloat( arrayWarpValues[1] );
		// Early versions were absolute in beats. They should be relative.
		if( ( fVersion < VERSION_SPLIT_TIMING && fNewBeat > fBeat ) )
		{
			out.AddSegment( WarpSegment(BeatToNoteRow(fBeat), fNewBeat - fBeat) );
		}
		else if( fNewBeat > 0 )
			out.AddSegment( WarpSegment(BeatToNoteRow(fBeat), fNewBeat) );
		else
		{
			LOG->UserLog("Song file",
				     this->GetSongTitle(),
				     "has an invalid Warp at beat %f, BPM %f.",
				     fBeat, fNewBeat );
		}
	}
}

void SSCLoader::ProcessLabels( TimingData &out, const RString sParam )
{
	vector<RString> arrayLabelExpressions;
	split( sParam, ",", arrayLabelExpressions );
	
	for( unsigned b=0; b<arrayLabelExpressions.size(); b++ )
	{
		vector<RString> arrayLabelValues;
		split( arrayLabelExpressions[b], "=", arrayLabelValues );
		if( arrayLabelValues.size() != 2 )
		{
			LOG->UserLog("Song file",
				     this->GetSongTitle(),
				     "has an invalid #LABELS value \"%s\" (must have exactly one '='), ignored.",
				     arrayLabelExpressions[b].c_str() );
			continue;
		}
		
		const float fBeat = StringToFloat( arrayLabelValues[0] );
		RString sLabel = arrayLabelValues[1];
		TrimRight(sLabel);
		if( fBeat >= 0.0f )
			out.AddSegment( LabelSegment(BeatToNoteRow(fBeat), sLabel) );
		else 
		{
			LOG->UserLog("Song file",
				     this->GetSongTitle(),
				     "has an invalid Label at beat %f called %s.",
				     fBeat, sLabel.c_str() );
		}
		
	}
}

void SSCLoader::ProcessCombos( TimingData &out, const RString line, const int rowsPerBeat )
{
	vector<RString> arrayComboExpressions;
	split( line, ",", arrayComboExpressions );
	
	for( unsigned f=0; f<arrayComboExpressions.size(); f++ )
	{
		vector<RString> arrayComboValues;
		split( arrayComboExpressions[f], "=", arrayComboValues );
		unsigned size = arrayComboValues.size();
		if( size < 2 )
		{
			LOG->UserLog("Song file",
				     this->GetSongTitle(),
				     "has an invalid #COMBOS value \"%s\" (must have at least one '='), ignored.",
				     arrayComboExpressions[f].c_str() );
			continue;
		}
		const float fComboBeat = StringToFloat( arrayComboValues[0] );
		const int iCombos = StringToInt( arrayComboValues[1] );
		const int iMisses = (size == 2 ? iCombos : StringToInt(arrayComboValues[2]));
		out.AddSegment( ComboSegment( BeatToNoteRow(fComboBeat), iCombos, iMisses ) );
	}
}

void SSCLoader::ProcessScrolls( TimingData &out, const RString sParam )
{
	vector<RString> vs1;
	split( sParam, ",", vs1 );
	
	FOREACH_CONST( RString, vs1, s1 )
	{
		vector<RString> vs2;
		split( *s1, "=", vs2 );
		
		if( vs2.size() < 2 )
		{
			LOG->UserLog("Song file",
				     this->GetSongTitle(),
				     "has an scroll change with %i values.",
				     static_cast<int>(vs2.size()) );
			continue;
		}

		const float fBeat = StringToFloat( vs2[0] );
		const float fRatio = StringToFloat( vs2[1] );

		if( fBeat < 0 )
		{
			LOG->UserLog("Song file",
				     this->GetSongTitle(),
				     "has an scroll change with beat %f.",
				     fBeat );
			continue;
		}

		out.AddSegment( ScrollSegment(BeatToNoteRow(fBeat), fRatio) );
	}
}

bool SSCLoader::LoadNoteDataFromSimfile( const RString & cachePath, Steps &out )
{
	LOG->Trace( "Loading notes from %s", cachePath.c_str() );
	
	MsdFile msd;
	if (!msd.ReadFile(cachePath, true))
	{
		LOG->UserLog("Unable to load any notes from",
			     cachePath,
			     "for this reason: %s",
			     msd.GetError().c_str());
		return false;
	}
	
	bool tryingSteps = false;
	float storedVersion = 0;
	const unsigned values = msd.GetNumValues();
	
	for (unsigned i = 0; i < values; i++)
	{
		const MsdFile::value_t &params = msd.GetValue(i);
		RString valueName = params[0];
		valueName.MakeUpper();
		RString matcher = params[1]; // mainly for debugging.
		Trim(matcher);
		
		if (valueName=="VERSION")
		{
			storedVersion = StringToFloat(matcher);
		}
		if (tryingSteps)
		{
			if( valueName=="STEPSTYPE" )
			{
				if (out.m_StepsType != GAMEMAN->StringToStepsType(matcher))
					tryingSteps = false;
			}
			else if( valueName=="CHARTNAME")
			{
				if (storedVersion >= VERSION_CHART_NAME_TAG && out.GetChartName() != matcher)
					tryingSteps = false;
			}
			else if( valueName=="DESCRIPTION" )
			{
				if (storedVersion < VERSION_CHART_NAME_TAG)
				{
					if (out.GetChartName() != matcher)
						tryingSteps = false;
				}
				else if (out.GetDescription() != matcher)
					tryingSteps = false;
			}
			
			else if( valueName=="DIFFICULTY" )
			{
				if (out.GetDifficulty() != StringToDifficulty(matcher))
					tryingSteps = false;
			}
			
			else if( valueName=="METER" )
			{
				if (out.GetMeter() != StringToInt(matcher))
					tryingSteps = false;
			}
			
			else if( valueName=="CREDIT" )
			{
				if (out.GetCredit() != matcher)
					tryingSteps = false;
			}
			
			else if( valueName=="NOTES" || valueName=="NOTES2" )
			{
				out.SetSMNoteData(matcher);
				out.TidyUpData();
				return true;
			}
		}
		else
		{
			if(valueName == "NOTEDATA")
			{
				tryingSteps = true;
			}
		}
	}
	return false;
}

bool SSCLoader::LoadFromSimfile( const RString &sPath, Song &out, bool bFromCache )
{
	LOG->Trace( "Song::LoadFromSSCFile(%s)", sPath.c_str() );

	MsdFile msd;
	if( !msd.ReadFile( sPath, true ) )
	{
		LOG->UserLog( "Song file", sPath, "couldn't be opened: %s", msd.GetError().c_str() );
		return false;
	}

	out.m_SongTiming.m_sFile = sPath; // songs still have their fallback timing.
	out.m_sSongFileName = sPath;

	int state = GETTING_SONG_INFO;
	const unsigned values = msd.GetNumValues();
	Steps* pNewNotes = NULL;
	TimingData stepsTiming;
	bool bHasOwnTiming = false;

	for( unsigned i = 0; i < values; i++ )
	{
		const MsdFile::value_t &sParams = msd.GetValue(i);
		RString sValueName = sParams[0];
		sValueName.MakeUpper();

		switch (state)
		{
			case GETTING_SONG_INFO:
			{
				if( sValueName=="VERSION" )
				{
					out.m_fVersion = StringToFloat( sParams[1] );
				}

				else if( sValueName=="TITLE" )
				{
					out.m_sMainTitle = sParams[1];
					this->SetSongTitle(sParams[1]);
				}

				else if( sValueName=="SUBTITLE" )
				{
					out.m_sSubTitle = sParams[1];
				}

				else if( sValueName=="ARTIST" )
				{
					out.m_sArtist = sParams[1];
				}

				else if( sValueName=="TITLETRANSLIT" )
				{
					out.m_sMainTitleTranslit = sParams[1];
				}

				else if( sValueName=="SUBTITLETRANSLIT" )
				{
					out.m_sSubTitleTranslit = sParams[1];
				}

				else if( sValueName=="ARTISTTRANSLIT" )
				{
					out.m_sArtistTranslit = sParams[1];
				}

				else if( sValueName=="GENRE" )
				{
					out.m_sGenre = sParams[1];
				}

				else if( sValueName=="ORIGIN" )
				{
					out.m_sOrigin = sParams[1];
				}

				else if( sValueName=="CREDIT" )
				{
					out.m_sCredit = sParams[1];
				}

				else if( sValueName=="BANNER" )
				{
					out.m_sBannerFile = sParams[1];
				}

				else if( sValueName=="BACKGROUND" )
				{
					out.m_sBackgroundFile = sParams[1];
				}

				else if( sValueName=="PREVIEWVID" )
				{
					out.m_sPreviewVidFile = sParams[1];
				}

				else if( sValueName=="JACKET" )
				{
					out.m_sJacketFile = sParams[1];
				}

				else if( sValueName=="CDIMAGE" )
				{
					out.m_sCDFile = sParams[1];
				}

				else if( sValueName=="DISCIMAGE" )
				{
					out.m_sDiscFile = sParams[1];
				}

				else if( sValueName=="LYRICSPATH" )
				{
					out.m_sLyricsFile = sParams[1];
				}

				else if( sValueName=="CDTITLE" )
				{
					out.m_sCDTitleFile = sParams[1];
				}

				else if( sValueName=="MUSIC" )
				{
					out.m_sMusicFile = sParams[1];
				}

				else if( sValueName=="INSTRUMENTTRACK" )
				{
					SMLoader::ProcessInstrumentTracks( out, sParams[1] );
				}

				else if( sValueName=="MUSICLENGTH" )
				{
					if( !bFromCache )
						continue;
					out.m_fMusicLengthSeconds = StringToFloat( sParams[1] );
				}

				else if( sValueName=="LASTBEATHINT" )
				{
					// unable to parse due to tag position. Ignore.
				}
				
				else if (sValueName == "LASTSECONDHINT")
				{
					out.SetSpecifiedLastSecond(StringToFloat(sParams[1]));
				}

				else if( sValueName=="MUSICBYTES" )
				{
					; // ignore
				}

				else if( sValueName=="SAMPLESTART" )
				{
					out.m_fMusicSampleStartSeconds = HHMMSSToSeconds( sParams[1] );
				}

				else if( sValueName=="SAMPLELENGTH" )
				{
					out.m_fMusicSampleLengthSeconds = HHMMSSToSeconds( sParams[1] );
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

				else if( sValueName=="SELECTABLE" )
				{
					if(sParams[1].EqualsNoCase("YES"))
						out.m_SelectionDisplay = out.SHOW_ALWAYS;
					else if(sParams[1].EqualsNoCase("NO"))
						out.m_SelectionDisplay = out.SHOW_NEVER;
					// ROULETTE from 3.9 is no longer in use.
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

				else if( sValueName=="KEYSOUNDS" )
				{
					RString keysounds = sParams[1];
					if( keysounds.length() >= 2 && keysounds.substr(0, 2) == "\\#" )
						keysounds = keysounds.substr(1);
					split( keysounds, ",", out.m_vsKeysoundFile );
				}

				// Attacks loaded from file
				else if( sValueName=="ATTACKS" )
				{
					ProcessAttackString(out.m_sAttackString, sParams);
					ProcessAttacks(out.m_Attacks, sParams);
				}

				else if( sValueName=="OFFSET" )
				{
					out.m_SongTiming.m_fBeat0OffsetInSeconds = StringToFloat( sParams[1] );
				}
				/* Below are the song based timings that should only be used
				 * if the steps do not have their own timing. */
				else if( sValueName=="STOPS" )
				{
					SMLoader::ProcessStops(out.m_SongTiming, sParams[1]);
				}
				else if( sValueName=="DELAYS" )
				{
					SMLoader::ProcessDelays(out.m_SongTiming, sParams[1]);
				}

				else if( sValueName=="BPMS" )
				{
					SMLoader::ProcessBPMs(out.m_SongTiming, sParams[1]);
				}
				
				else if( sValueName=="WARPS" ) // Older versions allowed em here.
				{
					ProcessWarps( out.m_SongTiming, sParams[1], out.m_fVersion );
				}
				
				else if( sValueName=="LABELS" )
				{
					ProcessLabels( out.m_SongTiming, sParams[1] );
				}

				else if( sValueName=="TIMESIGNATURES" )
				{
					SMLoader::ProcessTimeSignatures(out.m_SongTiming, sParams[1]);
				}

				else if( sValueName=="TICKCOUNTS" )
				{
					SMLoader::ProcessTickcounts(out.m_SongTiming, sParams[1]);
				}

				else if( sValueName=="COMBOS" )
				{
					ProcessCombos( out.m_SongTiming, sParams[1] );
				}
				else if (sValueName=="SPEEDS")
				{
					ProcessSpeeds(out.m_SongTiming, sParams[1]);
				}
				else if (sValueName=="SCROLLS")
				{
					ProcessScrolls(out.m_SongTiming, sParams[1]);
				}
				else if (sValueName=="FAKES")
				{
					ProcessFakes(out.m_SongTiming, sParams[1]);
				}

				/* The following are cache tags. Never fill their values
				 * directly: only from the cached version. */
				else if( sValueName=="FIRSTBEAT" || sValueName=="LASTBEAT" )
				{
					// no longer used.
				}
				else if (sValueName=="FIRSTSECOND")
				{
					if( bFromCache )
						out.SetFirstSecond(StringToFloat(sParams[1]));
				}

				else if( sValueName=="LASTSECOND" )
				{
					if( bFromCache )
						out.SetLastSecond(StringToFloat(sParams[1]));
				}

				else if( sValueName=="SONGFILENAME" )
				{
					if( bFromCache )
						out.m_sSongFileName = sParams[1];
				}

				else if( sValueName=="HASMUSIC" )
				{
					if( bFromCache )
						out.m_bHasMusic = StringToInt( sParams[1] ) != 0;
				}

				else if( sValueName=="HASBANNER" )
				{
					if( bFromCache )
						out.m_bHasBanner = StringToInt( sParams[1] ) != 0;
				}

				// This tag will get us to the next section.
				else if( sValueName=="NOTEDATA" )
				{
					state = GETTING_STEP_INFO;
					pNewNotes = out.CreateSteps();
					stepsTiming = TimingData( out.m_SongTiming.m_fBeat0OffsetInSeconds );
					bHasOwnTiming = false;
				}
				break;
			}
			case GETTING_STEP_INFO:
			{
				if (sValueName == "CHARTNAME")
				{
					pNewNotes->SetChartName(sParams[1]);
				}
				if( sValueName=="STEPSTYPE" )
				{
					pNewNotes->m_StepsType = GAMEMAN->StringToStepsType( sParams[1] );
				}

				else if( sValueName=="CHARTSTYLE" )
				{
					pNewNotes->SetChartStyle( sParams[1] );
				}

				else if( sValueName=="DESCRIPTION" )
				{
					if (out.m_fVersion < VERSION_CHART_NAME_TAG)
					{
						pNewNotes->SetChartName(sParams[1]);
					}
					else
					{
						pNewNotes->SetDescription(sParams[1]);
					}
				}

				else if( sValueName=="DIFFICULTY" )
				{
					pNewNotes->SetDifficulty( StringToDifficulty( sParams[1] ) );
				}

				else if( sValueName=="METER" )
				{
					pNewNotes->SetMeter( StringToInt( sParams[1] ) );
				}

				else if( sValueName=="RADARVALUES" )
				{
					if (bFromCache)
					{
						vector<RString> saValues;
						split( sParams[1], ",", saValues, true );
						
						int categories = NUM_RadarCategory;
						if( out.m_fVersion < VERSION_RADAR_FAKE )
							categories -= 1;
						
						if( saValues.size() == (unsigned)categories * NUM_PLAYERS )
						{
							RadarValues v[NUM_PLAYERS];
							FOREACH_PlayerNumber( pn )
							{
								// Can't use the foreach anymore due to flexible radar lines.
								for( RadarCategory rc = (RadarCategory)0; rc < categories; 
								    enum_add<RadarCategory>( rc, +1 ) )
								{
									v[pn][rc] = StringToFloat( saValues[pn*categories + rc] );
								}
							}
							pNewNotes->SetCachedRadarValues( v );
						}
					}
					else 
					{
						// just recalc at time.
					}

				}

				else if( sValueName=="CREDIT" )
				{
					pNewNotes->SetCredit( sParams[1] );
				}

				else if( sValueName=="NOTES" || sValueName=="NOTES2" )
				{
					state = GETTING_SONG_INFO;
					if( bHasOwnTiming )
						pNewNotes->m_Timing = stepsTiming;
					pNewNotes->SetSMNoteData( sParams[1] );
					pNewNotes->TidyUpData();
					pNewNotes->SetFilename(sPath);
					out.AddSteps( pNewNotes );
				}
				
				else if( sValueName=="BPMS" )
				{
					if (out.m_fVersion >= VERSION_SPLIT_TIMING)
					{
						if( SMLoader::ProcessBPMs(stepsTiming, sParams[1]) )
							bHasOwnTiming = true;
					}
				}
				
				else if( sValueName=="STOPS" )
				{
					if (out.m_fVersion >= VERSION_SPLIT_TIMING)
					{
						SMLoader::ProcessStops(stepsTiming, sParams[1]);
						bHasOwnTiming = true;
					}
				}
				
				else if( sValueName=="DELAYS" )
				{
					if (out.m_fVersion >= VERSION_SPLIT_TIMING)
					{
						SMLoader::ProcessDelays(stepsTiming, sParams[1]);
						bHasOwnTiming = true;
					}
				}
				
				else if( sValueName=="TIMESIGNATURES" )
				{
					if (out.m_fVersion >= VERSION_SPLIT_TIMING)
					{
						SMLoader::ProcessTimeSignatures(stepsTiming, sParams[1]);
						bHasOwnTiming = true;
					}
				}
				
				else if( sValueName=="TICKCOUNTS" )
				{
					if (out.m_fVersion >= VERSION_SPLIT_TIMING)
					{
						SMLoader::ProcessTickcounts(stepsTiming, sParams[1]);
						bHasOwnTiming = true;
					}
				}
				
				else if( sValueName=="COMBOS" )
				{
					if (out.m_fVersion >= VERSION_SPLIT_TIMING)
					{
						ProcessCombos(stepsTiming, sParams[1]);
						bHasOwnTiming = true;
					}
				}
				
				else if( sValueName=="WARPS" )
				{
					if (out.m_fVersion >= VERSION_SPLIT_TIMING)
					{
						ProcessWarps(stepsTiming, sParams[1], out.m_fVersion);
						bHasOwnTiming = true;
					}
				}
				
				else if( sValueName=="SPEEDS" )
				{
					if (out.m_fVersion >= VERSION_SPLIT_TIMING)
					{
						ProcessSpeeds( stepsTiming, sParams[1] );
						bHasOwnTiming = true;
					}
				}
				
				else if( sValueName=="SCROLLS" )
				{
					if (out.m_fVersion >= VERSION_SPLIT_TIMING)
					{
						ProcessScrolls( stepsTiming, sParams[1] );
						bHasOwnTiming = true;
					}
				}
				
				else if( sValueName=="FAKES" )
				{
					if (out.m_fVersion >= VERSION_SPLIT_TIMING)
					{
						ProcessFakes( stepsTiming, sParams[1] );
						bHasOwnTiming = true;
					}
				}
				
				else if( sValueName=="LABELS" )
				{
					if (out.m_fVersion >= VERSION_SPLIT_TIMING)
					{
						ProcessLabels(stepsTiming, sParams[1]);
						bHasOwnTiming = true;
					}
				}
				/* If this is called, the chart does not use the same attacks
				 * as the Song's timing. No other changes are required. */
				else if( sValueName=="ATTACKS" )
				{
					if (out.m_fVersion >= VERSION_SPLIT_TIMING)
					{
						ProcessAttackString(pNewNotes->m_sAttackString, sParams);
						ProcessAttacks(pNewNotes->m_Attacks, sParams);
					}
				}

				else if( sValueName=="OFFSET" )
				{
					if (out.m_fVersion >= VERSION_SPLIT_TIMING)
					{
						stepsTiming.m_fBeat0OffsetInSeconds = StringToFloat( sParams[1] );
						bHasOwnTiming = true;
					}
				}

				else if( sValueName=="DISPLAYBPM" )
				{
					// #DISPLAYBPM:[xxx][xxx:xxx]|[*]; 
					if( sParams[1] == "*" )
						pNewNotes->SetDisplayBPM(DISPLAY_BPM_RANDOM);
					else 
					{
						pNewNotes->SetDisplayBPM(DISPLAY_BPM_SPECIFIED);
						float min = StringToFloat(sParams[1]);
						pNewNotes->SetMinBPM(min);
						if(sParams[2].empty())
							pNewNotes->SetMaxBPM(min);
						else
							pNewNotes->SetMaxBPM(StringToFloat(sParams[2]));
					}
				}
				else if( sValueName=="STEPFILENAME" )
				{
					state = GETTING_SONG_INFO;
					if( bHasOwnTiming )
						pNewNotes->m_Timing = stepsTiming;
					pNewNotes->SetFilename(sParams[1]);
					out.AddSteps( pNewNotes );
				}
				break;
			}
		}
	}
	out.m_fVersion = STEPFILE_VERSION_NUMBER;
	TidyUpData(out, bFromCache);
	return true;
}

bool SSCLoader::LoadEditFromFile( RString sEditFilePath, ProfileSlot slot, bool bAddStepsToSong )
{
	LOG->Trace( "SSCLoader::LoadEditFromFile(%s)", sEditFilePath.c_str() );

	int iBytes = FILEMAN->GetFileSizeInBytes( sEditFilePath );
	if( iBytes > MAX_EDIT_STEPS_SIZE_BYTES )
	{
		LOG->UserLog("Edit file",
			     sEditFilePath,
			     "is unreasonably large. It won't be loaded." );
		return false;
	}

	MsdFile msd;
	if( !msd.ReadFile( sEditFilePath, true ) ) // unescape
	{
		LOG->UserLog("Edit file",
			     sEditFilePath,
			     "couldn't be opened: %s", msd.GetError().c_str() );
		return false;
	}

	return LoadEditFromMsd( msd, sEditFilePath, slot, bAddStepsToSong );
}

bool SSCLoader::LoadEditFromMsd(const MsdFile &msd,
				const RString &sEditFilePath,
				ProfileSlot slot,
				bool bAddStepsToSong )
{
	Song* pSong = NULL;
	Steps* pNewNotes = NULL;
	bool bSSCFormat = false;
	bool bHasOwnTiming = false;
	TimingData stepsTiming;

	for( unsigned i=0; i<msd.GetNumValues(); i++ )
	{
		int iNumParams = msd.GetNumParams(i);
		const MsdFile::value_t &sParams = msd.GetValue(i);
		RString sValueName = sParams[0];
		sValueName.MakeUpper();

		// handle the data
		if( sValueName=="VERSION" )
		{
			pSong->m_fVersion = StringToFloat( sParams[1] );
		}
		
		else if( sValueName=="SONG" )
		{
			if( pSong )
			{
				LOG->UserLog( "Edit file", sEditFilePath, "has more than one #SONG tag." );
				return false;
			}

			RString sSongFullTitle = sParams[1];
			this->SetSongTitle(sParams[1]);
			sSongFullTitle.Replace( '\\', '/' );

			pSong = SONGMAN->FindSong( sSongFullTitle );
			if( pSong == NULL )
			{
				LOG->UserLog("Edit file",
					     sEditFilePath,
					     "requires a song \"%s\" that isn't present.",
					     sSongFullTitle.c_str() );
				return false;
			}

			if( pSong->GetNumStepsLoadedFromProfile(slot) >= MAX_EDITS_PER_SONG_PER_PROFILE )
			{
				LOG->UserLog("Song file",
					     sSongFullTitle,
					     "already has the maximum number of edits allowed for ProfileSlotP%d.",
					     slot+1 );
				return false;
			}
		}

		else if( sValueName=="NOTEDATA" )
		{
			pNewNotes = pSong->CreateSteps();
			bSSCFormat = true;
		}
		if( sValueName=="STEPSTYPE" )
		{
			pNewNotes->m_StepsType = GAMEMAN->StringToStepsType( sParams[1] );
			bSSCFormat = true;
		}

		else if( sValueName=="CHARTSTYLE" )
		{
			pNewNotes->SetChartStyle( sParams[1] );
			bSSCFormat = true;
		}

		else if( sValueName=="DESCRIPTION" )
		{
			pNewNotes->SetDescription( sParams[1] );
			bSSCFormat = true;
		}

		else if( sValueName=="DIFFICULTY" )
		{
			pNewNotes->SetDifficulty( StringToDifficulty( sParams[1] ) );
			bSSCFormat = true;
		}

		else if( sValueName=="METER" )
		{
			pNewNotes->SetMeter( StringToInt( sParams[1] ) );
			bSSCFormat = true;
		}

		else if( sValueName=="RADARVALUES" )
		{
			vector<RString> saValues;
			split( sParams[1], ",", saValues, true );
			if( saValues.size() == NUM_RadarCategory * NUM_PLAYERS )
			{
				RadarValues v[NUM_PLAYERS];
				FOREACH_PlayerNumber( pn )
					FOREACH_ENUM( RadarCategory, rc )
						v[pn][rc] = StringToFloat( saValues[pn*NUM_RadarCategory + rc] );
				pNewNotes->SetCachedRadarValues( v );
			}
			bSSCFormat = true;
		}

		else if( sValueName=="CREDIT" )
		{
			pNewNotes->SetCredit( sParams[1] );
			bSSCFormat = true;
		}

		else if( sValueName=="BPMS" )
		{
			if( SMLoader::ProcessBPMs(stepsTiming, sParams[1]) )
				bHasOwnTiming = true;
			bSSCFormat = true;
		}
		
		else if( sValueName=="STOPS" )
		{
			SMLoader::ProcessStops(stepsTiming, sParams[1]);
			bSSCFormat = true;
		}
		
		else if( sValueName=="DELAYS" )
		{
			SMLoader::ProcessDelays(stepsTiming, sParams[1]);
			bSSCFormat = true;
		}
		
		else if( sValueName=="TIMESIGNATURES" )
		{
			SMLoader::ProcessTimeSignatures(stepsTiming, sParams[1]);
			bSSCFormat = true;
		}
		
		else if( sValueName=="TICKCOUNTS" )
		{
			SMLoader::ProcessTickcounts(stepsTiming, sParams[1]);
			bSSCFormat = true;
		}
		
		else if( sValueName=="COMBOS" )
		{
			ProcessCombos(stepsTiming, sParams[1]);
			bSSCFormat = true;
		}
		
		else if( sValueName=="WARPS" )
		{
			ProcessWarps(stepsTiming, sParams[1], pSong->m_fVersion);
			bSSCFormat = true;
		}
		
		else if( sValueName=="SPEEDS" )
		{
			ProcessSpeeds( stepsTiming, sParams[1] );
			bSSCFormat = true;
		}
		
		else if( sValueName=="FAKES" )
		{
			ProcessFakes( stepsTiming, sParams[1] );
			bSSCFormat = true;
		}
		
		else if( sValueName=="LABELS" )
		{
			ProcessLabels(stepsTiming, sParams[1]);
			bSSCFormat = true;
		}
		else if( sValueName=="NOTES" )
		{
			if( pSong == NULL )
			{
				LOG->UserLog("Edit file",
					     sEditFilePath,
					     "doesn't have a #SONG tag preceeding the first #NOTES tag." );
				return false;
			}

			if ( !bSSCFormat && iNumParams < 7 )
			{
				LOG->UserLog("Edit file",
					     sEditFilePath,
					     "has %d fields in a #NOTES tag, but should have at least 7.",
					     iNumParams );
				continue;
			}

			if( !bAddStepsToSong )
				return true;

			if( bSSCFormat )
			{
				if ( bHasOwnTiming )
				{
					pNewNotes->m_Timing = stepsTiming;
				}
				pNewNotes->SetSMNoteData( sParams[1] );
				pNewNotes->TidyUpData();
				pSong->AddSteps( pNewNotes );
			}
			else
			{
				pNewNotes = pSong->CreateSteps();
				LoadFromTokens(sParams[1],
					       sParams[2],
					       sParams[3],
					       sParams[4],
					       sParams[5],
					       sParams[6],
					       *pNewNotes);
			}

			pNewNotes->SetLoadedFromProfile( slot );
			pNewNotes->SetDifficulty( Difficulty_Edit );
			pNewNotes->SetFilename( sEditFilePath );

			if( pSong->IsEditAlreadyLoaded(pNewNotes) )
			{
				LOG->UserLog("Edit file",
					     sEditFilePath,
					     "is a duplicate of another edit that was already loaded." );
				SAFE_DELETE( pNewNotes );
				return false;
			}

			pSong->AddSteps( pNewNotes );
			return true; // Only allow one Steps per edit file!
		}
		else
		{
			LOG->UserLog("Edit file",
				     sEditFilePath,
				     "has an unexpected value \"%s\".",
				     sValueName.c_str() );
		}
	}

	//return true;
	// only load a SSC edit if it passes the checks. -aj
	return bSSCFormat;
}

/*
 * (c) 2011 Jason Felds
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
