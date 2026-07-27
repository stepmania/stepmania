#include "global.h"
#include <cerrno>
#include <cstring>
#include "NotesWriterSSC.h"
#include "BackgroundUtil.h"
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

#include <vector>


/**
 * @brief Turn a vector of lines into a single line joined by newline characters.
 * @param lines the list of lines to join.
 * @return the joined lines. */
static RString JoinLineList( std::vector<RString> &lines )
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

	std::vector<RString> *m_pvsLines;
	RString m_sNext;

	TimingTagWriter( std::vector<RString>* pvsLines ): m_pvsLines (pvsLines) { }

	void Write( const int row, const char *value )
	{
		m_pvsLines->push_back( m_sNext + ssprintf( "%.6f=%s", NoteRowToBeat(row), value ) );
		m_sNext = ",";
	}

	void Write( const int row, const float value )        { Write( row, ssprintf( "%.6f",  value ) ); }
	void Write( const int row, const int value )          { Write( row, ssprintf( "%d",    value ) ); }
	void Write( const int row, const int a, const int b ) { Write( row, ssprintf( "%d=%d", a, b ) );  }
	void Write( const int row, const float a, const float b ) { Write( row, ssprintf( "%.6f=%.6f", a, b) ); }
	void Write( const int row, const float a, const float b, const unsigned short c )
		{ Write( row, ssprintf( "%.6f=%.6f=%hd", a, b, c) ); }

	void Init( const RString sTag ) { m_sNext = "#" + sTag + ":"; }
	void Finish( ) { m_pvsLines->push_back( ( m_sNext != "," ? m_sNext : RString("") ) + ";" ); }

};

