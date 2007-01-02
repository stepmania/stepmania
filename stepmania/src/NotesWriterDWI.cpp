#include "global.h"
#include "NotesWriterDWI.h"
#include "NoteTypes.h"
#include "NoteData.h"
#include "RageUtil.h"
#include "RageLog.h"
#include "RageFileManager.h"
#include "RageFile.h"
#include "NoteDataUtil.h"
#include "RageFile.h"
#include "song.h"
#include "Steps.h"

static RString OptimizeDWIString( RString holds, RString taps );

/* Output is an angle bracket expression without angle brackets, eg. "468". */
static RString NotesToDWIString( const TapNote tnCols[6] )
{
	const char dirs[] = { '4', 'C', '2', '8', 'D', '6' };
	RString taps, holds, ret;
	for( int col = 0; col < 6; ++col )
	{
		switch( tnCols[col].type )
		{
		case TapNote::empty:
		case TapNote::mine:
			continue;
		}

		if( tnCols[col].type == TapNote::hold_head )
			holds += dirs[col];
		else
			taps += dirs[col];
	}

	if( holds.size() + taps.size() == 0 )
		return "0";

//	RString combine = taps;
//	for( unsigned i = 0; i < holds.size(); ++i )
//		combine += ssprintf("%c!%c", holds[i], holds[i]);

//	if( holds.size() + taps.size() > 1 )
//		combine = ssprintf("<%s>", combine.c_str() );

//	return combine;

	/* More than one. */
	return OptimizeDWIString( holds, taps );
/*	struct DWICharLookup {
		char c;
		bool bCol[6];	
	} const lookup[] = {
		{ '0', { 0, 0, 0, 0, 0, 0 } },
		{ '1', { 1, 0, 1, 0, 0, 0 } },
		{ '2', { 0, 0, 1, 0, 0, 0 } },
		{ '3', { 0, 0, 1, 0, 0, 1 } },
		{ '4', { 1, 0, 0, 0, 0, 0 } },
		{ '6', { 0, 0, 0, 0, 0, 1 } },
		{ '7', { 1, 0, 0, 1, 0, 0 } },
		{ '8', { 0, 0, 0, 1, 0, 0 } },
		{ '9', { 0, 0, 0, 1, 0, 1 } },
		{ 'A', { 0, 0, 1, 1, 0, 0 } },
		{ 'B', { 1, 0, 0, 0, 0, 1 } },
		{ 'C', { 0, 1, 0, 0, 0, 0 } },
		{ 'D', { 0, 0, 0, 0, 1, 0 } },
		{ 'E', { 1, 1, 0, 0, 0, 0 } },
		{ 'F', { 0, 1, 1, 0, 0, 0 } },
		{ 'G', { 0, 1, 0, 1, 0, 0 } },
		{ 'H', { 0, 1, 0, 0, 0, 1 } },
		{ 'I', { 1, 0, 0, 0, 1, 0 } },
		{ 'J', { 0, 0, 1, 0, 1, 0 } },
		{ 'K', { 0, 0, 0, 1, 1, 0 } },
		{ 'L', { 0, 0, 0, 0, 1, 1 } },
		{ 'M', { 0, 1, 0, 0, 1, 0 } },
	};
	const int iNumLookups = sizeof(lookup) / sizeof(*lookup);

	for( int i=0; i<iNumLookups; i++ )
	{
		const DWICharLookup& l = lookup[i];
		if( l.bCol[0]==bCol1 && l.bCol[1]==bCol2 && l.bCol[2]==bCol3 && l.bCol[3]==bCol4 && l.bCol[4]==bCol5 && l.bCol[5]==bCol6 )
			return l.c;
	}
	LOG->Warn( "Failed to find the DWI character for the row %d %d %d %d %d %d", bCol1, bCol2, bCol3, bCol4, bCol5, bCol6 );
	return '0';*/
}

static RString NotesToDWIString( TapNote tnCol1, TapNote tnCol2, TapNote tnCol3, TapNote tnCol4, TapNote tnCol5, TapNote tnCol6 )
{
	TapNote tnCols[6];
	tnCols[0] = tnCol1;
	tnCols[1] = tnCol2;
	tnCols[2] = tnCol3;
	tnCols[3] = tnCol4;
	tnCols[4] = tnCol5;
	tnCols[5] = tnCol6;
	return NotesToDWIString( tnCols );
}

static RString NotesToDWIString( TapNote tnCol1, TapNote tnCol2, TapNote tnCol3, TapNote tnCol4 )
{
	return NotesToDWIString( tnCol1, TAP_EMPTY, tnCol2, tnCol3, TAP_EMPTY, tnCol4 );
}

