/* NoteTypes - Types for holding tap notes and scores. */

#ifndef NOTE_TYPES_H
#define NOTE_TYPES_H

#include "GameConstantsAndTypes.h"
class XNode;

struct TapNoteResult
{
	TapNoteResult()
	{
		tns = TNS_None;
		fTapNoteOffset = 0;
	}
	TapNoteScore tns;

	/* Offset, in seconds, for a tap grade.  Negative numbers mean the note
	 * was hit early; positive numbers mean it was hit late.  These values are
	 * only meaningful for graded taps (tns >= TNS_W5). */
	float fTapNoteOffset;

	/* If the whole row has been judged, all taps on the row will be set to hidden. */
	bool bHidden;

	// XML
	XNode* CreateNode() const;
	void LoadFromNode( const XNode* pNode );
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
		hns = HNS_None;
		fLife = 1.0f;
		iLastHeldRow = 0;
		bHeld = bActive = false;
	}

	float GetLastHeldBeat() const;

	bool bHeld;
	bool bActive;

	// XML
	XNode* CreateNode() const;
	void LoadFromNode( const XNode* pNode );
};


struct TapNote
{
	enum Type { 
		empty, 		// no note here
		tap, 
		hold_head,	// graded like a TAP_TAP if
		hold_tail,	/* In 2sand3s mode, holds are deleted and TAP_HOLD_END is added: */
		mine,		// don't step!
		attack,
		autoKeysound,
 	} type;
	enum SubType
	{
		hold_head_hold,
		hold_head_roll,
		SubType_invalid
	} subType;	// only used if type == hold_head
	enum Source {
		original,	// part of the original NoteData
		addition,	// additional note added by a transform
		removed,	// Removed taps, e.g. in Little - play keysounds here as if
					// judged W2, but don't bother rendering or judging this
					// step.  Also used for when we implement auto-scratch,
					// and for if/when we do a "reduce" modifier that cancels out
					// all but N keys on a line [useful for beat->dance autogen, too].
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

	/* hold_head only: */
	int iDuration;

	TapNoteResult result;

	/* hold_head only: */
	HoldNoteResult HoldResult;


	// XML
	XNode* CreateNode() const;
	void LoadFromNode( const XNode* pNode );

	
	TapNote() {}
	TapNote( 
		Type type_,
		SubType subType_,
		Source source_, 
		CString sAttackModifiers_,
		float fAttackDurationSeconds_,
		bool bKeysound_,
		int iKeysoundIndex_ )
	{
		type = type_;
		subType = subType_;
		source = source_;
		sAttackModifiers = sAttackModifiers_;
		fAttackDurationSeconds = fAttackDurationSeconds_;
		bKeysound = bKeysound_;
		iKeysoundIndex = iKeysoundIndex_;
		iDuration = 0;
	}
	bool operator==( const TapNote &other ) const
	{
#define COMPARE(x)	if(x!=other.x) return false;
		COMPARE(type);
		COMPARE(subType);
		COMPARE(source);
		COMPARE(sAttackModifiers);
		COMPARE(fAttackDurationSeconds);
		COMPARE(bKeysound);
		COMPARE(iKeysoundIndex);
		COMPARE(iDuration);
#undef COMPARE
		return true;
	}
};

extern TapNote TAP_EMPTY;					// '0'
extern TapNote TAP_ORIGINAL_TAP;			// '1'
extern TapNote TAP_ORIGINAL_HOLD_HEAD;		// '2'
extern TapNote TAP_ORIGINAL_ROLL_HEAD;		// '4'
extern TapNote TAP_ORIGINAL_MINE;			// 'M'
extern TapNote TAP_ORIGINAL_ATTACK;			// 'A'
extern TapNote TAP_ORIGINAL_AUTO_KEYSOUND;	// 'K'
extern TapNote TAP_ADDITION_TAP;
extern TapNote TAP_ADDITION_MINE;

// TODO: Don't have a hard-coded track limit.
const int MAX_NOTE_TRACKS = 16;

/* This is a divisor for our "fixed-point" time/beat representation.  It must be evenly divisible
 * by 2, 3, and 4, to exactly represent 8th, 12th and 16th notes. */
const int ROWS_PER_BEAT	= 48;

/* In the editor, enforce a reasonable limit on the number of notes. */
const int MAX_NOTES_PER_MEASURE = 50;

const int BEATS_PER_MEASURE = 4;
const int ROWS_PER_MEASURE = ROWS_PER_BEAT * BEATS_PER_MEASURE;
const int MAX_NOTE_ROW = (1<<30);

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
	NOTE_TYPE_192ND,
	NUM_NOTE_TYPES,
	NOTE_TYPE_INVALID
};
const CString& NoteTypeToString( NoteType nt );
float NoteTypeToBeat( NoteType nt );
NoteType GetNoteType( int row );
NoteType BeatToNoteType( float fBeat );
bool IsNoteOfType( int row, NoteType t );

/* This is more accurate: by computing the integer and fractional parts separately, we
 * can avoid storing very large numbers in a float and possibly losing precision.  It's
 * slower; use this once less stuff uses BeatToNoteRow. */
/*
inline int   BeatToNoteRow( float fBeatNum )
{
	float fraction = fBeatNum - truncf(fBeatNum);
	int integer = int(fBeatNum) * ROWS_PER_BEAT;
	return integer + lrintf(fraction * ROWS_PER_BEAT);
}
*/
inline int   BeatToNoteRow( float fBeatNum )			{ return lrintf( fBeatNum * ROWS_PER_BEAT ); }	// round
inline int   BeatToNoteRowNotRounded( float fBeatNum )	{ return (int)( fBeatNum * ROWS_PER_BEAT ); }
inline float NoteRowToBeat( int iRow )					{ return iRow / (float)ROWS_PER_BEAT; }

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
