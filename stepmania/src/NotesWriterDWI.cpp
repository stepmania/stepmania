#include "global.h"
#include "NotesWriterDWI.h"
#include "NoteTypes.h"
#include "NoteData.h"
#include "RageUtil.h"
#include "RageLog.h"
#include "RageFile.h"

/* Output is an angle bracket expression without angle brackets, eg. "468". */
CString NotesWriterDWI::NotesToDWIString( const TapNote cNoteCols[6] )
{
	const char dirs[] = { '4', 'C', '2', '8', 'D', '6' };
	CString taps, holds, ret;
	for( int col = 0; col < 6; ++col )
	{
		if( cNoteCols[col] == TAP_EMPTY )
			continue;

		if( cNoteCols[col] == TAP_HOLD_HEAD )
			holds += dirs[col];
		else
			taps += dirs[col];
	}

	if( holds.size() + taps.size() == 0 )
		return "0";

//	CString combine = taps;
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

CString NotesWriterDWI::NotesToDWIString( TapNote cNoteCol1, TapNote cNoteCol2, TapNote cNoteCol3, TapNote cNoteCol4, TapNote cNoteCol5, TapNote cNoteCol6 )
{
	const TapNote cNoteCols[6] = {
		cNoteCol1, cNoteCol2, cNoteCol3, cNoteCol4, cNoteCol5, cNoteCol6
	};

	return NotesToDWIString( cNoteCols );
}

CString NotesWriterDWI::NotesToDWIString( TapNote cNoteCol1, TapNote cNoteCol2, TapNote cNoteCol3, TapNote cNoteCol4 )
{
	return NotesToDWIString( cNoteCol1, TAP_EMPTY, cNoteCol2, cNoteCol3, TAP_EMPTY, cNoteCol4 );
}

char NotesWriterDWI::OptimizeDWIPair( char c1, char c2 )
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

CString NotesWriterDWI::OptimizeDWIString( CString holds, CString taps )
{
	/* First, sort the holds and taps in ASCII order.  This puts 2468 first.
	 * This way 1379 combinations will always be found first, so we'll always
	 * do eg. 1D, not 2I. */
	sort( holds.begin(), holds.end() );
	sort( taps.begin(), taps.end() );

	/* Combine characters as much as possible. */
	CString comb_taps, comb_holds;

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
	CString ret;
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

void NotesWriterDWI::WriteDWINotesField( FILE* fp, const Notes &out, int start )
{
	NoteData notedata;
	out.GetNoteData( &notedata );
	notedata.ConvertHoldNotesTo2sAnd3s();

	const int iLastMeasure = int( notedata.GetLastBeat()/BEATS_PER_MEASURE );
	for( int m=0; m<=iLastMeasure; m++ )	// foreach measure
	{
		NoteType nt = NoteDataUtil::GetSmallestNoteTypeForMeasure( notedata, m );

		double fCurrentIncrementer;
		switch( nt )
		{
		case NOTE_TYPE_4TH:
		case NOTE_TYPE_8TH:	
			fCurrentIncrementer = 1.0/8 * BEATS_PER_MEASURE;
			break;
		case NOTE_TYPE_12TH:
		case NOTE_TYPE_24TH:
			fprintf( fp, "[" );
			fCurrentIncrementer = 1.0/24 * BEATS_PER_MEASURE;
			break;
		case NOTE_TYPE_16TH:
			fprintf( fp, "(" );
			fCurrentIncrementer = 1.0/16 * BEATS_PER_MEASURE;
			break;
		case NOTE_TYPE_32ND:
			/* XXX: This is valid for 64ths, too, but we don't
			 * have NOTE_TYPE_64TH, so we'll write 64ths as
			 * 192nds. */
			fprintf( fp, "{" );
			fCurrentIncrementer = 1.0/64 * BEATS_PER_MEASURE;
			break;
		default:
			ASSERT(0);
			// fall though
		case NOTE_TYPE_INVALID:
			fprintf( fp, "`" );
			fCurrentIncrementer = 1.0/192 * BEATS_PER_MEASURE;
		}

		double fFirstBeatInMeasure = m * BEATS_PER_MEASURE;
		double fLastBeatInMeasure = (m+1) * BEATS_PER_MEASURE;

		for( double b=fFirstBeatInMeasure; b<=fLastBeatInMeasure-1/64.0f; b+=fCurrentIncrementer )	// need the -0.0001 to account for rounding errors
		{
			int row = BeatToNoteRow( (float)b );

			CString str;
			switch( out.m_NotesType )
			{
			case NOTES_TYPE_DANCE_SINGLE:
			case NOTES_TYPE_DANCE_COUPLE:
			case NOTES_TYPE_DANCE_DOUBLE:
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
			case NOTES_TYPE_DANCE_SOLO:
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
			fprintf( fp, "%s", str.c_str() );
		}

		switch( nt )
		{
		case NOTE_TYPE_4TH:
		case NOTE_TYPE_8TH:	
			break;
		case NOTE_TYPE_12TH:
		case NOTE_TYPE_24TH:
			fprintf( fp, "]" );
			break;
		case NOTE_TYPE_16TH:
			fprintf( fp, ")" );
			break;
		case NOTE_TYPE_32ND:
			fprintf( fp, "}" );
			break;
		default:
			ASSERT(0);
			// fall though
		case NOTE_TYPE_INVALID:
			fprintf( fp, "'" );
			break;
		}
		fprintf( fp, "\n" );
	}
}

bool NotesWriterDWI::WriteDWINotesTag( FILE* fp, const Notes &out )
{
	LOG->Trace( "Notes::WriteDWINotesTag" );

	switch( out.m_NotesType )
	{
	case NOTES_TYPE_DANCE_SINGLE:	fprintf( fp, "#SINGLE:" );	break;
	case NOTES_TYPE_DANCE_COUPLE:	fprintf( fp, "#COUPLE:" );	break;
	case NOTES_TYPE_DANCE_DOUBLE:	fprintf( fp, "#DOUBLE:" );	break;
	case NOTES_TYPE_DANCE_SOLO:		fprintf( fp, "#SOLO:" );	break;
	default:	return false;	// not a type supported by DWI
	}

	switch( out.GetDifficulty() )
	{
	case DIFFICULTY_BEGINNER:	fprintf( fp, "BEGINNER:" ); break;
	case DIFFICULTY_EASY:		fprintf( fp, "BASIC:" );	break;
	case DIFFICULTY_MEDIUM:		fprintf( fp, "ANOTHER:" );	break;
	case DIFFICULTY_HARD:		fprintf( fp, "MANIAC:" );	break;
	case DIFFICULTY_CHALLENGE:	fprintf( fp, "SMANIAC:" );	break;
	default:	ASSERT(0);	return false;
	}

	fprintf( fp, "%d:\n", out.GetMeter() );
	return true;
}

bool NotesWriterDWI::Write( CString sPath, const Song &out )
{
	FILE* fp = Ragefopen( sPath, "w" );	
	if( fp == NULL )
		RageException::Throw( "Error opening song file '%s' for writing.", sPath.c_str() );

	/* Write transliterations, if we have them, since DWI doesn't support UTF-8. */
	fprintf( fp, "#TITLE:%s;\n", out.GetFullTranslitTitle().c_str() );
	fprintf( fp, "#ARTIST:%s;\n", out.GetTranslitArtist().c_str() );
	ASSERT( out.m_BPMSegments[0].m_fStartBeat == 0 );
	fprintf( fp, "#BPM:%.3f;\n", out.m_BPMSegments[0].m_fBPM );
	fprintf( fp, "#GAP:%d;\n", int(-roundf( out.m_fBeat0OffsetInSeconds*1000 )) );
	fprintf( fp, "#SAMPLESTART:%.3f;\n", out.m_fMusicSampleStartSeconds );
	fprintf( fp, "#SAMPLELENGTH:%.3f;\n", out.m_fMusicSampleLengthSeconds );
	if( out.m_sCDTitleFile.size() )
		fprintf( fp, "#CDTITLE:%s;\n", out.m_sCDTitleFile.c_str() );
	switch( out.m_DisplayBPMType )
	{
	case Song::DISPLAY_ACTUAL:
		// write nothing
		break;
	case Song::DISPLAY_SPECIFIED:
		if( out.m_fSpecifiedBPMMin == out.m_fSpecifiedBPMMax )
			fprintf( fp, "#DISPLAYBPM:%i;\n", (int) out.m_fSpecifiedBPMMin );
		else
			fprintf( fp, "#DISPLAYBPM:%i..%i;\n", (int) out.m_fSpecifiedBPMMin, (int) out.m_fSpecifiedBPMMax );
		break;
	case Song::DISPLAY_RANDOM:
		fprintf( fp, "#DISPLAYBPM:*" );
		break;
	}

	if( !out.m_StopSegments.empty() )
	{
		fprintf( fp, "#FREEZE:" );

		for( unsigned i=0; i<out.m_StopSegments.size(); i++ )
		{
			const StopSegment &fs = out.m_StopSegments[i];
			fprintf( fp, "%.3f=%.3f", BeatToNoteRow( fs.m_fStartBeat ) * 4.0f / ROWS_PER_BEAT,
				roundf(fs.m_fStopSeconds*1000) );
			if( i != out.m_StopSegments.size()-1 )
				fprintf( fp, "," );
		}
		fprintf( fp, ";\n" );
	}

	if( out.m_BPMSegments.size() > 1)
	{
		fprintf( fp, "#CHANGEBPM:" );
		for( unsigned i=1; i<out.m_BPMSegments.size(); i++ )
		{
			const BPMSegment &bs = out.m_BPMSegments[i];
			fprintf( fp, "%.3f=%.3f", BeatToNoteRow( bs.m_fStartBeat ) * 4.0f / ROWS_PER_BEAT, bs.m_fBPM );
			if( i != out.m_BPMSegments.size()-1 )
				fprintf( fp, "," );
		}
		fprintf( fp, ";\n" );
	}

	for( unsigned i=0; i<out.m_apNotes.size(); i++ ) 
	{
		if( out.m_apNotes[i]->IsAutogen() )
			continue;	// don't save autogen notes

		if(!WriteDWINotesTag( fp, *out.m_apNotes[i] ))
			continue;

		WriteDWINotesField( fp, *out.m_apNotes[i], 0 );
		if(out.m_apNotes[i]->m_NotesType==NOTES_TYPE_DANCE_DOUBLE ||
		   out.m_apNotes[i]->m_NotesType==NOTES_TYPE_DANCE_COUPLE)
		{
			fprintf( fp, ":\n" );
			WriteDWINotesField( fp, *out.m_apNotes[i], 4 );
		}

		fprintf( fp, ";\n" );
	}
	
	fclose( fp );

	return true;
}
/*
 * Copyright (c) 2003 by the person(s) listed below.  All rights reserved.
 *	Chris Danford
 *	Glenn Maynard
 */