static char OptimizeDWIPair( char c1, char c2 )
{
	typedef pair<char,char> cpair;
	static map< cpair, char > joins;
	static bool Initialized = false;
	if(!Initialized)
	{
		Initialized = true;
		/* The first character in the pair is always the lowest. */
		joins[ cpair('2', '4') ] = '1';
		joins[ cpair('2', '6') ] = '3';
		joins[ cpair('4', '8') ] = '7';
		joins[ cpair('6', '8') ] = '9';
		joins[ cpair('2', '8') ] = 'A';
		joins[ cpair('4', '6') ] = 'B';
		joins[ cpair('C', 'D') ] = 'M';
		joins[ cpair('4', 'C') ] = 'E';
		joins[ cpair('2', 'C') ] = 'F';
		joins[ cpair('8', 'C') ] = 'G';
		joins[ cpair('6', 'C') ] = 'H';
		joins[ cpair('4', 'D') ] = 'I';
		joins[ cpair('2', 'D') ] = 'J';
		joins[ cpair('8', 'D') ] = 'K';
		joins[ cpair('6', 'D') ] = 'L';
	}

	if( c1 > c2 )
		swap( c1, c2 );

	map< cpair, char >::const_iterator it = joins.find( cpair(c1, c2) );
	ASSERT( it != joins.end() );

	return it->second;
}

RString OptimizeDWIString( RString holds, RString taps )
{
	/* First, sort the holds and taps in ASCII order.  This puts 2468 first.
	 * This way 1379 combinations will always be found first, so we'll always
	 * do eg. 1D, not 2I. */
	sort( holds.begin(), holds.end() );
	sort( taps.begin(), taps.end() );

	/* Combine characters as much as possible. */
	RString comb_taps, comb_holds;

	/* 24 -> 1 */
	while( taps.size() > 1 )
	{
		comb_taps += OptimizeDWIPair( taps[0], taps[1] );
		taps.erase(0, 2);
	}

	/* 2!24!4 -> 1!1 */
	while( holds.size() > 1 )
	{
		const char to = OptimizeDWIPair( holds[0], holds[1] );
		holds.erase(0, 2);
		comb_holds += ssprintf( "%c!%c", to, to );
	}

	ASSERT( taps.size() <= 1 );
	ASSERT( holds.size() <= 1 );

	/* 24!4 -> 1!4 */
	while( holds.size() == 1 && taps.size() == 1 )
	{
		const char to = OptimizeDWIPair( taps[0], holds[0] );
		comb_holds += ssprintf( "%c!%c", to, holds[0] );
		taps.erase(0, 1);
		holds.erase(0, 1);
	}

	/* Now we have at most one single tap and one hold remaining, and any
	 * number of taps and holds in comb_taps and comb_holds. */
	RString ret;
	ret += taps;
	ret += comb_taps;
	if( holds.size() == 1 )
		ret += ssprintf( "%c!%c", holds[0], holds[0] );
	ret += comb_holds;

	if( ret.size() == 1 || (ret.size() == 3 && ret[1] == '!') )
		return ret;
	else
		return ssprintf( "<%s>", ret.c_str() );
}

