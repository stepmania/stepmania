#include "global.h"
/*
-----------------------------------------------------------------------------
 File: NoteTypes.cpp

 Desc: 

 Copyright (c) 2001-2002 by the person(s) listed below.  All rights reserved.
	Chris Danford
-----------------------------------------------------------------------------
*/
#include "NoteTypes.h"


float NoteTypeToBeat( NoteType nt )
{
	switch( nt )
	{
	case NOTE_TYPE_4TH:		return 1.0f;	// quarter notes
	case NOTE_TYPE_8TH:		return 1.0f/2;	// eighth notes
	case NOTE_TYPE_12TH:	return 1.0f/3;	// triplets
	case NOTE_TYPE_16TH:	return 1.0f/4;	// sixteenth notes
	case NOTE_TYPE_24TH:	return 1.0f/6;	// twenty-forth notes
	case NOTE_TYPE_32ND:	return 1.0f/8;	// thirty-second notes
	case NOTE_TYPE_48TH:	return 1.0f/12;
	case NOTE_TYPE_64TH:	return 1.0f/16;
	// MD 11/03/03 - NOTE_TYPE_INVALID should be treated as equivalent to
	//               NOTE_TYPE_192ND; NOTE_TYPE_96TH should not exist.
	// MD 11/12/03 - And, really, since NOTE_TYPE_192ND had to be added, we'll
	//				 keep the behavior of NOTE_TYPE_INVALID being the same, but
	//				 it shouldn't ever come up anyway.
	case NOTE_TYPE_192ND:	return 1.0f/48;
	default:	ASSERT(0); // and fall through
	case NOTE_TYPE_INVALID:	return 1.0f/48;
	}
}

NoteType GetNoteType( int iNoteIndex )
{ 
	if(      iNoteIndex % (ROWS_PER_MEASURE/4) == 0)	return NOTE_TYPE_4TH;
	else if( iNoteIndex % (ROWS_PER_MEASURE/8) == 0)	return NOTE_TYPE_8TH;
	else if( iNoteIndex % (ROWS_PER_MEASURE/12) == 0)	return NOTE_TYPE_12TH;
	else if( iNoteIndex % (ROWS_PER_MEASURE/16) == 0)	return NOTE_TYPE_16TH;
	else if( iNoteIndex % (ROWS_PER_MEASURE/24) == 0)	return NOTE_TYPE_24TH;
	else if( iNoteIndex % (ROWS_PER_MEASURE/32) == 0)	return NOTE_TYPE_32ND;
	else if( iNoteIndex % (ROWS_PER_MEASURE/48) == 0)	return NOTE_TYPE_48TH;
	else if( iNoteIndex % (ROWS_PER_MEASURE/64) == 0)	return NOTE_TYPE_64TH;
	else												return NOTE_TYPE_INVALID;
};

CString NoteTypeToString( NoteType nt )
{
	switch( nt )
	{
	case NOTE_TYPE_4TH:		return "4th";
	case NOTE_TYPE_8TH:		return "8th";
	case NOTE_TYPE_12TH:	return "12th";
	case NOTE_TYPE_16TH:	return "16th";
	case NOTE_TYPE_24TH:	return "24th";
	case NOTE_TYPE_32ND:	return "32nd";
	case NOTE_TYPE_48TH:	return "48th";
	case NOTE_TYPE_64TH:	return "64th";
	case NOTE_TYPE_192ND:	// fall through
	case NOTE_TYPE_INVALID:	return "192nd";
	default:	ASSERT(0);	return "";
	}
}

NoteType BeatToNoteType( float fBeat )
{ 
	return GetNoteType( BeatToNoteRow(fBeat) );
}

bool IsNoteOfType( int iNoteIndex, NoteType t )
{ 
	return GetNoteType(iNoteIndex) == t;
}