static void GetTimingTags( std::vector<RString> &lines, const TimingData &timing, bool bIsSong = false )
{
	TimingTagWriter w ( &lines );

	// timing.TidyUpData(); // UGLY: done via const_cast. do we really -need- this here?
	unsigned i = 0;

	w.Init( "BPMS" );
	const std::vector<TimingSegment*> &bpms = timing.GetTimingSegments(SEGMENT_BPM);
	for (; i < bpms.size(); i++)
	{
		const BPMSegment *bs = ToBPM( bpms[i] );
		w.Write( bs->GetRow(), bs->GetBPM() );
	}
	w.Finish();

	w.Init( "STOPS" );
	const std::vector<TimingSegment*> &stops = timing.GetTimingSegments(SEGMENT_STOP);
	for (i = 0; i < stops.size(); i++)
	{
		const StopSegment *ss = ToStop( stops[i] );
		w.Write( ss->GetRow(), ss->GetPause() );
	}
	w.Finish();

	w.Init( "DELAYS" );
	const std::vector<TimingSegment*> &delays = timing.GetTimingSegments(SEGMENT_DELAY);
	for (i = 0; i < delays.size(); i++)
	{
		const DelaySegment *ss = ToDelay( delays[i] );
		w.Write( ss->GetRow(), ss->GetPause() );
	}
	w.Finish();

	w.Init( "WARPS" );
	const std::vector<TimingSegment*> &warps = timing.GetTimingSegments(SEGMENT_WARP);
	for (i = 0; i < warps.size(); i++)
	{
		const WarpSegment *ws = ToWarp( warps[i] );
		w.Write( ws->GetRow(), ws->GetLength() );
	}
	w.Finish();

	const std::vector<TimingSegment*> &tSigs = timing.GetTimingSegments(SEGMENT_TIME_SIG);
	ASSERT( !tSigs.empty() );
	w.Init( "TIMESIGNATURES" );
	for (i = 0; i < tSigs.size(); i++)
	{
		const TimeSignatureSegment *ts = ToTimeSignature( tSigs[i] );
		w.Write( ts->GetRow(), ts->GetNum(), ts->GetDen() );
	}
	w.Finish();

	const std::vector<TimingSegment*> &ticks = timing.GetTimingSegments(SEGMENT_TICKCOUNT);
	ASSERT( !ticks.empty() );
	w.Init( "TICKCOUNTS" );
	for (i = 0; i < ticks.size(); i++)
	{
		const TickcountSegment *ts = ToTickcount( ticks[i] );
		w.Write( ts->GetRow(), ts->GetTicks() );
	}
	w.Finish();

	const std::vector<TimingSegment*> &combos = timing.GetTimingSegments(SEGMENT_COMBO);
	ASSERT( !combos.empty() );
	w.Init( "COMBOS" );
	for (i = 0; i < combos.size(); i++)
	{
		const ComboSegment *cs = ToCombo( combos[i] );
		if (cs->GetCombo() == cs->GetMissCombo())
			w.Write( cs->GetRow(), cs->GetCombo() );
		else
			w.Write( cs->GetRow(), cs->GetCombo(), cs->GetMissCombo() );
	}
	w.Finish();

	// Song Timing should only have the initial value.
	const std::vector<TimingSegment*> &speeds = timing.GetTimingSegments(SEGMENT_SPEED);
	w.Init( "SPEEDS" );
	for (i = 0; i < speeds.size(); i++)
	{
		SpeedSegment *ss = ToSpeed( speeds[i] );
		w.Write( ss->GetRow(), ss->GetRatio(), ss->GetDelay(), ss->GetUnit() );
	}
	w.Finish();

	w.Init( "SCROLLS" );
	const std::vector<TimingSegment*> &scrolls = timing.GetTimingSegments(SEGMENT_SCROLL);
	for (i = 0; i < scrolls.size(); i++)
	{
		ScrollSegment *ss = ToScroll( scrolls[i] );
		w.Write( ss->GetRow(), ss->GetRatio() );
	}
	w.Finish();

	if( !bIsSong )
	{
		const std::vector<TimingSegment*> &fakes = timing.GetTimingSegments(SEGMENT_FAKE);
		w.Init( "FAKES" );
		for (i = 0; i < fakes.size(); i++)
		{
			FakeSegment *fs = ToFake( fakes[i] );
			w.Write( fs->GetRow(), fs->GetLength() );
		}
		w.Finish();
	}

	w.Init( "LABELS" );
	const std::vector<TimingSegment*> &labels = timing.GetTimingSegments(SEGMENT_LABEL);
	for (i = 0; i < labels.size(); i++)
	{
		LabelSegment *ls = static_cast<LabelSegment *>(labels[i]);
		if (!ls->GetLabel().empty())
			w.Write( ls->GetRow(), ls->GetLabel().c_str() );
	}
	w.Finish();
}

static void WriteTimingTags( RageFile &f, const TimingData &timing, bool bIsSong = false )
{
	f.PutLine(ssprintf("#BPMS:%s;",
			   join(",\r\n", timing.ToVectorString(SEGMENT_BPM, 3)).c_str()));
	f.PutLine(ssprintf("#STOPS:%s;",
			   join(",\r\n", timing.ToVectorString(SEGMENT_STOP, 3)).c_str()));
	f.PutLine(ssprintf("#DELAYS:%s;",
			   join(",\r\n", timing.ToVectorString(SEGMENT_DELAY, 3)).c_str()));
	f.PutLine(ssprintf("#WARPS:%s;",
			   join(",\r\n", timing.ToVectorString(SEGMENT_WARP, 3)).c_str()));
	f.PutLine(ssprintf("#TIMESIGNATURES:%s;",
			   join(",\r\n", timing.ToVectorString(SEGMENT_TIME_SIG, 3)).c_str()));
	f.PutLine(ssprintf("#TICKCOUNTS:%s;",
			   join(",\r\n", timing.ToVectorString(SEGMENT_TICKCOUNT, 3)).c_str()));
	f.PutLine(ssprintf("#COMBOS:%s;",
			   join(",\r\n", timing.ToVectorString(SEGMENT_COMBO, 3)).c_str()));
	f.PutLine(ssprintf("#SPEEDS:%s;",
			   join(",\r\n", timing.ToVectorString(SEGMENT_SPEED, 3)).c_str()));
	f.PutLine(ssprintf("#SCROLLS:%s;",
			   join(",\r\n", timing.ToVectorString(SEGMENT_SCROLL, 3)).c_str()));
	f.PutLine(ssprintf("#FAKES:%s;",
			   join(",\r\n", timing.ToVectorString(SEGMENT_FAKE, 3)).c_str()));
	f.PutLine(ssprintf("#LABELS:%s;",
			   join(",\r\n", timing.ToVectorString(SEGMENT_LABEL, 3)).c_str()));

}

