#include "global.h"
#include "NotesWriterDWI.h"
#include "NoteTypes.h"
#include "NoteData.h"
#include "RageUtil.h"
#include "RageLog.h"

char NotesWriterDWI::NotesToDWIChar( bool bCol1, bool bCol2, bool bCol3, bool bCol4, bool bCol5, bool bCol6 )
{
	struct DWICharLookup {
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
	return '0';
}

char NotesWriterDWI::NotesToDWIChar( bool bCol1, bool bCol2, bool bCol3, bool bCol4 )
{
	return NotesToDWIChar( bCol1, 0, bCol2, bCol3, 0, bCol4 );
}

CString NotesWriterDWI::NotesToDWIString( TapNote cNoteCol1, TapNote cNoteCol2, TapNote cNoteCol3, TapNote cNoteCol4, TapNote cNoteCol5, TapNote cNoteCol6 )
{
	char cShow = NotesToDWIChar( 
		cNoteCol1!=TAP_EMPTY,
		cNoteCol2!=TAP_EMPTY,
		cNoteCol3!=TAP_EMPTY,
		cNoteCol4!=TAP_EMPTY,
		cNoteCol5!=TAP_EMPTY,
		cNoteCol6!=TAP_EMPTY );
	char cHold = NotesToDWIChar( 
		cNoteCol1==TAP_HOLD_HEAD, 
		cNoteCol2==TAP_HOLD_HEAD, 
		cNoteCol3==TAP_HOLD_HEAD, 
		cNoteCol4==TAP_HOLD_HEAD,
		cNoteCol5==TAP_HOLD_HEAD,
		cNoteCol6==TAP_HOLD_HEAD );

	if( cHold != '0' )
		return ssprintf( "%c!%c", cShow, cHold );
	else
		return ssprintf( "%c", cShow );
}

CString NotesWriterDWI::NotesToDWIString( TapNote cNoteCol1, TapNote cNoteCol2, TapNote cNoteCol3, TapNote cNoteCol4 )
{
	return NotesToDWIString( cNoteCol1, TAP_EMPTY, cNoteCol2, cNoteCol3, TAP_EMPTY, cNoteCol4 );
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
		default:
			ASSERT(0);
			// fall though
		case NOTE_TYPE_32ND:
		case NOTE_TYPE_INVALID:
			fprintf( fp, "{" );
			fCurrentIncrementer = 1.0/64 * BEATS_PER_MEASURE;
			break;
		}

		double fFirstBeatInMeasure = m * BEATS_PER_MEASURE;
		double fLastBeatInMeasure = (m+1) * BEATS_PER_MEASURE;

		for( double b=fFirstBeatInMeasure; b<=fLastBeatInMeasure-1/64.0f; b+=fCurrentIncrementer )	// need the -0.0001 to account for rounding errors
		{
			int row = BeatToNoteRow( (float)b );

			switch( out.m_NotesType )
			{
			case NOTES_TYPE_DANCE_SINGLE:
			case NOTES_TYPE_DANCE_COUPLE:
			case NOTES_TYPE_DANCE_DOUBLE:
				fprintf( fp, NotesToDWIString( 
					notedata.GetTapNote(start+0, row), 
					notedata.GetTapNote(start+1, row),
					notedata.GetTapNote(start+2, row),
					notedata.GetTapNote(start+3, row) ) );

				// Blank out the notes so we don't write them again if the incrementer is small
				notedata.SetTapNote(start+0, row, TAP_EMPTY);
				notedata.SetTapNote(start+1, row, TAP_EMPTY);
				notedata.SetTapNote(start+2, row, TAP_EMPTY);
				notedata.SetTapNote(start+3, row, TAP_EMPTY);
				break;
			case NOTES_TYPE_DANCE_SOLO:
				fprintf( fp, NotesToDWIString( 
					notedata.GetTapNote(0, row),
					notedata.GetTapNote(1, row),
					notedata.GetTapNote(2, row),
					notedata.GetTapNote(3, row),
					notedata.GetTapNote(4, row),
					notedata.GetTapNote(5, row) ) );

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
		default:
			ASSERT(0);
			// fall though
		case NOTE_TYPE_32ND:
		case NOTE_TYPE_INVALID:
			fprintf( fp, "}" );
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
	case DIFFICULTY_BEGINNER://	fprintf( fp, "BEGINNER:" ); break;
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
	FILE* fp = fopen( sPath, "w" );	
	if( fp == NULL )
		RageException::Throw( "Error opening song file '%s' for writing.", sPath.GetString() );

	/* Write transliterations, if we have them, since DWI doesn't support UTF-8. */
	fprintf( fp, "#TITLE:%s;\n", out.GetTranslitMainTitle().GetString() );
	fprintf( fp, "#ARTIST:%s;\n", out.GetTranslitArtist().GetString() );
	ASSERT( out.m_BPMSegments[0].m_fStartBeat == 0 );
	fprintf( fp, "#BPM:%.3f;\n", out.m_BPMSegments[0].m_fBPM );
	fprintf( fp, "#GAP:%d;\n", int(-roundf( out.m_fBeat0OffsetInSeconds*1000 )) );
	fprintf( fp, "#SAMPLESTART:%.3f;\n", out.m_fMusicSampleStartSeconds );
	fprintf( fp, "#SAMPLELENGTH:%.3f;\n", out.m_fMusicSampleLengthSeconds );

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
