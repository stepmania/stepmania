#include "global.h"
#include "NotesLoaderDWI.h"
#include "RageLog.h"
#include "MsdFile.h"
#include "RageUtil.h"
#include "RageUtil_CharConversions.h"
#include "NoteData.h"
#include "Song.h"
#include "Steps.h"
#include "GameInput.h"
#include "NotesLoader.h"
#include "PrefsManager.h"
#include "Difficulty.h"

#include <map>

using std::vector;

Difficulty DwiCompatibleStringToDifficulty( const std::string& sDC );

static std::map<int,int> g_mapDanceNoteToNoteDataColumn;

/** @brief The different types of core DWI arrows and pads. */
enum DanceNotes
{
	DANCE_NOTE_NONE = 0,
	DANCE_NOTE_PAD1_LEFT,
	DANCE_NOTE_PAD1_UPLEFT,
	DANCE_NOTE_PAD1_DOWN,
	DANCE_NOTE_PAD1_UP,
	DANCE_NOTE_PAD1_UPRIGHT,
	DANCE_NOTE_PAD1_RIGHT,
	DANCE_NOTE_PAD2_LEFT,
	DANCE_NOTE_PAD2_UPLEFT,
	DANCE_NOTE_PAD2_DOWN,
	DANCE_NOTE_PAD2_UP,
	DANCE_NOTE_PAD2_UPRIGHT,
	DANCE_NOTE_PAD2_RIGHT
};

/**
 * @brief Turn the individual character to the proper note.
 * @param c The character in question.
 * @param i The player.
 * @param note1Out The first result based on the character.
 * @param note2Out The second result based on the character.
 * @param sPath the path to the file.
 */
static void DWIcharToNote( char c, GameController i, int &note1Out, int &note2Out, const std::string &sPath )
{
	switch( c )
	{
	case '0':	note1Out = DANCE_NOTE_NONE;		note2Out = DANCE_NOTE_NONE;		break;
	case '1':	note1Out = DANCE_NOTE_PAD1_DOWN;	note2Out = DANCE_NOTE_PAD1_LEFT;	break;
	case '2':	note1Out = DANCE_NOTE_PAD1_DOWN;	note2Out = DANCE_NOTE_NONE;		break;
	case '3':	note1Out = DANCE_NOTE_PAD1_DOWN;	note2Out = DANCE_NOTE_PAD1_RIGHT;	break;
	case '4':	note1Out = DANCE_NOTE_PAD1_LEFT;	note2Out = DANCE_NOTE_NONE;		break;
	case '5':	note1Out = DANCE_NOTE_NONE;		note2Out = DANCE_NOTE_NONE;		break;
	case '6':	note1Out = DANCE_NOTE_PAD1_RIGHT;	note2Out = DANCE_NOTE_NONE;		break;
	case '7':	note1Out = DANCE_NOTE_PAD1_UP;		note2Out = DANCE_NOTE_PAD1_LEFT;	break;
	case '8':	note1Out = DANCE_NOTE_PAD1_UP;		note2Out = DANCE_NOTE_NONE;		break;
	case '9':	note1Out = DANCE_NOTE_PAD1_UP;		note2Out = DANCE_NOTE_PAD1_RIGHT;	break;
	case 'A':	note1Out = DANCE_NOTE_PAD1_UP;		note2Out = DANCE_NOTE_PAD1_DOWN;	break;
	case 'B':	note1Out = DANCE_NOTE_PAD1_LEFT;	note2Out = DANCE_NOTE_PAD1_RIGHT;	break;
	case 'C':	note1Out = DANCE_NOTE_PAD1_UPLEFT;	note2Out = DANCE_NOTE_NONE;		break;
	case 'D':	note1Out = DANCE_NOTE_PAD1_UPRIGHT;	note2Out = DANCE_NOTE_NONE;		break;
	case 'E':	note1Out = DANCE_NOTE_PAD1_LEFT;	note2Out = DANCE_NOTE_PAD1_UPLEFT;	break;
	case 'F':	note1Out = DANCE_NOTE_PAD1_UPLEFT;	note2Out = DANCE_NOTE_PAD1_DOWN;	break;
	case 'G':	note1Out = DANCE_NOTE_PAD1_UPLEFT;	note2Out = DANCE_NOTE_PAD1_UP;		break;
	case 'H':	note1Out = DANCE_NOTE_PAD1_UPLEFT;	note2Out = DANCE_NOTE_PAD1_RIGHT;	break;
	case 'I':	note1Out = DANCE_NOTE_PAD1_LEFT;	note2Out = DANCE_NOTE_PAD1_UPRIGHT;	break;
	case 'J':	note1Out = DANCE_NOTE_PAD1_DOWN;	note2Out = DANCE_NOTE_PAD1_UPRIGHT;	break;
	case 'K':	note1Out = DANCE_NOTE_PAD1_UP;		note2Out = DANCE_NOTE_PAD1_UPRIGHT;	break;
	case 'L':	note1Out = DANCE_NOTE_PAD1_UPRIGHT;	note2Out = DANCE_NOTE_PAD1_RIGHT;	break;
	case 'M':	note1Out = DANCE_NOTE_PAD1_UPLEFT;	note2Out = DANCE_NOTE_PAD1_UPRIGHT;	break;
	default:
			LOG->UserLog( "Song file", sPath, "has an invalid DWI note character '%c'.", c );
			note1Out = DANCE_NOTE_NONE;		note2Out = DANCE_NOTE_NONE;		break;
	}

	switch( i )
	{
	case GameController_1:
		break;
	case GameController_2:
		if( note1Out != DANCE_NOTE_NONE )
			note1Out += 6;
		if( note2Out != DANCE_NOTE_NONE )
			note2Out += 6;
		break;
	default:
		FAIL_M(fmt::sprintf("Invalid GameController: %i", i));
	}
}

