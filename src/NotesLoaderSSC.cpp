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

#include <cstddef>
#include <vector>


// Everything from this line to the creation of parser_helper exists to
// speed up parsing by allowing the use of std::map.  All these functions
// are put into a map of function pointers which is used when loading.
// -Kyz
/****************************************************************/
struct StepsTagInfo
{
	SSCLoader* loader;
	Song* song;
	Steps* steps;
	TimingData* timing;
	const MsdFile::value_t* params;
	const RString& path;
	bool has_own_timing;
	bool ssc_format;
	bool from_cache;
	bool for_load_edit;
	StepsTagInfo(SSCLoader* l, Song* s, const RString& p, bool fc)
		:loader(l), song(s), path(p), has_own_timing(false), ssc_format(false),
		 from_cache(fc), for_load_edit(false)
	{}
};
struct SongTagInfo
{
	SSCLoader* loader;
	Song* song;
	const MsdFile::value_t* params;
	const RString& path;
	bool from_cache;
	SongTagInfo(SSCLoader* l, Song* s, const RString& p, bool fc)
		:loader(l), song(s), path(p), from_cache(fc)
	{}
};
// LoadNoteDataFromSimfile uses LoadNoteDataTagIDs because its parts operate
// on state variables internal to the function.
enum LoadNoteDataTagIDs
{
	LNDID_version,
	LNDID_stepstype,
	LNDID_chartname,
	LNDID_description,
	LNDID_difficulty,
	LNDID_meter,
	LNDID_credit,
	LNDID_notes,
	LNDID_notes2,
	LNDID_notedata
};

typedef void (*steps_tag_func_t)(StepsTagInfo& info);
typedef void (*song_tag_func_t)(SongTagInfo& info);

