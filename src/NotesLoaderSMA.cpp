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

#include <vector>


void SMALoader::ProcessMultipliers( TimingData &out, const int iRowsPerBeat, const RString sParam )
{
	std::vector<RString> arrayMultiplierExpressions;
	split( sParam, ",", arrayMultiplierExpressions );

	for( unsigned f=0; f<arrayMultiplierExpressions.size(); f++ )
	{
		std::vector<RString> arrayMultiplierValues;
		split( arrayMultiplierExpressions[f], "=", arrayMultiplierValues );
		unsigned size = arrayMultiplierValues.size();
		if( size < 2 )
		{
			LOG->UserLog("Song file",
				     this->GetSongTitle(),
				     "has an invalid #MULTIPLIER value \"%s\" (must have at least one '='), ignored.",
				     arrayMultiplierExpressions[f].c_str() );
			continue;
		}
		const float fComboBeat = RowToBeat( arrayMultiplierValues[0], iRowsPerBeat );
		const int iCombos = StringToInt( arrayMultiplierValues[1] ); // always true.
		// hoping I'm right here: SMA files can use 6 values after the row/beat.
		const int iMisses = (size == 2 || size == 4 ?
							 iCombos :
							 StringToInt(arrayMultiplierValues[2]));
		out.AddSegment( ComboSegment(BeatToNoteRow(fComboBeat), iCombos, iMisses) );
	}
}

void SMALoader::ProcessBeatsPerMeasure( TimingData &out, const RString sParam )
{
	std::vector<RString> vs1;
	std::vector<TimeSignatureSegment> segments;
	split( sParam, ",", vs1 );

	for (RString const &s1 : vs1)
	{
		std::vector<RString> vs2;
		split( s1, "=", vs2 );

		if( vs2.size() < 2 )
		{
			LOG->UserLog("Song file",
				     this->GetSongTitle(),
				     "has an invalid beats per measure change with %i values.",
				     static_cast<int>(vs2.size()) );
			continue;
		}
		const float fBeat = StringToFloat( vs2[0] );
		const int iNumerator = StringToInt( vs2[1] );

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
		segments.push_back( TimeSignatureSegment(BeatToNoteRow(fBeat), iNumerator) );
	}

	// If there are any time signatures defined, but there isn't one
	// for the very first beat of the song, then add one.
	// Without it, calls to functions like TimingData::NoteRowToMeasureAndBeat
	// can fail for charts that are otherwise valid.
	if ( segments.size() > 0 && segments[0].GetRow() > 0 )
	{
		out.AddSegment( TimeSignatureSegment(0, 4, 4) );
	}

	for( TimeSignatureSegment segment: segments )
	{
		out.AddSegment( segment );
	}
}

