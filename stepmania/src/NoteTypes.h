#ifndef NOTETYPES_H
#define NOTETYPES_H
/*
-----------------------------------------------------------------------------
 File: NoteTypes.h

 Desc: 

 Copyright (c) 2001-2002 by the person(s) listed below.  All rights reserved.
	Chris Danford
-----------------------------------------------------------------------------
*/

#include "PlayerOptions.h"

// '1' = tap note
// '2' = hold note begin
// '3' = hold note end  ('1' can also end a HoldNote) ('3' without a matching '2' is ignored
// ... for future expansion


typedef unsigned char TapNote;

static const TapNote TAP_EMPTY		= '0';
static const TapNote TAP_TAP		= '1'; /* impatient? */
static const TapNote TAP_HOLD_HEAD	= '2';	// graded like a TAP_TAP

/* In 2sand3s mode, holds are deleted and TAP_HOLD_END is added: */
static const TapNote TAP_HOLD_TAIL	= '3';

/* In 4s mode, holds and TAP_HOLD_HEAD are deleted and TAP_HOLD is added: */
static const TapNote TAP_HOLD		= '4';

// additional note added by a transform
static const TapNote TAP_ADDITION	= '5';

// mine note - don't step!
static const TapNote TAP_MINE		= '6';

// MD 11/12/03 - for future modifiers
// TAP_ADD_HOLD is to TAP_HOLD what TAP_ADDITION is to TAP_TAP.
// There is no equivalent for TAP_MINE.
static const TapNote TAP_ADD_HOLD	= '7';	// is this supposed to be TAP_ADD_HOLD_HEAD? -Chris


// BM-specific
// Removed taps, e.g. in Little - play keysounds here as if
// judged Perfect, but don't bother rendering or judging this
// step.  Also used for when we implement auto-scratch in BM,
// and for if/when we do a "reduce" modifier that cancels out
// all but N keys on a line [useful for BM->DDR autogen, too].
static const TapNote TAP_REMOVED	= '8';

// KM-specific
// Removed hold body (...why?) - acts as follows:
// 1 - if we're using a sustained-sound gametype [Keyboardmania], and
//     we've already hit the start of the sound (?? we put Holds Off on?)
//     then this is triggered automatically to keep the sound going
// 2 - if we're NOT [anything else], we ignore this.
// Equivalent to all 4s aside from the first one.
static const TapNote TAP_REMOVED_HOLD_BODY	= ':';

// KM-specific
// A hold removed by Little, I guess.
static const TapNote TAP_REMOVED_HOLD_HEAD	= '.';

// KM-specific
// Kills an auto-sound from a hold.
static const TapNote TAP_REMOVED_HOLD_TAIL	= '\'';

// MD 11/12/03 end


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
	// MD 10/26/03 - BMS reader needs a whole lot more than 13 tracks - more like 16
	//   because we have 11-16, 18, 19, 21-26, 28, 29 for IIDX double (bm-double7)
	TRACK_14,
	TRACK_15,
	TRACK_16,
	// MD 10/26/03 end
	MAX_NOTE_TRACKS		// leave this at the end
};

const int BEATS_PER_MEASURE = 4;
const int ROWS_PER_BEAT	= 48;	// It is important that this number is evenly divisible by 2, 3, and 4.
const int ROWS_PER_MEASURE = ROWS_PER_BEAT * BEATS_PER_MEASURE;

enum NoteType 
{ 
	NOTE_TYPE_4TH,	// quarter note
	NOTE_TYPE_8TH,	// eighth note
	NOTE_TYPE_12TH,	// triplet
	NOTE_TYPE_16TH,	// sixteenth note
	NOTE_TYPE_24TH,	// twenty-fourth note
	NOTE_TYPE_32ND,	// thirty-second note
	// MD 11/02/03 - added finer divisions
	NOTE_TYPE_48TH, // forty-eighth note
	NOTE_TYPE_64TH,	// sixty-fourth note
	// Not having this triggers asserts all over the place.  Go figure.
	NOTE_TYPE_192ND,
	NUM_NOTE_TYPES,
	NOTE_TYPE_INVALID
};

float NoteTypeToBeat( NoteType nt );
NoteType GetNoteType( int iNoteIndex );
NoteType BeatToNoteType( float fBeat );
bool IsNoteOfType( int iNoteIndex, NoteType t );
CString NoteTypeToString( NoteType nt );



struct HoldNote
{
	HoldNote( int t, float s, float e ) { iTrack=t; fStartBeat=s; fEndBeat=e; }
	int		iTrack;	
	float	fStartBeat;
	float	fEndBeat;
};

struct AttackNote
{
	AttackNote( int t, float b, CString m, float d ) { iTrack=t; fBeat=b; sModifiers=m; fDurationSeconds=d; }
	int		iTrack;	
	float	fBeat;
	CString sModifiers;
	float	fDurationSeconds;
};


inline int   BeatToNoteRow( float fBeatNum )			{ return int( fBeatNum * ROWS_PER_BEAT + 0.5f); };	// round
inline int   BeatToNoteRowNotRounded( float fBeatNum )	{ return (int)( fBeatNum * ROWS_PER_BEAT ); };
inline float NoteRowToBeat( float fNoteIndex )			{ return fNoteIndex / (float)ROWS_PER_BEAT;	 };
inline float NoteRowToBeat( int iNoteIndex )			{ return NoteRowToBeat( (float)iNoteIndex );	 };




#endif
