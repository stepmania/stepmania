#include "global.h"
#include "NotesLoaderKSF.h"
#include "RageUtil_CharConversions.h"
#include "MsdFile.h"
#include "RageLog.h"
#include "RageUtil.h"
#include "NoteData.h"
#include "NoteTypes.h"
#include "Song.h"
#include "Steps.h"

static void HandleBunki( TimingData &timing, const float fEarlyBPM, 
			const float fCurBPM, const float fGap, 
			const float fPos )
{
	const float BeatsPerSecond = fEarlyBPM / 60.0f;
	const float beat = (fPos + fGap) * BeatsPerSecond;
	LOG->Trace( "BPM %f, BPS %f, BPMPos %f, beat %f",
		   fEarlyBPM, BeatsPerSecond, fPos, beat );
	timing.AddSegment( BPMSegment(BeatToNoteRow(beat), fCurBPM) );
}

static bool LoadFromKSFFile( const RString &sPath, Steps &out, Song &song, bool bKIUCompliant )
{
	LOG->Trace( "Steps::LoadFromKSFFile( '%s' )", sPath.c_str() );

	MsdFile msd;
	if( !msd.ReadFile( sPath, false ) )  // don't unescape
	{
		LOG->UserLog( "Song file", sPath, "couldn't be opened: %s", msd.GetError().c_str() );
		return false;
	}

	// this is the value we read for TICKCOUNT
	int iTickCount = -1;
	// used to adapt weird tickcounts
	//float fScrollRatio = 1.0f; -- uncomment when ready to use.
	vector<RString> vNoteRows;

	// According to Aldo_MX, there is a default BPM and it's 60. -aj
	bool bDoublesChart = false;
	
	TimingData stepsTiming;
	float SMGap1 = 0, SMGap2 = 0, BPM1 = -1, BPMPos2 = -1, BPM2 = -1, BPMPos3 = -1, BPM3 = -1;

	for( unsigned i=0; i<msd.GetNumValues(); i++ )
	{
		const MsdFile::value_t &sParams = msd.GetValue( i );
		RString sValueName = sParams[0];
		sValueName.MakeUpper();

		/* handle the data...well, not this data: not related to steps.
		 * Skips INTRO, MUSICINTRO, TITLEFILE, DISCFILE, SONGFILE. */
		if (sValueName=="TITLE" || EndsWith(sValueName, "INTRO")
		    || EndsWith(sValueName, "FILE") )
		{

		}
		else if( sValueName=="BPM" )
		{
			BPM1 = StringToFloat(sParams[1]);
			stepsTiming.AddSegment( BPMSegment(0, BPM1) );
		}
		else if( sValueName=="BPM2" )
		{
			if (bKIUCompliant)
			{
				BPM2 = StringToFloat( sParams[1] );
			}
			else
			{
				// LOG an error.
			}
		}
		else if( sValueName=="BPM3" )
		{
			if (bKIUCompliant)
			{
				BPM3 = StringToFloat( sParams[1] );
			}
			else
			{
				// LOG an error.
			}
		}
		else if( sValueName=="BUNKI" )
		{
			if (bKIUCompliant)
			{
				BPMPos2 = StringToFloat( sParams[1] ) / 100.0f;
			}
			else
			{
				// LOG an error.
			}
		}
		else if( sValueName=="BUNKI2" )
		{
			if (bKIUCompliant)
			{
				BPMPos3 = StringToFloat( sParams[1] ) / 100.0f;
			}
			else
			{
				// LOG an error.
			}
		}
		else if( sValueName=="STARTTIME" )
		{
			SMGap1 = -StringToFloat( sParams[1] )/100;
			stepsTiming.m_fBeat0OffsetInSeconds = SMGap1;
		}
		// This is currently required for more accurate KIU BPM changes.  
		else if( sValueName=="STARTTIME2" )
		{
			if (bKIUCompliant)
			{
				SMGap2 = -StringToFloat( sParams[1] )/100;
			}
			else
			{
				// LOG an error.
			}
		}
		else if ( sValueName=="STARTTIME3" )
		{
			// STARTTIME3 only ensures this is a KIU compliant simfile.
			bKIUCompliant = true;
		}
		
		else if( sValueName=="TICKCOUNT" )
		{
			iTickCount = StringToInt( sParams[1] );
			if( iTickCount <= 0 )
			{
				LOG->UserLog( "Song file", sPath, "has an invalid tick count: %d.", iTickCount );
				return false;
			}
			stepsTiming.AddSegment( TickcountSegment(0, iTickCount));
		}
		
		else if( sValueName=="DIFFICULTY" )
		{
			out.SetMeter( max(StringToInt(sParams[1]), 1) );
		}
		// new cases from Aldo_MX's fork:
		else if( sValueName=="PLAYER" )
		{
			RString sPlayer = sParams[1];
			sPlayer.MakeLower();
			if( sPlayer.find( "double" ) != string::npos )
				bDoublesChart = true;
		}
		// This should always be last.
		else if( sValueName=="STEP" )
		{
			RString theSteps = sParams[1];
			TrimLeft( theSteps );
			split( theSteps, "\n", vNoteRows, true );
		}
	}

	if( iTickCount == -1 )
	{
		iTickCount = 4;
		LOG->UserLog( "Song file", sPath, "doesn't have a TICKCOUNT. Defaulting to %i.", iTickCount );
	}
	
	// Prepare BPM stuff already if the file uses KSF syntax.
	if( bKIUCompliant )
	{
		if( BPM2 > 0 && BPMPos2 > 0 )
		{
			HandleBunki( stepsTiming, BPM1, BPM2, SMGap1, BPMPos2 );
		}
		
		if( BPM3 > 0 && BPMPos3 > 0 )
		{
			HandleBunki( stepsTiming, BPM2, BPM3, SMGap2, BPMPos3 );
		}
	}

	NoteData notedata;	// read it into here

	{
		RString sDir, sFName, sExt;
		splitpath( sPath, sDir, sFName, sExt );
		sFName.MakeLower();

		out.SetDescription(sFName);
		// Check another before anything else... is this okay? -DaisuMaster
		if( sFName.find("another") != string::npos )
		{
			out.SetDifficulty( Difficulty_Edit );
			if( !out.GetMeter() ) out.SetMeter( 25 );
		}
		else if(sFName.find("wild") != string::npos || 
			sFName.find("wd") != string::npos || 
			sFName.find("crazy+") != string::npos || 
			sFName.find("cz+") != string::npos || 
			sFName.find("hardcore") != string::npos )
		{
			out.SetDifficulty( Difficulty_Challenge );
			if( !out.GetMeter() ) out.SetMeter( 20 );
		}
		else if(sFName.find("crazy") != string::npos || 
			sFName.find("cz") != string::npos || 
			sFName.find("nightmare") != string::npos || 
			sFName.find("nm") != string::npos || 
			sFName.find("crazydouble") != string::npos )
		{
			out.SetDifficulty( Difficulty_Hard );
			if( !out.GetMeter() ) out.SetMeter( 14 ); // Set the meters to the Pump scale, not DDR.
		}
		else if(sFName.find("hard") != string::npos || 
			sFName.find("hd") != string::npos || 
			sFName.find("freestyle") != string::npos || 
			sFName.find("fs") != string::npos || 
			sFName.find("double") != string::npos )
		{
			out.SetDifficulty( Difficulty_Medium );
			if( !out.GetMeter() ) out.SetMeter( 8 );
		}
		else if(sFName.find("easy") != string::npos || 
			sFName.find("ez") != string::npos || 
			sFName.find("normal") != string::npos )
		{
			// I wonder if I should leave easy fall into the Beginner difficulty... -DaisuMaster
			out.SetDifficulty( Difficulty_Easy );
			if( !out.GetMeter() ) out.SetMeter( 4 );
		}
		else if(sFName.find("beginner") != string::npos || 
			sFName.find("practice") != string::npos || sFName.find("pr") != string::npos  )
		{
			out.SetDifficulty( Difficulty_Beginner );
			if( !out.GetMeter() ) out.SetMeter( 4 );
		}
		else
		{
			out.SetDifficulty( Difficulty_Hard );
			if( !out.GetMeter() ) out.SetMeter( 10 );
		}

		out.m_StepsType = StepsType_pump_single;

		// Check for "halfdouble" before "double".
		if(sFName.find("halfdouble") != string::npos || 
		   sFName.find("half-double") != string::npos || 
		   sFName.find("h_double") != string::npos || 
		   sFName.find("hdb") != string::npos )
			out.m_StepsType = StepsType_pump_halfdouble;
		// Handle bDoublesChart from above as well. -aj
		else if(sFName.find("double") != string::npos || 
			sFName.find("nightmare") != string::npos || 
			sFName.find("freestyle") != string::npos || 
			sFName.find("db") != string::npos || 
			sFName.find("nm") != string::npos || 
			sFName.find("fs") != string::npos || bDoublesChart )
			out.m_StepsType = StepsType_pump_double;
		else if( sFName.find("_1") != string::npos )
			out.m_StepsType = StepsType_pump_single;
		else if( sFName.find("_2") != string::npos )
			out.m_StepsType = StepsType_pump_couple;
	}

	switch( out.m_StepsType )
	{
	case StepsType_pump_single: notedata.SetNumTracks( 5 ); break;
	case StepsType_pump_couple: notedata.SetNumTracks( 10 ); break;
	case StepsType_pump_double: notedata.SetNumTracks( 10 ); break;
	case StepsType_pump_routine: notedata.SetNumTracks( 10 ); break; // future files may have this?
	case StepsType_pump_halfdouble: notedata.SetNumTracks( 6 ); break;
	default: FAIL_M( ssprintf("%i", out.m_StepsType) );
	}

	int t = 0;
	int iHoldStartRow[13];
	for( t=0; t<13; t++ )
		iHoldStartRow[t] = -1;

	bool bTickChangeNeeded = false;
	int newTick = -1;
	float fCurBeat = 0.0f;
	float prevBeat = 0.0f; // Used for hold tails.

	for( unsigned r=0; r<vNoteRows.size(); r++ )
	{
		RString& sRowString = vNoteRows[r];
		StripCrnl( sRowString );

		if( sRowString == "" )
			continue;	// skip

		// All 2s indicates the end of the song.
		else if( sRowString == "2222222222222" )
		{
			// Finish any holds that didn't get...well, finished.
			for( t=0; t < notedata.GetNumTracks(); t++ )
			{
				if( iHoldStartRow[t] != -1 )	// this ends the hold
				{
					if( iHoldStartRow[t] == BeatToNoteRow(prevBeat) )
						notedata.SetTapNote( t, iHoldStartRow[t], TAP_ORIGINAL_TAP );
					else
						notedata.AddHoldNote(t,
								     iHoldStartRow[t],
								     BeatToNoteRow(prevBeat),
								     TAP_ORIGINAL_HOLD_HEAD );
				}
			}
			/* have this row be the last moment in the song, unless
			 * a future step ends later. */
			//float curTime = stepsTiming.GetElapsedTimeFromBeat(fCurBeat);
			//if (curTime > song.GetSpecifiedLastSecond())
			//{
			//	song.SetSpecifiedLastSecond(curTime);
			//}

			song.SetSpecifiedLastSecond( song.GetSpecifiedLastSecond() + 4 );

			break;
		}

		else if( BeginsWith(sRowString, "|") )
		{
			/*
			if (bKIUCompliant)
			{
				// Log an error, ignore the line.
				continue;
			}
			*/
			// gotta do something tricky here: if the bpm is below one then a couple of calculations
			// for scrollsegments will be made, example, bpm 0.2, tick 4000, the scrollsegment will
			// be 0. if the tickcount is non a stepmania standard then it will be adapted, a scroll
			// segment will then be added based on approximations. -DaisuMaster
			// eh better do it considering the tickcount (high tickcounts)

			// I'm making some experiments, please spare me...
			//continue;

			RString temp = sRowString.substr(2,sRowString.size()-3);
			float numTemp = StringToFloat(temp);
			if (BeginsWith(sRowString, "|T")) 
			{
				// duh
				iTickCount = static_cast<int>(numTemp);
				// I have been owned by the man -DaisuMaster
				stepsTiming.SetTickcountAtBeat( fCurBeat, clamp(iTickCount, 0, ROWS_PER_BEAT) );
			}
			else if (BeginsWith(sRowString, "|B")) 
			{
				// BPM
				stepsTiming.SetBPMAtBeat( fCurBeat, numTemp );
			}
			else if (BeginsWith(sRowString, "|E"))
			{
				// DelayBeat
				float fCurDelay = 60 / stepsTiming.GetBPMAtBeat(fCurBeat) * numTemp / iTickCount;
				fCurDelay += stepsTiming.GetDelayAtRow(BeatToNoteRow(fCurBeat) );
				stepsTiming.SetDelayAtBeat( fCurBeat, fCurDelay );
			}
			else if (BeginsWith(sRowString, "|D"))
			{
				// Delays
				float fCurDelay = stepsTiming.GetStopAtRow(BeatToNoteRow(fCurBeat) );
				fCurDelay += numTemp / 1000;
				stepsTiming.SetDelayAtBeat( fCurBeat, fCurDelay );
			}
			else if (BeginsWith(sRowString, "|M") || BeginsWith(sRowString, "|C"))
			{
				// multipliers/combo
				ComboSegment seg( BeatToNoteRow(fCurBeat), int(numTemp) );
				stepsTiming.AddSegment( seg );
			}
			else if (BeginsWith(sRowString, "|S"))
			{
				// speed segments
			}
			else if (BeginsWith(sRowString, "|F"))
			{
				// fakes
			}
			else if (BeginsWith(sRowString, "|X"))
			{
				// scroll segments
				ScrollSegment seg = ScrollSegment( BeatToNoteRow(fCurBeat), numTemp );
				stepsTiming.AddSegment( seg );
				//return true;
			}

			continue;
		}

		// Half-doubles is offset; "0011111100000".
		if( out.m_StepsType == StepsType_pump_halfdouble )
			sRowString.erase( 0, 2 );

		// Update TICKCOUNT for Direct Move files.
		if( bTickChangeNeeded )
		{
			iTickCount = newTick;
			bTickChangeNeeded = false;
		}

		for( t=0; t < notedata.GetNumTracks(); t++ )
		{
			if( sRowString[t] == '4' )
			{
				/* Remember when each hold starts; ignore the middle. */
				if( iHoldStartRow[t] == -1 )
					iHoldStartRow[t] = BeatToNoteRow(fCurBeat);
				continue;
			}

			if( iHoldStartRow[t] != -1 )	// this ends the hold
			{
				int iEndRow = BeatToNoteRow(prevBeat);
				if( iHoldStartRow[t] == iEndRow )
					notedata.SetTapNote( t, iHoldStartRow[t], TAP_ORIGINAL_TAP );
				else
				{
					//notedata.AddHoldNote( t, iHoldStartRow[t], iEndRow , TAP_ORIGINAL_PUMP_HEAD );
					notedata.AddHoldNote( t, iHoldStartRow[t], iEndRow , TAP_ORIGINAL_HOLD_HEAD );
				}
				iHoldStartRow[t] = -1;
			}

			TapNote tap;
			switch( sRowString[t] )
			{
			case '0':	tap = TAP_EMPTY;		break;
			case '1':	tap = TAP_ORIGINAL_TAP;		break;
				//allow setting more notetypes on ksf files, this may come in handy (it should) -DaisuMaster
			case 'M':
			case 'm':
						tap = TAP_ORIGINAL_MINE;
						break;
			case 'F':
			case 'f':
						tap = TAP_ORIGINAL_FAKE;
						break;
			case 'L':
			case 'l':
						tap = TAP_ORIGINAL_LIFT;
						break;
			default:
				LOG->UserLog( "Song file", sPath, "has an invalid row \"%s\"; corrupt notes ignored.",
					      sRowString.c_str() );
				//return false;
				tap = TAP_EMPTY;
				break;
			}

			notedata.SetTapNote(t, BeatToNoteRow(fCurBeat), tap);
		}
		prevBeat = fCurBeat;
		fCurBeat = prevBeat + 1.0f / iTickCount;
	}

	out.SetNoteData( notedata );
	out.m_Timing = stepsTiming;

	out.TidyUpData();

	out.SetSavedToDisk( true );	// we're loading from disk, so this is by definintion already saved

	return true;
}