void SMALoader::ProcessSpeeds( TimingData &out, const RString line, const int rowsPerBeat )
{
	std::vector<RString> vs1;
	split( line, ",", vs1 );

	for (std::vector<RString>::const_iterator s1 = vs1.begin(); s1 != vs1.end(); ++s1)
	{
		std::vector<RString> vs2;
		vs2.clear(); // trying something.
		RString loopTmp = *s1;
		Trim( loopTmp );
		split( loopTmp, "=", vs2 );

		if( vs2.size() == 2 ) // First one always seems to have 2.
		{
			// Aldo_MX: 4 is the default value in SMA, although SM5 requires 0 for the first segment :/
			vs2.push_back(s1 == vs1.begin() ? "0" : "4");
		}

		if( vs2.size() < 3 )
		{
			LOG->UserLog("Song file",
				     this->GetSongTitle(),
				     "has an speed change with %i values.",
				     static_cast<int>(vs2.size()) );
			continue;
		}

		const float fBeat = RowToBeat( vs2[0], rowsPerBeat );

		RString backup = vs2[2];
		Trim(vs2[2], "s");
		Trim(vs2[2], "S");

		const float fRatio = StringToFloat( vs2[1] );
		const float fDelay = StringToFloat( vs2[2] );

		SpeedSegment::BaseUnit unit = ((backup != vs2[2]) ?
			SpeedSegment::UNIT_SECONDS : SpeedSegment::UNIT_BEATS);


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

bool SMALoader::LoadFromSimfile( const RString &sPath, Song &out, bool bFromCache )
{
	LOG->Trace( "Song::LoadFromSMAFile(%s)", sPath.c_str() );

	MsdFile msd;
	if( !msd.ReadFile( sPath, true ) )  // unescape
	{
		LOG->UserLog( "Song file", sPath, "couldn't be opened: %s", msd.GetError().c_str() );
		return false;
	}

	out.m_SongTiming.m_sFile = sPath; // songs still have their fallback timing.
	out.m_sSongFileName = sPath;

	Steps* pNewNotes = nullptr;
	int iRowsPerBeat = -1; // Start with an invalid value: needed for checking.
	std::vector<std::pair<float, float>> vBPMChanges, vStops;

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
		{
			out.m_sMainTitle = sParams[1];
			this->SetSongTitle(sParams[1]);
		}

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

		else if( sValueName=="PREVIEW" )
			out.m_sPreviewVidFile = sParams[1];

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
		{
			// can't identify at this position: ignore.
		}
		else if( sValueName=="MUSICBYTES" )
			; /* ignore */

		// Cache tags: ignore.
		else if (sValueName=="FIRSTBEAT" || sValueName=="LASTBEAT" ||
			 sValueName=="SONGFILENAME" || sValueName=="HASMUSIC" ||
			 sValueName=="HASBANNER" )
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
				std::vector<RString> arrayBeatChangeExpressions;
				split( sParams[1], ",", arrayBeatChangeExpressions );

				std::vector<RString> arrayBeatChangeValues;
				split( arrayBeatChangeExpressions[0], "=", arrayBeatChangeValues );
				iRowsPerBeat = StringToInt(arrayBeatChangeValues[1]);
			}
			else
			{
				// This should generally return song timing
				TimingData &timing = ( pNewNotes ? pNewNotes->m_Timing : out.m_SongTiming);
				ProcessBPMsAndStops(timing, vBPMChanges, vStops);

			}
		}

		else if( sValueName=="BEATSPERMEASURE" )
		{
			TimingData &timing = ( pNewNotes ? pNewNotes->m_Timing : out.m_SongTiming);
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
				LOG->UserLog("Song file",
					     sPath,
					     "has an unknown #SELECTABLE value, \"%s\"; ignored.",
					     sParams[1].c_str() );
		}

		else if( sValueName.Left(strlen("BGCHANGES"))=="BGCHANGES" || sValueName=="ANIMATIONS" )
		{
			SMLoader::ProcessBGChanges( out, sValueName, sPath, sParams[1]);
		}

		else if( sValueName=="FGCHANGES" )
		{
			std::vector<std::vector<RString> > aFGChanges;
			ParseBGChangesString(sParams[1], aFGChanges, out.GetSongDir());

			for (const auto &b : aFGChanges)
			{
				BackgroundChange change;
				if (LoadFromBGChangesVector(change, b))
					out.AddForegroundChange(change);
			}
		}

		else if( sValueName=="OFFSET" )
		{
			TimingData &timing = ( pNewNotes ? pNewNotes->m_Timing : out.m_SongTiming);
			timing.m_fBeat0OffsetInSeconds = StringToFloat( sParams[1] );
		}

		else if( sValueName=="BPMS" )
		{
			vBPMChanges.clear();
			ParseBPMs( vBPMChanges, sParams[1], iRowsPerBeat );
		}

		else if( sValueName=="STOPS" || sValueName=="FREEZES" )
		{
			vStops.clear();
			ParseStops( vStops, sParams[1], iRowsPerBeat );
		}

		else if( sValueName=="DELAYS" )
		{
			TimingData &timing = ( pNewNotes ? pNewNotes->m_Timing : out.m_SongTiming);
			ProcessDelays( timing, sParams[1], iRowsPerBeat );
		}

		else if( sValueName=="TICKCOUNT" )
		{
			TimingData &timing = ( pNewNotes ? pNewNotes->m_Timing : out.m_SongTiming);
			ProcessTickcounts( timing, sParams[1], iRowsPerBeat );
		}

		else if( sValueName=="SPEED" )
		{
			TimingData &timing = ( pNewNotes ? pNewNotes->m_Timing : out.m_SongTiming);
			RString tmp = sParams[1];
			Trim( tmp );
			ProcessSpeeds( timing, tmp, iRowsPerBeat );
		}

		else if( sValueName=="MULTIPLIER" )
		{
			TimingData &timing = ( pNewNotes ? pNewNotes->m_Timing : out.m_SongTiming);
			ProcessMultipliers( timing, iRowsPerBeat, sParams[1] );
		}

		else if( sValueName=="FAKES" )
		{
			TimingData &timing = ( pNewNotes ? pNewNotes->m_Timing : out.m_SongTiming);
			ProcessFakes( timing, sParams[1], iRowsPerBeat );
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
			ProcessAttackString(out.m_sAttackString, sParams);
			ProcessAttacks(out.m_Attacks, sParams);
		}

		else if( sValueName=="NOTES" || sValueName=="NOTES2" )
		{
			if( iNumParams < 7 )
			{
				LOG->UserLog("Song file",
					     sPath,
					     "has %d fields in a #NOTES tag, but should have at least 7.",
					     iNumParams );
				continue;
			}

			pNewNotes = new Steps(&out);

			LoadFromTokens(
					 sParams[1],
					 sParams[2],
					 sParams[3],
					 sParams[4],
					 sParams[5],
					 sParams[6],
					 *pNewNotes );
			pNewNotes->SetFilename(sPath);
			out.AddSteps( pNewNotes );

			// Handle timing changes and convert negative bpms/stops
			TimingData &timing = ( pNewNotes ? pNewNotes->m_Timing : out.m_SongTiming);
			ProcessBPMsAndStops(timing, vBPMChanges, vStops);
		}
		else if( sValueName=="TIMESIGNATURES" || sValueName=="LEADTRACK"  )
			;
		else
			LOG->UserLog("Song file",
				     sPath,
				     "has an unexpected value named \"%s\".",
				     sValueName.c_str() );
	}
	TidyUpData(out, false);
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