// Functions for song tags go below this line. -Kyz
/****************************************************************/
void SetVersion(SongTagInfo& info)
{
	info.song->m_fVersion = StringToFloat((*info.params)[1]);
}
void SetTitle(SongTagInfo& info)
{
	info.song->m_sMainTitle = (*info.params)[1];
	info.loader->SetSongTitle((*info.params)[1]);
}
void SetSubtitle(SongTagInfo& info)
{
	info.song->m_sSubTitle = (*info.params)[1];
}
void SetArtist(SongTagInfo& info)
{
	info.song->m_sArtist = (*info.params)[1];
}
void SetMainTitleTranslit(SongTagInfo& info)
{
	info.song->m_sMainTitleTranslit = (*info.params)[1];
}
void SetSubtitleTranslit(SongTagInfo& info)
{
	info.song->m_sSubTitleTranslit = (*info.params)[1];
}
void SetArtistTranslit(SongTagInfo& info)
{
	info.song->m_sArtistTranslit = (*info.params)[1];
}
void SetGenre(SongTagInfo& info)
{
	info.song->m_sGenre = (*info.params)[1];
}
void SetOrigin(SongTagInfo& info)
{
	info.song->m_sOrigin = (*info.params)[1];
}
void SetCredit(SongTagInfo& info)
{
	info.song->m_sCredit = (*info.params)[1];
	Trim(info.song->m_sCredit);
}
void SetBanner(SongTagInfo& info)
{
	info.song->m_sBannerFile = (*info.params)[1];
}
void SetBackground(SongTagInfo& info)
{
	info.song->m_sBackgroundFile = (*info.params)[1];
}
void SetPreviewVid(SongTagInfo& info)
{
	info.song->m_sPreviewVidFile = (*info.params)[1];
}
void SetJacket(SongTagInfo& info)
{
	info.song->m_sJacketFile = (*info.params)[1];
}
void SetCDImage(SongTagInfo& info)
{
	info.song->m_sCDFile = (*info.params)[1];
}
void SetDiscImage(SongTagInfo& info)
{
	info.song->m_sDiscFile = (*info.params)[1];
}
void SetLyricsPath(SongTagInfo& info)
{
	info.song->m_sLyricsFile = (*info.params)[1];
}
void SetCDTitle(SongTagInfo& info)
{
	info.song->m_sCDTitleFile = (*info.params)[1];
}
void SetMusic(SongTagInfo& info)
{
	info.song->m_sMusicFile = (*info.params)[1];
}
void SetPreview(SongTagInfo& info)
{
	info.song->m_PreviewFile= (*info.params)[1];
}
void SetInstrumentTrack(SongTagInfo& info)
{
	info.loader->ProcessInstrumentTracks(*info.song, (*info.params)[1]);
}
void SetMusicLength(SongTagInfo& info)
{
	if(info.from_cache)
	info.song->m_fMusicLengthSeconds = StringToFloat((*info.params)[1]);
}
void SetLastSecondHint(SongTagInfo& info)
{
	info.song->SetSpecifiedLastSecond(StringToFloat((*info.params)[1]));
}
void SetSampleStart(SongTagInfo& info)
{
	info.song->m_fMusicSampleStartSeconds = HHMMSSToSeconds((*info.params)[1]);
}
void SetSampleLength(SongTagInfo& info)
{
	info.song->m_fMusicSampleLengthSeconds = HHMMSSToSeconds((*info.params)[1]);
}
void SetDisplayBPM(SongTagInfo& info)
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
void SetSelectable(SongTagInfo& info)
{
	if((*info.params)[1].EqualsNoCase("YES"))
	{ info.song->m_SelectionDisplay = info.song->SHOW_ALWAYS; }
	else if((*info.params)[1].EqualsNoCase("NO"))
	{ info.song->m_SelectionDisplay = info.song->SHOW_NEVER; }
	// ROULETTE from 3.9 is no longer in use.
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
void SetBGChanges(SongTagInfo& info)
{
	info.loader->ProcessBGChanges(*info.song, (*info.params)[0], info.path, (*info.params)[1]);
}
void SetFGChanges(SongTagInfo& info)
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
void SetKeysounds(SongTagInfo& info)
{
	RString keysounds = (*info.params)[1];
	if(keysounds.length() >= 2 && keysounds.substr(0, 2) == "\\#")
	{ keysounds = keysounds.substr(1); }
	split(keysounds, ",", info.song->m_vsKeysoundFile);
}
void SetAttacks(SongTagInfo& info)
{
	info.loader->ProcessAttackString(info.song->m_sAttackString, (*info.params));
	info.loader->ProcessAttacks(info.song->m_Attacks, (*info.params));
}
void SetOffset(SongTagInfo& info)
{
	info.song->m_SongTiming.m_fBeat0OffsetInSeconds = StringToFloat((*info.params)[1]);
}
void SetSongStops(SongTagInfo& info)
{
	info.loader->ProcessStops(info.song->m_SongTiming, (*info.params)[1]);
}
void SetSongDelays(SongTagInfo& info)
{
	info.loader->ProcessDelays(info.song->m_SongTiming, (*info.params)[1]);
}
void SetSongBPMs(SongTagInfo& info)
{
	info.loader->ProcessBPMs(info.song->m_SongTiming, (*info.params)[1]);
}
void SetSongWarps(SongTagInfo& info)
{
	info.loader->ProcessWarps( info.song->m_SongTiming, (*info.params)[1], info.song->m_fVersion );
}
void SetSongLabels(SongTagInfo& info)
{
	info.loader->ProcessLabels( info.song->m_SongTiming, (*info.params)[1] );
}
void SetSongTimeSignatures(SongTagInfo& info)
{
	info.loader->ProcessTimeSignatures(info.song->m_SongTiming, (*info.params)[1]);
}
void SetSongTickCounts(SongTagInfo& info)
{
	info.loader->ProcessTickcounts(info.song->m_SongTiming, (*info.params)[1]);
}
void SetSongCombos(SongTagInfo& info)
{
	info.loader->ProcessCombos( info.song->m_SongTiming, (*info.params)[1] );
}
void SetSongSpeeds(SongTagInfo& info)
{
	info.loader->ProcessSpeeds(info.song->m_SongTiming, (*info.params)[1]);
}
void SetSongScrolls(SongTagInfo& info)
{
	info.loader->ProcessScrolls(info.song->m_SongTiming, (*info.params)[1]);
}
void SetSongFakes(SongTagInfo& info)
{
	info.loader->ProcessFakes(info.song->m_SongTiming, (*info.params)[1]);
}
void SetFirstSecond(SongTagInfo& info)
{
	if(info.from_cache)
	{ info.song->SetFirstSecond(StringToFloat((*info.params)[1])); }
}
void SetLastSecond(SongTagInfo& info)
{
	if(info.from_cache)
	{ info.song->SetLastSecond(StringToFloat((*info.params)[1])); }
}
void SetSongFilename(SongTagInfo& info)
{
	if(info.from_cache)
	{ info.song->m_sSongFileName = (*info.params)[1]; }
}
void SetHasMusic(SongTagInfo& info)
{
	if(info.from_cache)
	{ info.song->m_bHasMusic = StringToInt((*info.params)[1]) != 0; }
}
void SetHasBanner(SongTagInfo& info)
{
	if(info.from_cache)
	{ info.song->m_bHasBanner = StringToInt((*info.params)[1]) != 0; }
}

// Functions for steps tags go below this line. -Kyz
/****************************************************************/
void SetStepsVersion(StepsTagInfo& info)
{
	info.song->m_fVersion = StringToFloat((*info.params)[1]);
}
void SetChartName(StepsTagInfo& info)
{
	RString name= (*info.params)[1];
	Trim(name);
	info.steps->SetChartName(name);
}
void SetStepsType(StepsTagInfo& info)
{
	info.steps->m_StepsType = GAMEMAN->StringToStepsType((*info.params)[1]);
	info.steps->m_StepsTypeStr= (*info.params)[1];
	info.ssc_format= true;
}
void SetChartStyle(StepsTagInfo& info)
{
	info.steps->SetChartStyle((*info.params)[1]);
	info.ssc_format= true;
}
void SetDescription(StepsTagInfo& info)
{
	RString name= (*info.params)[1];
	Trim(name);
	if(info.song->m_fVersion < VERSION_CHART_NAME_TAG && !info.for_load_edit)
	{
		info.steps->SetChartName(name);
	}
	else
	{
		info.steps->SetDescription(name);
	}
	info.ssc_format= true;
}
void SetDifficulty(StepsTagInfo& info)
{
	info.steps->SetDifficulty(StringToDifficulty((*info.params)[1]));
	info.ssc_format= true;
}
void SetMeter(StepsTagInfo& info)
{
	info.steps->SetMeter(StringToInt((*info.params)[1]));
	info.ssc_format= true;
}
void SetRadarValues(StepsTagInfo& info)
{
	if(info.from_cache || info.for_load_edit)
	{
		std::vector<RString> values;
		split((*info.params)[1], ",", values, true);
		// Instead of trying to use the version to figure out how many
		// categories to expect, look at the number of values and split them
		// evenly. -Kyz
		std::size_t cats_per_player= values.size() / NUM_PlayerNumber;
		RadarValues v[NUM_PLAYERS];
		FOREACH_PlayerNumber(pn)
		{
			for(std::size_t i= 0; i < cats_per_player; ++i)
			{
				v[pn][i]= StringToFloat(values[pn * cats_per_player + i]);
			}
		}
		info.steps->SetCachedRadarValues(v);
	}
	else
	{
		// just recalc at time.
	}
	info.ssc_format= true;
}
void SetCredit(StepsTagInfo& info)
{
	info.steps->SetCredit((*info.params)[1]);
	info.ssc_format= true;
}
void SetStepsMusic(StepsTagInfo& info)
{
	info.steps->SetMusicFile((*info.params)[1]);
}
void SetStepsBPMs(StepsTagInfo& info)
{
	if(info.song->m_fVersion >= VERSION_SPLIT_TIMING || info.for_load_edit)
	{
		info.loader->ProcessBPMs(*info.timing, (*info.params)[1]);
		info.has_own_timing = true;
	}
	info.ssc_format= true;
}
void SetStepsStops(StepsTagInfo& info)
{
	if(info.song->m_fVersion >= VERSION_SPLIT_TIMING || info.for_load_edit)
	{
		info.loader->ProcessStops(*info.timing, (*info.params)[1]);
		info.has_own_timing = true;
	}
	info.ssc_format= true;
}
void SetStepsDelays(StepsTagInfo& info)
{
	if(info.song->m_fVersion >= VERSION_SPLIT_TIMING || info.for_load_edit)
	{
		info.loader->ProcessDelays(*info.timing, (*info.params)[1]);
		info.has_own_timing = true;
	}
	info.ssc_format= true;
}
void SetStepsTimeSignatures(StepsTagInfo& info)
{
	if(info.song->m_fVersion >= VERSION_SPLIT_TIMING || info.for_load_edit)
	{
		info.loader->ProcessTimeSignatures(*info.timing, (*info.params)[1]);
		info.has_own_timing = true;
	}
	info.ssc_format= true;
}
void SetStepsTickCounts(StepsTagInfo& info)
{
	if(info.song->m_fVersion >= VERSION_SPLIT_TIMING || info.for_load_edit)
	{
		info.loader->ProcessTickcounts(*info.timing, (*info.params)[1]);
		info.has_own_timing = true;
	}
	info.ssc_format= true;
}
void SetStepsCombos(StepsTagInfo& info)
{
	if(info.song->m_fVersion >= VERSION_SPLIT_TIMING || info.for_load_edit)
	{
		info.loader->ProcessCombos(*info.timing, (*info.params)[1]);
		info.has_own_timing = true;
	}
	info.ssc_format= true;
}
void SetStepsWarps(StepsTagInfo& info)
{
	if(info.song->m_fVersion >= VERSION_SPLIT_TIMING || info.for_load_edit)
	{
		info.loader->ProcessWarps(*info.timing, (*info.params)[1], info.song->m_fVersion);
		info.has_own_timing = true;
	}
	info.ssc_format= true;
}
void SetStepsSpeeds(StepsTagInfo& info)
{
	if(info.song->m_fVersion >= VERSION_SPLIT_TIMING || info.for_load_edit)
	{
		info.loader->ProcessSpeeds(*info.timing, (*info.params)[1]);
		info.has_own_timing = true;
	}
	info.ssc_format= true;
}
void SetStepsScrolls(StepsTagInfo& info)
{
	if(info.song->m_fVersion >= VERSION_SPLIT_TIMING || info.for_load_edit)
	{
		info.loader->ProcessScrolls(*info.timing, (*info.params)[1]);
		info.has_own_timing = true;
	}
	info.ssc_format= true;
}
void SetStepsFakes(StepsTagInfo& info)
{
	if(info.song->m_fVersion >= VERSION_SPLIT_TIMING || info.for_load_edit)
	{
		info.loader->ProcessFakes(*info.timing, (*info.params)[1]);
		info.has_own_timing = true;
	}
	info.ssc_format= true;
}
void SetStepsLabels(StepsTagInfo& info)
{
	if(info.song->m_fVersion >= VERSION_SPLIT_TIMING || info.for_load_edit)
	{
		info.loader->ProcessLabels(*info.timing, (*info.params)[1]);
		info.has_own_timing = true;
	}
	info.ssc_format= true;
}
void SetStepsAttacks(StepsTagInfo& info)
{
	if(info.song->m_fVersion >= VERSION_SPLIT_TIMING || info.for_load_edit)
	{
		info.loader->ProcessAttackString(info.steps->m_sAttackString, *info.params);
		info.loader->ProcessAttacks(info.steps->m_Attacks, *info.params);
	}
}
void SetStepsOffset(StepsTagInfo& info)
{
	if(info.song->m_fVersion >= VERSION_SPLIT_TIMING || info.for_load_edit)
	{
		info.timing->m_fBeat0OffsetInSeconds = StringToFloat((*info.params)[1]);
		info.has_own_timing = true;
	}
}
void SetStepsDisplayBPM(StepsTagInfo& info)
{
	// #DISPLAYBPM:[xxx][xxx:xxx]|[*];
	if((*info.params)[1] == "*")
	{ info.steps->SetDisplayBPM(DISPLAY_BPM_RANDOM); }
	else if((*info.params)[1] != "")
	{
		info.steps->SetDisplayBPM(DISPLAY_BPM_SPECIFIED);
		float min = StringToFloat((*info.params)[1]);
		info.steps->SetMinBPM(min);
		if((*info.params)[2].empty())
		{ info.steps->SetMaxBPM(min); }
		else
		{ info.steps->SetMaxBPM(StringToFloat((*info.params)[2])); }
	}
}


typedef std::map<RString, steps_tag_func_t> steps_handler_map_t;
typedef std::map<RString, song_tag_func_t> song_handler_map_t;
typedef std::map<RString, LoadNoteDataTagIDs> load_note_data_handler_map_t;

struct ssc_parser_helper_t
{
	steps_handler_map_t steps_tag_handlers;
	song_handler_map_t song_tag_handlers;
	load_note_data_handler_map_t load_note_data_handlers;
	// Unless signed, the comments in this tag list are not by me.  They were
	// moved here when converting from the else if chain. -Kyz
	ssc_parser_helper_t()
	{
		song_tag_handlers["VERSION"]= &SetVersion;
		song_tag_handlers["TITLE"]= &SetTitle;
		song_tag_handlers["SUBTITLE"]= &SetSubtitle;
		song_tag_handlers["ARTIST"]= &SetArtist;
		song_tag_handlers["TITLETRANSLIT"]= &SetMainTitleTranslit;
		song_tag_handlers["SUBTITLETRANSLIT"]= &SetSubtitleTranslit;
		song_tag_handlers["ARTISTTRANSLIT"]= &SetArtistTranslit;
		song_tag_handlers["GENRE"]= &SetGenre;
		song_tag_handlers["ORIGIN"]= &SetOrigin;
		song_tag_handlers["CREDIT"]= &SetCredit;
		song_tag_handlers["BANNER"]= &SetBanner;
		song_tag_handlers["BACKGROUND"]= &SetBackground;
		song_tag_handlers["PREVIEWVID"]= &SetPreviewVid;
		song_tag_handlers["JACKET"]= &SetJacket;
		song_tag_handlers["CDIMAGE"]= &SetCDImage;
		song_tag_handlers["DISCIMAGE"]= &SetDiscImage;
		song_tag_handlers["LYRICSPATH"]= &SetLyricsPath;
		song_tag_handlers["CDTITLE"]= &SetCDTitle;
		song_tag_handlers["MUSIC"]= &SetMusic;
		song_tag_handlers["PREVIEW"]= &SetPreview;
		song_tag_handlers["INSTRUMENTTRACK"]= &SetInstrumentTrack;
		song_tag_handlers["MUSICLENGTH"]= &SetMusicLength;
		song_tag_handlers["LASTSECONDHINT"]= &SetLastSecondHint;
		song_tag_handlers["SAMPLESTART"]= &SetSampleStart;
		song_tag_handlers["SAMPLELENGTH"]= &SetSampleLength;
		song_tag_handlers["DISPLAYBPM"]= &SetDisplayBPM;
		song_tag_handlers["SELECTABLE"]= &SetSelectable;
		// It's a bit odd to have the tag that exists for backwards compatibility
		// in this list and not the replacement, but the BGCHANGES tag has a
		// number on the end, allowing up to NUM_BackgroundLayer tags, so it
		// can't fit in the map. -Kyz
		song_tag_handlers["ANIMATIONS"]= &SetBGChanges;
		song_tag_handlers["FGCHANGES"]= &SetFGChanges;
		song_tag_handlers["KEYSOUNDS"]= &SetKeysounds;
		song_tag_handlers["ATTACKS"]= &SetAttacks;
		song_tag_handlers["OFFSET"]= &SetOffset;
		/* Below are the song based timings that should only be used
		 * if the steps do not have their own timing. */
		song_tag_handlers["STOPS"]= &SetSongStops;
		song_tag_handlers["DELAYS"]= &SetSongDelays;
		song_tag_handlers["BPMS"]= &SetSongBPMs;
		song_tag_handlers["WARPS"]= &SetSongWarps;
		song_tag_handlers["LABELS"]= &SetSongLabels;
		song_tag_handlers["TIMESIGNATURES"]= &SetSongTimeSignatures;
		song_tag_handlers["TICKCOUNTS"]= &SetSongTickCounts;
		song_tag_handlers["COMBOS"]= &SetSongCombos;
		song_tag_handlers["SPEEDS"]= &SetSongSpeeds;
		song_tag_handlers["SCROLLS"]= &SetSongScrolls;
		song_tag_handlers["FAKES"]= &SetSongFakes;
		/* The following are cache tags. Never fill their values
		 * directly: only from the cached version. */
		song_tag_handlers["FIRSTSECOND"]= &SetFirstSecond;
		song_tag_handlers["LASTSECOND"]= &SetLastSecond;
		song_tag_handlers["SONGFILENAME"]= &SetSongFilename;
		song_tag_handlers["HASMUSIC"]= &SetHasMusic;
		song_tag_handlers["HASBANNER"]= &SetHasBanner;
		/* Tags that no longer exist, listed for posterity.  May their names
		 * never be forgotten for their service to Stepmania. -Kyz
		 * LASTBEATHINT: // unable to parse due to tag position. Ignore.
		 * MUSICBYTES: // ignore
		 * FIRSTBEAT: // no longer used.
		 * LASTBEAT: // no longer used.
		 */

		steps_tag_handlers["VERSION"]= &SetStepsVersion;
		steps_tag_handlers["CHARTNAME"]= &SetChartName;
		steps_tag_handlers["STEPSTYPE"]= &SetStepsType;
		steps_tag_handlers["CHARTSTYLE"]= &SetChartStyle;
		steps_tag_handlers["DESCRIPTION"]= &SetDescription;
		steps_tag_handlers["DIFFICULTY"]= &SetDifficulty;
		steps_tag_handlers["METER"]= &SetMeter;
		steps_tag_handlers["RADARVALUES"]= &SetRadarValues;
		steps_tag_handlers["CREDIT"]= &SetCredit;
		steps_tag_handlers["MUSIC"]= &SetStepsMusic;
		steps_tag_handlers["BPMS"]= &SetStepsBPMs;
		steps_tag_handlers["STOPS"]= &SetStepsStops;
		steps_tag_handlers["DELAYS"]= &SetStepsDelays;
		steps_tag_handlers["TIMESIGNATURES"]= &SetStepsTimeSignatures;
		steps_tag_handlers["TICKCOUNTS"]= &SetStepsTickCounts;
		steps_tag_handlers["COMBOS"]= &SetStepsCombos;
		steps_tag_handlers["WARPS"]= &SetStepsWarps;
		steps_tag_handlers["SPEEDS"]= &SetStepsSpeeds;
		steps_tag_handlers["SCROLLS"]= &SetStepsScrolls;
		steps_tag_handlers["FAKES"]= &SetStepsFakes;
		steps_tag_handlers["LABELS"]= &SetStepsLabels;
		/* If this is called, the chart does not use the same attacks
		 * as the Song's timing. No other changes are required. */
		steps_tag_handlers["ATTACKS"]= &SetStepsAttacks;
		steps_tag_handlers["OFFSET"]= &SetStepsOffset;
		steps_tag_handlers["DISPLAYBPM"]= &SetStepsDisplayBPM;

		load_note_data_handlers["VERSION"]= LNDID_version;
		load_note_data_handlers["STEPSTYPE"]= LNDID_stepstype;
		load_note_data_handlers["CHARTNAME"]= LNDID_chartname;
		load_note_data_handlers["DESCRIPTION"]= LNDID_description;
		load_note_data_handlers["DIFFICULTY"]= LNDID_difficulty;
		load_note_data_handlers["METER"]= LNDID_meter;
		load_note_data_handlers["CREDIT"]= LNDID_credit;
		load_note_data_handlers["NOTES"]= LNDID_notes;
		load_note_data_handlers["NOTES2"]= LNDID_notes2;
		load_note_data_handlers["NOTEDATA"]= LNDID_notedata;
	}
};
ssc_parser_helper_t parser_helper;
// End parser_helper related functions. -Kyz
/****************************************************************/

void SSCLoader::ProcessBPMs( TimingData &out, const RString sParam )
{
	std::vector<RString> arrayBPMExpressions;
	split( sParam, ",", arrayBPMExpressions );

	for( unsigned b=0; b<arrayBPMExpressions.size(); b++ )
	{
		std::vector<RString> arrayBPMValues;
		split( arrayBPMExpressions[b], "=", arrayBPMValues );
		if( arrayBPMValues.size() != 2 )
		{
			LOG->UserLog("Song file",
				     this->GetSongTitle(),
				     "has an invalid #BPMS value \"%s\" (must have exactly one '='), ignored.",
				     arrayBPMExpressions[b].c_str() );
			continue;
		}

		const float fBeat = StringToFloat( arrayBPMValues[0] );
		const float fNewBPM = StringToFloat( arrayBPMValues[1] );
		if( fBeat >= 0 && fNewBPM > 0 )
		{
			out.AddSegment( BPMSegment(BeatToNoteRow(fBeat), fNewBPM) );
		}
		else
		{
			LOG->UserLog("Song file",
				     this->GetSongTitle(),
				     "has an invalid BPM at beat %f, BPM %f.",
				     fBeat, fNewBPM );
		}
	}
}

void SSCLoader::ProcessStops( TimingData &out, const RString sParam )
{
	std::vector<RString> arrayStopExpressions;
	split( sParam, ",", arrayStopExpressions );

	for( unsigned b=0; b<arrayStopExpressions.size(); b++ )
	{
		std::vector<RString> arrayStopValues;
		split( arrayStopExpressions[b], "=", arrayStopValues );
		if( arrayStopValues.size() != 2 )
		{
			LOG->UserLog("Song file",
				     this->GetSongTitle(),
				     "has an invalid #STOPS value \"%s\" (must have exactly one '='), ignored.",
				     arrayStopExpressions[b].c_str() );
			continue;
		}

		const float fBeat = StringToFloat( arrayStopValues[0] );
		const float fNewStop = StringToFloat( arrayStopValues[1] );
		if( fBeat >= 0 && fNewStop > 0 )
			out.AddSegment( StopSegment(BeatToNoteRow(fBeat), fNewStop) );
		else
		{
			LOG->UserLog("Song file",
				     this->GetSongTitle(),
				     "has an invalid Stop at beat %f, length %f.",
				     fBeat, fNewStop );
		}
	}
}

void SSCLoader::ProcessWarps( TimingData &out, const RString sParam, const float fVersion )
{
	std::vector<RString> arrayWarpExpressions;
	split( sParam, ",", arrayWarpExpressions );

	for( unsigned b=0; b<arrayWarpExpressions.size(); b++ )
	{
		std::vector<RString> arrayWarpValues;
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
	std::vector<RString> arrayLabelExpressions;
	split( sParam, ",", arrayLabelExpressions );

	for( unsigned b=0; b<arrayLabelExpressions.size(); b++ )
	{
		std::vector<RString> arrayLabelValues;
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
	std::vector<RString> arrayComboExpressions;
	split( line, ",", arrayComboExpressions );

	for( unsigned f=0; f<arrayComboExpressions.size(); f++ )
	{
		std::vector<RString> arrayComboValues;
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
	std::vector<RString> vs1;
	split( sParam, ",", vs1 );

	for (RString const &s1 : vs1)
	{
		std::vector<RString> vs2;
		split( s1, "=", vs2 );

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

		load_note_data_handler_map_t::iterator handler=
			parser_helper.load_note_data_handlers.find(valueName);
		if(handler != parser_helper.load_note_data_handlers.end())
		{
			if(tryingSteps)
			{
				switch(handler->second)
				{
					case LNDID_version:
						// Note that version is in both switches.  Formerly, it was
						// checked before the tryingSteps condition. -Kyz
						storedVersion = StringToFloat(matcher);
						break;
					case LNDID_stepstype:
						if(out.m_StepsType != GAMEMAN->StringToStepsType(matcher))
						{ tryingSteps = false; }
						break;
					case LNDID_chartname:
						if(storedVersion >= VERSION_CHART_NAME_TAG &&
							out.GetChartName() != matcher)
						{ tryingSteps = false; }
						break;
					case LNDID_description:
						if(storedVersion < VERSION_CHART_NAME_TAG)
						{
							if(out.GetChartName() != matcher)
							{ tryingSteps = false; }
						}
						else if(out.GetDescription() != matcher)
						{ tryingSteps = false; }
						break;
					case LNDID_difficulty:
						// Accept any difficulty if it's an edit because LoadEditFromMsd
						// forces edits onto Edit difficulty even if they have a difficulty
						// tag. -Kyz
						if(out.GetDifficulty() != StringToDifficulty(matcher) &&
							!(out.GetDifficulty() == Difficulty_Edit &&
								GetExtension(cachePath).MakeLower() == "edit"))
						{ tryingSteps = false; }
						break;
					case LNDID_meter:
						if(out.GetMeter() != StringToInt(matcher))
						{ tryingSteps = false; }
						break;
					case LNDID_credit:
						if(out.GetCredit() != matcher)
						{ tryingSteps = false; }
						break;
					case LNDID_notes:
					case LNDID_notes2:
						out.SetSMNoteData(matcher);
						out.TidyUpData();
						return true;
					default:
						break;
				}
			}
			else
			{
				switch(handler->second)
				{
					case LNDID_version:
						// Note that version is in both switches.  Formerly, it was
						// checked before the tryingSteps condition. -Kyz
						storedVersion = StringToFloat(matcher);
						break;
					case LNDID_notedata:
						tryingSteps = true;
						break;
					default:
						break;
				}
			}
		}
		else
		{
			// Silently ignore unrecognized tags, as was done before. -Kyz
		}
	}
	return false;
}

bool SSCLoader::LoadFromSimfile( const RString &sPath, Song &out, bool bFromCache )
{
	//LOG->Trace( "Song::LoadFromSSCFile(%s)", sPath.c_str() );

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
	Steps* pNewNotes = nullptr;
	TimingData stepsTiming;

	SongTagInfo reused_song_info(&*this, &out, sPath, bFromCache);
	StepsTagInfo reused_steps_info(&*this, &out, sPath, bFromCache);

	for( unsigned i = 0; i < values; i++ )
	{
		const MsdFile::value_t &sParams = msd.GetValue(i);
		RString sValueName = sParams[0];
		sValueName.MakeUpper();

		switch (state)
		{
			case GETTING_SONG_INFO:
			{
				reused_song_info.params= &sParams;
				song_handler_map_t::iterator handler=
					parser_helper.song_tag_handlers.find(sValueName);
				if(handler != parser_helper.song_tag_handlers.end())
				{
					handler->second(reused_song_info);
				}
				else if(sValueName.Left(strlen("BGCHANGES"))=="BGCHANGES")
				{
					SetBGChanges(reused_song_info);
				}
				// This tag will get us to the next section.
				else if(sValueName == "NOTEDATA")
				{
					state = GETTING_STEP_INFO;
					pNewNotes = out.CreateSteps();
					stepsTiming = TimingData(out.m_SongTiming.m_fBeat0OffsetInSeconds);
					reused_steps_info.has_own_timing = false;
					reused_steps_info.steps= pNewNotes;
					reused_steps_info.timing= &stepsTiming;
				}
				else
				{
					// Silently ignore unrecognized tags, as was done before. -Kyz
				}
				break;
			}
			case GETTING_STEP_INFO:
			{
				reused_steps_info.params= &sParams;
				steps_handler_map_t::iterator handler=
					parser_helper.steps_tag_handlers.find(sValueName);
				if(handler != parser_helper.steps_tag_handlers.end())
				{
					handler->second(reused_steps_info);
				}
				else if(sValueName=="NOTES" || sValueName=="NOTES2")
				{
					state = GETTING_SONG_INFO;
					if(reused_steps_info.has_own_timing)
					{ pNewNotes->m_Timing = stepsTiming; }
					reused_steps_info.has_own_timing = false;
					pNewNotes->SetSMNoteData(sParams[1]);
					pNewNotes->TidyUpData();
					pNewNotes->SetFilename(sPath);
					out.AddSteps(pNewNotes);
				}
				else if(sValueName=="STEPFILENAME")
				{
					state = GETTING_SONG_INFO;
					if(reused_steps_info.has_own_timing)
					{ pNewNotes->m_Timing = stepsTiming; }
					reused_steps_info.has_own_timing = false;
					pNewNotes->SetFilename(sParams[1]);
					out.AddSteps(pNewNotes);
				}
				else
				{
					// Silently ignore unrecognized tags, as was done before. -Kyz
				}
				break;
			}
		}
	}
	out.m_fVersion = STEPFILE_VERSION_NUMBER;
	TidyUpData(out, bFromCache);
	return true;
}

bool SSCLoader::LoadEditFromFile( RString sEditFilePath, ProfileSlot slot, bool bAddStepsToSong, Song *givenSong /* = nullptr */ )
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

	return LoadEditFromMsd( msd, sEditFilePath, slot, bAddStepsToSong, givenSong );
}

bool SSCLoader::LoadEditFromMsd(const MsdFile &msd,
				const RString &sEditFilePath,
				ProfileSlot slot,
				bool bAddStepsToSong,
				Song *givenSong /* = nullptr */ )
{
	Song* pSong = givenSong;
	Steps* pNewNotes = nullptr;
	TimingData stepsTiming;

	StepsTagInfo reused_steps_info(&*this, pSong, sEditFilePath, false);
	reused_steps_info.for_load_edit= true;
	reused_steps_info.timing= &stepsTiming;

	for(unsigned int i = 0; i < msd.GetNumValues(); ++i)
	{
		int iNumParams = msd.GetNumParams(i);
		const MsdFile::value_t &sParams = msd.GetValue(i);
		RString sValueName = sParams[0];
		sValueName.MakeUpper();

		if(pSong != nullptr)
		{
			reused_steps_info.params= &sParams;
			steps_handler_map_t::iterator handler=
				parser_helper.steps_tag_handlers.find(sValueName);
			if(pNewNotes != nullptr && handler != parser_helper.steps_tag_handlers.end())
			{
				handler->second(reused_steps_info);
			}
			else if(sValueName=="NOTEDATA")
			{
				pNewNotes = pSong->CreateSteps();
				reused_steps_info.steps= pNewNotes;
				reused_steps_info.ssc_format= true;
			}
			else if(sValueName=="NOTES")
			{
				if(pSong == nullptr)
				{
					LOG->UserLog("Edit file", sEditFilePath,
						"doesn't have a #SONG tag preceeding the first #NOTES tag,"
						" and is not in a valid song-specific folder.");
					return false;
				}

				if(!reused_steps_info.ssc_format && iNumParams < 7)
				{
					LOG->UserLog("Edit file", sEditFilePath,
						"has %d fields in a #NOTES tag, but should have at least 7.",
						iNumParams);
					continue;
				}

				// I have no idea what the purpose of this bAddStepsToSong flag is.
				// It looks like it causes LoadEditFromMsd to just throw away
				// whatever steps data was loaded, possibly leaking the memory used
				// by pNewNotes.  It was here before I rewrote LoadEditFromMsd to
				// use a map instead of an else if chain, so I preserved it. -Kyz
				if(!bAddStepsToSong)
				{ return true; }

				// Force the difficulty to edit in case the edit set its own
				// difficulty because IsEditAlreadyLoaded has an assert and edits
				// shouldn't be able to add charts of other difficulties. -Kyz
				if(pNewNotes != nullptr)
				{
					pNewNotes->SetDifficulty(Difficulty_Edit);
					if(pSong->IsEditAlreadyLoaded(pNewNotes))
					{
						LOG->UserLog("Edit file", sEditFilePath,
							"is a duplicate of another edit that was already loaded.");
						SAFE_DELETE(pNewNotes);
						return false;
					}
				}

				if(reused_steps_info.ssc_format)
				{
					if(reused_steps_info.has_own_timing)
					{ pNewNotes->m_Timing = stepsTiming; }
					pNewNotes->SetSMNoteData(sParams[1]);
					pNewNotes->TidyUpData();
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

				pNewNotes->SetLoadedFromProfile(slot);
				pNewNotes->SetDifficulty(Difficulty_Edit);
				pNewNotes->SetFilename(sEditFilePath);

				pSong->AddSteps(pNewNotes);
				return true; // Only allow one Steps per edit file!
			}
			else
			{
				LOG->UserLog("Edit file", sEditFilePath,
					"has an unexpected value \"%s\".", sValueName.c_str());
			}
		}
		else
		{
			if(sValueName == "SONG")
			{
				if(pSong)
				{
					/* LOG->UserLog("Edit file", sEditFilePath, "has more than one #SONG tag.");
						 return false; */
					continue;
				}

				RString sSongFullTitle = sParams[1];
				this->SetSongTitle(sParams[1]);
				sSongFullTitle.Replace('\\', '/');
				pSong = SONGMAN->FindSong(sSongFullTitle);
				reused_steps_info.song= pSong;
				if(pSong == nullptr)
				{
					LOG->UserLog("Edit file", sEditFilePath,
						"requires a song \"%s\" that isn't present.",
						sSongFullTitle.c_str());
					return false;
				}
				if(pSong->GetNumStepsLoadedFromProfile(slot) >= MAX_EDITS_PER_SONG_PER_PROFILE)
				{
					LOG->UserLog("Song file", sSongFullTitle,
						"already has the maximum number of edits allowed for ProfileSlotP%d.",
						slot+1);
					return false;
				}
				reused_steps_info.song= pSong;
			}
		}
	}
	// Edit had no valid #NOTES sections
	return false;
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