static void LoadTags( const RString &str, Song &out )
{
	/* str is either a #TITLE or a directory component.  Fill in missing information.
	 * str is either "title", "artist - title", or "artist - title - difficulty". */
	vector<RString> asBits;
	split( str, " - ", asBits, false );
	// Ignore the difficulty, since we get that elsewhere.
	if( asBits.size() == 3 && (
		asBits[2].EqualsNoCase("double") ||
		asBits[2].EqualsNoCase("easy") ||
		asBits[2].EqualsNoCase("normal") ||
		asBits[2].EqualsNoCase("hard") ||
		asBits[2].EqualsNoCase("crazy") ||
		asBits[2].EqualsNoCase("nightmare")) 
		)
	{
		asBits.erase( asBits.begin()+2, asBits.begin()+3 );
	}

	RString title, artist;
	if( asBits.size() == 2 )
	{
		artist = asBits[0];
		title = asBits[1];
	}
	else if( asBits.size() == 1 )
	{
		title = asBits[0];
	}

	// Convert, if possible. Most KSFs are in Korean encodings (CP942/EUC-KR).
	if( !ConvertString( title, "korean" ) )
		title = "";
	if( !ConvertString( artist, "korean" ) )
		artist = "";

	if( out.m_sMainTitle == "" )
		out.m_sMainTitle = title;
	if( out.m_sArtist == "" )
		out.m_sArtist = artist;
}

