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

static const TapNote TAP_EMPTY		= '0';
static const TapNote TAP_TAP		= '1'; /* impatient? */
static const TapNote TAP_HOLD_HEAD	= '2';

/* In 2sand3s mode, holds are deleted and TAP_HOLD_END is added: */
static const TapNote TAP_HOLD_TAIL	= '3';

/* In 4s mode, holds and TAP_HOLD_HEAD are deleted and TAP_HOLD is added: */
static const TapNote TAP_HOLD		= '4';

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
	TRACK_13,	// BMS reader needs 13 tracks
	MAX_NOTE_TRACKS		// leave this at the end
};

const int MAX_BEATS			= 3000;	// J-Paradise has about 2120
const int BEATS_PER_MEASURE = 4;
const int MAX_MEASURES		= MAX_BEATS / BEATS_PER_MEASURE;

const int ROWS_PER_BEAT	= 12;	// It is important that this number is evenly divisible by 2, 3, and 4.
const int ROWS_PER_MEASURE = ROWS_PER_BEAT * BEATS_PER_MEASURE;
const int MAX_TAP_NOTE_ROWS = MAX_BEATS*ROWS_PER_BEAT;

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

RageColor NoteTypeToColor( NoteType nt );
float NoteTypeToBeat( NoteType nt );
NoteType GetNoteType( int iNoteIndex );
bool IsNoteOfType( int iNoteIndex, NoteType t );
RageColor GetNoteColorFromIndex( int iNoteIndex );
RageColor GetNoteColorFromBeat( float fBeat );



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



