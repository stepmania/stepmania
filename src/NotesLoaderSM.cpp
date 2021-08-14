#include "global.h"
#include "NotesLoaderSM.h"
#include "BackgroundUtil.h"
#include "GameManager.h"
#include "MsdFile.h"
#include "NoteTypes.h"
#include "RageFileManager.h"
#include "RageLog.h"
#include "RageUtil.h"
#include "Song.h"
#include "SongManager.h"
#include "Steps.h"
#include "Attack.h"
#include "PrefsManager.h"

// Everything from this line to the creation of sm_parser_helper exists to
// speed up parsing by allowing the use of std::map.  All these functions
// are put into a map of function pointers which is used when loading.
// -Kyz
/****************************************************************/
struct SMSongTagInfo
{
	SMLoader* loader;
	Song* song;
	const MsdFile::value_t* params;
	const RString& path;
	vector< pair<float, float> > BPMChanges, Stops;
	SMSongTagInfo(SMLoader* l, Song* s, const RString& p)
		:loader(l), song(s), path(p)
	{}
};

typedef void (*song_tag_func_t)(SMSongTagInfo& info);

// Functions for song tags go below this line. -Kyz
/****************************************************************/
void SMSetTitle(SMSongTagInfo& info)
{
	info.song->m_sMainTitle = (*info.params)[1];
	info.loader->SetSongTitle((*info.params)[1]);
}
void SMSetSubtitle(SMSongTagInfo& info)
{
	info.song->m_sSubTitle = (*info.params)[1];
}
void SMSetArtist(SMSongTagInfo& info)
{
	info.song->m_sArtist = (*info.params)[1];
}
void SMSetTitleTranslit(SMSongTagInfo& info)
{
	info.song->m_sMainTitleTranslit = (*info.params)[1];
}
void SMSetSubtitleTranslit(SMSongTagInfo& info)
{
	info.song->m_sSubTitleTranslit = (*info.params)[1];
}
void SMSetArtistTranslit(SMSongTagInfo& info)
{
	info.song->m_sArtistTranslit = (*info.params)[1];
}
void SMSetGenre(SMSongTagInfo& info)
{
	info.song->m_sGenre = (*info.params)[1];
}
void SMSetCredit(SMSongTagInfo& info)
{
	info.song->m_sCredit = (*info.params)[1];
}
void SMSetBanner(SMSongTagInfo& info)
{
	info.song->m_sBannerFile = (*info.params)[1];
}
void SMSetBackground(SMSongTagInfo& info)
{
	info.song->m_sBackgroundFile = (*info.params)[1];
}
void SMSetLyricsPath(SMSongTagInfo& info)
{
	info.song->m_sLyricsFile = (*info.params)[1];
}
void SMSetCDTitle(SMSongTagInfo& info)
{
	info.song->m_sCDTitleFile = (*info.params)[1];
}
void SMSetMusic(SMSongTagInfo& info)
{
	info.song->m_sMusicFile = (*info.params)[1];
}
void SMSetOffset(SMSongTagInfo& info)
{
	info.song->m_SongTiming.m_fBeat0OffsetInSeconds = StringToFloat((*info.params)[1]);
}
void SMSetBPMs(SMSongTagInfo& info)
{
	info.BPMChanges.clear();
	info.loader->ParseBPMs(info.BPMChanges, (*info.params)[1]);
}
void SMSetStops(SMSongTagInfo& info)
{
	info.Stops.clear();
	info.loader->ParseStops(info.Stops, (*info.params)[1]);
}
void SMSetDelays(SMSongTagInfo& info)
{
	info.loader->ProcessDelays(info.song->m_SongTiming, (*info.params)[1]);
}
void SMSetTimeSignatures(SMSongTagInfo& info)
{
	info.loader->ProcessTimeSignatures(info.song->m_SongTiming, (*info.params)[1]);
}
void SMSetTickCounts(SMSongTagInfo& info)
{
	info.loader->ProcessTickcounts(info.song->m_SongTiming, (*info.params)[1]);
}
void SMSetInstrumentTrack(SMSongTagInfo& info)
{
	info.loader->ProcessInstrumentTracks(*info.song, (*info.params)[1]);
}
void SMSetSampleStart(SMSongTagInfo& info)
{
	info.song->m_fMusicSampleStartSeconds = HHMMSSToSeconds((*info.params)[1]);
}
void SMSetSampleLength(SMSongTagInfo& info)
{
	info.song->m_fMusicSampleLengthSeconds = HHMMSSToSeconds((*info.params)[1]);
}
void SMSetDisplayBPM(SMSongTagInfo& info)
{
	// #DISPLAYBPM:[xxx][xxx:xxx]|[*];
	if((*info.params)[1] == "*")
	{ info.song->m_DisplayBPMType = DISPLAY_BPM_RANDOM; }
	else
	{
		info.song->m_DisplayBPMType = DISPLAY_BPM_SPECIFIED;
		info.song->m_fSpecifiedBPMMin = StringToFloat((*info.params)[1]);
		if((*info.params)[2].empty())
		{ info.song->m_fSpecifiedBPMMax = info.song->m_fSpecifiedBPMMin; }
		else
		{ info.song->m_fSpecifiedBPMMax = StringToFloat((*info.params)[2]); }
	}
}
void SMSetSelectable(SMSongTagInfo& info)
{
	if((*info.params)[1].EqualsNoCase("YES"))
	{ info.song->m_SelectionDisplay = info.song->SHOW_ALWAYS; }
	else if((*info.params)[1].EqualsNoCase("NO"))
	{ info.song->m_SelectionDisplay = info.song->SHOW_NEVER; }
	// ROULETTE from 3.9. It was removed since UnlockManager can serve
	// the same purpose somehow. This, of course, assumes you're using
	// unlocks. -aj
	else if((*info.params)[1].EqualsNoCase("ROULETTE"))
	{ info.song->m_SelectionDisplay = info.song->SHOW_ALWAYS; }
	/* The following two cases are just fixes to make sure simfiles that
	 * used 3.9+ features are not excluded here */
	else if((*info.params)[1].EqualsNoCase("ES") || (*info.params)[1].EqualsNoCase("OMES"))
	{ info.song->m_SelectionDisplay = info.song->SHOW_ALWAYS; }
	else if(StringToInt((*info.params)[1]) > 0)
	{ info.song->m_SelectionDisplay = info.song->SHOW_ALWAYS; }
	else
	{ LOG->UserLog("Song file", info.path, "has an unknown #SELECTABLE value, \"%s\"; ignored.", (*info.params)[1].c_str()); }
}
void SMSetBGChanges(SMSongTagInfo& info)
{
	info.loader->ProcessBGChanges(*info.song, (*info.params)[0], info.path, (*info.params)[1]);
}
void SMSetFGChanges(SMSongTagInfo& info)
{
	std::vector<std::vector<RString> > aFGChanges;
	info.loader->ParseBGChangesString((*info.params)[1], aFGChanges, info.song->GetSongDir());

	for (const auto &b : aFGChanges)
	{
		BackgroundChange change;
		if (info.loader->LoadFromBGChangesVector(change, b))
			info.song->AddForegroundChange(change);
	}
}
void SMSetKeysounds(SMSongTagInfo& info)
{
	split((*info.params)[1], ",", info.song->m_vsKeysoundFile);
}
void SMSetAttacks(SMSongTagInfo& info)
{
	info.loader->ProcessAttackString(info.song->m_sAttackString, (*info.params));
	info.loader->ProcessAttacks(info.song->m_Attacks, (*info.params));
}