static void ProcessTickcounts( const RString & value, int & ticks, TimingData & timing )
{
	/* TICKCOUNT will be used below if there are DM compliant BPM changes
	 * and stops. It will be called again in LoadFromKSFFile for the
	 * actual steps. */
	ticks = StringToInt( value );
	CLAMP( ticks, 0, ROWS_PER_BEAT );

	if( ticks == 0 )
		ticks = TickcountSegment::DEFAULT_TICK_COUNT;

	timing.AddSegment( TickcountSegment(0, ticks) );
}

static bool LoadGlobalData( const RString &sPath, Song &out, bool &bKIUCompliant )
{
	MsdFile msd;
	if( !msd.ReadFile( sPath, false ) )  // don't unescape
	{
		LOG->UserLog( "Song file", sPath, "couldn't be opened: %s", msd.GetError().c_str() );
		return false;
	}

	// changed up there in case of something is found inside the SONGFILE tag in the head ksf -DaisuMaster
	// search for music with song in the file name
	vector<RString> arrayPossibleMusic;
	GetDirListing( out.GetSongDir() + RString("song.mp3"), arrayPossibleMusic );
	GetDirListing( out.GetSongDir() + RString("song.oga"), arrayPossibleMusic );
	GetDirListing( out.GetSongDir() + RString("song.ogg"), arrayPossibleMusic );
	GetDirListing( out.GetSongDir() + RString("song.wav"), arrayPossibleMusic );

	if( !arrayPossibleMusic.empty() )		// we found a match
		out.m_sMusicFile = arrayPossibleMusic[0];
	// ^this was below, at the end

	float SMGap1 = 0, SMGap2 = 0, BPM1 = -1, BPMPos2 = -1, BPM2 = -1, BPMPos3 = -1, BPM3 = -1;
	int iTickCount = -1;
	bKIUCompliant = false;
	vector<RString> vNoteRows;

	for( unsigned i=0; i < msd.GetNumValues(); i++ )
	{
		const MsdFile::value_t &sParams = msd.GetValue(i);
		RString sValueName = sParams[0];
		sValueName.MakeUpper();

		// handle the data
		if( sValueName=="TITLE" )
			LoadTags(sParams[1], out);
		else if( sValueName=="BPM" )
		{
			BPM1 = StringToFloat(sParams[1]);
			out.m_SongTiming.AddSegment( BPMSegment(0, BPM1) );
		}
		else if( sValueName=="BPM2" )
		{
			bKIUCompliant = true;
			BPM2 = StringToFloat( sParams[1] );
		}
		else if( sValueName=="BPM3" )
		{
			bKIUCompliant = true;
			BPM3 = StringToFloat( sParams[1] );
		}
		else if( sValueName=="BUNKI" )
		{
			bKIUCompliant = true;
			BPMPos2 = StringToFloat( sParams[1] ) / 100.0f;
		}
		else if( sValueName=="BUNKI2" )
		{
			bKIUCompliant = true;
			BPMPos3 = StringToFloat( sParams[1] ) / 100.0f;
		}
		else if( sValueName=="STARTTIME" )
		{
			SMGap1 = -StringToFloat( sParams[1] )/100;
			out.m_SongTiming.m_fBeat0OffsetInSeconds = SMGap1;
		}
		// This is currently required for more accurate KIU BPM changes.
		else if( sValueName=="STARTTIME2" )
		{
			bKIUCompliant = true;
			SMGap2 = -StringToFloat( sParams[1] )/100;
		}
		else if ( sValueName=="STARTTIME3" )
		{
			// STARTTIME3 only ensures this is a KIU compliant simfile.
			//bKIUCompliant = true;
		}
		else if ( sValueName=="TICKCOUNT" )
		{
			ProcessTickcounts(sParams[1], iTickCount, out.m_SongTiming);
		}
		else if ( sValueName=="STEP" )
		{
			/* STEP will always be the last header in a KSF file by design. Due to
			 * the Direct Move syntax, it is best to get the rows of notes here. */
			RString theSteps = sParams[1];
			TrimLeft( theSteps );
			split( theSteps, "\n", vNoteRows, true );
		}
		else if( sValueName=="DIFFICULTY" || sValueName=="PLAYER" )
		{
			/* DIFFICULTY and PLAYER are handled only in LoadFromKSFFile.
			Ignore those here. */
			continue;
		}
		// New cases noted in Aldo_MX's code:
		else if( sValueName=="MUSICINTRO" || sValueName=="INTRO" )
		{
			out.m_fMusicSampleStartSeconds = HHMMSSToSeconds( sParams[1] );
		}
		else if( sValueName=="TITLEFILE" )
		{
			out.m_sBackgroundFile = sParams[1];
		}
		else if( sValueName=="DISCFILE" )
		{
			out.m_sBannerFile = sParams[1];
		}
		else if( sValueName=="SONGFILE" )
		{
			out.m_sMusicFile = sParams[1];
		}
		//else if( sValueName=="INTROFILE" )
		//{
		//	nothing to add...
		//}
		// end new cases
		else
		{
			LOG->UserLog( "Song file", sPath, "has an unexpected value named \"%s\".",
				      sValueName.c_str() );
		}
	}

	//intro length in piu mixes is generally 7 seconds
	out.m_fMusicSampleLengthSeconds = 7.0f;

	/* BPM Change checks are done here.  If bKIUCompliant, it's short and sweet.
	 * Otherwise, the whole file has to be processed.  Right now, this is only 
	 * called once, for the initial file (often the Crazy steps).  Hopefully that
	 * will end up changing soon. */
	if( bKIUCompliant )
	{
		if( BPM2 > 0 && BPMPos2 > 0 )
		{
			HandleBunki( out.m_SongTiming, BPM1, BPM2, SMGap1, BPMPos2 );
		}

		if( BPM3 > 0 && BPMPos3 > 0 )
		{
			HandleBunki( out.m_SongTiming, BPM2, BPM3, SMGap2, BPMPos3 );
		}
	}
	else
	{
		float fCurBeat = 0.0f;
		bool bDMRequired = false;

		for( unsigned i=0; i < vNoteRows.size(); ++i )
		{
			RString& NoteRowString = vNoteRows[i];
			StripCrnl( NoteRowString );

			if( NoteRowString == "" )
				continue; // ignore empty rows.

			if( NoteRowString == "2222222222222" ) // Row of 2s = end. Confirm KIUCompliency here.
			{
				if (!bDMRequired)
					bKIUCompliant = true;
				break;
			}

			// This is where the DMRequired test will take place.
			if ( BeginsWith( NoteRowString, "|" ) )
			{
				// have a static timing for everything
				bDMRequired = true;
				continue;
			}
			else
			{
				// ignore whatever else...
				//continue;
			}
			
			fCurBeat += 1.0f / iTickCount;
		}
	}

	// Try to fill in missing bits of information from the pathname.
	{
		vector<RString> asBits;
		split( sPath, "/", asBits, true);

		ASSERT( asBits.size() > 1 );
		LoadTags( asBits[asBits.size()-2], out );
	}

	return true;
}