static void WriteDWINotesField( RageFile &f, const Steps &out, int start )
{
	NoteData notedata;
	out.GetNoteData( notedata );
	NoteDataUtil::InsertHoldTails( notedata );

	const int iLastMeasure = int( notedata.GetLastBeat()/BEATS_PER_MEASURE );
	for( int m=0; m<=iLastMeasure; m++ )	// foreach measure
	{
		NoteType nt = NoteDataUtil::GetSmallestNoteTypeForMeasure( notedata, m );

		double fCurrentIncrementer = 0;
		switch( nt )
		{
		case NOTE_TYPE_4TH:
		case NOTE_TYPE_8TH:	
			fCurrentIncrementer = 1.0/8 * BEATS_PER_MEASURE;
			break;
		case NOTE_TYPE_12TH:
		case NOTE_TYPE_24TH:
			f.Write( "[" );
			fCurrentIncrementer = 1.0/24 * BEATS_PER_MEASURE;
			break;
		case NOTE_TYPE_16TH:
			f.Write( "(" );
			fCurrentIncrementer = 1.0/16 * BEATS_PER_MEASURE;
			break;
		case NOTE_TYPE_32ND:
		case NOTE_TYPE_64TH:
			f.Write( "{" );
			fCurrentIncrementer = 1.0/64 * BEATS_PER_MEASURE;
			break;
		case NOTE_TYPE_48TH:
		case NOTE_TYPE_192ND:
		case NoteType_Invalid:
			// since, for whatever reason, the only way to do
			// 48ths is through a block of 192nds...
			f.Write(  "`" );
			fCurrentIncrementer = 1.0/192 * BEATS_PER_MEASURE;
			break;
		default:
			ASSERT_M(0, ssprintf("nt = %d",nt) );
			break;
		}

		double fFirstBeatInMeasure = m * BEATS_PER_MEASURE;
		double fLastBeatInMeasure = (m+1) * BEATS_PER_MEASURE;

		for( double b=fFirstBeatInMeasure; b<=fLastBeatInMeasure-1/64.0f; b+=fCurrentIncrementer )	// need the -0.0001 to account for rounding errors
		{
			int row = BeatToNoteRow( (float)b );

			RString str;
			switch( out.m_StepsType )
			{
			case STEPS_TYPE_DANCE_SINGLE:
			case STEPS_TYPE_DANCE_COUPLE:
			case STEPS_TYPE_DANCE_DOUBLE:
				str = NotesToDWIString( 
					notedata.GetTapNote(start+0, row), 
					notedata.GetTapNote(start+1, row),
					notedata.GetTapNote(start+2, row),
					notedata.GetTapNote(start+3, row) );

				// Blank out the notes so we don't write them again if the incrementer is small
				notedata.SetTapNote(start+0, row, TAP_EMPTY);
				notedata.SetTapNote(start+1, row, TAP_EMPTY);
				notedata.SetTapNote(start+2, row, TAP_EMPTY);
				notedata.SetTapNote(start+3, row, TAP_EMPTY);
				break;
			case STEPS_TYPE_DANCE_SOLO:
				str = NotesToDWIString( 
					notedata.GetTapNote(0, row),
					notedata.GetTapNote(1, row),
					notedata.GetTapNote(2, row),
					notedata.GetTapNote(3, row),
					notedata.GetTapNote(4, row),
					notedata.GetTapNote(5, row) );

				// Blank out the notes so we don't write them again if the incrementer is small
				notedata.SetTapNote(start+0, row, TAP_EMPTY);
				notedata.SetTapNote(start+1, row, TAP_EMPTY);
				notedata.SetTapNote(start+2, row, TAP_EMPTY);
				notedata.SetTapNote(start+3, row, TAP_EMPTY);
				notedata.SetTapNote(start+4, row, TAP_EMPTY);
				notedata.SetTapNote(start+5, row, TAP_EMPTY);
				break;
			default:
				ASSERT(0);	// not a type supported by DWI.  We shouldn't have called in here if that's the case
			}
			f.Write( str );
		}

		switch( nt )
		{
		case NOTE_TYPE_4TH:
		case NOTE_TYPE_8TH:	
			break;
		case NOTE_TYPE_12TH:
		case NOTE_TYPE_24TH:
			f.Write( "]" );
			break;
		case NOTE_TYPE_16TH:
			f.Write( ")" );
			break;
		case NOTE_TYPE_32ND:
		case NOTE_TYPE_64TH:
			f.Write( "}" );
			break;
		case NOTE_TYPE_48TH:
		case NOTE_TYPE_192ND:
		case NoteType_Invalid:
			f.Write( "'" );
			break;
		default:
			ASSERT(0);
			// fall though
		}
		f.PutLine( "" );
	}
}

static bool WriteDWINotesTag( RageFile &f, const Steps &out )
{
	if( out.GetDifficulty() == DIFFICULTY_EDIT )
		return false;	// not supported by DWI

	/* Flush dir cache when writing steps, so the old size isn't cached. */
	FILEMAN->FlushDirCache( Dirname(f.GetRealPath()) );

	LOG->Trace( "Steps::WriteDWINotesTag" );

	switch( out.m_StepsType )
	{
	case STEPS_TYPE_DANCE_SINGLE:	f.Write( "#SINGLE:" );	break;
	case STEPS_TYPE_DANCE_COUPLE:	f.Write( "#COUPLE:" );	break;
	case STEPS_TYPE_DANCE_DOUBLE:	f.Write( "#DOUBLE:" );	break;
	case STEPS_TYPE_DANCE_SOLO:		f.Write( "#SOLO:" );	break;
	default:	return false;	// not a type supported by DWI
	}

	switch( out.GetDifficulty() )
	{
	case DIFFICULTY_BEGINNER:	f.Write( "BEGINNER:" ); break;
	case DIFFICULTY_EASY:		f.Write( "BASIC:" );	break;
	case DIFFICULTY_MEDIUM:		f.Write( "ANOTHER:" );	break;
	case DIFFICULTY_HARD:		f.Write( "MANIAC:" );	break;
	case DIFFICULTY_CHALLENGE:	f.Write( "SMANIAC:" );	break;
	default:	ASSERT(0);	return false;
	}

	f.PutLine( ssprintf("%d:", out.GetMeter()) );
	return true;
}

