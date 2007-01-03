#include "global.h"
#include "NotesLoaderKSF.h"
#include "RageUtil_CharConversions.h"
#include "MsdFile.h"
#include "RageLog.h"
#include "RageUtil.h"
#include "NoteData.h"
#include "NoteTypes.h"
#include "song.h"
#include "Steps.h"

#if 0
void KSFLoader::RemoveHoles( NoteData &out, const Song &song )
{
	/* Start at the second BPM segment; the first one is already aligned. */
	for( unsigned seg = 1; seg < song.m_Timing.m_BPMSegments.size(); ++seg )
	{
//		const float FromBeat = song.m_Timing.m_BPMSegments[seg].m_fStartBeat;
		const float FromBeat = song.m_Timing.m_BPMSegments[seg].m_fStartBeat * song.m_BPMSegments[seg].m_fBPM / song.m_BPMSegments[0].m_fBPM;
		const int FromRow = (int) BeatToNoteRow(FromBeat);
		const int ToRow = (int) BeatToNoteRow(song.m_Timing.m_BPMSegments[seg].m_fStartBeat);

		LOG->Trace("from %f (%i) to (%i)", FromBeat, FromRow, ToRow);
//		const int ToRow = lrintf(FromRow * song.m_Timing.m_BPMSegments[0].m_fBPM / song.m_BPMSegments[seg].m_fBPM);
//		const int Rows = out.GetLastRow() - FromRow + 1;
//		int LastRow;
//		if(seg+1 < song.m_Timing.m_BPMSegments().size())
//			LastRow = (int) NoteRowToBeat( song.m_Timing.m_BPMSegments[seg+1].m_fStartBeat ) - 1;
//		else
//			LastRow = out.GetLastRow();
		NoteData tmp;
		tmp.SetNumTracks( out.GetNumTracks() );
		tmp.CopyRange( &out, FromRow, MAX_NOTE_ROW );
		out.ClearRange( FromRow, MAX_NOTE_ROW );
		out.CopyRange( &tmp, 0, MAX_NOTE_ROW, ToRow );
	}

/*
	for( t = 0; t < notedata.GetNumTracks(); ++t )
	{
		const float CurBPM = song.GetBPMAtBeat( NoteRowToBeat(row) );
		song.m_Timing.m_BPMSegments.size()
		for( int row = 0; row <= notedata.GetLastRow(); ++row )
		{
			TapNote tn = notedata.GetTapNote(t, row);
			if( tn == TAP_EMPTY )
				continue;

			const int RealRow = lrintf(row * OrigBPM / CurBPM);
			if( RealRow == row )
				continue;
			LOG->Trace("from %i to %i", row, RealRow);
			notedata.SetTapNote( t, RealRow, tn );
			notedata.SetTapNote( t, row, TAP_EMPTY );
		}
	}
*/
}
#endif

