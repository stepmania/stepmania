#include "stdafx.h"
/*
-----------------------------------------------------------------------------
 File: NoteTypes.cpp

 Desc: 

 Copyright (c) 2001-2002 by the person(s) listed below.  All rights reserved.
	Chris Danford
-----------------------------------------------------------------------------
*/
#include "NoteTypes.h"


RageColor NoteTypeToColor( NoteType nt )
{
	switch( nt )
	{
	case NOTE_TYPE_4TH:		return RageColor(1,0,0,1);	// red
	case NOTE_TYPE_8TH:		return RageColor(0,0,1,1);	// blue
	case NOTE_TYPE_12TH:	return RageColor(1,0,1,1);	// purple
	case NOTE_TYPE_16TH:	return RageColor(1,1,0,1);	// yellow
	case NOTE_TYPE_24TH:	return RageColor(0,1,1,1);	// light blue
	default:
		ASSERT(0);
	case NOTE_TYPE_32ND:	// fall through
		return RageColor(0.5f,0.5f,0.5f,1);	// gray
	}		
};

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
	default:	ASSERT(0);	return 0;
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
	else												return NOTE_TYPE_INVALID;
};

bool IsNoteOfType( int iNoteIndex, NoteType t )
{ 
	return GetNoteType(iNoteIndex) == t;
}

RageColor GetNoteColorFromIndex( int iIndex )
{ 
	return NoteTypeToColor( GetNoteType(iIndex) );
}

RageColor GetNoteColorFromBeat( float fBeat )
{ 
	return GetNoteColorFromIndex( BeatToNoteRow(fBeat) );
}

