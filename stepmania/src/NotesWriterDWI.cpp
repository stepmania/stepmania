#include "stdafx.h"
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
		{ '0', 0, 0, 0, 0, 0, 0 },
		{ '1', 0, 1, 1, 0, 0, 0 },
		{ '2', 0, 0, 1, 0, 0, 0 },
		{ '3', 0, 0, 1, 0, 1, 0 },
		{ '4', 0, 1, 0, 0, 0, 0 },
		{ '6', 0, 0, 0, 0, 1, 0 },
		{ '7', 0, 1, 0, 1, 0, 0 },
		{ '8', 0, 0, 0, 1, 0, 0 },
		{ '9', 0, 0, 0, 1, 1, 0 },
		{ 'A', 0, 0, 1, 1, 0, 0 },
		{ 'B', 0, 1, 0, 0, 1, 0 },
		{ 'C', 0, 1, 0, 0, 0, 0 },
		{ 'D', 0, 0, 0, 0, 1, 0 },
		{ 'E', 1, 1, 0, 0, 0, 0 },
		{ 'F', 0, 1, 1, 0, 0, 0 },
		{ 'G', 0, 1, 0, 1, 0, 0 },
		{ 'H', 0, 1, 0, 0, 0, 1 },
		{ 'I', 1, 0, 0, 0, 1, 0 },
		{ 'J', 0, 0, 1, 0, 1, 0 },
		{ 'K', 0, 0, 0, 1, 1, 0 },
		{ 'L', 0, 0, 0, 0, 1, 1 },
		{ 'M', 0, 1, 0, 0, 1, 0 },
	};
	const int iNumLookups = sizeof(lookup) / sizeof(*lookup);

	for( int i=0; i<iNumLookups; i++ )
	{
		const DWICharLookup& l = lookup[i];
		if( l.bCol[0]==bCol1 && l.bCol[1]==bCol2 && l.bCol[2]==bCol3 && l.bCol[3]==bCol4 && l.bCol[4]==bCol5 && l.bCol[5]==bCol6 )
			return l.c;
	}
	ASSERT(0);
	return '0';
}

char NotesWriterDWI::NotesToDWIChar( bool bCol1, bool bCol2, bool bCol3, bool bCol4 )
{
	return NotesToDWIChar( 0, bCol1, bCol2, bCol3, bCol4, 0 );
}

CString NotesWriterDWI::NotesToDWIString( char cNoteCol1, char cNoteCol2, char cNoteCol3, char cNoteCol4, char cNoteCol5, char cNoteCol6 )
{
	char cShow = NotesToDWIChar( cNoteCol1!='0', cNoteCol2!='0', cNoteCol3!='0', cNoteCol4!='0', cNoteCol5!='0', cNoteCol6!='0' );
	char cHold = NotesToDWIChar( cNoteCol1=='2', cNoteCol2=='2', cNoteCol3=='2', cNoteCol4=='2', cNoteCol5=='2', cNoteCol6=='2' );
	
	if( cHold != '0' )
		return ssprintf( "%c!%c", cShow, cHold );
	else
		return cShow;
}

CString NotesWriterDWI::NotesToDWIString( char cNoteCol1, char cNoteCol2, char cNoteCol3, char cNoteCol4 )
{
	return NotesToDWIString( '0', cNoteCol1, cNoteCol2, cNoteCol3, cNoteCol4, '0' );
}