bool KSFLoader::LoadFromKSFFile( const RString &sPath, Steps &out, const Song &song )
{
	LOG->Trace( "Steps::LoadFromKSFFile( '%s' )", sPath.c_str() );

	MsdFile msd;
	if( !msd.ReadFile( sPath, false ) )  // don't unescape
	{
		LOG->UserLog( "Song file", sPath, "couldn't be opened: %s", msd.GetError().c_str() );
		return false;
	}

	m_iTickCount = -1;	// this is the value we read for TICKCOUNT
	m_vNoteRows.clear(); //Start from a clean slate.

	for( unsigned i=0; i<msd.GetNumValues(); i++ )
	{
		const MsdFile::value_t &sParams = msd.GetValue( i );
		RString sValueName = sParams[0];

		// handle the data
		if( 0==stricmp(sValueName,"TICKCOUNT") )
		{
			m_iTickCount = atoi( sParams[1] );
			if( m_iTickCount <= 0 )
			{
				LOG->UserLog( "Song file", sPath, "has an invalid tick count: %d.", m_iTickCount );
				return false;
			}
		}
		else if( 0==stricmp(sValueName,"STEP") )
		{
			RString theSteps = sParams[1];
			TrimLeft( theSteps );
			split( theSteps, "\n", m_vNoteRows, true );
		}
		else if( 0==stricmp(sValueName,"DIFFICULTY") )
		{
			out.SetMeter( max(atoi(sParams[1]), 0) );
		}
	}

	if( m_iTickCount == -1 )
	{
		m_iTickCount = 2;
		LOG->UserLog( "Song file", sPath, "doesn't have a TICKCOUNT. Defaulting to %i.", m_iTickCount );
	}

	NoteData notedata;	// read it into here

	{
		RString sDir, sFName, sExt;
		splitpath( sPath, sDir, sFName, sExt );
		sFName.MakeLower();

		out.SetDescription(sFName);
		if( sFName.find("crazy") != string::npos || sFName.find("nightmare") != string::npos || 
			sFName.find("crazydouble") != string::npos )
		{
			out.SetDifficulty( DIFFICULTY_HARD );
			if( !out.GetMeter() ) out.SetMeter( 14 ); // Set the meters to the Pump scale, not DDR.
		}
		else if( sFName.find("hard") != string::npos || sFName.find("freestyle") != string::npos ||
			sFName.find("double") != string::npos )
		{
			out.SetDifficulty( DIFFICULTY_MEDIUM );
			if( !out.GetMeter() ) out.SetMeter( 8 );
		}
		else if( sFName.find("easy") != string::npos || sFName.find("normal") != string::npos )
		{
			out.SetDifficulty( DIFFICULTY_EASY );
			if( !out.GetMeter() ) out.SetMeter( 4 );
		}
		else
		{
			out.SetDifficulty( DIFFICULTY_MEDIUM );
			if( !out.GetMeter() ) out.SetMeter( 8 );
		}

		out.m_StepsType = STEPS_TYPE_PUMP_SINGLE;

		/* Check for "halfdouble" before "double". */
		if( sFName.find("halfdouble") != string::npos || sFName.find("h_double") != string::npos )
			out.m_StepsType = STEPS_TYPE_PUMP_HALFDOUBLE;
		else if( sFName.find("double") != string::npos || sFName.find("nightmare") != string::npos )
			out.m_StepsType = STEPS_TYPE_PUMP_DOUBLE;
		else if( sFName.find("_1") != string::npos )
			out.m_StepsType = STEPS_TYPE_PUMP_SINGLE;
		else if( sFName.find("_2") != string::npos )
			out.m_StepsType = STEPS_TYPE_PUMP_COUPLE;
	}

	switch( out.m_StepsType )
	{
	case STEPS_TYPE_PUMP_SINGLE: notedata.SetNumTracks( 5 ); break;
	case STEPS_TYPE_PUMP_COUPLE: notedata.SetNumTracks( 10 ); break;
	case STEPS_TYPE_PUMP_DOUBLE: notedata.SetNumTracks( 10 ); break;
	case STEPS_TYPE_PUMP_HALFDOUBLE: notedata.SetNumTracks( 6 ); break;
	default: FAIL_M( ssprintf("%i", out.m_StepsType) );
	}

	int t = 0;
	int iHoldStartRow[13];
	for( t=0; t<13; t++ )
		iHoldStartRow[t] = -1;

	m_bTickChangeNeeded = false;
	int newTick = -1;
	m_fCurBeat = 0.0f;
	float prevBeat = 0.0f; // Used for hold tails.
	for( unsigned r=0; r<m_vNoteRows.size(); r++ )
	{
		RString& sRowString = m_vNoteRows[r];
		StripCrnl( sRowString );
		
		if( sRowString == "" )
			continue;	// skip

		// All 2s indicates the end of the song.
		if( sRowString == "2222222222222" )
		{
			// Finish any holds that didn't get...well, finished.		
			for( t=0; t < notedata.GetNumTracks(); t++ )
			{
				if( iHoldStartRow[t] != -1 )	// this ends the hold
				{
					if( iHoldStartRow[t] == BeatToNoteRow(prevBeat) )
						notedata.SetTapNote( t, iHoldStartRow[t], TAP_ORIGINAL_TAP );
					else
						notedata.AddHoldNote( t, iHoldStartRow[t], BeatToNoteRow(prevBeat) , TAP_ORIGINAL_HOLD_HEAD );
				}
			}
			break;
		}

		if( sRowString.size() != 13 )
		{	
			if( m_bKIUCompliant )
			{
				LOG->UserLog( "Song file", sPath, "has illegal syntax \"%s\" which can't be in KIU complient files.",
					      sRowString.c_str() );
				return false;
			}
			if( BeginsWith(sRowString, "|B") || BeginsWith(sRowString, "|D") )
			{
				// These don't have to be worried about here: the changes and stops were already added.
				continue;
			}
			if ( BeginsWith(sRowString, "|T") )
			{
				RString temp = sRowString.substr(2,sRowString.size()-3);
				newTick = atoi(temp);
				m_bTickChangeNeeded = true;
				continue;
			}
			else
			{
				LOG->UserLog( "Song file", sPath, "has a RowString with an improper length \"%s\"; corrupt notes ignored.",
					      sRowString.c_str() );
				return false;
			}
		}

		/* Half-doubles is offset; "0011111100000". */
		if( out.m_StepsType == STEPS_TYPE_PUMP_HALFDOUBLE )
			sRowString.erase( 0, 2 );

		// Update TICKCOUNT for Direct Move files.
		if( m_bTickChangeNeeded )
		{
			m_iTickCount = newTick;
			m_bTickChangeNeeded = false;
		}
		
		for( t=0; t < notedata.GetNumTracks(); t++ )
		{
			if( sRowString[t] == '4' )
			{
				/* Remember when each hold starts; ignore the middle. */
				if( iHoldStartRow[t] == -1 )
					iHoldStartRow[t] = BeatToNoteRow(m_fCurBeat);					
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
			default:
				LOG->UserLog( "Song file", sPath, "has an invalid row \"%s\"; corrupt notes ignored.",
					      sRowString.c_str() );
				return false;
			}

			notedata.SetTapNote(t, BeatToNoteRow(m_fCurBeat), tap);
		}
		prevBeat = m_fCurBeat;
		m_fCurBeat = prevBeat + 1.0f / m_iTickCount;		
	}

	/* We need to remove holes where the BPM increases. */
//	if( song.m_Timing.m_BPMSegments.size() > 1 )
//		RemoveHoles( notedata, song );

	out.SetNoteData( notedata );

	out.TidyUpData();

	out.SetSavedToDisk( true );	// we're loading from disk, so this is by definintion already saved

	return true;
}