/**
 * @brief Determine the note column[s] to place notes.
 * @param c The character in question.
 * @param i The player.
 * @param col1Out The first result based on the character.
 * @param col2Out The second result based on the character.
 * @param sPath the path to the file.
 */
static void DWIcharToNoteCol( char c, GameController i, int &col1Out, int &col2Out, const std::string &sPath )
{
	int note1, note2;
	DWIcharToNote( c, i, note1, note2, sPath );

	if( note1 != DANCE_NOTE_NONE )
		col1Out = g_mapDanceNoteToNoteDataColumn[note1];
	else
		col1Out = -1;

	if( note2 != DANCE_NOTE_NONE )
		col2Out = g_mapDanceNoteToNoteDataColumn[note2];
	else
		col2Out = -1;
}

/**
 * @brief Determine if the note in question is a 192nd note.
 *
 * DWI used to use <...> to indicate 1/192nd notes; at some
 * point, <...> was changed to indicate jumps, and `' was used for
 * 1/192nds.  So, we have to do a check to figure out what it really
 * means.  If it contains 0s, it's most likely 192nds; otherwise,
 * it's most likely a jump.  Search for a 0 before the next >:
 * @param sStepData the step data.
 * @param pos the position of the step data.
 * @return true if it's a 192nd note, false otherwise.
 */
static bool Is192( const std::string &sStepData, size_t pos )
{
	while( pos < sStepData.size() )
	{
		if( sStepData[pos] == '>' )
			return false;
		if( sStepData[pos] == '0' )
			return true;
		++pos;
	}

	return false;
}
/** @brief All DWI files use 4 beats per measure. */
const int BEATS_PER_MEASURE = 4;

/* We prefer the normal names; recognize a number of others, too. (They'll get
 * normalized when written to SMs, etc.) */
Difficulty DwiCompatibleStringToDifficulty( const std::string& sDC )
{
	std::string s2 = Rage::make_lower(sDC);
	if( s2 == "beginner" )			return Difficulty_Beginner;
	else if( s2 == "easy" )		return Difficulty_Easy;
	else if( s2 == "basic" )		return Difficulty_Easy;
	else if( s2 == "light" )		return Difficulty_Easy;
	else if( s2 == "medium" )		return Difficulty_Medium;
	else if( s2 == "another" )		return Difficulty_Medium;
	else if( s2 == "trick" )		return Difficulty_Medium;
	else if( s2 == "standard" )	return Difficulty_Medium;
	else if( s2 == "difficult")	return Difficulty_Medium;
	else if( s2 == "hard" )		return Difficulty_Hard;
	else if( s2 == "ssr" )			return Difficulty_Hard;
	else if( s2 == "maniac" )		return Difficulty_Hard;
	else if( s2 == "heavy" )		return Difficulty_Hard;
	else if( s2 == "smaniac" )		return Difficulty_Challenge;
	else if( s2 == "challenge" )	return Difficulty_Challenge;
	else if( s2 == "expert" )		return Difficulty_Challenge;
	else if( s2 == "oni" )			return Difficulty_Challenge;
	else if( s2 == "edit" )		return Difficulty_Edit;
	else							return Difficulty_Invalid;
}

