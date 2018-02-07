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

static void write_tag( RageFile& f, RString const& format,
	RString const& value )
{
	if( !value.empty() )
		f.PutLine( ssprintf(format, SmEscape(value).c_str()) );
}

static void write_tag( vector<RString> &v, RString const& format, const TimingData &so_timing, const TimingData &st_timing, TimingSegmentType tst )
{
	RString const value1 = join(",\r", so_timing.ToVectorString(tst, 3)).c_str();
	RString const value2 = join(",\r", st_timing.ToVectorString(tst, 3)).c_str();

	if( !value2.empty() )
	{
		if( tst == SEGMENT_BPM || value2 != value1 )
			v.push_back( ssprintf(format, SmEscape(value2).c_str()) );
	}
}

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

static RString TimingVectorValueToString( const TimingData &timing, TimingSegmentType tst )
{
	return join(",\r", timing.ToVectorString(tst, 3)).c_str();
}

static void GetTimingTags( vector<RString> &lines, const TimingData &so_timing, const TimingData &st_timing, bool bIsSong = false )
{
	write_tag( lines, "#BPMS:%s;", so_timing, st_timing, SEGMENT_BPM );
	write_tag( lines, "#STOPS:%s;", so_timing, st_timing, SEGMENT_STOP );
	write_tag( lines, "#DELAYS:%s;", so_timing, st_timing, SEGMENT_DELAY );
	write_tag( lines, "#WARPS:%s;", so_timing, st_timing, SEGMENT_WARP );
	write_tag( lines, "#TIMESIGNATURES:%s;", so_timing, st_timing, SEGMENT_TIME_SIG );
	write_tag( lines, "#TICKCOUNTS:%s;", so_timing, st_timing, SEGMENT_TICKCOUNT );
	write_tag( lines, "#COMBOS:%s;", so_timing, st_timing, SEGMENT_COMBO );
	write_tag( lines, "#SPEEDS:%s;", so_timing, st_timing, SEGMENT_SPEED );
	write_tag( lines, "#SCROLLS:%s;", so_timing, st_timing, SEGMENT_SCROLL );
	write_tag( lines, "#FAKES:%s;", so_timing, st_timing, SEGMENT_FAKE );
	write_tag( lines, "#LABELS:%s;", so_timing, st_timing, SEGMENT_LABEL );
}

static void WriteTimingTags( RageFile &f, const TimingData &timing, bool bIsSong = false )
{
	write_tag( f, "#BPMS:%s;", TimingVectorValueToString(timing, SEGMENT_BPM) );
	write_tag( f, "#STOPS:%s;", TimingVectorValueToString(timing, SEGMENT_STOP) );
	write_tag( f, "#DELAYS:%s;", TimingVectorValueToString(timing, SEGMENT_DELAY) );
	write_tag( f, "#WARPS:%s;", TimingVectorValueToString(timing, SEGMENT_WARP) );

	if( TimingVectorValueToString(timing, SEGMENT_TIME_SIG) != "0=4=4" )
		write_tag( f, "#TIMESIGNATURES:%s;", TimingVectorValueToString(timing, SEGMENT_TIME_SIG) );

	if( TimingVectorValueToString(timing, SEGMENT_TICKCOUNT) != "0=4" )
		write_tag( f, "#TICKCOUNTS:%s;", TimingVectorValueToString(timing, SEGMENT_TICKCOUNT) );

	if( TimingVectorValueToString(timing, SEGMENT_COMBO) != "0=1" )
		write_tag( f, "#COMBOS:%s;", TimingVectorValueToString(timing, SEGMENT_COMBO) );

	if( TimingVectorValueToString(timing, SEGMENT_SPEED) != "0=1=0=0" )
		write_tag( f, "#SPEEDS:%s;", TimingVectorValueToString(timing, SEGMENT_SPEED) );

	if( TimingVectorValueToString(timing, SEGMENT_SCROLL) != "0=1" )
		write_tag( f, "#SCROLLS:%s;", TimingVectorValueToString(timing, SEGMENT_SCROLL) );

	write_tag( f, "#FAKES:%s;", TimingVectorValueToString(timing, SEGMENT_FAKE) );

	if( TimingVectorValueToString(timing, SEGMENT_LABEL) != "0=Song Start" )
		write_tag( f, "#LABELS:%s;", TimingVectorValueToString(timing, SEGMENT_LABEL) );
}

/**
 * @brief Write out the common tags for .SSC files.
 * @param f the file in question.
 * @param out the Song in question. */
