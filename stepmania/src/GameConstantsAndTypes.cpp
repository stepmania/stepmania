#include "stdafx.h"
/*
-----------------------------------------------------------------------------
 File: GameConstantsAndTypes.cpp

 Desc: See header.

 Copyright (c) 2001-2002 by the person(s) listed below.  All rights reserved.
	Chris Danford
-----------------------------------------------------------------------------
*/

#include "GameConstantsAndTypes.h"


D3DXCOLOR NoteTypeToColor( NoteType nt )
{
	switch( nt )
	{
	case NOTE_4TH:	return D3DXCOLOR(1,0,0,1);	// red
	case NOTE_8TH:	return D3DXCOLOR(0,0,1,1);	// blue
	case NOTE_12TH:	return D3DXCOLOR(1,0,1,1);	// purple
	case NOTE_16TH:	return D3DXCOLOR(1,1,0,1);	// yellow
	default:		ASSERT( false );	return D3DXCOLOR(1,1,1,1);
	}		
};

float NoteTypeToBeat( NoteType nt )
{
	switch( nt )
	{
	case NOTE_4TH:	return 1.0f;	// quarter notes
	case NOTE_8TH:	return 1.0f/2;	// eighth notes
	case NOTE_12TH:	return 1.0f/3;	// triplets
	case NOTE_16TH:	return 1.0f/4;	// sixteenth notes
	default:	ASSERT( false );	return 0;
	}
};

NoteType GetNoteType( int iNoteIndex )
{ 
	if( iNoteIndex % (ELEMENTS_PER_MEASURE/4) == 0)
		return NOTE_4TH;
	else if( iNoteIndex % (ELEMENTS_PER_MEASURE/8) == 0)
		return NOTE_8TH;
	else if( iNoteIndex % (ELEMENTS_PER_MEASURE/12) == 0)
		return NOTE_12TH;
	else if( iNoteIndex % (ELEMENTS_PER_MEASURE/16) == 0)
		return NOTE_16TH;
//	ASSERT(0);
	return NOTE_INVALID;
};

bool IsNoteOfType( int iNoteIndex, NoteType t )
{ 
	return GetNoteType(iNoteIndex) == t;
};

D3DXCOLOR GetNoteColorFromIndex( int iStepIndex )
{ 
	for( int t=0; t<NUM_NOTE_TYPES; t++ )
	{
		if( IsNoteOfType( iStepIndex, (NoteType)t ) )
			return NoteTypeToColor( (NoteType)t );
	}
	return D3DXCOLOR(0.5f,0.5f,0.5f,1);
};



D3DXCOLOR DifficultyClassToColor( DifficultyClass dc )
{
	switch( dc )
	{
	case CLASS_EASY:	return D3DXCOLOR(1,1,0,1);	// yellow
	case CLASS_MEDIUM:	return D3DXCOLOR(1,0,0,1);	// red
	case CLASS_HARD:	return D3DXCOLOR(0,1,0,1);	// green
	default:	ASSERT(0);	return D3DXCOLOR();	// invalid DifficultyClass
	}
}

CString DifficultyClassToString( DifficultyClass dc )
{
	switch( dc )
	{
	case CLASS_EASY:	return "easy";
	case CLASS_MEDIUM:	return "medium";
	case CLASS_HARD:	return "hard";
	default:	ASSERT(0);	return "";	// invalid DifficultyClass
	}
}

DifficultyClass StringToDifficultyClass( CString sDC )
{
	for( int i=0; i<NUM_DIFFICULTY_CLASSES; i++ )
	{
		DifficultyClass dc = (DifficultyClass)i;
		if( sDC == DifficultyClassToString(dc) )
			return dc;
	}
	return CLASS_INVALID;
}


int NotesTypeToNumTracks( NotesType nt )
{
	switch( nt )
	{
	case NOTES_TYPE_DANCE_SINGLE:	return 4;
	case NOTES_TYPE_DANCE_DOUBLE:	return 8;
	case NOTES_TYPE_DANCE_COUPLE:	return 8;
	case NOTES_TYPE_DANCE_SOLO:		return 6;
	case NOTES_TYPE_PUMP_SINGLE:	return 5;
	case NOTES_TYPE_PUMP_DOUBLE:	return 10;
	case NOTES_TYPE_EZ2_SINGLE:		return 5; // Single: TL,LHH,D,RHH,TR
	case NOTES_TYPE_EZ2_SINGLE_HARD:		return 5; // Single: TL,LHH,D,RHH,TR
	case NOTES_TYPE_EZ2_DOUBLE:		return 10; // Double: Single x2
	case NOTES_TYPE_EZ2_REAL:		return 7; // Real: TL,LHH,LHL,D,RHL,RHH,TR
	case NOTES_TYPE_EZ2_SINGLE_VERSUS:		return 10; 
	case NOTES_TYPE_EZ2_SINGLE_HARD_VERSUS:		return 10; 
	case NOTES_TYPE_EZ2_REAL_VERSUS:		return 10;
	default:	ASSERT(0);		return -1;	// invalid NotesType
	}
}