typedef std::map<RString, song_tag_func_t> song_handler_map_t;

struct sm_parser_helper_t
{
	song_handler_map_t song_tag_handlers;
	// Unless signed, the comments in this tag list are not by me.  They were
	// moved here when converting from the else if chain. -Kyz
	sm_parser_helper_t()
	{
		song_tag_handlers["TITLE"]= &SMSetTitle;
		song_tag_handlers["SUBTITLE"]= &SMSetSubtitle;
		song_tag_handlers["ARTIST"]= &SMSetArtist;
		song_tag_handlers["TITLETRANSLIT"]= &SMSetTitleTranslit;
		song_tag_handlers["SUBTITLETRANSLIT"]= &SMSetSubtitleTranslit;
		song_tag_handlers["ARTISTTRANSLIT"]= &SMSetArtistTranslit;
		song_tag_handlers["GENRE"]= &SMSetGenre;
		song_tag_handlers["CREDIT"]= &SMSetCredit;
		song_tag_handlers["BANNER"]= &SMSetBanner;
		song_tag_handlers["BACKGROUND"]= &SMSetBackground;
		// Save "#LYRICS" for later, so we can add an internal lyrics tag.
		song_tag_handlers["LYRICSPATH"]= &SMSetLyricsPath;
		song_tag_handlers["CDTITLE"]= &SMSetCDTitle;
		song_tag_handlers["MUSIC"]= &SMSetMusic;
		song_tag_handlers["OFFSET"]= &SMSetOffset;
		song_tag_handlers["BPMS"]= &SMSetBPMs;
		song_tag_handlers["STOPS"]= &SMSetStops;
		song_tag_handlers["FREEZES"]= &SMSetStops;
		song_tag_handlers["DELAYS"]= &SMSetDelays;
		song_tag_handlers["TIMESIGNATURES"]= &SMSetTimeSignatures;
		song_tag_handlers["TICKCOUNTS"]= &SMSetTickCounts;
		song_tag_handlers["INSTRUMENTTRACK"]= &SMSetInstrumentTrack;
		song_tag_handlers["SAMPLESTART"]= &SMSetSampleStart;
		song_tag_handlers["SAMPLELENGTH"]= &SMSetSampleLength;
		song_tag_handlers["DISPLAYBPM"]= &SMSetDisplayBPM;
		song_tag_handlers["SELECTABLE"]= &SMSetSelectable;
		// It's a bit odd to have the tag that exists for backwards compatibility
		// in this list and not the replacement, but the BGCHANGES tag has a
		// number on the end, allowing up to NUM_BackgroundLayer tags, so it
		// can't fit in the map. -Kyz
		song_tag_handlers["ANIMATIONS"]= &SMSetBGChanges;
		song_tag_handlers["FGCHANGES"]= &SMSetFGChanges;
		song_tag_handlers["KEYSOUNDS"]= &SMSetKeysounds;
		// Attacks loaded from file
		song_tag_handlers["ATTACKS"]= &SMSetAttacks;
		/* Tags that no longer exist, listed for posterity.  May their names
		 * never be forgotten for their service to Stepmania. -Kyz
		 * LASTBEATHINT: // unable to identify at this point: ignore
		 * MUSICBYTES: // ignore
		 * FIRSTBEAT: // cache tags from older SM files: ignore.
		 * LASTBEAT: // cache tags from older SM files: ignore.
		 * SONGFILENAME: // cache tags from older SM files: ignore.
		 * HASMUSIC: // cache tags from older SM files: ignore.
		 * HASBANNER: // cache tags from older SM files: ignore.
		 * SAMPLEPATH: // SamplePath was used when the song has a separate preview clip. -aj
		 * LEADTRACK: // XXX: Does anyone know what LEADTRACK is for? -Wolfman2000
		 * MUSICLENGTH: // Loaded from the cache now. -Kyz
		 */
	}
};
sm_parser_helper_t sm_parser_helper;
// End sm_parser_helper related functions. -Kyz
/****************************************************************/

void SMLoader::SetSongTitle(const RString & title)
{
	this->songTitle = title;
}

RString SMLoader::GetSongTitle() const
{
	return this->songTitle;
}

bool SMLoader::LoadFromDir( const RString &sPath, Song &out, bool load_autosave )
{
	vector<RString> aFileNames;
	GetApplicableFiles( sPath, aFileNames, load_autosave );
	return LoadFromSimfile( sPath + aFileNames[0], out );
}

