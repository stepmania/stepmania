#include "global.h"
#include <cerrno>
#include <cstring>
#include "NotesWriterSSC.h"
#include "BackgroundUtil.h"
#include "Foreach.h"
#include "GameManager.h"
#include "LocalizedString.h"
#include "NoteTypes.h"
#include "Profile.h"
#include "ProfileManager.h"
#include "RageFile.h"
#include "RageFileManager.h"
#include "RageLog.h"
#include "RageUtil.h"
#include "Song.h"
#include "Steps.h"

/**
 * @brief Turn a vector of lines into a single line joined by newline characters.
 * @param lines the list of lines to join.
 * @return the joined lines. */
static RString JoinLineList( vector<RString> &lines )
{
	for( unsigned i = 0; i < lines.size(); ++i )
		TrimRight( lines[i] );

	// Skip leading blanks.
	unsigned j = 0;
	while( j < lines.size() && lines.size() == 0 )
		++j;

	return join( "\r\n", lines.begin()+j, lines.end() );
}


// A utility class to write timing tags more easily!
struct TimingTagWriter {

	vector<RString> *m_pvsLines;
	RString m_sNext;

	TimingTagWriter( vector<RString> *pvsLines ): m_pvsLines (pvsLines) { }

	void Write( const int row, const char *value )
	{
		m_pvsLines->push_back( m_sNext + ssprintf( "%s=%s", FormatDouble(NoteRowToBeat(row)).c_str(), value ) );
		m_sNext = ",";
	}

	void Write( const int row, const float value )        { Write( row, ssprintf( "%s", FormatDouble(value).c_str() ) ); }
	void Write( const int row, const int value )          { Write( row, ssprintf( "%d",    value ) ); }
	void Write( const int row, const int a, const int b ) { Write( row, ssprintf( "%d=%d", a, b ) );  }
	void Write( const int row, const float a, const float b ) { Write( row, ssprintf( "%s=%s", FormatDouble(a).c_str(), FormatDouble(b).c_str()) ); }
	void Write( const int row, const float a, const float b, const unsigned short c )
		{ Write( row, ssprintf( "%s=%s=%hd", FormatDouble(a).c_str(), FormatDouble(b).c_str(), c) ); }

	void Init( const RString sTag ) { m_sNext = "#" + sTag + ":"; }
	void Finish( ) { m_pvsLines->push_back( ( m_sNext != "," ? m_sNext : "" ) + ";" ); }
};