void NotesWriterDWI::WriteDWINotesField( FILE* fp, const Notes &out, int start )
{
	NoteData notedata;
	out.GetNoteData( &notedata );
	notedata.ConvertHoldNotesTo2sAnd3s();

	const int iLastMeasure = int( notedata.GetLastBeat()/BEATS_PER_MEASURE );
	for( int m=0; m<=iLastMeasure; m++ )	// foreach measure
	{
		NoteType nt = notedata.GetSmallestNoteTypeForMeasure( m );

		double fCurrentIncrementer;
		switch( nt )
		{
		case NOTE_TYPE_4TH:
		case NOTE_TYPE_8TH:	
			fCurrentIncrementer = 1.0/8 * BEATS_PER_MEASURE;
			break;
		case NOTE_TYPE_12TH:
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
		case NOTE_TYPE_INVALID:
			fprintf( fp, "<" );
			fCurrentIncrementer = 1.0/192 * BEATS_PER_MEASURE;
			break;
		}

		double fFirstBeatInMeasure = m * BEATS_PER_MEASURE;
		double fLastBeatInMeasure = (m+1) * BEATS_PER_MEASURE;

		for( double b=fFirstBeatInMeasure; b<fLastBeatInMeasure; b+=fCurrentIncrementer )
		{
			int row = BeatToNoteRow( (float)b );

			switch( out.m_NotesType )
			{
			case NOTES_TYPE_DANCE_SINGLE:
			case NOTES_TYPE_DANCE_COUPLE_1:
			case NOTES_TYPE_DANCE_COUPLE_2:
			case NOTES_TYPE_DANCE_DOUBLE:
				fprintf( fp, NotesToDWIString( notedata.m_TapNotes[start+0][row], 
												notedata.m_TapNotes[start+1][row],
												notedata.m_TapNotes[start+2][row],
												notedata.m_TapNotes[start+3][row] ) );
				break;
			case NOTES_TYPE_DANCE_SOLO:
				fprintf( fp, NotesToDWIString( notedata.m_TapNotes[0][row], notedata.m_TapNotes[1][row], notedata.m_TapNotes[2][row], notedata.m_TapNotes[3][row], notedata.m_TapNotes[4][row], notedata.m_TapNotes[5][row] ) );
				break;
			default:	return;	// not a type supported by DWI
			}
		}

		switch( nt )
		{
		case NOTE_TYPE_4TH:
		case NOTE_TYPE_8TH:	
			break;
		case NOTE_TYPE_12TH:
			fprintf( fp, "]" );
			break;
		case NOTE_TYPE_16TH:
			fprintf( fp, ")" );
			break;
		default:
			ASSERT(0);
			// fall though
		case NOTE_TYPE_INVALID:
			fprintf( fp, ">" );
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
	case NOTES_TYPE_DANCE_COUPLE_1:	fprintf( fp, "#COUPLE:" );	break;
	case NOTES_TYPE_DANCE_DOUBLE:	fprintf( fp, "#DOUBLE:" );	break;
	case NOTES_TYPE_DANCE_SOLO:		fprintf( fp, "#SOLO:" );	break;
	default:	return false;	// not a type supported by DWI
	}

	switch( out.m_DifficultyClass )
	{
	case CLASS_EASY:	fprintf( fp, "BASIC:" );	break;
	case CLASS_MEDIUM:	fprintf( fp, "ANOTHER:" );	break;
	case CLASS_HARD:	fprintf( fp, "MANIAC:" );	break;
	default:	ASSERT(0);	return false;
	}

	fprintf( fp, "%d:\n", out.m_iMeter );
	return true;
}

bool NotesWriterDWI::Write( CString sPath, const Song &out )
{
	FILE* fp = fopen( sPath, "w" );	
	if( fp == NULL )
		throw RageException( "Error opening song file '%s' for writing.", sPath );

	if(out.GetFullTitle().GetLength() != 0)
		fprintf( fp, "#TITLE:%s;\n", out.GetFullTitle() );
	if(out.m_sArtist.GetLength() != 0)
		fprintf( fp, "#ARTIST:%s;\n", out.m_sArtist );
	ASSERT( out.m_BPMSegments[0].m_fStartBeat == 0 );
	fprintf( fp, "#BPM:%.2f;\n", out.m_BPMSegments[0].m_fBPM );
	fprintf( fp, "#GAP:%d;\n", -roundf( out.m_fBeat0OffsetInSeconds*1000 ) );

	if( out.m_StopSegments.GetSize() )
	{
		fprintf( fp, "#FREEZE:" );

		for( int i=0; i<out.m_StopSegments.GetSize(); i++ )
		{
			const StopSegment &fs = out.m_StopSegments[i];
			fprintf( fp, "%.2f=%.2f", BeatToNoteRow( fs.m_fStartBeat ) / ROWS_PER_BEAT * 4.0f, roundf(fs.m_fStopSeconds*1000) );
			if( i != out.m_StopSegments.GetSize()-1 )
				fprintf( fp, "," );
		}
		fprintf( fp, ";\n" );
	}

	if( out.m_BPMSegments.GetSize() > 1)
	{
		fprintf( fp, "#CHANGEBPM:" );
		for( int i=1; i<out.m_BPMSegments.GetSize(); i++ )
		{
			const BPMSegment &bs = out.m_BPMSegments[i];
			fprintf( fp, "%.2f=%.2f", BeatToNoteRow( bs.m_fStartBeat ) / ROWS_PER_BEAT * 4.0f, bs.m_fBPM );
			if( i != out.m_BPMSegments.GetSize()-1 )
				fprintf( fp, "," );
		}
		fprintf( fp, ";\n" );
	}

	/* Save all Notes for this file.
	 *
	 * Hmm.  We can have any number of notes for any tag; this means we can
	 * have any number of COUPLE_1 and COUPLE_2.  We don't force them to be
	 * a pair; one player can play couples on normal and the other can play
	 * on hard.
	 *
	 * .DWIs pair up couples; we only write couples where we have a pairing
	 * (eg. we have both COUPLE_1 and COUPLE_2 for the same difficulty.)
	 * If we don't have both, write neither, since "COUPLE" in DWI without
	 * a second side is meaningless.  (I think.)
	 *
	 * We can have any number of pairings; only write one pair for each
	 * difficulty.
	 */

	Notes *c[NUM_DIFFICULTY_CLASSES][2];
	memset(c, 0, sizeof(c));

	for( int i=0; i<out.m_apNotes.GetSize(); i++ ) 
	{
		if(out.m_apNotes[i]->m_NotesType == NOTES_TYPE_DANCE_COUPLE_1) {
			c[out.m_apNotes[i]->m_DifficultyClass][0] = out.m_apNotes[i];
			continue;
		}
		if(out.m_apNotes[i]->m_NotesType == NOTES_TYPE_DANCE_COUPLE_2) {
			c[out.m_apNotes[i]->m_DifficultyClass][1] = out.m_apNotes[i];
			continue;
		}

		if(!WriteDWINotesTag( fp, *out.m_apNotes[i] ))
			continue;

		WriteDWINotesField( fp, *out.m_apNotes[i], 0 );
		if(out.m_apNotes[i]->m_NotesType==NOTES_TYPE_DANCE_DOUBLE)
		{
			fprintf( fp, ":\n" );
			WriteDWINotesField( fp, *out.m_apNotes[i], 4 );
		}

		fprintf( fp, ";\n" );
	}

	for( i=0; i < NUM_DIFFICULTY_CLASSES; ++i )
	{
		/* Skip if we don't have both sides for this couples. */
		if( !c[i][0] || !c[i][1] ) continue;

		WriteDWINotesTag( fp, *c[i][0] );
		WriteDWINotesField( fp, *c[i][0], 0 );
		fprintf( fp, ":\n" );
		WriteDWINotesField( fp, *c[i][1], 0 );
		fprintf( fp, ";\n" );
	}
	
	fclose( fp );

	return true;
}
