#ifndef NOTE_TYPES_H
#define NOTE_TYPES_H

#include "GameConstantsAndTypes.h"

struct TapNoteResult
{
	TapNoteResult()
	{
		tns = TNS_NONE;
		fTapNoteOffset = 0;
	}
	TapNoteScore tns;

	/* Offset, in seconds, for a tap grade.  Negative numbers mean the note
	 * was hit early; positive numbers mean it was hit late.  These values are
	 * only meaningful for graded taps (tns >= TNS_BOO). */
	float fTapNoteOffset;
};

struct HoldNoteResult
{
	HoldNoteScore hns;

	/* 1.0 means this HoldNote has full life.
	 * 0.0 means this HoldNote is dead
	 * When this value hits 0.0 for the first time, m_HoldScore becomes HSS_NG.
	 * If the life is > 0.0 when the HoldNote ends, then m_HoldScore becomes HSS_OK. */
	float fLife;

	/* Last index where fLife was greater than 0.  If the tap was missed, this will
	 * be the first index of the hold. */
	int iLastHeldRow;

	HoldNoteResult()
	{
		hns = HNS_NONE;
		fLife = 1.0f;
		iLastHeldRow = 0;
	}

	float GetLastHeldBeat() const;
};


struct TapNote
{
	enum Type { 
		empty, 		// no note here
		tap, 
		hold_head,	// graded like a TAP_TAP
		hold_tail,	/* In 2sand3s mode, holds are deleted and TAP_HOLD_END is added: */
		hold,		/* In 4s mode, holds and TAP_HOLD_HEAD are deleted and TAP_HOLD is added: */
		mine,		// don't step!
		attack,
		autoKeysound,
 	} type;
	enum Source {
		original,	// part of the original NoteData
		addition,	// additional note added by a transform
		removed,	// Removed taps, e.g. in Little - play keysounds here as if
					// judged Perfect, but don't bother rendering or judging this
					// step.  Also used for when we implement auto-scratch in BM,
					// and for if/when we do a "reduce" modifier that cancels out
					// all but N keys on a line [useful for BM->DDR autogen, too].
					// Removed hold body (...why?) - acts as follows:
					// 1 - if we're using a sustained-sound gametype [Keyboardmania], and
					//     we've already hit the start of the sound (?? we put Holds Off on?)
					//     then this is triggered automatically to keep the sound going
					// 2 - if we're NOT [anything else], we ignore this.
					// Equivalent to all 4s aside from the first one.
	} source;

	// Only valid if type == attack.
	CString sAttackModifiers;
	float fAttackDurationSeconds;

	bool bKeysound;	// true if this note plays a keysound when hit

	int iKeysoundIndex;	// index into Song's vector of keysound files.  
							// Only valid if bKeysound.

	TapNote() {}
	TapNote( 
		Type type_, 
		Source source_, 
		CString sAttackModifiers_,
		float fAttackDurationSeconds_,
		bool bKeysound_,
		int iKeysoundIndex_ )
	{
		type = type_;
		source = source_;
		sAttackModifiers = sAttackModifiers_;
		fAttackDurationSeconds = fAttackDurationSeconds_;
		bKeysound = bKeysound_;
		iKeysoundIndex = iKeysoundIndex_;
	}
	bool operator==( const TapNote &other ) const
	{
#define COMPARE(x)	if(x!=other.x) return false;
		COMPARE(type);
		COMPARE(source);
		COMPARE(sAttackModifiers);
		COMPARE(fAttackDurationSeconds);
		COMPARE(bKeysound);
		COMPARE(iKeysoundIndex);
#undef COMPARE
		return true;
	}

	/* This data is only used and manipulated by NoteDataWithScoring.  It's only in
	 * here for the sake of efficiency. */
	TapNoteResult result;
};

const unsigned MAX_NUM_ATTACKS = 2*2*2;	// 3 bits to hold the attack index currently

extern TapNote TAP_EMPTY;					// '0'
extern TapNote TAP_ORIGINAL_TAP;			// '1'
extern TapNote TAP_ORIGINAL_HOLD_HEAD;		// '2'
extern TapNote TAP_ORIGINAL_HOLD_TAIL;		// '3'
extern TapNote TAP_ORIGINAL_HOLD;			// '4'
extern TapNote TAP_ORIGINAL_MINE;			// 'M'
extern TapNote TAP_ORIGINAL_ATTACK;			// 'A'
extern TapNote TAP_ORIGINAL_AUTO_KEYSOUND;	// 'K'
extern TapNote TAP_ADDITION_TAP;
extern TapNote TAP_ADDITION_MINE;