void KSFLoader::GetApplicableFiles( const RString &sPath, vector<RString> &out )
{
	GetDirListing( sPath + RString("*.ksf"), out );
}

void KSFLoader::LoadTags( const RString &str, Song &out )
{
	/* str is either a #TITLE or a directory component.  Fill in missing information.
	 * str is either "title", "artist - title", or "artist - title - difficulty". */
	vector<RString> asBits;
	split( str, " - ", asBits, false );
	/* Ignore the difficulty, since we get that elsewhere. */
	if( asBits.size() == 3 && (
		!stricmp(asBits[2], "double") ||
		!stricmp(asBits[2], "easy") ||
		!stricmp(asBits[2], "normal") ||
		!stricmp(asBits[2], "hard") ||
		!stricmp(asBits[2], "crazy") ||
		!stricmp(asBits[2], "nightmare")) 
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

	/* Convert, if possible.  Most KSFs are in Korean encodings (CP942/EUC-KR). */
	if( !ConvertString( title, "korean" ) )
		title = "";
	if( !ConvertString( artist, "korean" ) )
		artist = "";

	if( out.m_sMainTitle == "" )
		out.m_sMainTitle = title;
	if( out.m_sArtist == "" )
		out.m_sArtist = artist;
}

bool KSFLoader::LoadGlobalData( const RString &sPath, Song &out )
{
	MsdFile msd;
	if( !msd.ReadFile( sPath, false ) )  // don't unescape
	{
		LOG->UserLog( "Song file", sPath, "couldn't be opened: %s", msd.GetError().c_str() );
		return false;
	}

	float SMGap1 = 0, SMGap2 = 0, BPM1 = -1, BPMPos2 = -1, BPM2 = -1, BPMPos3 = -1, BPM3 = -1;
	m_iTickCount = -1;
	m_bKIUCompliant = false;
	m_vNoteRows.clear();
	
	for( unsigned i=0; i < msd.GetNumValues(); i++ )
	{
		const MsdFile::value_t &sParams = msd.GetValue(i);
		RString sValueName = sParams[0];

		// handle the data
		if( 0==stricmp(sValueName,"TITLE") )
			LoadTags(sParams[1], out);
		else if( 0==stricmp(sValueName,"BPM") )
		{
			BPM1 = StringToFloat(sParams[1]);
			out.AddBPMSegment( BPMSegment(0, BPM1) );
		}
		else if( 0==stricmp(sValueName,"BPM2") )
		{
			m_bKIUCompliant = true;
			BPM2 = StringToFloat( sParams[1] );
		}
		else if( 0==stricmp(sValueName,"BPM3") )
		{
			m_bKIUCompliant = true;
			BPM3 = StringToFloat( sParams[1] );
		}
		else if( 0==stricmp(sValueName,"BUNKI") )
		{
			m_bKIUCompliant = true;
			BPMPos2 = StringToFloat( sParams[1] ) / 100.0f;
		}
		else if( 0==stricmp(sValueName,"BUNKI2") )
		{
			m_bKIUCompliant = true;
			BPMPos3 = StringToFloat( sParams[1] ) / 100.0f;
		}
		else if( 0==stricmp(sValueName,"STARTTIME") )
		{
			SMGap1 = -StringToFloat( sParams[1] )/100;
			out.m_Timing.m_fBeat0OffsetInSeconds = SMGap1;
		}
		// This is currently required for more accurate KIU BPM changes.  
		else if( 0==stricmp(sValueName,"STARTTIME2") )
		{
			m_bKIUCompliant = true;
			SMGap2 = -StringToFloat( sParams[1] )/100;
		}
		else if ( 0==stricmp(sValueName,"STARTTIME3") )
		{
			// STARTTIME3 only ensures this is a KIU complient simfile.
			m_bKIUCompliant = true;
		}
		else if ( 0==stricmp(sValueName,"TICKCOUNT") )
		{
			/* TICKCOUNT is will be used below if there are DM complient BPM changes and stops.
			 * It will be called again in LoadFromKSFFile for the actual steps. */
			m_iTickCount = atoi( sParams[1] );
			m_iTickCount = m_iTickCount > 0 ? m_iTickCount : 2;
		}
		else if ( 0==stricmp(sValueName,"STEP") )
		{
			/* STEP will always be the last header in a KSF file by design.  Due to
			 * the Direct Move syntax, it is best to get the rows of notes here. */
			RString theSteps = sParams[1];
			TrimLeft( theSteps );
			split( theSteps, "\n", m_vNoteRows, true );
		}

		else if( 0==stricmp(sValueName,"DIFFICULTY"))
		{
			/* DIFFICULTY is handled only in LoadFromKSFFile.  Ignore it here. */
			continue;
		}
		else
		{
			LOG->UserLog( "Song file", sPath, "has an unexpected value named \"%s\".",
				      sValueName.c_str() );
		}
	}

	/* BPM Change checks are done here.  If m_bKIUCompliant, it's short and sweet.
	 * Otherwise, the whole file has to be processed.  Right now, this is only 
	 * called once, for the initial file (often the Crazy steps).  Hopefully that
	 * will end up changing soon. */
	if( m_bKIUCompliant )
	{
		if( BPM2 > 0 && BPMPos2 > 0 )
		{
			const float BeatsPerSecond = BPM1 / 60.0f;
			const float beat = (BPMPos2 + SMGap1) * BeatsPerSecond;
			LOG->Trace( "BPM %f, BPS %f, BPMPos2 %f, beat %f",
				    BPM1, BeatsPerSecond, BPMPos2, beat );
			out.AddBPMSegment( BPMSegment(BeatToNoteRow(beat), BPM2) );
		}
		
		if( BPM3 > 0 && BPMPos3 > 0 )
		{
			const float BeatsPerSecond = BPM2 / 60.0f;
			//The line below isn't perfect, but works better than previous versions.
			const float beat = (BPMPos3 + SMGap2) * BeatsPerSecond;
			LOG->Trace( "BPM %f, BPS %f, BPMPos3 %f, beat %f",
			    BPM2, BeatsPerSecond, BPMPos3, beat );
			out.AddBPMSegment( BPMSegment(BeatToNoteRow(beat), BPM3) );
		}
	}

	else
	{
		int tickToChange = m_iTickCount;
		m_fCurBeat = 0.0f;
		float speedToChange = 0.0f, timeToStop = 0.0f;
		m_bDMRequired = m_bBPMChangeNeeded = m_bBPMStopNeeded = m_bTickChangeNeeded = false;

		for (unsigned i=0; i < m_vNoteRows.size(); i++)
		{
			RString& NoteRowString = m_vNoteRows[i];
			StripCrnl( NoteRowString );
			
			if( NoteRowString == "" )
				continue;	// Empty rows do us no good.
			
			if( NoteRowString == "2222222222222" ) // Row of 2s = end.  Confirm KIUCompliency here.
			{
				if (!m_bDMRequired)
					m_bKIUCompliant = true;
				break;
			}

			/* This is where the DMRequired test will take place. */
			if( NoteRowString.size() != 13)
			{
				if (BeginsWith(NoteRowString, "|T") || 
					BeginsWith(NoteRowString, "|B") || BeginsWith(NoteRowString, "|D") )
				{
					m_bDMRequired = true;
					RString temp = NoteRowString.substr(2,NoteRowString.size()-3);
					float numTemp = StringToFloat(temp);
					if (BeginsWith(NoteRowString, "|T")) 
					{
						m_bTickChangeNeeded = true;
						tickToChange = (int)numTemp;
						continue;
					}
					else if (BeginsWith(NoteRowString, "|B")) 
					{
						m_bBPMChangeNeeded = true;
						speedToChange = numTemp;
						continue;
					}
					else
					{
						m_bBPMStopNeeded = true;
						timeToStop = numTemp / 1000.0f;
						continue;
					}
				}
				else
				{
					/* Quit while we're ahead if any bad syntax is spotted. */
					LOG->UserLog( "Song file", sPath, "has an invalid RowString \"%s\".",
						      NoteRowString.c_str() );
					return false;
				}
			}

			if( m_bTickChangeNeeded )
			{
				m_iTickCount = tickToChange;
				m_bTickChangeNeeded = false;
			}
			if( m_bBPMChangeNeeded )
			{
				LOG->Trace( "Adding tempo change of %f BPM at beat %f", speedToChange, m_fCurBeat );
				out.AddBPMSegment( BPMSegment(BeatToNoteRow(m_fCurBeat), speedToChange) );
				m_bBPMChangeNeeded = false;
			}
			if( m_bBPMStopNeeded )
			{
				LOG->Trace( "Adding tempo freeze of %f seconds at beat %f", timeToStop, m_fCurBeat );
				out.AddStopSegment( StopSegment(BeatToNoteRow(m_fCurBeat),timeToStop) );
				m_bBPMStopNeeded = false;
			}
			m_fCurBeat += 1.0f / m_iTickCount;
		}
	}

	/* Try to fill in missing bits of information from the pathname. */
	{
		vector<RString> asBits;
		split( sPath, "/", asBits, true);

		ASSERT( asBits.size() > 1 );
		LoadTags( asBits[asBits.size()-2], out );
	}

	// search for music with song in the file name
	vector<RString> arrayPossibleMusic;
	GetDirListing( out.GetSongDir() + RString("song.mp3"), arrayPossibleMusic );
	GetDirListing( out.GetSongDir() + RString("song.ogg"), arrayPossibleMusic );
	GetDirListing( out.GetSongDir() + RString("song.wav"), arrayPossibleMusic );

	if( !arrayPossibleMusic.empty() )		// we found a match
		out.m_sMusicFile = arrayPossibleMusic[0];

	return true;
}

bool KSFLoader::LoadFromDir( const RString &sDir, Song &out )
{
	LOG->Trace( "Song::LoadFromKSFDir(%s)", sDir.c_str() );

	vector<RString> arrayKSFFileNames;
	GetDirListing( sDir + RString("*.ksf"), arrayKSFFileNames );

	/* We shouldn't have been called to begin with if there were no KSFs. */
	ASSERT( arrayKSFFileNames.size() );

	/* If only the first file is read, it will cause problems for other simfiles with
	 * different BPM changes and tickcounts.  This command will probably have to be
	 * changed in the future. */
	if( !LoadGlobalData(out.GetSongDir() + arrayKSFFileNames[0], out) )
		return false;

	// load the Steps from the rest of the KSF files
	for( unsigned i=0; i<arrayKSFFileNames.size(); i++ ) 
	{
		Steps* pNewNotes = new Steps;
		if( !LoadFromKSFFile(out.GetSongDir() + arrayKSFFileNames[i], *pNewNotes, out) )
		{
			delete pNewNotes;
			continue;
		}

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
