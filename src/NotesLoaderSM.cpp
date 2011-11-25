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

void SMLoader::SetSongTitle(const RString & title)
{
	this->songTitle = title;
}

RString SMLoader::GetSongTitle() const
{
	return this->songTitle;
}

bool SMLoader::LoadFromDir( const RString &sPath, Song &out )
{
	vector<RString> aFileNames;
	GetApplicableFiles( sPath, aFileNames );
	
	if( aFileNames.size() > 1 )
	{
		// Need to break this up first.
		RString tmp = "Song " + sPath + " has more than one";
		LOG->UserLog(tmp, this->GetFileExtension(), "file. There can only be one!");
		return false;
	}
	
	ASSERT( aFileNames.size() == 1 );
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

	//	LOG->Trace( "Steps::LoadFromTokens()" );

	// backwards compatibility hacks:
	// HACK: We eliminated "ez2-single-hard", but we should still handle it.
	if( sStepsType == "ez2-single-hard" )
		sStepsType = "ez2-single";

	// HACK: "para-single" used to be called just "para"
	if( sStepsType == "para" )
		sStepsType = "para-single";

	out.m_StepsType = GAMEMAN->StringToStepsType( sStepsType );
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
		vector<RString> aBGChangeExpressions;
		split( sParam, ",", aBGChangeExpressions );
		
		for( unsigned b=0; b<aBGChangeExpressions.size(); b++ )
		{
			BackgroundChange change;
			if( LoadFromBGChangesString( change, aBGChangeExpressions[b] ) )
				out.AddBackgroundChange( iLayer, change );
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
			attack.fStartSecond = strtof( sBits[1], NULL );
		else if( !sBits[0].CompareNoCase("LEN") )
			attack.fSecsRemaining = strtof( sBits[1], NULL );
		else if( !sBits[0].CompareNoCase("END") )
			end = strtof( sBits[1], NULL );
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

bool SMLoader::ProcessBPMs( TimingData &out, const RString line, const int rowsPerBeat )
{
	vector<RString> arrayBPMChangeExpressions;
	split( line, ",", arrayBPMChangeExpressions );
	
	// prepare storage variables for negative BPMs -> Warps.
	float negBeat = -1;
	float negBPM = 1;
	float highspeedBeat = -1;
	bool bNotEmpty = false;
	
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

		bNotEmpty = true;

		const float fBeat = RowToBeat( arrayBPMChangeValues[0], rowsPerBeat );
		const float fNewBPM = StringToFloat( arrayBPMChangeValues[1] );

		if( fNewBPM < 0.0f )
		{
			negBeat = fBeat;
			negBPM = fNewBPM;
		}
		else if( fNewBPM > 0.0f )
		{
			// add in a warp.
			if( negBPM < 0 )
			{
				float endBeat = fBeat + (fNewBPM / -negBPM) * (fBeat - negBeat);
				out.AddSegment( WarpSegment(BeatToNoteRow(negBeat), endBeat - negBeat) );

				negBeat = -1;
				negBPM = 1;
			}
			// too fast. make it a warp.
			if( fNewBPM > FAST_BPM_WARP )
			{
				highspeedBeat = fBeat;
			}
			else
			{
				// add in a warp.
				if( highspeedBeat > 0 )
				{
					out.AddSegment( WarpSegment(BeatToNoteRow(highspeedBeat), fBeat - highspeedBeat) );
					highspeedBeat = -1;
				}
				{
					out.AddSegment( BPMSegment(BeatToNoteRow(fBeat), fNewBPM) );
				}
			}
		}
	}

	return bNotEmpty;
}

void SMLoader::ProcessStops( TimingData &out, const RString line, const int rowsPerBeat )
{
	vector<RString> arrayFreezeExpressions;
	split( line, ",", arrayFreezeExpressions );
	
	// Prepare variables for negative stop conversion.
	float negBeat = -1;
	float negPause = 0;

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

		// Process the prior stop.
		if( negPause > 0 )
		{
			float oldBPM = out.GetBPMAtRow(BeatToNoteRow(negBeat));
			float fSecondsPerBeat = 60 / oldBPM;
			float fSkipBeats = negPause / fSecondsPerBeat;

			if( negBeat + fSkipBeats > fFreezeBeat )
				fSkipBeats = fFreezeBeat - negBeat;

			out.AddSegment( WarpSegment(BeatToNoteRow(negBeat), fSkipBeats));

			negBeat = -1;
			negPause = 0;
		}

		if( fFreezeSeconds < 0.0f )
		{
			negBeat = fFreezeBeat;
			negPause = -fFreezeSeconds;
		}
		else if( fFreezeSeconds > 0.0f )
		{
			out.AddSegment( StopSegment(BeatToNoteRow(fFreezeBeat), fFreezeSeconds) );
		}
	}

	// Process the prior stop if there was one.
	if( negPause > 0 )
	{
		float oldBPM = out.GetBPMAtBeat(negBeat);
		float fSecondsPerBeat = 60 / oldBPM;
		float fSkipBeats = negPause / fSecondsPerBeat;

		out.AddSegment( WarpSegment(BeatToNoteRow(negBeat), fSkipBeats) );
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

	FOREACH_CONST( RString, vs1, s1 )
	{
		vector<RString> vs2;
		split( *s1, "=", vs2 );

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

	FOREACH_CONST( RString, vs1, s1 )
	{
		vector<RString> vs2;
		split( *s1, "=", vs2 );

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

bool SMLoader::LoadFromBGChangesString( BackgroundChange &change, const RString &sBGChangeExpression )
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
	LOG->Trace( "Song::LoadFromSMFile(%s)", sPath.c_str() );

	MsdFile msd;
	if( !msd.ReadFile( sPath, true ) )  // unescape
	{
		LOG->UserLog( "Song file", sPath, "couldn't be opened: %s", msd.GetError().c_str() );
		return false;
	}

	out.m_SongTiming.m_sFile = sPath;
	out.m_sSongFileName = sPath;

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

		// Save "#LYRICS" for later, so we can add an internal lyrics tag.
		else if( sValueName=="LYRICSPATH" )
			out.m_sLyricsFile = sParams[1];

		else if( sValueName=="CDTITLE" )
			out.m_sCDTitleFile = sParams[1];

		else if( sValueName=="MUSIC" )
			out.m_sMusicFile = sParams[1];

		else if( sValueName=="OFFSET" )
		{
			out.m_SongTiming.m_fBeat0OffsetInSeconds = StringToFloat( sParams[1] );
		}
		else if( sValueName=="BPMS" )
		{
			ProcessBPMs(out.m_SongTiming, sParams[1]);
		}

		else if( sValueName=="STOPS" || sValueName=="FREEZES" )
		{
			ProcessStops(out.m_SongTiming, sParams[1]);
		}

		else if( sValueName=="DELAYS" )
		{
			ProcessDelays(out.m_SongTiming, sParams[1]);
		}

		else if( sValueName=="TIMESIGNATURES" )
		{
			ProcessTimeSignatures(out.m_SongTiming, sParams[1]);
		}

		else if( sValueName=="TICKCOUNTS" )
		{
			ProcessTickcounts(out.m_SongTiming, sParams[1]);
		}

		else if( sValueName=="INSTRUMENTTRACK" )
		{
			ProcessInstrumentTracks( out, sParams[1] );
		}

		else if( sValueName=="MUSICLENGTH" )
		{
			if( !bFromCache )
				continue;
			out.m_fMusicLengthSeconds = StringToFloat( sParams[1] );
		}

		else if( sValueName=="LASTBEATHINT" )
		{
			// unable to identify at this point: ignore
		}

		else if( sValueName=="MUSICBYTES" )
			; /* ignore */

		// cache tags from older SM files: ignore.
		else if(sValueName=="FIRSTBEAT" || sValueName=="LASTBEAT" ||
			sValueName=="SONGFILENAME" || sValueName=="HASMUSIC" ||
			sValueName=="HASBANNER")
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
			ProcessBGChanges( out, sValueName, sPath, sParams[1]);
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
			ProcessAttackString(out.m_sAttackString, sParams);
			ProcessAttacks(out.m_Attacks, sParams);
		}

		else if( sValueName=="NOTES" || sValueName=="NOTES2" )
		{
			if( iNumParams < 7 )
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
				*pNewNotes );

			pNewNotes->SetFilename(sPath);
			out.AddSteps( pNewNotes );
		}
		// XXX: Does anyone know what LEADTRACK is for? -Wolfman2000
		else if( sValueName=="LEADTRACK" )
			;
		else
			LOG->UserLog( "Song file", sPath, "has an unexpected value named \"%s\".", sValueName.c_str() );
	}

	// Ensure all warps from negative time changes are in order.
	out.m_SongTiming.SortSegments( SEGMENT_WARP );
	TidyUpData( out, bFromCache );
	return true;
}

bool SMLoader::LoadEditFromFile( RString sEditFilePath, ProfileSlot slot, bool bAddStepsToSong )
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

	return LoadEditFromMsd( msd, sEditFilePath, slot, bAddStepsToSong );
}

bool SMLoader::LoadEditFromBuffer( const RString &sBuffer, const RString &sEditFilePath, ProfileSlot slot )
{
	MsdFile msd;
	msd.ReadFromString( sBuffer, true ); // unescape
	return LoadEditFromMsd( msd, sEditFilePath, slot, true );
}

bool SMLoader::LoadEditFromMsd( const MsdFile &msd, const RString &sEditFilePath, ProfileSlot slot, bool bAddStepsToSong )
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
			this->SetSongTitle(sParams[1]);
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

	return true;

}

void SMLoader::GetApplicableFiles( const RString &sPath, vector<RString> &out )
{
	GetDirListing( sPath + RString("*" + this->GetFileExtension() ), out );
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