bool NotesWriterDWI::Write( RString sPath, const Song &out )
{
	RageFile f;
	if( !f.Open( sPath, RageFile::WRITE ) )
	{
		LOG->UserLog( "Song file", sPath, "couldn't be opened for writing: %s", f.GetError().c_str() );
		return false;
	}

	/* Write transliterations, if we have them, since DWI doesn't support UTF-8. */
	f.PutLine( ssprintf("#TITLE:%s;", DwiEscape(out.GetTranslitFullTitle()).c_str()) );
	f.PutLine( ssprintf("#ARTIST:%s;", DwiEscape(out.GetTranslitArtist()).c_str()) );
	ASSERT( out.m_Timing.m_BPMSegments[0].m_iStartIndex == 0 );
	f.PutLine( ssprintf("#FILE:%s;", DwiEscape(out.m_sMusicFile).c_str()) );
	f.PutLine( ssprintf("#BPM:%.3f;", out.m_Timing.m_BPMSegments[0].GetBPM()) );
	f.PutLine( ssprintf("#GAP:%ld;", -lroundf( out.m_Timing.m_fBeat0OffsetInSeconds*1000 )) );
	f.PutLine( ssprintf("#SAMPLESTART:%.3f;", out.m_fMusicSampleStartSeconds) );
	f.PutLine( ssprintf("#SAMPLELENGTH:%.3f;", out.m_fMusicSampleLengthSeconds) );
	if( out.m_sCDTitleFile.size() )
		f.PutLine( ssprintf("#CDTITLE:%s;", DwiEscape(out.m_sCDTitleFile).c_str()) );
	switch( out.m_DisplayBPMType )
	{
	case Song::DISPLAY_ACTUAL:
		// write nothing
		break;
	case Song::DISPLAY_SPECIFIED:
		if( out.m_fSpecifiedBPMMin == out.m_fSpecifiedBPMMax )
			f.PutLine( ssprintf("#DISPLAYBPM:%i;\n", (int) out.m_fSpecifiedBPMMin) );
		else
			f.PutLine( ssprintf("#DISPLAYBPM:%i..%i;\n", (int) out.m_fSpecifiedBPMMin, (int) out.m_fSpecifiedBPMMax) );
		break;
	case Song::DISPLAY_RANDOM:
		f.PutLine( "#DISPLAYBPM:*" );
		break;
	}

	if( !out.m_Timing.m_StopSegments.empty() )
	{
		f.Write( "#FREEZE:" );

		for( unsigned i=0; i<out.m_Timing.m_StopSegments.size(); i++ )
		{
			const StopSegment &fs = out.m_Timing.m_StopSegments[i];
			f.Write( ssprintf("%.3f=%.3f", fs.m_iStartRow * 4.0f / ROWS_PER_BEAT,
				roundf(fs.m_fStopSeconds*1000)) );
			if( i != out.m_Timing.m_StopSegments.size()-1 )
				f.Write( "," );
		}
		f.PutLine( ";" );
	}

	if( out.m_Timing.m_BPMSegments.size() > 1)
	{
		f.Write( "#CHANGEBPM:" );
		for( unsigned i=1; i<out.m_Timing.m_BPMSegments.size(); i++ )
		{
			const BPMSegment &bs = out.m_Timing.m_BPMSegments[i];
			f.Write( ssprintf("%.3f=%.3f", bs.m_iStartIndex * 4.0f / ROWS_PER_BEAT, bs.GetBPM() ) );
			if( i != out.m_Timing.m_BPMSegments.size()-1 )
				f.Write( "," );
		}
		f.PutLine( ";" );
	}

	const vector<Steps*>& vpSteps = out.GetAllSteps();
	for( unsigned i=0; i<vpSteps.size(); i++ ) 
	{
		const Steps* pSteps = vpSteps[i];
		if( pSteps->IsAutogen() )
			continue;	// don't save autogen notes

		if( !WriteDWINotesTag( f, *pSteps ))
			continue;

		WriteDWINotesField( f, *pSteps, 0 );
		if( pSteps->m_StepsType==STEPS_TYPE_DANCE_DOUBLE ||
		    pSteps->m_StepsType==STEPS_TYPE_DANCE_COUPLE )
		{
			f.PutLine( ":" );
			WriteDWINotesField( f, *pSteps, 4 );
		}

		f.PutLine( ";" );
	}
	
	return true;
}

/*
 * (c) 2001-2006 Chris Danford, Glenn Maynard
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