static void WriteGlobalTags( RageFile &f, const Song &out )
{
	write_tag( f, "#VERSION:%s;", "0.84" );
	write_tag( f, "#TITLE:%s;", out.m_sMainTitle );
	write_tag( f, "#SUBTITLE:%s;",out.m_sSubTitle );
	write_tag( f, "#ARTIST:%s;", out.m_sArtist );
	write_tag( f, "#TITLETRANSLIT:%s;", out.m_sMainTitleTranslit );
	write_tag( f, "#SUBTITLETRANSLIT:%s;", out.m_sSubTitleTranslit );
	write_tag( f, "#ARTISTTRANSLIT:%s;", out.m_sArtistTranslit );
	write_tag( f, "#GENRE:%s;", out.m_sGenre );
	write_tag( f, "#ORIGIN:%s;", out.m_sOrigin );
	write_tag( f, "#CREDIT:%s;", out.m_sCredit );
	write_tag( f, "#BANNER:%s;", out.m_sBannerFile );
	write_tag( f, "#BACKGROUND:%s;", out.m_sBackgroundFile );
	write_tag( f, "#PREVIEWVID:%s;", out.m_sPreviewVidFile );
	write_tag( f, "#JACKET:%s;", out.m_sJacketFile );
	write_tag( f, "#CDIMAGE:%s;", out.m_sCDFile );
	write_tag( f, "#DISCIMAGE:%s;", out.m_sDiscFile );
	write_tag( f, "#LYRICSPATH:%s;", out.m_sLyricsFile );
	write_tag( f, "#CDTITLE:%s;", out.m_sCDTitleFile );
	write_tag( f, "#MUSIC:%s;", out.m_sMusicFile );
	write_tag( f, "#PREVIEW:%s;", out.m_PreviewFile );

	{
		vector<RString> vs = out.GetInstrumentTracksToVectorString();
		if( !vs.empty() )
		{
			RString s = join( ",", vs );
			f.PutLine( "#INSTRUMENTTRACK:" + s + ";\n" );
		}
	}

	if( out.m_SongTiming.m_fBeat0OffsetInSeconds != 0 )
		f.PutLine( ssprintf( "#OFFSET:%s;", FormatDouble("%.3f", out.m_SongTiming.m_fBeat0OffsetInSeconds).c_str() ) );

	f.PutLine( ssprintf( "#SAMPLESTART:%s;", FormatDouble("%.3f", out.m_fMusicSampleStartSeconds).c_str() ) );
	f.PutLine( ssprintf( "#SAMPLELENGTH:%s;", FormatDouble("%.3f", out.m_fMusicSampleLengthSeconds).c_str() ) );

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
				f.PutLine( ssprintf( "#DISPLAYBPM:%s:%s;", FormatDouble("%.3f", out.m_fSpecifiedBPMMin).c_str(), FormatDouble("%.3f", out.m_fSpecifiedBPMMax).c_str() ) );
			break;
		case DISPLAY_BPM_RANDOM:
			f.PutLine( ssprintf( "#DISPLAYBPM:*;" ) );
			break;
		default:
			break;
	}

	WriteTimingTags( f, out.m_SongTiming, true );

	if( out.GetSpecifiedLastSecond() > 0 )
		f.PutLine( ssprintf("#LASTSECONDHINT:%s;", FormatDouble("%.3f", out.GetSpecifiedLastSecond()).c_str()) );

	FOREACH_BackgroundLayer( b )
	{
		if( out.GetBackgroundChanges(b).size() > 1 )
		{
			if( b==0 )
				f.Write( "#BGCHANGES:" );
			else
				f.Write( ssprintf("#BGCHANGES%d:", b+1) );

			FOREACH_CONST( BackgroundChange, out.GetBackgroundChanges(b), bgc )
			{
				if( bgc != (out.GetBackgroundChanges(b)).end()-1 )
					f.PutLine( (*bgc).ToString() + "," );
				else
					f.PutLine( (*bgc).ToString() + ";" );
			}
		}
		else if( !out.GetBackgroundChanges(b).empty() )
		{
			FOREACH_CONST( BackgroundChange, out.GetBackgroundChanges(b), bgc )
			{
				if( b==0 && (*bgc).m_fStartBeat != 0 )
					f.PutLine( ssprintf("#BGCHANGES:%s;", (*bgc).ToString()) );
				else if( b > 0 )
					f.PutLine( ssprintf("#BGCHANGES%d:%s;", b+1, (*bgc).ToString()) );

				break;
			}
		}
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

	if( out.m_vsKeysoundFile.size() )
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

	// attacks section
	vector<RString> getAttacksString;

	if( !out.m_Attacks.empty() )
	{		
		for(unsigned j = 0; j < out.m_Attacks.size(); j++)
		{
			const Attack &a = out.m_Attacks[j];
			getAttacksString.push_back( ssprintf("TIME=%s:LEN=%s:MODS=%s",
				FormatDouble("%.3f", a.fStartSecond).c_str(), FormatDouble("%.3f", a.fSecsRemaining).c_str(), a.sModifiers.c_str()) );
		}

		f.PutLine( ssprintf("#ATTACKS:%s;", join(": ", getAttacksString)) );
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
	push_back_tag( lines, "#CHARTNAME:%s;", in.GetChartName() );
	lines.push_back( ssprintf( "#STEPSTYPE:%s;", in.m_StepsTypeStr.c_str() ) );
	push_back_tag( lines, "#DESCRIPTION:%s;", in.GetDescription() );
	push_back_tag( lines, "#CHARTSTYLE:%s;", in.GetChartStyle() );
	lines.push_back( ssprintf( "#DIFFICULTY:%s;", DifficultyToString(in.GetDifficulty()).c_str() ) );
	lines.push_back( ssprintf( "#METER:%d;", in.GetMeter() ) );
	push_back_tag( lines, "#MUSIC:%s;", in.GetMusicFile() );
	
	if( bSavingCache )
	{
		vector<RString> asRadarValues;
		FOREACH_PlayerNumber( pn )
		{
			const RadarValues &rv = in.GetRadarValues( pn );
			FOREACH_ENUM( RadarCategory, rc )
				asRadarValues.push_back( ssprintf("%s", FormatDouble("%.3f", rv[rc]).c_str()) );
		}

		lines.push_back( ssprintf( "#RADARVALUES:%s;", join(",",asRadarValues).c_str() ) );
	}

	push_back_tag( lines, "#CREDIT:%s;", in.GetCredit() );

	// If the Steps TimingData is not empty, then they have their own
	// timing.  Write out the corresponding tags.
	if( !in.m_Timing.empty() )
	{
		//lines.push_back( ssprintf( "#OFFSET:%s;", FormatDouble("%.3f", in.m_Timing.m_fBeat0OffsetInSeconds).c_str() ) );
		GetTimingTags( lines, song.m_SongTiming, in.m_Timing );

			// todo: get this to output similar to course mods -aj
		if( song.GetAttackString() != in.GetAttackString() )
		{
			if( !in.m_Attacks.empty() )
			{	
				vector<RString> getAttacksString;

				for(unsigned j = 0; j < in.m_Attacks.size(); j++)
				{
					const Attack &a = in.m_Attacks[j];
					getAttacksString.push_back( ssprintf("TIME=%s:LEN=%s:MODS=%s",
						FormatDouble("%.3f", a.fStartSecond).c_str(), FormatDouble("%.3f", a.fSecsRemaining).c_str(), a.sModifiers.c_str()) );
				}

				lines.push_back( ssprintf("#ATTACKS:%s;", join(": ", getAttacksString)) );
			}
		}

		if( (in.GetMinBPM() != song.m_fSpecifiedBPMMin) && (in.GetMaxBPM() != song.m_fSpecifiedBPMMax) )
		{
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
						lines.push_back( ssprintf( "#DISPLAYBPM:%s;", FormatDouble(small).c_str() ) );
					else
						lines.push_back( ssprintf( "#DISPLAYBPM:%s:%s;", FormatDouble(small).c_str(), FormatDouble(big).c_str() ) );
					break;
				}
				case DISPLAY_BPM_RANDOM:
					lines.push_back( ssprintf( "#DISPLAYBPM:*;" ) );
					break;
				default:
					break;
			}
		}
	}
	
	if( bSavingCache )
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
		f.PutLine( "" );
		f.PutLine( ssprintf( "// cache tags:" ) );
		f.PutLine( ssprintf( "#FIRSTSECOND:%s;", FormatDouble("%.3f", out.GetFirstSecond()).c_str() ) );
		f.PutLine( ssprintf( "#LASTSECOND:%s;", FormatDouble("%.3f", out.GetLastSecond()).c_str() ) );
		f.PutLine( ssprintf( "#SONGFILENAME:%s;", out.m_sSongFileName.c_str() ) );
		f.PutLine( ssprintf( "#HASMUSIC:%i;", out.m_bHasMusic ) );
		f.PutLine( ssprintf( "#HASBANNER:%i;", out.m_bHasBanner ) );
		f.PutLine( ssprintf( "#MUSICLENGTH:%s;", FormatDouble("%.3f", out.m_fMusicLengthSeconds).c_str() ) );
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