static StepsType GetTypeFromMode(const std::string &mode)
{
	if( mode == "SINGLE" )
		return StepsType_dance_single;
	else if( mode == "DOUBLE" )
		return StepsType_dance_double;
	else if( mode == "COUPLE" )
		return StepsType_dance_couple;
	else if( mode == "SOLO" )
		return StepsType_dance_solo;
	ASSERT_M(0, "Unrecognized DWI notes format " + mode + "!");
	return StepsType_Invalid; // just in case.
}

static NoteData ParseNoteData(std::string &step1, std::string &step2,
			      Steps &out, const std::string &path)
{
	g_mapDanceNoteToNoteDataColumn.clear();
	switch( out.m_StepsType )
	{
		case StepsType_dance_single:
			g_mapDanceNoteToNoteDataColumn[DANCE_NOTE_PAD1_LEFT] = 0;
			g_mapDanceNoteToNoteDataColumn[DANCE_NOTE_PAD1_DOWN] = 1;
			g_mapDanceNoteToNoteDataColumn[DANCE_NOTE_PAD1_UP] = 2;
			g_mapDanceNoteToNoteDataColumn[DANCE_NOTE_PAD1_RIGHT] = 3;
			break;
		case StepsType_dance_double:
		case StepsType_dance_couple:
			g_mapDanceNoteToNoteDataColumn[DANCE_NOTE_PAD1_LEFT] = 0;
			g_mapDanceNoteToNoteDataColumn[DANCE_NOTE_PAD1_DOWN] = 1;
			g_mapDanceNoteToNoteDataColumn[DANCE_NOTE_PAD1_UP] = 2;
			g_mapDanceNoteToNoteDataColumn[DANCE_NOTE_PAD1_RIGHT] = 3;
			g_mapDanceNoteToNoteDataColumn[DANCE_NOTE_PAD2_LEFT] = 4;
			g_mapDanceNoteToNoteDataColumn[DANCE_NOTE_PAD2_DOWN] = 5;
			g_mapDanceNoteToNoteDataColumn[DANCE_NOTE_PAD2_UP] = 6;
			g_mapDanceNoteToNoteDataColumn[DANCE_NOTE_PAD2_RIGHT] = 7;
			break;
		case StepsType_dance_solo:
			g_mapDanceNoteToNoteDataColumn[DANCE_NOTE_PAD1_LEFT] = 0;
			g_mapDanceNoteToNoteDataColumn[DANCE_NOTE_PAD1_UPLEFT] = 1;
			g_mapDanceNoteToNoteDataColumn[DANCE_NOTE_PAD1_DOWN] = 2;
			g_mapDanceNoteToNoteDataColumn[DANCE_NOTE_PAD1_UP] = 3;
			g_mapDanceNoteToNoteDataColumn[DANCE_NOTE_PAD1_UPRIGHT] = 4;
			g_mapDanceNoteToNoteDataColumn[DANCE_NOTE_PAD1_RIGHT] = 5;
			break;
			DEFAULT_FAIL( out.m_StepsType );
	}

	NoteData newNoteData;
	newNoteData.SetNumTracks( g_mapDanceNoteToNoteDataColumn.size() );

	for( int pad=0; pad<2; pad++ )		// foreach pad
	{
		std::string sStepData;
		switch( pad )
		{
			case 0:
				sStepData = step1;
				break;
			case 1:
				if( step2 == "" )	// no data
					continue;	// skip
				sStepData = step2;
				break;
				DEFAULT_FAIL( pad );
		}

		Rage::replace(sStepData, "\n", "");
		Rage::replace(sStepData, "\r", "");
		Rage::replace(sStepData, "\t", "");
		Rage::replace(sStepData, " ", "");

		double fCurrentBeat = 0;
		double fCurrentIncrementer = 1.0/8 * BEATS_PER_MEASURE;

		for( size_t i=0; i<sStepData.size(); )
		{
			char c = sStepData[i++];
			switch( c )
			{
					// begins a series
				case '(':
					fCurrentIncrementer = 1.0/16 * BEATS_PER_MEASURE;
					break;
				case '[':
					fCurrentIncrementer = 1.0/24 * BEATS_PER_MEASURE;
					break;
				case '{':
					fCurrentIncrementer = 1.0/64 * BEATS_PER_MEASURE;
					break;
				case '`':
					fCurrentIncrementer = 1.0/192 * BEATS_PER_MEASURE;
					break;

					// ends a series
				case ')':
				case ']':
				case '}':
				case '\'':
				case '>':
					fCurrentIncrementer = 1.0/8 * BEATS_PER_MEASURE;
					break;

				default:	// this is a note character
				{
					if( c == '!' )
					{
						LOG->UserLog(
							     "Song file",
							     path,
							     "has an unexpected character: '!'." );
						continue;
					}

					bool jump = false;
					if( c == '<' )
					{
						/* Arr.  Is this a jump or a 1/192 marker? */
						if( Is192( sStepData, i ) )
						{
							fCurrentIncrementer = 1.0/192 * BEATS_PER_MEASURE;
							break;
						}

						/* It's a jump.
						 * We need to keep reading notes until we hit a >. */
						jump = true;
						i++;
					}

					const int iIndex = BeatToNoteRow( (float)fCurrentBeat );
					i--;
					do {
						c = sStepData[i++];

						if( jump && c == '>' )
							break;

						int iCol1, iCol2;
						DWIcharToNoteCol(
								 c,
								 (GameController)pad,
								 iCol1,
								 iCol2,
								 path );

						if( iCol1 != -1 )
							newNoteData.SetTapNote(iCol1,
									       iIndex,
									       TAP_ORIGINAL_TAP);
						if( iCol2 != -1 )
							newNoteData.SetTapNote(iCol2,
									       iIndex,
									       TAP_ORIGINAL_TAP);

						if(i>=sStepData.length())
						{
							break;
							//we ran out of data
							//while looking for the ending > mark
						}

						if( sStepData[i] == '!' )
						{
							i++;
							const char holdChar = sStepData[i++];

							DWIcharToNoteCol(holdChar,
									 (GameController)pad,
									 iCol1,
									 iCol2,
									 path );

							if( iCol1 != -1 )
								newNoteData.SetTapNote(iCol1,
										       iIndex,
										       TAP_ORIGINAL_HOLD_HEAD);
							if( iCol2 != -1 )
								newNoteData.SetTapNote(iCol2,
										       iIndex,
										       TAP_ORIGINAL_HOLD_HEAD);
						}
					}
					while( jump );
					fCurrentBeat += fCurrentIncrementer;
				}
					break;
			}
		}
	}

	/* Fill in iDuration. */
	for( int t=0; t<newNoteData.GetNumTracks(); ++t )
	{
		FOREACH_NONEMPTY_ROW_IN_TRACK( newNoteData, t, iHeadRow )
		{
			TapNote tn = newNoteData.GetTapNote( t, iHeadRow  );
			if( tn.type != TapNoteType_HoldHead )
				continue;

			int iTailRow = iHeadRow;
			bool bFound = false;
			while( !bFound && newNoteData.GetNextTapNoteRowForTrack(t, iTailRow) )
			{
				const TapNote &TailTap = newNoteData.GetTapNote( t, iTailRow );
				if( TailTap.type == TapNoteType_Empty )
					continue;

				newNoteData.SetTapNote( t, iTailRow, TAP_EMPTY );
				tn.iDuration = iTailRow - iHeadRow;
				newNoteData.SetTapNote( t, iHeadRow, tn );
				bFound = true;
			}

			if( !bFound )
			{
				/* The hold was never closed.  */
				LOG->UserLog("Song file",
					     path,
					     "failed to close a hold note in \"%s\" on track %i",
					     DifficultyToString(out.GetDifficulty()).c_str(),
					     t);

				newNoteData.SetTapNote( t, iHeadRow, TAP_EMPTY );
			}
		}
	}

	ASSERT( newNoteData.GetNumTracks() > 0 );
	return newNoteData;
}

