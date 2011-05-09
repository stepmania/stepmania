#include "global.h"
#include "NotesLoaderSSC.h"
#include "BackgroundUtil.h"
#include "GameManager.h"
#include "MsdFile.h" // No JSON here.
#include "NoteTypes.h"
#include "NotesLoaderSM.h" // For loading SM style edits.
#include "RageFileManager.h"
#include "RageLog.h"
#include "RageUtil.h"
#include "Song.h"
#include "SongManager.h"
#include "Steps.h"
#include "Attack.h"
#include "PrefsManager.h"

/**
 * @brief A custom .edit file can only be so big before we have to reject it.
 */
const int MAX_EDIT_STEPS_SIZE_BYTES = 60*1024; // 60 KB

/**
 * @brief Attempt to load any background changes in use by this song.
 * 
 * This code is right now copied from NotesLoaderSM. There may be a time
 * when we add to this code, or perhaps just refactor it properly.
 * @param change a reference to the background change.
 * @param sBGChangeExpression a reference to the list of changes to be made.
 * @return its success or failure.
 */
bool LoadFromBGSSCChangesString( BackgroundChange &change, const RString &sBGChangeExpression )
{
	vector<RString> aBGChangeValues;
	split( sBGChangeExpression, "=", aBGChangeValues, false );

	aBGChangeValues.resize( min((int)aBGChangeValues.size(),11) );

	switch( aBGChangeValues.size() )
	{
		case 11:
			change.m_def.m_sColor2 = aBGChangeValues[10];
			change.m_def.m_sColor2.Replace( '^', ',' );
			change.m_def.m_sColor2 = RageColor::NormalizeColorString( change.m_def.m_sColor2 );
			// fall through
		case 10:
			change.m_def.m_sColor1 = aBGChangeValues[9];
			change.m_def.m_sColor1.Replace( '^', ',' );
			change.m_def.m_sColor1 = RageColor::NormalizeColorString( change.m_def.m_sColor1 );
			// fall through
		case 9:
			change.m_sTransition = aBGChangeValues[8];
			// fall through
		case 8:
			change.m_def.m_sFile2 = aBGChangeValues[7];
			// fall through
		case 7:
			change.m_def.m_sEffect = aBGChangeValues[6];
			// fall through
		case 6:
			// param 7 overrides this.
			// Backward compatibility:
			if( change.m_def.m_sEffect.empty() )
			{
				bool bLoop = atoi( aBGChangeValues[5] ) != 0;
				if( !bLoop )
					change.m_def.m_sEffect = SBE_StretchNoLoop;
			}
			// fall through
		case 5:
			// param 7 overrides this.
			// Backward compatibility:
			if( change.m_def.m_sEffect.empty() )
			{
				bool bRewindMovie = atoi( aBGChangeValues[4] ) != 0;
				if( bRewindMovie )
					change.m_def.m_sEffect = SBE_StretchRewind;
			}
			// fall through
		case 4:
			// param 9 overrides this.
			// Backward compatibility:
			if( change.m_sTransition.empty() )
				change.m_sTransition = (atoi( aBGChangeValues[3] ) != 0) ? "CrossFade" : "";
			// fall through
		case 3:
			change.m_fRate = StringToFloat( aBGChangeValues[2] );
			// fall through
		case 2:
			change.m_def.m_sFile1 = aBGChangeValues[1];
			// fall through
		case 1:
			change.m_fStartBeat = StringToFloat( aBGChangeValues[0] );
			// fall through
	}

	return aBGChangeValues.size() >= 2;
}

bool SSCLoader::LoadFromDir( const RString &sPath, Song &out )
{
	vector<RString> aFileNames;
	GetApplicableFiles( sPath,  aFileNames );

	if( aFileNames.size() > 1 )
	{
		LOG->UserLog( "Song", sPath, "has more than one SSC file. Only one SSC file is allowed per song." );
		return false;
	}

	ASSERT( aFileNames.size() == 1 ); // Ensure one was found entirely.

	return LoadFromSSCFile( sPath + aFileNames[0], out );
}