/**
 * @brief Write out the common tags for .SSC files.
 * @param f the file in question.
 * @param out the Song in question. */
static void WriteGlobalTags( RageFile &f, const Song &out )
{
	f.PutLine( ssprintf( "#VERSION:%.2f;", STEPFILE_VERSION_NUMBER ) );
	f.PutLine( ssprintf( "#TITLE:%s;", SmEscape(out.m_sMainTitle).c_str() ) );
	f.PutLine( ssprintf( "#SUBTITLE:%s;", SmEscape(out.m_sSubTitle).c_str() ) );
	f.PutLine( ssprintf( "#ARTIST:%s;", SmEscape(out.m_sArtist).c_str() ) );
	f.PutLine( ssprintf( "#TITLETRANSLIT:%s;", SmEscape(out.m_sMainTitleTranslit).c_str() ) );
	f.PutLine( ssprintf( "#SUBTITLETRANSLIT:%s;", SmEscape(out.m_sSubTitleTranslit).c_str() ) );
	f.PutLine( ssprintf( "#ARTISTTRANSLIT:%s;", SmEscape(out.m_sArtistTranslit).c_str() ) );
	f.PutLine( ssprintf( "#GENRE:%s;", SmEscape(out.m_sGenre).c_str() ) );
	f.PutLine( ssprintf( "#ORIGIN:%s;", SmEscape(out.m_sOrigin).c_str() ) );
	f.PutLine( ssprintf( "#CREDIT:%s;", SmEscape(out.m_sCredit).c_str() ) );
	f.PutLine( ssprintf( "#BANNER:%s;", SmEscape(out.m_sBannerFile).c_str() ) );
	f.PutLine( ssprintf( "#BACKGROUND:%s;", SmEscape(out.m_sBackgroundFile).c_str() ) );
	f.PutLine( ssprintf( "#PREVIEWVID:%s;", SmEscape(out.m_sPreviewVidFile).c_str() ) );
	f.PutLine( ssprintf( "#JACKET:%s;", SmEscape(out.m_sJacketFile).c_str() ) );
	f.PutLine( ssprintf( "#CDIMAGE:%s;", SmEscape(out.m_sCDFile).c_str() ) );
	f.PutLine( ssprintf( "#DISCIMAGE:%s;", SmEscape(out.m_sDiscFile).c_str() ) );
	f.PutLine( ssprintf( "#LYRICSPATH:%s;", SmEscape(out.m_sLyricsFile).c_str() ) );
	f.PutLine( ssprintf( "#CDTITLE:%s;", SmEscape(out.m_sCDTitleFile).c_str() ) );
	f.PutLine( ssprintf( "#MUSIC:%s;", SmEscape(out.m_sMusicFile).c_str() ) );
	if(!out.m_PreviewFile.empty())
	{
		f.PutLine(ssprintf("#PREVIEW:%s;", SmEscape(out.m_PreviewFile).c_str()));
	}

	{
		std::vector<RString> vs = out.GetInstrumentTracksToVectorString();
		if( !vs.empty() )
		{
			RString s = join( ",", vs );
			f.PutLine( "#INSTRUMENTTRACK:" + s + ";\n" );
		}
	}
	f.PutLine( ssprintf( "#OFFSET:%.6f;", out.m_SongTiming.m_fBeat0OffsetInSeconds ) );
	f.PutLine( ssprintf( "#SAMPLESTART:%.6f;", out.m_fMusicSampleStartSeconds ) );
	f.PutLine( ssprintf( "#SAMPLELENGTH:%.6f;", out.m_fMusicSampleLengthSeconds ) );

	f.Write( "#SELECTABLE:" );
	switch(out.m_SelectionDisplay)
	{
		default:
			ASSERT_M(0, "An invalid selectable value was found for this song!");
			[[fallthrough]];
		case Song::SHOW_ALWAYS:
			f.Write( "YES" );
			break;
#if 0
		case Song::SHOW_NONSTOP:
			f.Write( "NONSTOP" );
			break;
#endif
		case Song::SHOW_NEVER:
			f.Write( "NO" );
			break;
	}
	f.PutLine( ";" );

	switch( out.m_DisplayBPMType )
	{
		case DISPLAY_BPM_ACTUAL:
			// write nothing
			break;
		case DISPLAY_BPM_SPECIFIED:
			if( out.m_fSpecifiedBPMMin == out.m_fSpecifiedBPMMax )
				f.PutLine( ssprintf( "#DISPLAYBPM:%.6f;", out.m_fSpecifiedBPMMin ) );
			else
				f.PutLine( ssprintf( "#DISPLAYBPM:%.6f:%.6f;", out.m_fSpecifiedBPMMin, out.m_fSpecifiedBPMMax ) );
			break;
		case DISPLAY_BPM_RANDOM:
			f.PutLine( ssprintf( "#DISPLAYBPM:*;" ) );
			break;
		default:
			break;
	}

	WriteTimingTags( f, out.m_SongTiming, true );

	if( out.GetSpecifiedLastSecond() > 0 )
		f.PutLine( ssprintf("#LASTSECONDHINT:%.6f;", out.GetSpecifiedLastSecond()) );

	FOREACH_BackgroundLayer( b )
	{
		if( b==0 )
			f.Write( "#BGCHANGES:" );
		else if( out.GetBackgroundChanges(b).empty() )
			continue;	// skip
		else
			f.Write( ssprintf("#BGCHANGES%d:", b+1) );

		for (BackgroundChange const &bgc : out.GetBackgroundChanges(b))
			f.PutLine( bgc.ToString() +"," );

		/* If there's an animation plan at all, add a dummy "-nosongbg-" tag to
		 * indicate that this file doesn't want a song BG entry added at the end.
		 * See SSCLoader::TidyUpData. This tag will be removed on load. Add it
		 * at a very high beat, so it won't cause problems if loaded in older versions. */
		if( b==0 && !out.GetBackgroundChanges(b).empty() )
			f.PutLine( "99999=-nosongbg-=1.000=0=0=0 // don't automatically add -songbackground-" );
		f.PutLine( ";" );
	}

	if( out.GetForegroundChanges().size() )
	{
		f.Write( "#FGCHANGES:" );
		for (BackgroundChange const &bgc : out.GetForegroundChanges())
		{
			f.PutLine( bgc.ToString() +"," );
		}
		f.PutLine( ";" );
	}

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

	// attacks section
	//f.PutLine( ssprintf("#ATTACKS:%s;", out.GetAttackString().c_str()) );
	f.PutLine( "#ATTACKS:" );
	for(unsigned j = 0; j < out.m_Attacks.size(); j++)
	{
		const Attack &a = out.m_Attacks[j];
		f.Write( ssprintf( "  TIME=%.2f:LEN=%.2f:MODS=%s",
			a.fStartSecond, a.fSecsRemaining, a.sModifiers.c_str() ) );

		if( j+1 < out.m_Attacks.size() )
			f.Write( ":" );
	}
	f.Write( ";" );
	f.PutLine("");
}