/**
 * @brief Look through the notes tag to extract the data.
 * @param sMode the steps type.
 * @param sDescription the difficulty.
 * @param sNumFeet the meter.
 * @param sStepData1 the guaranteed step data.
 * @param sStepData2 used if sMode is double or couple.
 * @param out the step data.
 * @param sPath the path to the file.
 * @return the success or failure of the operation.
 */
static bool LoadFromDWITokens(
	std::string sMode,
	std::string sDescription,
	std::string sNumFeet,
	std::string sStepData1,
	std::string sStepData2,
	Steps &out,
	const std::string &sPath )
{
	CHECKPOINT_M( "DWILoader::LoadFromDWITokens()" );

	out.m_StepsType = GetTypeFromMode(sMode);

	// if the meter is empty, force it to 1.
	if( sNumFeet.empty() )
		sNumFeet = "1";

	out.SetMeter(std::stoi(sNumFeet));

	out.SetDifficulty( DwiCompatibleStringToDifficulty(sDescription) );

	out.SetNoteData( ParseNoteData(sStepData1, sStepData2, out, sPath) );

	out.TidyUpData();

	out.SetSavedToDisk( true );	// we're loading from disk, so this is by definintion already saved
	return true;
}

/**
 * @brief Turn the DWI style timestamp into a compatible time for our system.
 *
 * This value can be in either "HH:MM:SS.sssss", "MM:SS.sssss", "SSS.sssss"
 * or milliseconds.
 * @param arg1 Either hours, minutes, or seconds, depending on other args.
 * @param arg2 Either minutes or seconds, depending on other args.
 * @param arg3 Seconds if not empty.
 * @return the proper timestamp.
 */