void KSFLoader::GetApplicableFiles( const RString &sPath, vector<RString> &out )
{
	GetDirListing( sPath + RString("*.ksf"), out );
}

bool KSFLoader::LoadNoteDataFromSimfile( const RString & cachePath, Steps &out )
{
	bool KIUCompliant = false;
	Song dummy;
	if (!LoadGlobalData(cachePath, dummy, KIUCompliant))
		return false;
	Steps *notes = dummy.CreateSteps();
	if (LoadFromKSFFile(cachePath, *notes, dummy, KIUCompliant))
	{
		KIUCompliant = true; // yeah, reusing a variable.
		out.SetNoteData(notes->GetNoteData());
	}
	delete notes;
	return KIUCompliant;
}

bool KSFLoader::LoadFromDir( const RString &sDir, Song &out )
{
	LOG->Trace( "KSFLoader::LoadFromDir(%s)", sDir.c_str() );

	vector<RString> arrayKSFFileNames;
	GetDirListing( sDir + RString("*.ksf"), arrayKSFFileNames );

	// We shouldn't have been called to begin with if there were no KSFs.
	ASSERT( arrayKSFFileNames.size() != 0 );

	bool bKIUCompliant = false;
	/* With Split Timing, there has to be a backup Song Timing in case
	 * anything goes wrong. As these files are kept in alphabetical
	 * order (hopefully), it is best to use the LAST file for timing 
	 * purposes, for that is the "normal", or easiest difficulty.
	 * Usually. */
	// Nevermind, kiu compilancy is screwing things up:
	// IE, I have two simfiles, oh wich each have four ksf files, the first one has
	// the first ksf with directmove timing changes, and the rest are not, everything
	// goes fine. In the other hand I have my second simfile with the first ksf file
	// without directmove timing changes and the rest have changes, changes are not
	// loaded due to kiucompilancy in the first ksf file.
	// About the "normal" thing, my simfiles' ksfs uses non-standard naming so
	// the last chart is usually nightmare or normal, I use easy and normal
	// indistinctly for SM so it shouldn't matter, I use piu fiesta/ex naming
	// for directmove though, and we're just gathering basic info anyway, and
	// most of the time all the KSF files have the same info in the #TITLE:; section
	unsigned files = arrayKSFFileNames.size();
	RString dir = out.GetSongDir();
	if( !LoadGlobalData(dir + arrayKSFFileNames[files - 1], out, bKIUCompliant) )
		return false;

	out.m_sSongFileName = dir + arrayKSFFileNames[files - 1];
	// load the Steps from the rest of the KSF files
	for( unsigned i=0; i<files; i++ ) 
	{
		Steps* pNewNotes = out.CreateSteps();
		if( !LoadFromKSFFile(dir + arrayKSFFileNames[i], *pNewNotes, out, bKIUCompliant) )
		{
			delete pNewNotes;
			continue;
		}
		pNewNotes->SetFilename(dir + arrayKSFFileNames[i]);
		out.AddSteps( pNewNotes );
	}
	return true;
}

/*
 * (c) 2001-2006 Chris Danford, Glenn Maynard, Jason Felds
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