// TODO: Don't have a hard-coded track limit.
const int MAX_NOTE_TRACKS = 16;

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

	// Why is this high of resolution needed?  It's breaking NoteSkins
	// with note-coloring, and the extra resolution will take up more 
	// memory.  Does any game actually use this?  -Chris

	// MD 11/02/03 - added finer divisions
	NOTE_TYPE_48TH, // forty-eighth note
	NOTE_TYPE_64TH,	// sixty-fourth note
	// Not having this triggers asserts all over the place.  Go figure.
	NOTE_TYPE_192ND,
	NUM_NOTE_TYPES,
	NOTE_TYPE_INVALID
};

float NoteTypeToBeat( NoteType nt );
NoteType GetNoteType( int row );
NoteType BeatToNoteType( float fBeat );
bool IsNoteOfType( int row, NoteType t );
CString NoteTypeToString( NoteType nt );

inline int   BeatToNoteRow( float fBeatNum )			{ return int( fBeatNum * ROWS_PER_BEAT + 0.5f); };	// round
inline int   BeatToNoteRowNotRounded( float fBeatNum )	{ return (int)( fBeatNum * ROWS_PER_BEAT ); };
inline float NoteRowToBeat( float fNoteIndex )			{ return fNoteIndex / (float)ROWS_PER_BEAT;	 };
inline float NoteRowToBeat( int row )			{ return NoteRowToBeat( (float)row );	 };



struct HoldNote
{
	HoldNote( int t, int s, int e ) { iTrack=t; iStartRow=s; iEndRow=e; bHeld = bActive = false; }
	bool RowIsInRange( int row ) const { return iStartRow <= row && row <= iEndRow; }
	bool RangeOverlaps( int start, int end ) const
	{
		/* If the range doesn't overlap us, then start and end are either both before
		 * us or both after us. */
		return !( (start < iStartRow && end < iStartRow) ||
				  (start > iEndRow && end > iEndRow) );
	}
	bool RangeOverlaps( const HoldNote &hn ) const { return RangeOverlaps(hn.iStartRow, hn.iEndRow); }
	bool RangeInside( int start, int end ) const { return iStartRow <= start && end <= iEndRow; }
	bool ContainedByRange( int start, int end ) const { return start <= iStartRow && iEndRow <= end; }

	float GetStartBeat() const { return NoteRowToBeat( iStartRow ); }
	float GetEndBeat() const { return NoteRowToBeat( iEndRow ); }

	/* Invariant: iStartRow <= iEndRow */
	int		iStartRow;
	int		iEndRow;
	int		iTrack;	

	/* This data is only used and manipulated by NoteDataWithScoring.  It's only in
	 * here for the sake of efficiency. */
	HoldNoteResult result;
	bool bHeld;
	bool bActive;
};

#endif

/*
 * (c) 2001-2004 Chris Danford, Glenn Maynard
 * All rights reserved.
 * 
 * Permission is hereby granted, free of charge, to any person obtaining a
 * copy of this software and associated documentation files (the
 * "Software"), to deal in the Software without restriction, including
 * without limitation the rights to use, copy, modify, merge, publish,
 * distribute, and/or sell copies of the Software, and to permit persons to
 * whom the Software is furnished to do so, provided that the above
 * copyright notice(s) and this permission notice appear in all copies of
 * the Software and that both the above copyright notice(s) and this
 * permission notice appear in supporting documentation.
 * 
 * THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS
 * OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF
 * MERCHANTABILITY, FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT OF
 * THIRD PARTY RIGHTS. IN NO EVENT SHALL THE COPYRIGHT HOLDER OR HOLDERS
 * INCLUDED IN THIS NOTICE BE LIABLE FOR ANY CLAIM, OR ANY SPECIAL INDIRECT
 * OR CONSEQUENTIAL DAMAGES, OR ANY DAMAGES WHATSOEVER RESULTING FROM LOSS
 * OF USE, DATA OR PROFITS, WHETHER IN AN ACTION OF CONTRACT, NEGLIGENCE OR
 * OTHER TORTIOUS ACTION, ARISING OUT OF OR IN CONNECTION WITH THE USE OR
 * PERFORMANCE OF THIS SOFTWARE.
 */