float SMLoader::RowToBeat( RString line, const int rowsPerBeat )
{
	RString backup = line;
	Trim(line, "r");
	Trim(line, "R");
	if( backup != line )
	{
		return StringToFloat( line ) / rowsPerBeat;
	}
	else
	{
		return StringToFloat( line );
	}
}

void SMLoader::LoadFromTokens( 
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

	// LOG->Trace( "Steps::LoadFromTokens(), %s", sStepsType.c_str() );

	// backwards compatibility hacks:
	// HACK: We eliminated "ez2-single-hard", but we should still handle it.
	if( sStepsType == "ez2-single-hard" )
		sStepsType = "ez2-single";

	// HACK: "para-single" used to be called just "para"
	if( sStepsType == "para" )
		sStepsType = "para-single";

	out.m_StepsType = GAMEMAN->StringToStepsType( sStepsType );
	out.m_StepsTypeStr = sStepsType;
	out.SetDescription( sDescription );
	out.SetCredit( sDescription ); // this is often used for both.
	out.SetChartName(sDescription); // yeah, one more for good measure.
	out.SetDifficulty( OldStyleStringToDifficulty(sDifficulty) );

	// Handle hacks that originated back when StepMania didn't have
	// Difficulty_Challenge. (At least v1.64, possibly v3.0 final...)
	if( out.GetDifficulty() == Difficulty_Hard )
	{
		// HACK: SMANIAC used to be Difficulty_Hard with a special description.
		if( sDescription.CompareNoCase("smaniac") == 0 ) 
			out.SetDifficulty( Difficulty_Challenge );

		// HACK: CHALLENGE used to be Difficulty_Hard with a special description.
		if( sDescription.CompareNoCase("challenge") == 0 ) 
			out.SetDifficulty( Difficulty_Challenge );
	}

	if( sMeter.empty() )
	{
		// some simfiles (e.g. X-SPECIALs from Zenius-I-Vanisher) don't
		// have a meter on certain steps. Make the meter 1 in these instances.
		sMeter = "1";
	}
	out.SetMeter( StringToInt(sMeter) );

	out.SetSMNoteData( sNoteData );

	out.TidyUpData();
}

void SMLoader::ProcessBGChanges( Song &out, const RString &sValueName, const RString &sPath, const RString &sParam )
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
		std::vector<std::vector<RString> > aBGChanges;
		ParseBGChangesString(sParam, aBGChanges, out.GetSongDir());

		for (const auto &b : aBGChanges)
		{
			BackgroundChange change;
			if(LoadFromBGChangesVector( change, b))
				out.AddBackgroundChange(iLayer, change);
		}
	}
}

void SMLoader::ProcessAttackString( vector<RString> & attacks, MsdFile::value_t params )
{
	for( unsigned s=1; s < params.params.size(); ++s )
	{
		RString tmp = params[s];
		Trim(tmp);
		if (tmp.size() > 0)
			attacks.push_back( tmp );
	}
}

void SMLoader::ProcessAttacks( AttackArray &attacks, MsdFile::value_t params )
{
	Attack attack;
	float end = -9999;
	
	for( unsigned j=1; j < params.params.size(); ++j )
	{
		vector<RString> sBits;
		split( params[j], "=", sBits, false );
		
		// Need an identifer and a value for this to work
		if( sBits.size() < 2 )
			continue;
		
		Trim( sBits[0] );
		
		if( !sBits[0].CompareNoCase("TIME") )
			attack.fStartSecond = strtof( sBits[1], nullptr );
		else if( !sBits[0].CompareNoCase("LEN") )
			attack.fSecsRemaining = strtof( sBits[1], nullptr );
		else if( !sBits[0].CompareNoCase("END") )
			end = strtof( sBits[1], nullptr );
		else if( !sBits[0].CompareNoCase("MODS") )
		{
			Trim(sBits[1]);
			attack.sModifiers = sBits[1];
			
			if( end != -9999 )
			{
				attack.fSecsRemaining = end - attack.fStartSecond;
				end = -9999;
			}
			
			if( attack.fSecsRemaining < 0.0f )
				attack.fSecsRemaining = 0.0f;
			
			attacks.push_back( attack );
		}
	}
}

void SMLoader::ProcessInstrumentTracks( Song &out, const RString &sParam )
{
	vector<RString> vs1;
	split( sParam, ",", vs1 );
	for (RString const &s : vs1)
	{
		vector<RString> vs2;
		split( s, "=", vs2 );
		if( vs2.size() >= 2 )
		{
			InstrumentTrack it = StringToInstrumentTrack( vs2[0] );
			if( it != InstrumentTrack_Invalid )
				out.m_sInstrumentTrackFile[it] = vs2[1];
		}
	}
}

void SMLoader::ParseBPMs( vector< pair<float, float> > &out, const RString line, const int rowsPerBeat )
{
	vector<RString> arrayBPMChangeExpressions;
	split( line, ",", arrayBPMChangeExpressions );

	for( unsigned b=0; b<arrayBPMChangeExpressions.size(); b++ )
	{
		vector<RString> arrayBPMChangeValues;
		split( arrayBPMChangeExpressions[b], "=", arrayBPMChangeValues );
		if( arrayBPMChangeValues.size() != 2 )
		{
			LOG->UserLog("Song file",
				     this->GetSongTitle(),
				     "has an invalid #BPMs value \"%s\" (must have exactly one '='), ignored.",
				     arrayBPMChangeExpressions[b].c_str() );
			continue;
		}

		const float fBeat = RowToBeat( arrayBPMChangeValues[0], rowsPerBeat );
		const float fNewBPM = StringToFloat( arrayBPMChangeValues[1] );
		if( fNewBPM == 0 ) {
			LOG->UserLog("Song file", this->GetSongTitle(),
				     "has a zero BPM; ignored.");
			continue;
		}

		out.push_back( make_pair(fBeat, fNewBPM) );
	}
}

