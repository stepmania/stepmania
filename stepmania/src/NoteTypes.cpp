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


D3DXCOLOR NoteTypeToColor( NoteType nt )
{
	switch( nt )
	{
	case NOTE_TYPE_4TH:		return D3DXCOLOR(1,0,0,1);	// red
	case NOTE_TYPE_8TH:		return D3DXCOLOR(0,0,1,1);	// blue
	case NOTE_TYPE_12TH:	return D3DXCOLOR(1,0,1,1);	// purple
	case NOTE_TYPE_16TH:	return D3DXCOLOR(1,1,0,1);	// yellow
	default:	ASSERT(0);	return D3DXCOLOR(0.5f,0.5f,0.5f,1);
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
	default:	ASSERT(0);	return 0;
	}
}

NoteType GetNoteType( int iNoteIndex )
{ 
	if(      iNoteIndex % (ROWS_PER_MEASURE/4) == 0)	return NOTE_TYPE_4TH;
	else if( iNoteIndex % (ROWS_PER_MEASURE/8) == 0)	return NOTE_TYPE_8TH;
	else if( iNoteIndex % (ROWS_PER_MEASURE/12) == 0)	return NOTE_TYPE_12TH;
	else if( iNoteIndex % (ROWS_PER_MEASURE/16) == 0)	return NOTE_TYPE_16TH;
	else												return NOTE_TYPE_INVALID;
};

bool IsNoteOfType( int iNoteIndex, NoteType t )
{ 
	return GetNoteType(iNoteIndex) == t;
}

D3DXCOLOR GetNoteColorFromIndex( int iIndex )
{ 
	for( int t=0; t<NUM_NOTE_TYPES; t++ )
	{
		if( IsNoteOfType( iIndex, (NoteType)t ) )
			return NoteTypeToColor( (NoteType)t );
	}
	return D3DXCOLOR(0.5f,0.5f,0.5f,1);
}

D3DXCOLOR GetNoteColorFromBeat( float fBeat )
{ 
	return GetNoteColorFromIndex( BeatToNoteRow(fBeat) );
}