bool SSCLoader::LoadFromSSCFile( const RString &sPath, Song &out, bool bFromCache )
{
	LOG->Trace( "Song::LoadFromSSCFile(%s)", sPath.c_str() );

	MsdFile msd;
	if( !msd.ReadFile( sPath, true ) )
	{
		LOG->UserLog( "Song file", sPath, "couldn't be opened: %s", msd.GetError().c_str() );
		return false;
	}

	out.m_Timing.m_sFile = sPath; // songs still have their fallback timing.

	int state = GETTING_SONG_INFO;
	const unsigned values = msd.GetNumValues();
	Steps* pNewNotes = NULL;

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
					if( !bFromCache )
						continue;
					out.m_fMusicLengthSeconds = StringToFloat( sParams[1] );
				}

				else if( sValueName=="LASTBEATHINT" )
				{
					out.m_fSpecifiedLastBeat = StringToFloat( sParams[1] );
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
					if(!stricmp(sParams[1],"YES"))
						out.m_SelectionDisplay = out.SHOW_ALWAYS;
					else if(!stricmp(sParams[1],"NO"))
						out.m_SelectionDisplay = out.SHOW_NEVER;
					// ROULETTE from 3.9 is no longer in use.
					else if(!stricmp(sParams[1],"ROULETTE"))
						out.m_SelectionDisplay = out.SHOW_ALWAYS;
					/* The following two cases are just fixes to make sure simfiles that
					 * used 3.9+ features are not excluded here */
					else if(!stricmp(sParams[1],"ES") || !stricmp(sParams[1],"OMES"))
						out.m_SelectionDisplay = out.SHOW_ALWAYS;
					else if( atoi(sParams[1]) > 0 )
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
							if( LoadFromBGSSCChangesString( change, aBGChangeExpressions[b] ) )
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
						if( LoadFromBGSSCChangesString( change, aFGChangeExpressions[b] ) )
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

				else if( sValueName=="OFFSET" )
				{
					out.m_Timing.m_fBeat0OffsetInSeconds = StringToFloat( sParams[1] );
				}
				/* Below are the song based timings that should only be used
				 * if the steps do not have their own timing. */
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

						const float fFreezeBeat = StringToFloat( arrayFreezeValues[0] );
						const float fFreezeSeconds = StringToFloat( arrayFreezeValues[1] );
						StopSegment new_seg( BeatToNoteRow(fFreezeBeat), fFreezeSeconds );

						if(fFreezeSeconds > 0.0f)
						{
							// LOG->Trace( "Adding a freeze segment: beat: %f, seconds = %f", new_seg.m_fStartBeat, new_seg.m_fStopSeconds );
							out.m_Timing.AddStopSegment( new_seg );
						}
						else
						{
							// negative stops (hi JS!) -aj
							if( PREFSMAN->m_bQuirksMode )
							{
								// LOG->Trace( "Adding a negative freeze segment: beat: %f, seconds = %f", new_seg.m_fStartBeat, new_seg.m_fStopSeconds );
								out.m_Timing.AddStopSegment( new_seg );
							}
							else
								LOG->UserLog( "Song file", "(UNKNOWN)", "has an invalid stop at beat %f, length %f.", fFreezeBeat, fFreezeSeconds );
						}
					}
				}
				else if( sValueName=="DELAYS" )
				{
					vector<RString> arrayDelayExpressions;
					split( sParams[1], ",", arrayDelayExpressions );

					for( unsigned f=0; f<arrayDelayExpressions.size(); f++ )
					{
						vector<RString> arrayDelayValues;
						split( arrayDelayExpressions[f], "=", arrayDelayValues );
						if( arrayDelayValues.size() != 2 )
						{
							// XXX: Hard to tell which file caused this.
							LOG->UserLog( "Song file", "(UNKNOWN)", "has an invalid #%s value \"%s\" (must have exactly one '='), ignored.",
									 sValueName.c_str(), arrayDelayExpressions[f].c_str() );
							continue;
						}

						const float fFreezeBeat = StringToFloat( arrayDelayValues[0] );
						const float fFreezeSeconds = StringToFloat( arrayDelayValues[1] );

						StopSegment new_seg( BeatToNoteRow(fFreezeBeat), fFreezeSeconds, true );

						// LOG->Trace( "Adding a delay segment: beat: %f, seconds = %f", new_seg.m_fStartBeat, new_seg.m_fStopSeconds );

						if(fFreezeSeconds > 0.0f)
							out.m_Timing.AddStopSegment( new_seg );
						else
							LOG->UserLog( "Song file", "(UNKNOWN)", "has an invalid delay at beat %f, length %f.", fFreezeBeat, fFreezeSeconds );
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

						const float fBeat = StringToFloat( arrayBPMChangeValues[0] );
						const float fNewBPM = StringToFloat( arrayBPMChangeValues[1] );

						if(fNewBPM > 0.0f)
							out.m_Timing.AddBPMSegment( BPMSegment(BeatToNoteRow(fBeat), fNewBPM) );
						else
						{
							out.m_Timing.m_bHasNegativeBpms = true;
							// only add Negative BPMs in quirks mode -aj
							if( PREFSMAN->m_bQuirksMode )
								out.m_Timing.AddBPMSegment( BPMSegment(BeatToNoteRow(fBeat), fNewBPM) );
							else
								LOG->UserLog( "Song file", "(UNKNOWN)", "has an invalid BPM change at beat %f, BPM %f.", fBeat, fNewBPM );
						}
					}
				}
				
				else if( sValueName=="WARPS" )
				{
					vector<RString> arrayWarpExpressions;
					split( sParams[1], ",", arrayWarpExpressions );
					
					for( unsigned b=0; b<arrayWarpExpressions.size(); b++ )
					{
						vector<RString> arrayWarpValues;
						split( arrayWarpExpressions[b], "=", arrayWarpValues );
						// XXX: Hard to tell which file caused this.
						if( arrayWarpValues.size() != 2 )
						{
							LOG->UserLog( "Song file", "(UNKNOWN)", "has an invalid #%s value \"%s\" (must have exactly one '='), ignored.",
								     sValueName.c_str(), arrayWarpExpressions[b].c_str() );
							continue;
						}
						
						const float fBeat = StringToFloat( arrayWarpValues[0] );
						const float fNewBeat = StringToFloat( arrayWarpValues[1] );
						
						if(fNewBeat > fBeat)
							out.m_Timing.AddWarpSegment( WarpSegment(fBeat, fNewBeat) );
						else
						{
							LOG->UserLog( "Song file", "(UNKNOWN)", "has an invalid Warp at beat %f, BPM %f.", fBeat, fNewBeat );
						}
					}
				}
				
				else if( sValueName=="LABELS" )
				{
					vector<RString> arrayLabelExpressions;
					split( sParams[1], ",", arrayLabelExpressions );
					
					for( unsigned b=0; b<arrayLabelExpressions.size(); b++ )
					{
						vector<RString> arrayLabelValues;
						split( arrayLabelExpressions[b], "=", arrayLabelValues );
						if( arrayLabelValues.size() != 2 )
						{
							LOG->UserLog( "Song file", "(UNKNOWN)", "has an invalid #%s value \"%s\" (must have exactly one '='), ignored.",
								     sValueName.c_str(), arrayLabelExpressions[b].c_str() );
							continue;
						}
						
						const float fBeat = StringToFloat( arrayLabelValues[0] );
						RString sLabel = arrayLabelValues[1];
						TrimRight(sLabel);
						if( fBeat >= 0.0f )
							out.m_Timing.AddLabelSegment( LabelSegment(fBeat, sLabel) );
						else 
						{
							LOG->UserLog( "Song file", "(UNKNOWN)", "has an invalid Label at beat %f called %s.", fBeat, sLabel.c_str() );
						}

					}
				}

				else if( sValueName=="TIMESIGNATURES" )
				{
					vector<RString> vs1;
					split( sParams[1], ",", vs1 );

					FOREACH_CONST( RString, vs1, s1 )
					{
						vector<RString> vs2;
						split( *s1, "=", vs2 );

						if( vs2.size() < 3 )
						{
							LOG->UserLog( "Song file", "(UNKNOWN)", "has an invalid time signature change with %i values.", (int)vs2.size() );
							continue;
						}

						const float fBeat = StringToFloat( vs2[0] );

						TimeSignatureSegment seg;
						seg.m_iStartRow = BeatToNoteRow(fBeat);
						seg.m_iNumerator = atoi( vs2[1] ); 
						seg.m_iDenominator = atoi( vs2[2] ); 

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

						if( seg.m_iDenominator < 1 )
						{
							LOG->UserLog( "Song file", "(UNKNOWN)", "has an invalid time signature change with beat %f, iDenominator %i.", fBeat, seg.m_iDenominator );
							continue;
						}

						out.m_Timing.AddTimeSignatureSegment( seg );
					}
				}

				else if( sValueName=="TICKCOUNTS" )
				{
					vector<RString> arrayTickcountExpressions;
					split( sParams[1], ",", arrayTickcountExpressions );

					for( unsigned f=0; f<arrayTickcountExpressions.size(); f++ )
					{
						vector<RString> arrayTickcountValues;
						split( arrayTickcountExpressions[f], "=", arrayTickcountValues );
						if( arrayTickcountValues.size() != 2 )
						{
							// XXX: Hard to tell which file caused this.
							LOG->UserLog( "Song file", "(UNKNOWN)", "has an invalid #%s value \"%s\" (must have exactly one '='), ignored.",
									 sValueName.c_str(), arrayTickcountExpressions[f].c_str() );
							continue;
						}

						const float fTickcountBeat = StringToFloat( arrayTickcountValues[0] );
						const int iTicks = atoi( arrayTickcountValues[1] );
						TickcountSegment new_seg( BeatToNoteRow(fTickcountBeat), iTicks );

						if(iTicks >= 1 && iTicks <= ROWS_PER_BEAT ) // Constants
						{
							// LOG->Trace( "Adding a tickcount segment: beat: %f, ticks = %d", fTickcountBeat, iTicks );
							out.m_Timing.AddTickcountSegment( new_seg );
						}
						else
						{
							LOG->UserLog( "Song file", "(UNKNOWN)", "has an invalid tickcount at beat %f, ticks %d.", fTickcountBeat, iTicks );
						}
					}
				}

				else if( sValueName=="COMBOS" )
				{
					vector<RString> arrayComboExpressions;
					split( sParams[1], ",", arrayComboExpressions );

					for( unsigned f=0; f<arrayComboExpressions.size(); f++ )
					{
						vector<RString> arrayComboValues;
						split( arrayComboExpressions[f], "=", arrayComboValues );
						if( arrayComboValues.size() != 2 )
						{
							LOG->UserLog( "Song file", "(UNKNOWN)", "has an invalid #%s value \"%s\" (must have exactly one '='), ignored.",
									 sValueName.c_str(), arrayComboExpressions[f].c_str() );
							continue;
						}
						const float fComboBeat = StringToFloat( arrayComboValues[0] );
						const int iCombos = atoi( arrayComboValues[1] );
						ComboSegment new_seg( BeatToNoteRow( fComboBeat ), iCombos );
						out.m_Timing.AddComboSegment( new_seg );
					}
				}

				/* The following are cache tags. Never fill their values
				 * directly: only from the cached version. */
				else if( sValueName=="FIRSTBEAT" )
				{
					if( bFromCache )
						out.m_fFirstBeat = StringToFloat( sParams[1] );
				}

				else if( sValueName=="LASTBEAT" )
				{
					if( bFromCache )
						out.m_fLastBeat = StringToFloat( sParams[1] );
				}

				else if( sValueName=="SONGFILENAME" )
				{
					if( bFromCache )
						out.m_sSongFileName = sParams[1];
				}

				else if( sValueName=="HASMUSIC" )
				{
					if( bFromCache )
						out.m_bHasMusic = atoi( sParams[1] ) != 0;
				}

				else if( sValueName=="HASBANNER" )
				{
					if( bFromCache )
						out.m_bHasBanner = atoi( sParams[1] ) != 0;
				}

				// This tag will get us to the next section.
				else if( sValueName=="NOTEDATA" )
				{
					state = GETTING_STEP_INFO;
					pNewNotes = out.CreateSteps();
				}
				break;
			}
			case GETTING_STEP_INFO:
			{
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
					pNewNotes->SetDescription( sParams[1] );
				}

				else if( sValueName=="DIFFICULTY" )
				{
					pNewNotes->SetDifficulty( DwiCompatibleStringToDifficulty( sParams[1] ) );
				}

				else if( sValueName=="METER" )
				{
					pNewNotes->SetMeter( atoi( sParams[1] ) );
				}

				else if( sValueName=="RADARVALUES" )
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

				else if( sValueName=="CREDIT" )
				{
					pNewNotes->SetCredit( sParams[1] );
				}

				else if( sValueName=="NOTES" || sValueName=="NOTES2" )
				{
					state = GETTING_SONG_INFO;
					//pNewNotes->m_Timing = out.m_Timing;
					pNewNotes->SetSMNoteData( sParams[1] );
					pNewNotes->TidyUpData();
					out.AddSteps( pNewNotes );
				}

				else if( sValueName=="BPMS" )
				{
					/*
					state = GETTING_STEP_TIMING_INFO;
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

						const float fBeat = StringToFloat( arrayBPMChangeValues[0] );
						const float fNewBPM = StringToFloat( arrayBPMChangeValues[1] );

						if(fNewBPM > 0.0f)
							pNewNotes->m_Timing.AddBPMSegment( BPMSegment(BeatToNoteRow(fBeat), fNewBPM) );
						else
						{
							pNewNotes->m_Timing.m_bHasNegativeBpms = true;
							// only add Negative BPMs in quirks mode -aj
							if( PREFSMAN->m_bQuirksMode )
								pNewNotes->m_Timing.AddBPMSegment( BPMSegment(BeatToNoteRow(fBeat), fNewBPM) );
							else
								LOG->UserLog( "Song file", "(UNKNOWN)", "has an invalid BPM change at beat %f, BPM %f.", fBeat, fNewBPM );
						}
					}
					 */
				}
				break;
			}
			case GETTING_STEP_TIMING_INFO:
			{
				if( sValueName=="STOPS" )
				{
					// copy from above when it's time.
				}
				else if( sValueName=="DELAYS" )
				{
					// copy from above when it's time.
				}
				else if( sValueName=="TIMESIGNATURES" )
				{
					// copy from above when it's time.
				}

				else if( sValueName=="TICKCOUNTS" )
				{
					// copy from above when it's time.
				}
				else if( sValueName=="COMBOS" )
				{
					// copy from above when it's time.
				}
				else if( sValueName=="WARPS" || sValueName=="LABELS" )
				{
					// copy from above when it's time.
				}
				else if( sValueName=="ATTACKS" )
				{
					// TODO: Look into Step attacks vs Song Attacks. -Wolfman2000
					/*
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
					*/
				}
				else if( sValueName=="OFFSET" )
				{/*
					pNewNotes->m_Timing.m_fBeat0OffsetInSeconds = StringToFloat( sParams[1] );
				  */
				}

				else if( sValueName=="NOTES" )
				{
					state = GETTING_SONG_INFO;
					// pNewNotes->m_Timing.m_fBeat0OffsetInSeconds = out.m_Timing.m_fBeat0OffsetInSeconds;
					pNewNotes->SetSMNoteData( sParams[1] );
					pNewNotes->TidyUpData();
					out.AddSteps( pNewNotes );
				}
				break;
			}
		}
	}
	out.m_fVersion = STEPFILE_VERSION_NUMBER;
	return true;
}