void SMLoader::ParseStops( vector< pair<float, float> > &out, const RString line, const int rowsPerBeat )
{
	vector<RString> arrayFreezeExpressions;
	split( line, ",", arrayFreezeExpressions );
	
	for( unsigned f=0; f<arrayFreezeExpressions.size(); f++ )
	{
		vector<RString> arrayFreezeValues;
		split( arrayFreezeExpressions[f], "=", arrayFreezeValues );
		if( arrayFreezeValues.size() != 2 )
		{
			LOG->UserLog("Song file",
				     this->GetSongTitle(),
				     "has an invalid #STOPS value \"%s\" (must have exactly one '='), ignored.",
				     arrayFreezeExpressions[f].c_str() );
			continue;
		}

		const float fFreezeBeat = RowToBeat( arrayFreezeValues[0], rowsPerBeat );
		const float fFreezeSeconds = StringToFloat( arrayFreezeValues[1] );
		if( fFreezeSeconds == 0 ) {
			LOG->UserLog("Song file", this->GetSongTitle(),
				     "has a zero-length stop; ignored.");
			continue;
		}

		out.push_back( make_pair(fFreezeBeat, fFreezeSeconds) );
	}
}

// Utility function for sorting timing change data
namespace {
	bool compare_first(pair<float, float> a, pair<float, float> b) {
		return a.first < b.first;
	}
}

// Precondition: no BPM change or stop has 0 for its value (change.second).
//     (The ParseBPMs and ParseStops functions make sure of this.)
// Postcondition: all BPM changes, stops, and warps are added to the out
//     parameter, already sorted by beat.
void SMLoader::ProcessBPMsAndStops(TimingData &out,
		vector< pair<float, float> > &vBPMs,
		vector< pair<float, float> > &vStops)
{
	vector< pair<float, float> >::const_iterator ibpm, ibpmend;
	vector< pair<float, float> >::const_iterator istop, istopend;

	// Current BPM (positive or negative)
	float bpm = 0;
	// Beat at which the previous timing change occurred
	float prevbeat = 0;
	// Start/end of current warp (-1 if not currently warping)
	float warpstart = -1;
	float warpend = -1;
	// BPM prior to current warp, to detect if it has changed
	float prewarpbpm = 0;
	// How far off we have gotten due to negative changes
	float timeofs = 0;

	// Sort BPM changes and stops by beat.  Order matters.
	// TODO: Make sorted lists a precondition rather than sorting them here.
	// The caller may know that the lists are sorted already (e.g. if
	// loaded from cache).
	stable_sort(vBPMs.begin(), vBPMs.end(), compare_first);
	stable_sort(vStops.begin(), vStops.end(), compare_first);

	// Convert stops that come before beat 0.  All these really do is affect
	// where the arrows are with respect to the music, i.e. the song offset.
	// Positive stops subtract from the offset, and negative add to it.
	istop = vStops.begin();
	istopend = vStops.end();
	for (/* istop */; istop != istopend && istop->first < 0; istop++)
	{
		out.m_fBeat0OffsetInSeconds -= istop->second;
	}

	// Get rid of BPM changes that come before beat 0.  Positive BPMs before
	// the chart don't really do anything, so we just ignore them.  Negative
	// BPMs cause unpredictable behavior, so ignore them as well and issue a
	// warning.
	ibpm = vBPMs.begin();
	ibpmend = vBPMs.end();
	for (/* ibpm */; ibpm != ibpmend && ibpm->first <= 0; ibpm++)
	{
		bpm = ibpm->second;
		if (bpm < 0 && ibpm->first < 0)
		{
			LOG->UserLog("Song file", this->GetSongTitle(),
					"has a negative BPM prior to beat 0.  "
					"These cause problems; ignoring.");
		}
	}

	// It's beat 0.  Do you know where your BPMs are?
	if (bpm == 0)
	{
		// Nope.  Can we just use the next BPM value?
		if (ibpm == ibpmend)
		{
			// Nope.
			bpm = 60;
			LOG->UserLog("Song file", this->GetSongTitle(),
					"has no valid BPMs.  Defaulting to 60.");
		}
		else
		{
			// Yep.  Get the next BPM.
			ibpm++;
			bpm = ibpm->second;
			LOG->UserLog("Song file", this->GetSongTitle(),
					"does not establish a BPM before beat 0.  "
					"Using the value from the next BPM change.");
		}
	}
	// We always want to have an initial BPM.  If we start out warping, this
	// BPM will be added later.  If we start with a regular BPM, add it now.
	if (bpm > 0 && bpm <= FAST_BPM_WARP)
	{
		out.AddSegment(BPMSegment(BeatToNoteRow(0), bpm));
	}

	// Iterate over all BPMs and stops in tandem
	while (ibpm != ibpmend || istop != istopend)
	{
		// Get the next change in order, with BPMs taking precedence
		// when they fall on the same beat.
		bool changeIsBpm = istop == istopend || (ibpm != ibpmend && ibpm->first <= istop->first);
		const pair<float, float> & change = changeIsBpm ? *ibpm : *istop;

		// Calculate the effects of time at the current BPM.  "Infinite"
		// BPMs (SM4 warps) imply that zero time passes, so skip this
		// step in that case.
		if (bpm <= FAST_BPM_WARP)
		{
			timeofs += (change.first - prevbeat) * 60/bpm;

			// If we were in a warp and it finished during this
			// timeframe, create the warp segment.
			if (warpstart >= 0 && bpm > 0 && timeofs > 0)
			{
				// timeofs represents how far past the end we are
				warpend = change.first - (timeofs * bpm/60);
				out.AddSegment(WarpSegment(BeatToNoteRow(warpstart),
							warpend - warpstart));

				// If the BPM changed during the warp, put that
				// change at the beginning of the warp.
				if (bpm != prewarpbpm)
				{
					out.AddSegment(BPMSegment(BeatToNoteRow(warpstart), bpm));
				}
				// No longer warping
				warpstart = -1;
			}
		}

		// Save the current beat for the next round of calculations
		prevbeat = change.first;

		// Now handle the timing changes themselves
		if (changeIsBpm)
		{
			// Does this BPM change start a new warp?
			if (warpstart < 0 && (change.second < 0 || change.second > FAST_BPM_WARP))
			{
				// Yes.
				warpstart = change.first;
				prewarpbpm = bpm;
				timeofs = 0;
			}
			else if (warpstart < 0)
			{
				// No, and we aren't currently warping either.
				// Just a normal BPM change.
				out.AddSegment(BPMSegment(BeatToNoteRow(change.first), change.second));
			}
			bpm = change.second;
			ibpm++;
		}
		else
		{
			// Does this stop start a new warp?
			if (warpstart < 0 && change.second < 0)
			{
				// Yes.
				warpstart = change.first;
				prewarpbpm = bpm;
				timeofs = change.second;
			}
			else if (warpstart < 0)
			{
				// No, and we aren't currently warping either.
				// Just a normal stop.
				out.AddSegment(StopSegment(BeatToNoteRow(change.first), change.second));
			}
			else
			{
				// We're warping already.  Stops affect the time
				// offset directly.
				timeofs += change.second;

				// If a stop overcompensates for the time
				// deficit, the warp ends and we stop for the
				// amount it goes over.
				if (change.second > 0 && timeofs > 0)
				{
					warpend = change.first;
					out.AddSegment(WarpSegment(BeatToNoteRow(warpstart),
								warpend - warpstart));
					out.AddSegment(StopSegment(BeatToNoteRow(change.first), timeofs));

					// Now, are we still warping because of
					// the BPM value?
					if (bpm < 0 || bpm > FAST_BPM_WARP)
					{
						// Yep.
						warpstart = change.first;
						// prewarpbpm remains the same
						timeofs = 0;
					}
					else
					{
						// Nope, warp is done.  Add any
						// BPM change that happened in
						// the meantime.
						if (bpm != prewarpbpm)
						{
							out.AddSegment(BPMSegment(BeatToNoteRow(warpstart), bpm));
						}
						warpstart = -1;
					}
				}
			}
			istop++;
		}
	}

	// If we are still warping, we now have to consider the time remaining
	// after the last timing change.
	if (warpstart >= 0)
	{
		// Will this warp ever end?
		if (bpm < 0 || bpm > FAST_BPM_WARP)
		{
			// No, so it ends the entire chart immediately.
			// XXX There must be a less hacky and more accurate way
			// to do this.
			warpend = 99999999.0f;
		}
		else
		{
			// Yes.  Figure out when it will end.
			warpend = prevbeat - (timeofs * bpm/60);
		}
		out.AddSegment(WarpSegment(BeatToNoteRow(warpstart),
					warpend - warpstart));

		// As usual, record any BPM change that happened during the warp
		if (bpm != prewarpbpm)
		{
			out.AddSegment(BPMSegment(BeatToNoteRow(warpstart), bpm));
		}
	}
}