static float ParseBrokenDWITimestamp( const std::string &arg1, const std::string &arg2, const std::string &arg3 )
{
	if( arg1.empty() )
		return 0;

	/* 1+ args */
	if( arg2.empty() )
	{
		/* If the value contains a period, treat it as seconds; otherwise ms. */
		if( arg1.find_first_of(".") != arg1.npos )
		{
			return std::stof( arg1 );
		}
		return std::stof( arg1 ) / 1000.f;
	}

	/* 2+ args */
	if( arg3.empty() )
		return HHMMSSToSeconds( arg1+":"+arg2 );

	/* 3+ args */
	return HHMMSSToSeconds( arg1+":"+arg2+":"+arg3 );
}


void DWILoader::GetApplicableFiles( std::string const &sPath, vector<std::string> &out )
{
	GetDirListing( sPath + "*.dwi", out );
}

bool DWILoader::LoadNoteDataFromSimfile( const std::string &path, Steps &out )
{
	MsdFile msd;
	if( !msd.ReadFile( path, false ) )  // don't unescape
	{
		LOG->UserLog("Song file",
			     path,
			     "couldn't be opened: %s",
			     msd.GetError().c_str() );
		return false;
	}

	for( unsigned i=0; i<msd.GetNumValues(); i++ )
	{
		int iNumParams = msd.GetNumParams(i);
		const MsdFile::value_t &params = msd.GetValue(i);
		std::string valueName = params[0];
		Rage::ci_ascii_string tagName{ valueName.c_str() };

		if(tagName == "SINGLE" || tagName == "DOUBLE" || tagName == "COUPLE" || tagName == "SOLO")
		{
			if (out.m_StepsType != GetTypeFromMode(valueName))
			{
				continue;
			}
			if (out.GetDifficulty() != DwiCompatibleStringToDifficulty(params[1]))
			{	continue;
			}
			if (out.GetMeter() != std::stoi(params[2]))
			{
				continue;
			}
			std::string step1 = params[3];
			std::string step2 = (iNumParams==5) ? params[4] : std::string("");
			out.SetNoteData(ParseNoteData(step1, step2, out, path));
			return true;
		}
	}
	return false;
}