/**
 * @brief Retrieve the individual batches of NoteData.
 * @param song the Song in question.
 * @param in the Steps in question.
 * @param bSavingCache a flag to see if we're saving certain cache data.
 * @return the NoteData in RString form. */
static RString GetSSCNoteData( const Song &song, const Steps &in, bool bSavingCache )
{
	std::vector<RString> lines;

	lines.push_back( "" );
	// Escape to prevent some clown from making a comment of "\r\n;"
	lines.push_back( ssprintf("//---------------%s - %s----------------",
		in.m_StepsTypeStr.c_str(), SmEscape(in.GetDescription()).c_str()) );
	lines.push_back( "#NOTEDATA:;" ); // our new separator.
	lines.push_back( ssprintf( "#CHARTNAME:%s;", SmEscape(in.GetChartName()).c_str()));
	lines.push_back( ssprintf( "#STEPSTYPE:%s;", in.m_StepsTypeStr.c_str() ) );
	lines.push_back( ssprintf( "#DESCRIPTION:%s;", SmEscape(in.GetDescription()).c_str() ) );
	lines.push_back( ssprintf( "#CHARTSTYLE:%s;", SmEscape(in.GetChartStyle()).c_str() ) );
	lines.push_back( ssprintf( "#DIFFICULTY:%s;", DifficultyToString(in.GetDifficulty()).c_str() ) );
	lines.push_back( ssprintf( "#METER:%d;", in.GetMeter() ) );

	const RString& music= in.GetMusicFile();
	if(!music.empty())
	{
		lines.push_back(ssprintf("#MUSIC:%s;", music.c_str()));
	}

	std::vector<RString> asRadarValues;
	FOREACH_PlayerNumber( pn )
	{
		const RadarValues &rv = in.GetRadarValues( pn );
		FOREACH_ENUM( RadarCategory, rc )
			asRadarValues.push_back( ssprintf("%.6f", rv[rc]) );
	}
	lines.push_back( ssprintf( "#RADARVALUES:%s;", join(",",asRadarValues).c_str() ) );

	lines.push_back( ssprintf( "#CREDIT:%s;", SmEscape(in.GetCredit()).c_str() ) );

	// If the Steps TimingData is not empty, then they have their own
	// timing.  Write out the corresponding tags.
	if( !in.m_Timing.empty() )
	{
		lines.push_back( ssprintf( "#OFFSET:%.6f;", in.m_Timing.m_fBeat0OffsetInSeconds ) );
		GetTimingTags( lines, in.m_Timing );
	}

	// todo: get this to output similar to course mods -aj
	if (song.GetAttackString() != in.GetAttackString())
		lines.push_back( ssprintf("#ATTACKS:%s;", in.GetAttackString().c_str()));

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
				lines.push_back( ssprintf( "#DISPLAYBPM:%.6f;", small ) );
			else
				lines.push_back( ssprintf( "#DISPLAYBPM:%.6f:%.6f;", small, big ) );
			break;
		}
		case DISPLAY_BPM_RANDOM:
			lines.push_back( ssprintf( "#DISPLAYBPM:*;" ) );
			break;
		default:
			break;
	}
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

bool NotesWriterSSC::Write( RString sPath, const Song &out, const std::vector<Steps*>& vpStepsToSave, bool bSavingCache )
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
		f.PutLine( ssprintf( "// cache tags:" ) );
		f.PutLine( ssprintf( "#FIRSTSECOND:%.6f;", out.GetFirstSecond() ) );
		f.PutLine( ssprintf( "#LASTSECOND:%.6f;", out.GetLastSecond() ) );
		f.PutLine( ssprintf( "#SONGFILENAME:%s;", out.m_sSongFileName.c_str() ) );
		f.PutLine( ssprintf( "#HASMUSIC:%i;", out.m_bHasMusic ) );
		f.PutLine( ssprintf( "#HASBANNER:%i;", out.m_bHasBanner ) );
		f.PutLine( ssprintf( "#MUSICLENGTH:%.6f;", out.m_fMusicLengthSeconds ) );
		f.PutLine( ssprintf( "// end cache tags" ) );
	}

	// Save specified Steps to this file
	for (Steps const *pSteps : vpStepsToSave)
	{
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
	std::vector<RString> asParts;
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