void SMLoader::ProcessDelays( TimingData &out, const RString line, const int rowsPerBeat )
{
	vector<RString> arrayDelayExpressions;
	split( line, ",", arrayDelayExpressions );

	for( unsigned f=0; f<arrayDelayExpressions.size(); f++ )
	{
		vector<RString> arrayDelayValues;
		split( arrayDelayExpressions[f], "=", arrayDelayValues );
		if( arrayDelayValues.size() != 2 )
		{
			LOG->UserLog("Song file",
				     this->GetSongTitle(),
				     "has an invalid #DELAYS value \"%s\" (must have exactly one '='), ignored.",
				     arrayDelayExpressions[f].c_str() );
			continue;
		}
		const float fFreezeBeat = RowToBeat( arrayDelayValues[0], rowsPerBeat );
		const float fFreezeSeconds = StringToFloat( arrayDelayValues[1] );
		// LOG->Trace( "Adding a delay segment: beat: %f, seconds = %f", new_seg.m_fStartBeat, new_seg.m_fStopSeconds );

		if(fFreezeSeconds > 0.0f)
			out.AddSegment( DelaySegment(BeatToNoteRow(fFreezeBeat), fFreezeSeconds) );
		else
			LOG->UserLog(
				     "Song file",
				     this->GetSongTitle(),
				     "has an invalid delay at beat %f, length %f.",
				     fFreezeBeat, fFreezeSeconds );
	}
}

void SMLoader::ProcessTimeSignatures( TimingData &out, const RString line, const int rowsPerBeat )
{
	vector<RString> vs1;
	split( line, ",", vs1 );

	for (RString const &s1 : vs1)
	{
		vector<RString> vs2;
		split( s1, "=", vs2 );

		if( vs2.size() < 3 )
		{
			LOG->UserLog("Song file",
				GetSongTitle(),
				"has an invalid time signature change with %i values.",
				static_cast<int>(vs2.size()) );
			continue;
		}

		const float fBeat = RowToBeat( vs2[0], rowsPerBeat );
		const int iNumerator = StringToInt( vs2[1] );
		const int iDenominator = StringToInt( vs2[2] );

		if( fBeat < 0 )
		{
			LOG->UserLog("Song file",
				     this->GetSongTitle(),
				     "has an invalid time signature change with beat %f.",
				     fBeat );
			continue;
		}

		if( iNumerator < 1 )
		{
			LOG->UserLog("Song file",
				     this->GetSongTitle(),
				     "has an invalid time signature change with beat %f, iNumerator %i.",
				     fBeat, iNumerator );
			continue;
		}

		if( iDenominator < 1 )
		{
			LOG->UserLog("Song file",
				     this->GetSongTitle(),
				     "has an invalid time signature change with beat %f, iDenominator %i.",
				     fBeat, iDenominator );
			continue;
		}

		out.AddSegment( TimeSignatureSegment(BeatToNoteRow(fBeat), iNumerator, iDenominator) );
	}
}