bool DWILoader::LoadFromDir( const std::string &sPath_, Song &out, std::set<std::string> &BlacklistedImages )
{
	vector<std::string> aFileNames;
	GetApplicableFiles( sPath_, aFileNames );

	if( aFileNames.size() > 1 )
	{
		LOG->UserLog( "Song", sPath_, "has more than one DWI file. There should be only one!" );
		return false;
	}

	/* We should have exactly one; if we had none, we shouldn't have been called to begin with. */
	ASSERT( aFileNames.size() == 1 );
	const std::string sPath = sPath_ + aFileNames[0];

	LOG->Trace( "Song::LoadFromDWIFile(%s)", sPath.c_str() );

	MsdFile msd;
	if( !msd.ReadFile( sPath, false ) )  // don't unescape
	{
		LOG->UserLog( "Song file", sPath, "couldn't be opened: %s", msd.GetError().c_str() );
		return false;
	}

	out.m_sSongFileName = sPath;

	for( unsigned i=0; i<msd.GetNumValues(); i++ )
	{
		int iNumParams = msd.GetNumParams(i);
		const MsdFile::value_t &sParams = msd.GetValue(i);
		std::string sValueName = sParams[0];
		Rage::ci_ascii_string tagName{ sValueName.c_str() };

		if( iNumParams < 1 )
		{
			LOG->UserLog( "Song file", sPath, "has tag \"%s\" with no parameters.", sValueName.c_str() );
			continue;
		}

		// handle the data
		if( tagName == "FILE" )
			out.m_sMusicFile = sParams[1];

		else if(tagName == "TITLE" )
		{
			NotesLoader::GetMainAndSubTitlesFromFullTitle( sParams[1], out.m_sMainTitle, out.m_sSubTitle );

			/* As far as I know, there's no spec on the encoding of this text. (I didn't
			 * look very hard, though.)  I've seen at least one file in ISO-8859-1. */
			ConvertString( out.m_sMainTitle, "utf-8,english" );
			ConvertString( out.m_sSubTitle, "utf-8,english" );
		}

		else if(tagName == "ARTIST" )
		{
			out.m_sArtist = sParams[1];
			ConvertString( out.m_sArtist, "utf-8,english" );
		}

		else if(tagName == "GENRE" )
		{
			out.m_sGenre = sParams[1];
			ConvertString( out.m_sGenre, "utf-8,english" );
		}

		else if(tagName == "CDTITLE" )
			out.m_sCDTitleFile = sParams[1];

		else if(tagName == "BPM" )
		{
			const float fBPM = std::stof( sParams[1] );

			if( unlikely(fBPM <= 0.0f && !PREFSMAN->m_bQuirksMode) )
			{
				LOG->UserLog("Song file", sPath, "has an invalid BPM change at beat %f, BPM %f.",
					0.0f, fBPM );
			}
			else
			{
				out.m_SongTiming.AddSegment( BPMSegment(0, fBPM) );
			}
		}
		else if(tagName == "DISPLAYBPM" )
		{
			// #DISPLAYBPM:[xxx..xxx]|[xxx]|[*];
		    int iMin, iMax;
			/* We can't parse this as a float with sscanf, since '.' is a valid
			 * character in a float.  (We could do it with a regex, but it's not
			 * worth bothering with since we don't display fractional BPM anyway.) */
		    if( sscanf( sParams[1].c_str(), "%i..%i", &iMin, &iMax ) == 2 )
			{
				out.m_DisplayBPMType = DISPLAY_BPM_SPECIFIED;
				out.m_fSpecifiedBPMMin = (float) iMin;
				out.m_fSpecifiedBPMMax = (float) iMax;
			}
			else if( sscanf( sParams[1].c_str(), "%i", &iMin ) == 1 )
			{
				out.m_DisplayBPMType = DISPLAY_BPM_SPECIFIED;
				out.m_fSpecifiedBPMMin = out.m_fSpecifiedBPMMax = (float) iMin;
			}
			else
			{
				out.m_DisplayBPMType = DISPLAY_BPM_RANDOM;
			}
		}

		else if(tagName == "GAP" )
		{
			// the units of GAP is 1/1000 second
			out.m_SongTiming.m_fBeat0OffsetInSeconds = -std::stoi(sParams[1]) / 1000.0f;
		}
		else if(tagName == "SAMPLESTART" )
		{
			out.m_fMusicSampleStartSeconds = ParseBrokenDWITimestamp(sParams[1], sParams[2], sParams[3]);
		}
		else if(tagName == "SAMPLELENGTH" )
		{
			float sampleLength = ParseBrokenDWITimestamp(sParams[1], sParams[2], sParams[3]);
			if (sampleLength > 0 && sampleLength < 1) {
				// there were multiple versions of this tag allegedly: ensure a decent length if requested.
				sampleLength *= 1000;
			}
			out.m_fMusicSampleLengthSeconds = sampleLength;

		}

		else if(tagName == "FREEZE" )
		{
			auto arrayFreezeExpressions = Rage::split( sParams[1], "," );
			for (auto &freeze: arrayFreezeExpressions)
			{
				auto arrayFreezeValues = Rage::split(freeze, "=");
				if( arrayFreezeValues.size() != 2 )
				{
					LOG->UserLog( "Song file", sPath, "has an invalid FREEZE: '%s'.", freeze.c_str() );
					continue;
				}
				int iFreezeRow = BeatToNoteRow( std::stof(arrayFreezeValues[0]) / 4.0f );
				float fFreezeSeconds = std::stof( arrayFreezeValues[1] ) / 1000.0f;

				out.m_SongTiming.AddSegment( StopSegment(iFreezeRow, fFreezeSeconds) );
//				LOG->Trace( "Adding a freeze segment: beat: %f, seconds = %f", fFreezeBeat, fFreezeSeconds );
			}
		}

		else if(tagName == "CHANGEBPM" || tagName == "BPMCHANGE" )
		{
			auto arrayBPMChangeExpressions = Rage::split(sParams[1], ",");

			for (auto &change: arrayBPMChangeExpressions)
			{
				auto arrayBPMChangeValues = Rage::split(change, "=");
				if( arrayBPMChangeValues.size() != 2 )
				{
					LOG->UserLog( "Song file", sPath, "has an invalid CHANGEBPM: '%s'.", change.c_str() );
					continue;
				}

				int iStartIndex = BeatToNoteRow( std::stof(arrayBPMChangeValues[0]) / 4.0f );
				float fBPM = std::stof( arrayBPMChangeValues[1] );
				if( fBPM > 0.0f )
				{
					out.m_SongTiming.AddSegment( BPMSegment(iStartIndex, fBPM) );
				}
				else
				{
					LOG->UserLog( "Song file", sPath, "has an invalid BPM change at beat %f, BPM %f.",
						      NoteRowToBeat(iStartIndex), fBPM );
				}
			}
		}

		else if(tagName == "SINGLE" || tagName == "DOUBLE" || tagName == "COUPLE" || tagName == "SOLO")
		{
			Steps* pNewNotes = out.CreateSteps();
			LoadFromDWITokens(
				sParams[0],
				sParams[1],
				sParams[2],
				sParams[3],
				(iNumParams==5) ? sParams[4] : std::string(""),
				*pNewNotes,
				sPath
				);
			if( pNewNotes->m_StepsType != StepsType_Invalid )
			{
				pNewNotes->SetFilename( sPath );
				out.AddSteps( pNewNotes );
			}
			else
				delete pNewNotes;
		}
		else if(tagName == "DISPLAYTITLE" || tagName == "DISPLAYARTIST" )
		{
			/* We don't want to support these tags.  However, we don't want
			 * to pick up images used here as song images (eg. banners). */
			std::string param = sParams[1];
			/* "{foo} ... {foo2}" */
			size_t pos = 0;
			while( pos < std::string::npos )
			{

				size_t startpos = param.find('{', pos);
				if( startpos == std::string::npos )
					break;
				size_t endpos = param.find('}', startpos);
				if( endpos == std::string::npos )
					break;

				std::string sub = Rage::make_lower(param.substr( startpos+1, endpos-startpos-1 ));

				pos = endpos + 1;

				BlacklistedImages.insert( sub );
			}
		}
		else
		{
			// do nothing.  We don't care about this value name
		}
	}

	return true;
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
