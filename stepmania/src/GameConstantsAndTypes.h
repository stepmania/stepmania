#pragma once
/*
-----------------------------------------------------------------------------
 File: GameConstantsAndTypes.h

 Desc: Things that are used in many places and don't change often.

 Copyright (c) 2001-2002 by the person(s) listed below.  All rights reserved.
	Chris Danford
-----------------------------------------------------------------------------
*/

#include "D3DX8Math.h"	// for D3DXCOLOR


/////////////////////////////
// Screen Dimensions
/////////////////////////////
#define	SCREEN_WIDTH	(640)
#define	SCREEN_HEIGHT	(480)

#define	SCREEN_LEFT		(0)
#define	SCREEN_RIGHT	(SCREEN_WIDTH)
#define	SCREEN_TOP		(0)
#define	SCREEN_BOTTOM	(SCREEN_HEIGHT)

#define	CENTER_X		(SCREEN_LEFT + (SCREEN_RIGHT - SCREEN_LEFT)/2.0f)
#define	CENTER_Y		(SCREEN_TOP + (SCREEN_BOTTOM - SCREEN_TOP)/2.0f)

/////////////////////////
// Note definitions
/////////////////////////



enum RadarCategory	// starting from 12-o'clock rotating clockwise
{
	RADAR_STREAM = 0,
	RADAR_VOLTAGE,
	RADAR_AIR,
	RADAR_FREEZE,
	RADAR_CHAOS,
	NUM_RADAR_VALUES	// leave this at the end
};

enum DifficultyClass 
{ 
	CLASS_EASY,		// corresponds to Basic
	CLASS_MEDIUM,	// corresponds to Trick, Another, Standard
	CLASS_HARD,		// corresponds to Maniac, SSR, Heavy
	NUM_DIFFICULTY_CLASSES,
	CLASS_INVALID
};

D3DXCOLOR DifficultyClassToColor( DifficultyClass dc );

CString DifficultyClassToString( DifficultyClass dc );

DifficultyClass StringToDifficultyClass( CString sDC );


enum NotesType
{
	NOTES_TYPE_DANCE_SINGLE = 0,
	NOTES_TYPE_DANCE_DOUBLE,
	NOTES_TYPE_DANCE_COUPLE_1,
	NOTES_TYPE_DANCE_COUPLE_2,
	NOTES_TYPE_DANCE_SOLO,
	NOTES_TYPE_PUMP_SINGLE,
	NOTES_TYPE_PUMP_DOUBLE,
	NOTES_TYPE_PUMP_COUPLE,
	NOTES_TYPE_EZ2_SINGLE,
	NOTES_TYPE_EZ2_SINGLE_HARD,
	NOTES_TYPE_EZ2_DOUBLE,
	NOTES_TYPE_EZ2_REAL,
	NOTES_TYPE_PARA,
//	NOTES_TYPE_EZ2_SINGLE_VERSUS,		// Chris:  these should only be styles, not NotesTypes
//	NOTES_TYPE_EZ2_SINGLE_HARD_VERSUS,
//	NOTES_TYPE_EZ2_REAL_VERSUS,
	NUM_NOTES_TYPES,		// leave this at the end
	NOTES_TYPE_INVALID,
};

//////////////////////////
// Play mode stuff
//////////////////////////
enum PlayMode
{
	PLAY_MODE_ARCADE,
	PLAY_MODE_ONI,
	PLAY_MODE_ENDLESS,
	NUM_PLAY_MODES,
	PLAY_MODE_INVALID
};

enum PlayerNumber {
	PLAYER_1 = 0,
	PLAYER_2,
	NUM_PLAYERS,	// leave this at the end
	PLAYER_INVALID
};

D3DXCOLOR PlayerToColor( PlayerNumber pn );
D3DXCOLOR PlayerToColor( int p );


enum SongSortOrder { 
	SORT_GROUP, 
	SORT_TITLE, 
	SORT_BPM, 
	SORT_MOST_PLAYED, 
	NUM_SORT_ORDERS 
};


///////////////////////////
// Scoring stuff
///////////////////////////

enum TapNoteScore { 
	TNS_NONE, 
	TNS_MISS,
	TNS_BOO,
	TNS_GOOD,
	TNS_GREAT,
	TNS_PERFECT,
	NUM_TAP_NOTE_SCORES
};

inline int TapNoteScoreToDancePoints( TapNoteScore tns )
{
	switch( tns )
	{
	case TNS_PERFECT:	return +2;
	case TNS_GREAT:		return +1;
	case TNS_GOOD:		return +0;
	case TNS_BOO:		return -4;
	case TNS_MISS:		return -8;
	case TNS_NONE:		return 0;
	default:	ASSERT(0);	return 0;
	}
}

//enum TapNoteTiming { 
//	TNT_NONE, 
//	TNT_EARLY, 
//	TNT_LATE 
//};


enum HoldNoteScore 
{ 
	HNS_NONE,	// this HoldNote has not been scored yet
	HNS_OK,		// the HoldNote has passed and was successfully held all the way through
	HNS_NG,		// the HoldNote has passed and they missed it
	NUM_HOLD_NOTE_SCORES
};


inline int HoldNoteScoreToDancePoints( HoldNoteScore hns )
{
	switch( hns )
	{
	case HNS_OK:	return +6;
	case HNS_NG:	return +0;
	default:	ASSERT(0);	return 0;
	}
}