void SMLoader::ProcessTickcounts( TimingData &out, const RString line, const int rowsPerBeat )
{
	vector<RString> arrayTickcountExpressions;
	split( line, ",", arrayTickcountExpressions );

	for( unsigned f=0; f<arrayTickcountExpressions.size(); f++ )
	{
		vector<RString> arrayTickcountValues;
		split( arrayTickcountExpressions[f], "=", arrayTickcountValues );
		if( arrayTickcountValues.size() != 2 )
		{
			LOG->UserLog("Song file",
				     this->GetSongTitle(),
				     "has an invalid #TICKCOUNTS value \"%s\" (must have exactly one '='), ignored.",
				     arrayTickcountExpressions[f].c_str() );
			continue;
		}

		const float fTickcountBeat = RowToBeat( arrayTickcountValues[0], rowsPerBeat );
		int iTicks = clamp(atoi( arrayTickcountValues[1] ), 0, ROWS_PER_BEAT);

		out.AddSegment( TickcountSegment(BeatToNoteRow(fTickcountBeat), iTicks) );
	}
}

void SMLoader::ProcessSpeeds( TimingData &out, const RString line, const int rowsPerBeat )
{
	vector<RString> vs1;
	split( line, ",", vs1 );

	for (RString const &s1 : vs1)
	{
		vector<RString> vs2;
		split( s1, "=", vs2 );

		if( vs2[0] == 0 && vs2.size() == 2 ) // First one always seems to have 2.
		{
			vs2.push_back("0");
		}

		if( vs2.size() == 3 ) // use beats by default.
		{
			vs2.push_back("0");
		}

		if( vs2.size() < 4 )
		{
			LOG->UserLog("Song file",
				     this->GetSongTitle(),
				     "has an speed change with %i values.",
				     static_cast<int>(vs2.size()) );
			continue;
		}

		const float fBeat = RowToBeat( vs2[0], rowsPerBeat );
		const float fRatio = StringToFloat( vs2[1] );
		const float fDelay = StringToFloat( vs2[2] );

		// XXX: ugly...
		int iUnit = StringToInt(vs2[3]);
		SpeedSegment::BaseUnit unit = (iUnit == 0) ?
			SpeedSegment::UNIT_BEATS : SpeedSegment::UNIT_SECONDS;

		if( fBeat < 0 )
		{
			LOG->UserLog("Song file",
				     this->GetSongTitle(),
				     "has an speed change with beat %f.",
				     fBeat );
			continue;
		}

		if( fDelay < 0 )
		{
			LOG->UserLog("Song file",
				     this->GetSongTitle(),
				     "has an speed change with beat %f, length %f.",
				     fBeat, fDelay );
			continue;
		}

		out.AddSegment( SpeedSegment(BeatToNoteRow(fBeat), fRatio, fDelay, unit) );
	}
}

void SMLoader::ProcessFakes( TimingData &out, const RString line, const int rowsPerBeat )
{
	vector<RString> arrayFakeExpressions;
	split( line, ",", arrayFakeExpressions );

	for( unsigned b=0; b<arrayFakeExpressions.size(); b++ )
	{
		vector<RString> arrayFakeValues;
		split( arrayFakeExpressions[b], "=", arrayFakeValues );
		if( arrayFakeValues.size() != 2 )
		{
			LOG->UserLog("Song file",
				     this->GetSongTitle(),
				     "has an invalid #FAKES value \"%s\" (must have exactly one '='), ignored.",
				     arrayFakeExpressions[b].c_str() );
			continue;
		}

		const float fBeat = RowToBeat( arrayFakeValues[0], rowsPerBeat );
		const float fSkippedBeats = StringToFloat( arrayFakeValues[1] );

		if(fSkippedBeats > 0)
			out.AddSegment( FakeSegment(BeatToNoteRow(fBeat), fSkippedBeats) );
		else
		{
			LOG->UserLog("Song file",
				     this->GetSongTitle(),
				     "has an invalid Fake at beat %f, beats to skip %f.",
				     fBeat, fSkippedBeats );
		}
	}
}

bool SMLoader::LoadFromBGChangesVector( BackgroundChange &change, std::vector<RString> aBGChangeValues )
{
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
	{
		RString tmp = aBGChangeValues[7];
		tmp.MakeLower();
		if( ( tmp.find(".ini") != string::npos || tmp.find(".xml") != string::npos )
		   && !PREFSMAN->m_bQuirksMode )
		{
			return false;
		}
		change.m_def.m_sFile2 = aBGChangeValues[7];
		// fall through
	}
	case 7:
		change.m_def.m_sEffect = aBGChangeValues[6];
		// fall through
	case 6:
		// param 7 overrides this.
		// Backward compatibility:
		if( change.m_def.m_sEffect.empty() )
		{
			bool bLoop = StringToInt( aBGChangeValues[5] ) != 0;
			if( !bLoop )
				change.m_def.m_sEffect = SBE_StretchNoLoop;
		}
		// fall through
	case 5:
		// param 7 overrides this.
		// Backward compatibility:
		if( change.m_def.m_sEffect.empty() )
		{
			bool bRewindMovie = StringToInt( aBGChangeValues[4] ) != 0;
			if( bRewindMovie )
				change.m_def.m_sEffect = SBE_StretchRewind;
		}
		// fall through
	case 4:
		// param 9 overrides this.
		// Backward compatibility:
		if( change.m_sTransition.empty() )
			change.m_sTransition = (StringToInt( aBGChangeValues[3] ) != 0) ? "CrossFade" : "";
		// fall through
	case 3:
		change.m_fRate = StringToFloat( aBGChangeValues[2] );
		// fall through
	case 2:
	{
		RString tmp = aBGChangeValues[1];
		tmp.MakeLower();
		if( ( tmp.find(".ini") != string::npos || tmp.find(".xml") != string::npos )
		   && !PREFSMAN->m_bQuirksMode )
		{
			return false;
		}
		change.m_def.m_sFile1 = aBGChangeValues[1];
		// fall through
	}
	case 1:
		change.m_fStartBeat = StringToFloat( aBGChangeValues[0] );
		// fall through
	}

	return aBGChangeValues.size() >= 2;
}