static void GetTimingTags( vector<RString> &lines, const TimingData &timing, bool bIsSong = false )
{
	TimingTagWriter writer(&lines);

	// timing.TidyUpData(); // UGLY: done via const_cast. do we really -need- this here?
	#define WRITE_SEG_LOOP_OPEN(enum_type, seg_type, seg_name, to_func) \
		{ \
			vector<TimingSegment*> const& segs= timing.GetTimingSegments(enum_type); \
			if(!segs.empty()) \
			{ \
				writer.Init(seg_name); \
				for(auto&& seg : segs) \
				{ \
					const seg_type* segment= to_func(seg);

	#define WRITE_SEG_LOOP_CLOSE } writer.Finish(); } }

	WRITE_SEG_LOOP_OPEN(SEGMENT_BPM, BPMSegment, "BPMS", ToBPM);
	writer.Write(segment->GetRow(), segment->GetBPM());
	WRITE_SEG_LOOP_CLOSE;

	WRITE_SEG_LOOP_OPEN(SEGMENT_STOP, StopSegment, "STOPS", ToStop);
	writer.Write(segment->GetRow(), segment->GetPause());
	WRITE_SEG_LOOP_CLOSE;

	WRITE_SEG_LOOP_OPEN(SEGMENT_DELAY, DelaySegment, "DELAYS", ToDelay);
	writer.Write(segment->GetRow(), segment->GetPause());
	WRITE_SEG_LOOP_CLOSE;

	WRITE_SEG_LOOP_OPEN(SEGMENT_WARP, WarpSegment, "WARPS", ToWarp);
	writer.Write(segment->GetRow(), segment->GetLength());
	WRITE_SEG_LOOP_CLOSE;

	WRITE_SEG_LOOP_OPEN(SEGMENT_TIME_SIG, TimeSignatureSegment, "TIMESIGNATURESEGMENT", ToTimeSignature);
	writer.Write(segment->GetRow(), segment->GetNum(), segment->GetDen());
	WRITE_SEG_LOOP_CLOSE;

	WRITE_SEG_LOOP_OPEN(SEGMENT_TICKCOUNT, TickcountSegment, "TICKCOUNTS", ToTickcount);
	writer.Write(segment->GetRow(), segment->GetTicks());
	WRITE_SEG_LOOP_CLOSE;

	WRITE_SEG_LOOP_OPEN(SEGMENT_COMBO, ComboSegment, "COMBOS", ToCombo);
	if(segment->GetCombo() == segment->GetMissCombo())
	{
		writer.Write(segment->GetRow(), segment->GetCombo());
	}
	else
	{
		writer.Write(segment->GetRow(), segment->GetCombo(), segment->GetMissCombo());
	}
	WRITE_SEG_LOOP_CLOSE;

	WRITE_SEG_LOOP_OPEN(SEGMENT_SPEED, SpeedSegment, "SPEEDS", ToSpeed);
	writer.Write(segment->GetRow(), segment->GetRatio(), segment->GetDelay(), segment->GetUnit());
	WRITE_SEG_LOOP_CLOSE;

	WRITE_SEG_LOOP_OPEN(SEGMENT_SCROLL, ScrollSegment, "SCROLLS", ToScroll);
	writer.Write(segment->GetRow(), segment->GetRatio());
	WRITE_SEG_LOOP_CLOSE;

	// TODO: Investigate why someone wrote a condition for leaving fakes out of
	// the song timing tags, and put it in a function that is only used when
	// writing the steps timing tags. -Kyz
	if(!bIsSong)
	{
		WRITE_SEG_LOOP_OPEN(SEGMENT_FAKE, FakeSegment, "FAKES", ToFake);
		writer.Write(segment->GetRow(), segment->GetLength());
		WRITE_SEG_LOOP_CLOSE;
	}

	WRITE_SEG_LOOP_OPEN(SEGMENT_LABEL, LabelSegment, "LABELS", ToLabel);
	if(!segment->GetLabel().empty())
	{
		writer.Write(segment->GetRow(), segment->GetLabel());
	}
	WRITE_SEG_LOOP_CLOSE;

#undef WRITE_SEG_LOOP_OPEN
#undef WRITE_SEG_LOOP_CLOSE
}

static void write_tag(RageFile& f, RString const& format,
	RString const& value)
{
	if( !value.empty() )
	{
		f.PutLine( ssprintf(format, SmEscape(value).c_str()) );
	}
}

static void WriteTimingTags( RageFile &f, const TimingData &timing, bool bIsSong = false )
{
	static RString value;

	write_tag(f, "#BPMS:%s;",
				join(",", timing.ToVectorString(SEGMENT_BPM, 3)));
	write_tag(f, "#STOPS:%s;",
				join(",", timing.ToVectorString(SEGMENT_STOP, 3)));
	write_tag(f, "#DELAYS:%s;",
				join(",", timing.ToVectorString(SEGMENT_DELAY, 3)));
	write_tag(f, "#WARPS:%s;",
				join(",", timing.ToVectorString(SEGMENT_WARP, 3)));

	const vector<TimingSegment *> &timesigs = timing.GetTimingSegments(SEGMENT_TIME_SIG);
	for( unsigned i=0; i<timesigs.size(); i++ )
	{
		const TimeSignatureSegment *ts = ToTimeSignature(timesigs[i]);
		value = ssprintf( "%s=%i=%i", FormatDouble("%.3f", ts->GetBeat()).c_str(), ts->GetNum(), ts->GetDen() );

		if( timesigs.size()==1 && value == "0=4=4" ) break;
		write_tag( f, "#TIMESIGNATURES:%s;", join(",", timing.ToVectorString(SEGMENT_TIME_SIG, 3)) );	break;
	}
		
	const vector<TimingSegment *> &ticks = timing.GetTimingSegments(SEGMENT_TICKCOUNT);
	for( unsigned i=0; i<ticks.size(); i++ )
	{
		const TickcountSegment *tc = ToTickcount(ticks[i]);
		value = ssprintf( "%s=%i", FormatDouble("%.3f", tc->GetBeat()).c_str(), tc->GetTicks() );

		if( ticks.size()==1 && value == "0=4" ) break;
		write_tag( f, "#TICKCOUNTS:%s;", join(",", timing.ToVectorString(SEGMENT_TICKCOUNT, 3)) ); break;
	}

	const vector<TimingSegment *> &combos = timing.GetTimingSegments(SEGMENT_COMBO);
	for( unsigned i=0; i<combos.size(); i++ )
	{
		const ComboSegment *cb = ToCombo(combos[i]);
		value = ssprintf( "%s=%i", FormatDouble("%.3f", cb->GetBeat()).c_str(), cb->GetCombo() );

		if( combos.size()==1 && value == "0=1" ) break;
		write_tag( f, "#COMBOS:%s;", join(",", timing.ToVectorString(SEGMENT_COMBO, 3)) ); break;
	}

	const vector<TimingSegment *> &speeds = timing.GetTimingSegments(SEGMENT_SPEED);
	for( unsigned i=0; i<speeds.size(); i++ )
	{
		const SpeedSegment *sp = ToSpeed(speeds[i]);
		value = ssprintf( "%s=%s=%s=%u", FormatDouble("%.3f", sp->GetBeat()).c_str(), FormatDouble("%.3f", sp->GetRatio()).c_str(), 
				FormatDouble("%.3f", sp->GetDelay()).c_str(), sp->GetUnit() );

		if( speeds.size()==1 && value == "0=1=0=0" ) break;
		write_tag( f, "#SPEEDS:%s;", join(",", timing.ToVectorString(SEGMENT_SPEED, 3)) ); break;
	}

	const vector<TimingSegment *> &scrolls = timing.GetTimingSegments(SEGMENT_SCROLL);
	for( unsigned i=0; i<scrolls.size(); i++ )
	{
		const ScrollSegment *sc = ToScroll(scrolls[i]);
		value = ssprintf( "%s=%s", FormatDouble(sc->GetBeat()).c_str(), FormatDouble(sc->GetRatio()).c_str());

		if( scrolls.size()==1 && value=="0=1" ) break;
		write_tag(f, "#SCROLLS:%s;", join(",", timing.ToVectorString(SEGMENT_SCROLL, 3))); break;
	}

	write_tag( f, "#FAKES:%s;",
				join(",", timing.ToVectorString(SEGMENT_FAKE, 3)) );

	const vector<TimingSegment *> &labels = timing.GetTimingSegments(SEGMENT_LABEL);
	for( unsigned i=0; i<labels.size(); i++ )
	{
		const LabelSegment *lb = ToLabel(labels[i]);
		value = ssprintf( "%s=%s", FormatDouble(lb->GetBeat()).c_str(), lb->GetLabel().c_str() );

		if( labels.size()==1 && value=="0=Song Start" ) break;
		write_tag( f, "#LABELS:%s;", join(",", timing.ToVectorString(SEGMENT_LABEL, 3)) ); break;
	}
}

/**
 * @brief Write out the common tags for .SSC files.
 * @param f the file in question.
 * @param out the Song in question. */
static void WriteGlobalTags( RageFile &f, const Song &out )
{
	f.PutLine( ssprintf( "#VERSION:%.2f;", STEPFILE_VERSION_NUMBER ) );
	write_tag(f, "#TITLE:%s;", out.m_sMainTitle);
	write_tag(f, "#SUBTITLE:%s;", out.m_sSubTitle);
	write_tag(f, "#ARTIST:%s;", out.m_sArtist);
	write_tag(f, "#TITLETRANSLIT:%s;", out.m_sMainTitleTranslit);
	write_tag(f, "#SUBTITLETRANSLIT:%s;", out.m_sSubTitleTranslit);
	write_tag(f, "#ARTISTTRANSLIT:%s;", out.m_sArtistTranslit);
	write_tag(f, "#GENRE:%s;", out.m_sGenre);
	write_tag(f, "#ORIGIN:%s;", out.m_sOrigin);
	write_tag(f, "#CREDIT:%s;", out.m_sCredit);
	write_tag(f, "#BANNER:%s;", out.m_sBannerFile);
	write_tag(f, "#BACKGROUND:%s;", out.m_sBackgroundFile);
	write_tag(f, "#PREVIEWVID:%s;", out.m_sPreviewVidFile);
	write_tag(f, "#JACKET:%s;", out.m_sJacketFile);
	write_tag(f, "#CDIMAGE:%s;", out.m_sCDFile);
	write_tag(f, "#DISCIMAGE:%s;", out.m_sDiscFile);
	write_tag(f, "#LYRICSPATH:%s;", out.m_sLyricsFile);
	write_tag(f, "#CDTITLE:%s;", out.m_sCDTitleFile);
	write_tag(f, "#MUSIC:%s;", out.m_sMusicFile);
	write_tag(f, "#PREVIEW:%s;", out.m_PreviewFile);
	{
		vector<RString> vs = out.GetInstrumentTracksToVectorString();
		if( !vs.empty() )
		{
			RString s = join( ",", vs );
			f.PutLine( "#INSTRUMENTTRACK:" + s + ";\n" );
		}
	}
	
	if( out.m_SongTiming.m_fBeat0OffsetInSeconds != 0 )
	{
		write_tag(f, "#OFFSET:%s;", FormatDouble("%.3f", out.m_SongTiming.m_fBeat0OffsetInSeconds));
	}
	
	write_tag(f, "#SAMPLESTART:%s;", FormatDouble("%.3f", out.m_fMusicSampleStartSeconds));
	write_tag(f, "#SAMPLELENGTH:%s;", FormatDouble("%.3f", out.m_fMusicSampleLengthSeconds));
	
	switch(out.m_SelectionDisplay)
	{
		default: ASSERT_M(0, "An invalid selectable value was found for this song!"); // fall through
		case Song::SHOW_ALWAYS:		break;
		//case Song::SHOW_NONSTOP:	f.Write( "#SELECTABLE:NONSTOP;" );	break;
		case Song::SHOW_NEVER:		f.Write( "#SELECTABLE:NO;" );		break;
	}

	switch( out.m_DisplayBPMType )
	{
		case DISPLAY_BPM_ACTUAL:
			// write nothing
			break;
		case DISPLAY_BPM_SPECIFIED:
			if( out.m_fSpecifiedBPMMin == out.m_fSpecifiedBPMMax )
				f.PutLine( ssprintf( "#DISPLAYBPM:%s;", FormatDouble("%.3f", out.m_fSpecifiedBPMMin).c_str() ) );
			else
				f.PutLine( ssprintf( "#DISPLAYBPM:%s:%s;", FormatDouble("%.3f", out.m_fSpecifiedBPMMin).c_str(),
								FormatDouble("%.3f", out.m_fSpecifiedBPMMax).c_str() ) );
			break;
		case DISPLAY_BPM_RANDOM:
			f.PutLine( ssprintf( "#DISPLAYBPM:*;" ) );
			break;
		default:
			break;
	}

	WriteTimingTags( f, out.m_SongTiming, true );

	if( out.GetSpecifiedLastSecond() > 0 )
		f.PutLine( ssprintf("#LASTSECONDHINT:%.3f;", out.GetSpecifiedLastSecond()) );

	FOREACH_BackgroundLayer( b )
	{
		if( b==0 )
		{
			if( out.GetBackgroundChanges(b).empty() ) break;
			f.Write( "#BGCHANGES:" );
		}
		else if( out.GetBackgroundChanges(b).empty() )
			continue;	// skip
		else
			f.Write( ssprintf("#BGCHANGES%d:", b+1) );

		FOREACH_CONST( BackgroundChange, out.GetBackgroundChanges(b), bgc )
			f.PutLine( (*bgc).ToString() +"," );

		/* If there's an animation plan at all, add a dummy "-nosongbg-" tag to
		 * indicate that this file doesn't want a song BG entry added at the end.
		 * See SSCLoader::TidyUpData. This tag will be removed on load. Add it
		 * at a very high beat, so it won't cause problems if loaded in older versions. */
		if( b==0 && !out.GetBackgroundChanges(b).empty() )
			f.PutLine( "99999=-nosongbg-=1=0=0=0 // don't automatically add -songbackground-" );
		f.PutLine( ";" );
	}

	if( out.GetForegroundChanges().size() )
	{
		f.Write( "#FGCHANGES:" );
		FOREACH_CONST( BackgroundChange, out.GetForegroundChanges(), bgc )
		{
			f.PutLine( (*bgc).ToString() +"," );
		}
		f.PutLine( ";" );
	}

	if( !out.m_vsKeysoundFile.empty() )
	{
		f.Write( "#KEYSOUNDS:" );
		for( unsigned i=0; i<out.m_vsKeysoundFile.size(); i++ )
		{
			// some keysound files has the first sound that starts with #,
			// which makes MsdFile fail parsing the whole declaration.
			// in this case, add a backslash at the front
			// (#KEYSOUNDS:\#bgm.wav,01.wav,02.wav,..) and handle that on load.
			if( i == 0 && out.m_vsKeysoundFile[i].size() > 0 && out.m_vsKeysoundFile[i][0] == '#' )
				f.Write("\\");
			f.Write( out.m_vsKeysoundFile[i] );
			if( i != out.m_vsKeysoundFile.size()-1 )
				f.Write( "," );
		}
		f.PutLine( ";" );
	}
	
	if( !out.m_Attacks.empty() )
	{
		// attacks section
		//f.PutLine( ssprintf("#ATTACKS:%s;", out.GetAttackString().c_str()) );
		f.PutLine( "#ATTACKS:" );
		for(unsigned j = 0; j < out.m_Attacks.size(); j++)
		{
			const Attack &a = out.m_Attacks[j];
			f.Write( ssprintf( "  TIME=%.3f:LEN=%.3f:MODS=%s",
				a.fStartSecond, a.fSecsRemaining, a.sModifiers.c_str() ) );

			if( j+1 < out.m_Attacks.size() )
				f.Write( ":" );
		}
		f.Write( ";" );
	}
}

static void push_back_tag(vector<RString>& lines,
	RString const& format, RString const& value)
{
	if(!value.empty())
	{
		lines.push_back(ssprintf(format, SmEscape(value).c_str()));
	}
}

/**
 * @brief Retrieve the individual batches of NoteData.
 * @param song the Song in question.
 * @param in the Steps in question.
 * @param bSavingCache a flag to see if we're saving certain cache data.
 * @return the NoteData in RString form. */
static RString GetSSCNoteData( const Song &song, const Steps &in, bool bSavingCache )
{
	vector<RString> lines;

	lines.push_back( "" );
	// Escape to prevent some clown from making a comment of "\r\n;"
	lines.push_back( ssprintf("//---------------%s - %s----------------",
		in.m_StepsTypeStr.c_str(), SmEscape(in.GetDescription()).c_str()) );
	lines.push_back( "#NOTEDATA:;" ); // our new separator.
	push_back_tag(lines, "#CHARTNAME:%s;", in.GetChartName());
	push_back_tag(lines, "#STEPSTYPE:%s;", in.m_StepsTypeStr);
	push_back_tag(lines, "#DESCRIPTION:%s;", in.GetDescription());
	push_back_tag(lines, "#CHARTSTYLE:%s;", in.GetChartStyle());
	push_back_tag(lines, "#DIFFICULTY:%s;", DifficultyToString(in.GetDifficulty()));
	lines.push_back( ssprintf( "#METER:%s;", FormatDouble("%.2f", in.GetMeter()).c_str()) );

	const RString& music= in.GetMusicFile();
	if(!music.empty())
	{
		lines.push_back(ssprintf("#MUSIC:%s;", music.c_str()));
	}

	vector<RString> asRadarValues;
	FOREACH_PlayerNumber( pn )
	{
		const RadarValues &rv = in.GetRadarValues( pn );
		FOREACH_ENUM( RadarCategory, rc )
			asRadarValues.push_back( ssprintf("%s", FormatDouble("%.3f", rv[rc]).c_str()) );
	}
	lines.push_back( ssprintf( "#RADARVALUES:%s;", join(",",asRadarValues).c_str() ) );

	push_back_tag(lines, "#CREDIT:%s;", in.GetCredit());

	// If the Steps TimingData is not empty, then they have their own
	// timing.  Write out the corresponding tags.
	if( !in.m_Timing.empty() )
	{
		lines.push_back( ssprintf( "#OFFSET:%s;", FormatDouble("%.3f", in.m_Timing.m_fBeat0OffsetInSeconds).c_str() ) );
		GetTimingTags( lines, in.m_Timing );
		
		switch( in.GetDisplayBPM() )
		{
			case DISPLAY_BPM_ACTUAL:
				// write nothing
				break;
			case DISPLAY_BPM_SPECIFIED:
			{
				float small = in.GetMinBPM();
				float big = in.GetMaxBPM();
				if (small == big)
					lines.push_back( ssprintf( "#DISPLAYBPM:%s;", FormatDouble("%.3f", small).c_str() ) );
				else
					lines.push_back( ssprintf( "#DISPLAYBPM:%s:%s;", FormatDouble("%.3f", small).c_str(), FormatDouble("%.3f", big).c_str() ) );
				break;
			}
			case DISPLAY_BPM_RANDOM:
				lines.push_back( ssprintf( "#DISPLAYBPM:*;" ) );
				break;
			default:
				break;
		}
	}

	// todo: get this to output similar to course mods -aj
	if (song.GetAttackString() != in.GetAttackString())
		lines.push_back( ssprintf("#ATTACKS:%s;", in.GetAttackString().c_str()));
	
	if (bSavingCache)
	{
		lines.push_back(ssprintf("#STEPFILENAME:%s;", in.GetFilename().c_str()));
	}
	else
	{
		RString sNoteData;
		in.GetSMNoteData( sNoteData );

		lines.push_back( song.m_vsKeysoundFile.empty() ? "#NOTES:" : "#NOTES2:" );

		TrimLeft(sNoteData);
		split( sNoteData, "\n", lines, true );
		lines.push_back( ";" );
	}
	return JoinLineList( lines );
}

bool NotesWriterSSC::Write( RString sPath, const Song &out, const vector<Steps*>& vpStepsToSave, bool bSavingCache )
{
	int flags = RageFile::WRITE;

	/* If we're not saving cache, we're saving real data, so enable SLOW_FLUSH
	 * to prevent data loss. If we're saving cache, this will slow things down
	 * too much. */
	if( !bSavingCache )
		flags |= RageFile::SLOW_FLUSH;

	RageFile f;
	if( !f.Open( sPath, flags ) )
	{
		LOG->UserLog( "Song file", sPath, "couldn't be opened for writing: %s", f.GetError().c_str() );
		return false;
	}

	WriteGlobalTags( f, out );
	
	if( bSavingCache )
	{
		f.PutLine( ssprintf( "" ) );
		f.PutLine( ssprintf( "// cache tags:" ) );
		f.PutLine( ssprintf( "#FIRSTSECOND:%.3f;", out.GetFirstSecond() ) );
		f.PutLine( ssprintf( "#LASTSECOND:%.3f;", out.GetLastSecond() ) );
		f.PutLine( ssprintf( "#SONGFILENAME:%s;", out.m_sSongFileName.c_str() ) );
		f.PutLine( ssprintf( "#HASMUSIC:%i;", out.m_bHasMusic ) );
		f.PutLine( ssprintf( "#HASBANNER:%i;", out.m_bHasBanner ) );
		f.PutLine( ssprintf( "#MUSICLENGTH:%.3f;", out.m_fMusicLengthSeconds ) );
		f.PutLine( ssprintf( "// end cache tags" ) );
	}

	// Save specified Steps to this file
	FOREACH_CONST( Steps*, vpStepsToSave, s ) 
	{
		const Steps* pSteps = *s;
		RString sTag = GetSSCNoteData( out, *pSteps, bSavingCache );
		f.PutLine( sTag );
	}
	if( f.Flush() == -1 )
		return false;

	return true;
}

void NotesWriterSSC::GetEditFileContents( const Song *pSong, const Steps *pSteps, RString &sOut )
{
	sOut = "";
	RString sDir = pSong->GetSongDir();

	// "Songs/foo/bar"; strip off "Songs/".
	vector<RString> asParts;
	split( sDir, "/", asParts );
	if( asParts.size() )
		sDir = join( "/", asParts.begin()+1, asParts.end() );
	sOut += ssprintf( "#SONG:%s;\r\n", sDir.c_str() );
	sOut += GetSSCNoteData( *pSong, *pSteps, false );
}

RString NotesWriterSSC::GetEditFileName( const Song *pSong, const Steps *pSteps )
{
	/* Try to make a unique name. This isn't guaranteed. Edit descriptions are
	 * case-sensitive, filenames on disk are usually not, and we decimate certain
	 * characters for FAT filesystems. */
	RString sFile = pSong->GetTranslitFullTitle() + " - " + pSteps->GetDescription();

	// HACK:
	if( pSteps->m_StepsType == StepsType_dance_double )
		sFile += " (doubles)";

	sFile += ".edit";

	MakeValidFilename( sFile );
	return sFile;
}

static LocalizedString DESTINATION_ALREADY_EXISTS	("NotesWriterSSC", "Error renaming file.  Destination file '%s' already exists.");
static LocalizedString ERROR_WRITING_FILE		("NotesWriterSSC", "Error writing file '%s'.");
bool NotesWriterSSC::WriteEditFileToMachine( const Song *pSong, Steps *pSteps, RString &sErrorOut )
{
	RString sDir = PROFILEMAN->GetProfileDir( ProfileSlot_Machine ) + EDIT_STEPS_SUBDIR;

	RString sPath = sDir + GetEditFileName(pSong,pSteps);

	// Check to make sure that we're not clobering an existing file before opening.
	bool bFileNameChanging = 
		pSteps->GetSavedToDisk()  && 
		pSteps->GetFilename() != sPath;
	if( bFileNameChanging  &&  DoesFileExist(sPath) )
	{
		sErrorOut = ssprintf( DESTINATION_ALREADY_EXISTS.GetValue(), sPath.c_str() );
		return false;
	}

	RageFile f;
	if( !f.Open(sPath, RageFile::WRITE | RageFile::SLOW_FLUSH) )
	{
		sErrorOut = ssprintf( ERROR_WRITING_FILE.GetValue(), sPath.c_str() );
		return false;
	}

	RString sTag;
	GetEditFileContents( pSong, pSteps, sTag );
	if( f.PutLine(sTag) == -1 || f.Flush() == -1 )
	{
		sErrorOut = ssprintf( ERROR_WRITING_FILE.GetValue(), sPath.c_str() );
		return false;
	}

	/* If the file name of the edit has changed since the last save, then delete the old
	 * file after saving the new one. If we delete it first, then we'll lose data on error. */

	if( bFileNameChanging )
		FILEMAN->Remove( pSteps->GetFilename() );
	pSteps->SetFilename( sPath );

	return true;
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