void SSCLoader::GetApplicableFiles( const RString &sPath, vector<RString> &out )
{
	GetDirListing( sPath + RString("*.ssc"), out );
}

bool SSCLoader::LoadEditFromFile( RString sEditFilePath, ProfileSlot slot, bool bAddStepsToSong )
{
	LOG->Trace( "SSCLoader::LoadEditFromFile(%s)", sEditFilePath.c_str() );

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

bool SSCLoader::LoadEditFromMsd( const MsdFile &msd, const RString &sEditFilePath, ProfileSlot slot, bool bAddStepsToSong )
{
	Song* pSong = NULL;
	Steps* pNewNotes = NULL;
	bool bSSCFormat = false;

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
			pNewNotes->SetDifficulty( DwiCompatibleStringToDifficulty( sParams[1] ) );
			bSSCFormat = true;
		}

		else if( sValueName=="METER" )
		{
			pNewNotes->SetMeter( atoi( sParams[1] ) );
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

		// TimingData for Steps isn't set yet, but still prepare for it.
		else if( sValueName=="BPMS" )
		{
			bSSCFormat = true;
		}
		else if( sValueName=="STOPS" )
		{
			bSSCFormat = true;
		}
		else if( sValueName=="DELAYS" )
		{
			bSSCFormat = true;
		}
		else if( sValueName=="TIMESIGNATURES" )
		{
			bSSCFormat = true;
		}
		else if( sValueName=="TICKCOUNTS" )
		{
			bSSCFormat = true;
		}
		else if( sValueName=="COMBOS" )
		{
			bSSCFormat = true;
		}

		else if( sValueName=="NOTES" )
		{
			if( pSong == NULL )
			{
				LOG->UserLog( "Edit file", sEditFilePath, "doesn't have a #SONG tag preceeding the first #NOTES tag." );
				return false;
			}

			if ( !bSSCFormat && iNumParams < 7 )
			{
				LOG->UserLog( "Edit file", sEditFilePath, "has %d fields in a #NOTES tag, but should have at least 7.", iNumParams );
				continue;
			}

			if( !bAddStepsToSong )
				return true;

			if( bSSCFormat )
			{
				pNewNotes->SetSMNoteData( sParams[1] );
				pNewNotes->TidyUpData();
				pSong->AddSteps( pNewNotes );
			}
			else
			{
				pNewNotes = pSong->CreateSteps();
				SMLoader::LoadFromSMTokens( 
						 sParams[1], sParams[2], sParams[3], sParams[4], sParams[5], sParams[6],
						 *pNewNotes);
			}

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

	//return true;
	// only load a SSC edit if it passes the checks. -aj
	return bSSCFormat;
}

void SSCLoader::TidyUpData( Song &song, bool bFromCache )
{
	/*
	 * Hack: if the song has any changes at all (so it won't use a random BGA)
	 * and doesn't end with "-nosongbg-", add a song background BGC.  Remove
	 * "-nosongbg-" if it exists.
	 *
	 * This way, songs that were created earlier, when we added the song BG
	 * at the end by default, will still behave as expected; all new songs will
	 * have to add an explicit song BG tag if they want it.  This is really a
	 * formatting hack only; nothing outside of SMLoader ever sees "-nosongbg-".
	 */
	vector<BackgroundChange> &bg = song.GetBackgroundChanges(BACKGROUND_LAYER_1);
	if( !bg.empty() )
	{
		/* BGChanges have been sorted. On the odd chance that a BGChange exists
		 * with a very high beat, search the whole list. */
		bool bHasNoSongBgTag = false;

		for( unsigned i = 0; !bHasNoSongBgTag && i < bg.size(); ++i )
		{
			if( !bg[i].m_def.m_sFile1.CompareNoCase(NO_SONG_BG_FILE) )
			{
				bg.erase( bg.begin()+i );
				bHasNoSongBgTag = true;
			}
		}

		// If there's no -nosongbg- tag, add the song BG.
		if( !bHasNoSongBgTag ) do
		{
			/* If we're loading cache, -nosongbg- should always be in there. We
			 * must not call IsAFile(song.GetBackgroundPath()) when loading cache. */
			if( bFromCache )
				break;

			/* If BGChanges already exist after the last beat, don't add the
			 * background in the middle. */
			if( !bg.empty() && bg.back().m_fStartBeat-0.0001f >= song.m_fLastBeat )
				break;

			// If the last BGA is already the song BGA, don't add a duplicate.
			if( !bg.empty() && !bg.back().m_def.m_sFile1.CompareNoCase(song.m_sBackgroundFile) )
				break;

			if( !IsAFile( song.GetBackgroundPath() ) )
				break;

			bg.push_back( BackgroundChange(song.m_fLastBeat,song.m_sBackgroundFile) );
		} while(0);
	}
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