bool SMLoader::LoadNoteDataFromSimfile( const RString &path, Steps &out )
{
	MsdFile msd;
	if( !msd.ReadFile( path, true ) )  // unescape
	{
		LOG->UserLog("Song file",
			     path,
			     "couldn't be opened: %s",
			     msd.GetError().c_str() );
		return false;
	}
	for (unsigned i = 0; i<msd.GetNumValues(); i++)
	{
		int iNumParams = msd.GetNumParams(i);
		const MsdFile::value_t &sParams = msd.GetValue(i);
		RString sValueName = sParams[0];
		sValueName.MakeUpper();
		
		// The only tag we care about is the #NOTES tag.
		if( sValueName=="NOTES" || sValueName=="NOTES2" )
		{
			if( iNumParams < 7 )
			{
				LOG->UserLog("Song file",
					     path,
					     "has %d fields in a #NOTES tag, but should have at least 7.",
					     iNumParams );
				continue;
			}
			
			RString stepsType = sParams[1];
			RString description = sParams[2];
			RString difficulty = sParams[3];

			// HACK?: If this is a .edit fudge the edit difficulty
			if(path.Right(5).CompareNoCase(".edit") == 0) difficulty = "edit";

			Trim(stepsType);
			Trim(description);
			Trim(difficulty);
			// Remember our old versions.
			if (difficulty.CompareNoCase("smaniac") == 0)
			{
				difficulty = "Challenge";
			}
			
			/* Handle hacks that originated back when StepMania didn't have
			 * Difficulty_Challenge. TODO: Remove the need for said hacks. */
			if( difficulty.CompareNoCase("hard") == 0 )
			{
				/* HACK: Both SMANIAC and CHALLENGE used to be Difficulty_Hard.
				 * They were differentiated via aspecial description.
				 * Account for the rogue charts that do this. */
				// HACK: SMANIAC used to be Difficulty_Hard with a special description.
				if (description.CompareNoCase("smaniac") == 0 ||
					description.CompareNoCase("challenge") == 0) 
					difficulty = "Challenge";
			}
			
			if(!(out.m_StepsType == GAMEMAN->StringToStepsType( stepsType ) &&
			     out.GetDescription() == description &&
			     (out.GetDifficulty() == StringToDifficulty(difficulty) ||
				  out.GetDifficulty() == OldStyleStringToDifficulty(difficulty))))
			{
				continue;
			}
			
			RString noteData = sParams[6];
			Trim( noteData );
			out.SetSMNoteData( noteData );
			out.TidyUpData();
			return true;
		}
	}
	return false;
}

bool SMLoader::LoadFromSimfile( const RString &sPath, Song &out, bool bFromCache )
{
	//LOG->Trace( "Song::LoadFromSMFile(%s)", sPath.c_str() );

	MsdFile msd;
	if( !msd.ReadFile( sPath, true ) )  // unescape
	{
		LOG->UserLog( "Song file", sPath, "couldn't be opened: %s", msd.GetError().c_str() );
		return false;
	}

	out.m_SongTiming.m_sFile = sPath;
	out.m_sSongFileName = sPath;

	SMSongTagInfo reused_song_info(&*this, &out, sPath);

	for( unsigned i=0; i<msd.GetNumValues(); i++ )
	{
		int iNumParams = msd.GetNumParams(i);
		const MsdFile::value_t &sParams = msd.GetValue(i);
		RString sValueName = sParams[0];
		sValueName.MakeUpper();

		reused_song_info.params= &sParams;
		song_handler_map_t::iterator handler=
			sm_parser_helper.song_tag_handlers.find(sValueName);
		if(handler != sm_parser_helper.song_tag_handlers.end())
		{
		/* Don't use GetMainAndSubTitlesFromFullTitle; that's only for heuristically
		 * splitting other formats that *don't* natively support #SUBTITLE. */
			handler->second(reused_song_info);
		}
		else if(sValueName.Left(strlen("BGCHANGES")) == "BGCHANGES")
		{
			SMSetBGChanges(reused_song_info);
		}
		else if(sValueName == "NOTES" || sValueName == "NOTES2")
		{
			if(iNumParams < 7)
			{
				LOG->UserLog( "Song file", sPath, "has %d fields in a #NOTES tag, but should have at least 7.", iNumParams );
				continue;
			}

			Steps* pNewNotes = out.CreateSteps();
			LoadFromTokens( 
				sParams[1], 
				sParams[2], 
				sParams[3], 
				sParams[4], 
				sParams[5], 
				sParams[6],
				*pNewNotes);

			pNewNotes->SetFilename(sPath);
			out.AddSteps( pNewNotes );
		}
		else
		{
			LOG->UserLog("Song file", sPath, "has an unexpected value named \"%s\".", sValueName.c_str());
		}
	}

	// Turn negative time changes into warps
	ProcessBPMsAndStops(out.m_SongTiming, reused_song_info.BPMChanges, reused_song_info.Stops);

	TidyUpData( out, bFromCache );
	return true;
}

bool SMLoader::LoadEditFromFile( RString sEditFilePath, ProfileSlot slot, bool bAddStepsToSong, Song *givenSong /* =nullptr */ )
{
	LOG->Trace( "SMLoader::LoadEditFromFile(%s)", sEditFilePath.c_str() );

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

	return LoadEditFromMsd( msd, sEditFilePath, slot, bAddStepsToSong, givenSong );
}

bool SMLoader::LoadEditFromBuffer( const RString &sBuffer, const RString &sEditFilePath, ProfileSlot slot, Song *givenSong )
{
	MsdFile msd;
	msd.ReadFromString( sBuffer, true ); // unescape
	return LoadEditFromMsd( msd, sEditFilePath, slot, true, givenSong );
}

