#pragma once
/*
-----------------------------------------------------------------------------
 File: NoteTypes.h

 Desc: 

 Copyright (c) 2001-2002 by the person(s) listed below.  All rights reserved.
	Chris Danford
-----------------------------------------------------------------------------
*/

#include "GameConstantsAndTypes.h"
#include "PlayerOptions.h"

// '1' = tap note
// '2' = hold note begin
// '3' = hold note end  ('1' can also end a HoldNote) ('3' without a matching '2' is ignored
// ... for future expansion


typedef unsigned char TapNote;

enum 
{
	TRACK_1 = 0,
	TRACK_2,
	TRACK_3,
	TRACK_4,
	TRACK_5,
	TRACK_6,
	TRACK_7,
	TRACK_8,
	TRACK_9,
	TRACK_10,
	TRACK_11,
	TRACK_12,
	TRACK_13,
	MAX_NOTE_TRACKS		// leave this at the end
};

const int MAX_BEATS			= 1500;	// BMR's Pulse has about 1300
const int BEATS_PER_MEASURE = 4;
const int MAX_MEASURES		= MAX_BEATS / BEATS_PER_MEASURE;

const int ROWS_PER_BEAT	= 12;	// It is important that this number is evenly divisible by 2, 3, and 4.
const int ROWS_PER_MEASURE = ROWS_PER_BEAT * BEATS_PER_MEASURE;
const int MAX_TAP_NOTE_ROWS = MAX_BEATS*ROWS_PER_BEAT;

const int MAX_HOLD_NOTES = 400;	// BMR's Connected has about 300

enum NoteType 
{ 
	NOTE_TYPE_4TH,	// quarter note
	NOTE_TYPE_8TH,	// eighth note
	NOTE_TYPE_12TH,	// triplet
	NOTE_TYPE_16TH,	// sixteenth note
	NOTE_TYPE_24TH,	// twenty-forth note
	NOTE_TYPE_32ND,	// thirty-second note
	NUM_NOTE_TYPES,
	NOTE_TYPE_INVALID
};

D3DXCOLOR NoteTypeToColor( NoteType nt );
float NoteTypeToBeat( NoteType nt );
NoteType GetNoteType( int iNoteIndex );
bool IsNoteOfType( int iNoteIndex, NoteType t );
D3DXCOLOR GetNoteColorFromIndex( int iNoteIndex );
D3DXCOLOR GetNoteColorFromBeat( float fBeat );



struct HoldNote
{
	int		m_iTrack;	
	float	m_fStartBeat;
	float	m_fEndBeat;
};


inline int   BeatToNoteRow( float fBeatNum )			{ return int( fBeatNum * ROWS_PER_BEAT + 0.5f); };	// round
inline int   BeatToNoteRowNotRounded( float fBeatNum )	{ return (int)( fBeatNum * ROWS_PER_BEAT ); };
inline float NoteRowToBeat( float fNoteIndex )			{ return fNoteIndex / (float)ROWS_PER_BEAT;	 };
inline float NoteRowToBeat( int iNoteIndex )			{ return NoteRowToBeat( (float)iNoteIndex );	 };