NotesType StringToNotesType( CString sNotesType )
{
	sNotesType.MakeLower();
	if     ( sNotesType == "dance-single" )	return NOTES_TYPE_DANCE_SINGLE;
	else if( sNotesType == "dance-double" )	return NOTES_TYPE_DANCE_DOUBLE;
	else if( sNotesType == "dance-couple" )	return NOTES_TYPE_DANCE_COUPLE;
	else if( sNotesType == "dance-solo" )	return NOTES_TYPE_DANCE_SOLO;
	else if( sNotesType == "pump-single" )	return NOTES_TYPE_PUMP_SINGLE;
	else if( sNotesType == "pump-double" )	return NOTES_TYPE_PUMP_DOUBLE;
	else if( sNotesType == "ez2-single" )	return NOTES_TYPE_EZ2_SINGLE;
	else if( sNotesType == "ez2-single-hard" )	return NOTES_TYPE_EZ2_SINGLE_HARD;
	else if( sNotesType == "ez2-double" )	return NOTES_TYPE_EZ2_DOUBLE;
	else if( sNotesType == "ez2-real" )		return NOTES_TYPE_EZ2_REAL;
// 	else if( sNotesType == "ez2-real-versus" )		return NOTES_TYPE_EZ2_REAL_VERSUS;
//	else if( sNotesType == "ez2-single-versus" )		return NOTES_TYPE_EZ2_SINGLE_VERSUS;
	else	ASSERT(0);	return NOTES_TYPE_DANCE_SINGLE;	// invalid NotesType
}

CString NotesTypeToString( NotesType nt )
{
	switch( nt )
	{
	case NOTES_TYPE_DANCE_SINGLE:	return "dance-single";
	case NOTES_TYPE_DANCE_DOUBLE:	return "dance-double";
	case NOTES_TYPE_DANCE_COUPLE:	return "dance-couple";
	case NOTES_TYPE_DANCE_SOLO:		return "dance-solo";
	case NOTES_TYPE_PUMP_SINGLE:	return "pump-single";
	case NOTES_TYPE_PUMP_DOUBLE:	return "pump-double";
	case NOTES_TYPE_EZ2_SINGLE:		return "ez2-single";
	case NOTES_TYPE_EZ2_SINGLE_HARD:		return "ez2-single-hard";
	case NOTES_TYPE_EZ2_DOUBLE:		return "ez2-double";
	case NOTES_TYPE_EZ2_REAL:		return "ez2-real";
	// case NOTES_TYPE_EZ2_REAL_VERSUS:		return "ez2-real-versus";
	// case NOTES_TYPE_EZ2_SINGLE_VERSUS:		return "ez2-single-versus";
	default:	ASSERT(0);		return "";	// invalid NotesType
	}
}




D3DXCOLOR PlayerToColor( const PlayerNumber p ) 
{
	switch( p )
	{
		case PLAYER_1:	return D3DXCOLOR(0.4f,1.0f,0.8f,1);	// sea green
		case PLAYER_2:	return D3DXCOLOR(1.0f,0.5f,0.2f,1);	// orange
		default:	ASSERT( false ); return D3DXCOLOR(1,1,1,1);
	}
};

D3DXCOLOR PlayerToColor( int p ) 
{ 
	return PlayerToColor( (PlayerNumber)p ); 
}



Game StyleToGame( Style s )
{
	switch( s )
	{
	case STYLE_DANCE_SINGLE:
	case STYLE_DANCE_VERSUS:
	case STYLE_DANCE_DOUBLE:
	case STYLE_DANCE_COUPLE:
	case STYLE_DANCE_SOLO:
		return GAME_DANCE;
	case STYLE_PUMP_SINGLE:
	case STYLE_PUMP_VERSUS:
	case STYLE_PUMP_DOUBLE:
		return GAME_PUMP;
	case STYLE_EZ2_SINGLE:
	case STYLE_EZ2_SINGLE_HARD:
	case STYLE_EZ2_DOUBLE:
	case STYLE_EZ2_REAL:
	case STYLE_EZ2_SINGLE_VERSUS:
	case STYLE_EZ2_SINGLE_HARD_VERSUS:
	case STYLE_EZ2_REAL_VERSUS:
		return GAME_EZ2;
	default:
		ASSERT(0);	// invalid Style
		return GAME_DANCE;
	}
}