bool SMLoader::LoadEditFromMsd( const MsdFile &msd, const RString &sEditFilePath, ProfileSlot slot, bool bAddStepsToSong, Song *givenSong /* = nullptr */ )
{
	Song* pSong = givenSong;

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
				/* LOG->UserLog( "Edit file", sEditFilePath, "has more than one #SONG tag." );
				return false; */
				// May have been given the song from outside the file. Not worth checking for.
				continue;
			}

			RString sSongFullTitle = sParams[1];
			this->SetSongTitle(sParams[1]);
			sSongFullTitle.Replace( '\\', '/' );

			pSong = SONGMAN->FindSong( sSongFullTitle );
			if( pSong == nullptr )
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
			if( pSong == nullptr )
			{
				LOG->UserLog( "Edit file", sEditFilePath, "doesn't have a #SONG tag preceeding the first #NOTES tag, and is not in a valid song-specific folder." );
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
			LoadFromTokens( 
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

	// Edit had no valid #NOTES sections
	return false;
}

void SMLoader::GetApplicableFiles( const RString &sPath, vector<RString> &out, bool load_autosave )
{
	if(load_autosave)
	{
		GetDirListing( sPath + RString("*.ats" ), out );
	}
	else
	{
		GetDirListing( sPath + RString("*" + this->GetFileExtension() ), out );
	}
}

void SMLoader::TidyUpData( Song &song, bool bFromCache )
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

			float lastBeat = song.GetLastBeat();
			/* If BGChanges already exist after the last beat, don't add the
			 * background in the middle. */
			if( !bg.empty() && bg.back().m_fStartBeat-0.0001f >= lastBeat )
				break;

			// If the last BGA is already the song BGA, don't add a duplicate.
			if( !bg.empty() && !bg.back().m_def.m_sFile1.CompareNoCase(song.m_sBackgroundFile) )
				break;

			if( !IsAFile( song.GetBackgroundPath() ) )
				break;

			bg.push_back( BackgroundChange(lastBeat,song.m_sBackgroundFile) );
		} while(0);
	}
	if (bFromCache)
	{
		song.TidyUpData( bFromCache, true );
	}
}

std::vector<RString> SMLoader::GetSongDirFiles(const RString &sSongDir)
{
	if (!m_SongDirFiles.empty())
		return m_SongDirFiles;

	ASSERT(!sSongDir.empty());

	std::vector<RString> vsDirs;
	vsDirs.push_back(sSongDir);

	while (!vsDirs.empty())
	{
		RString d = vsDirs.back();
		vsDirs.pop_back();

		std::vector<RString> vsFiles;
		GetDirListing(d+"*", vsFiles, false, true);

		for (const RString& f : vsFiles)
		{
			if (IsADirectory(f))
				vsDirs.push_back(f+"/");

			m_SongDirFiles.push_back(f.substr(sSongDir.size()));
		}
	}

	return m_SongDirFiles;
}

void SMLoader::ParseBGChangesString(const RString& _sChanges, std::vector<std::vector<RString> > &vvsAddTo, const RString& sSongDir)
{
	// short circuit: empty string
	if (_sChanges.empty())
		return;

	// strip newlines (basically operates as both split and join at the same time)
	RString sChanges;
	size_t start = 0;
	do {
		size_t pos = _sChanges.find_first_of("\r\n", start);
		if (RString::npos == pos)
			pos = _sChanges.size();

		if (pos - start > 0) {
			if ((start == 0) && (pos == _sChanges.size()))
				sChanges = _sChanges;
			else
				sChanges += _sChanges.substr(start, pos - start);
		}
		start = pos + 1;
	} while (start <= _sChanges.size());

	// after removing newlines, do we have anything?
	if (sChanges.empty())
		return;

	// get the list of possible files/directories for the file parameters
	std::vector<RString> vsFiles = GetSongDirFiles(sSongDir);

	start = 0;
	int pnum = 0;
	do {
		switch (pnum) {
		// parameters 1 and 7 can be files or folder names
		case 1:
		case 7:
		{
			// see if one of the files in the song directory are listed.
			RString found;
			for (const auto& f : vsFiles)
			{
				// there aren't enough characters for this to match
				if ((sChanges.size() - start) < f.size())
					continue;

				// the string itself matches
				if (f.EqualsNoCase(sChanges.substr(start, f.size()).c_str())) 
				{
					size_t nextpos = start + f.size();

					// is this name followed by end-of-string, equals, or comma?
					if ((nextpos == sChanges.size()) || (sChanges[nextpos] == '=') || (sChanges[nextpos] == ','))
					{
						found = f;
						break;
					}
				}
			}
			// yes. use that as this parameter, even if it has commas or equals signs in it
			if (!found.empty())
			{
				vvsAddTo.back().push_back(found);
				start += found.size();
				// the next character should be a comma or equals. skip it
				if (start < sChanges.size())
				{
					if (sChanges[start] == '=')
						++pnum;
					else
					{
						ASSERT(sChanges[start] == ',');
						pnum = 0;
					}
					start += 1;
				}
				// move to the next parameter
				break;
			}
			// deliberate fall-through if not found. treat it as a normal string like before
		}
		// everything else should be safe
		default:
			if(0 == pnum) vvsAddTo.push_back(std::vector<RString>()); // first value of this set. create our vector

			{
				size_t eqpos = sChanges.find('=', start);
				size_t compos = sChanges.find(',', start);
				
				if ((eqpos == RString::npos) && (compos == RString::npos))
				{
					// neither = nor , were found in the remainder of the string. consume the rest of the string.
					vvsAddTo.back().push_back(sChanges.substr(start));
					start = sChanges.size() + 1;
				}
				else if ((eqpos != RString::npos) && (compos != RString::npos))
				{
					// both were found. which came first?
					if (eqpos < compos)
					{
						// equals. consume value and move to next value
						vvsAddTo.back().push_back(sChanges.substr(start, eqpos - start));
						start = eqpos + 1;
						++pnum;
					}
					else
					{
						// comma. consume value and move to next set
						vvsAddTo.back().push_back(sChanges.substr(start, compos - start));
						start = compos + 1;
						pnum = 0;
					}
				}
				else if (eqpos != RString::npos)
				{
					// found only equals. consume and move on.
					vvsAddTo.back().push_back(sChanges.substr(start, eqpos - start));
					start = eqpos + 1;
					++pnum;
				}
				else
				{
					// only foudn comma. consume and move on.
					vvsAddTo.back().push_back(sChanges.substr(start, compos - start));
					start = compos + 1;
					pnum = 0;
				}
				break;
			}
		}
	} while (start <= sChanges.size());
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
