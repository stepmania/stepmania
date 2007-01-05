/* NoteTypes - Types for holding tap notes and scores. */

#ifndef NOTE_TYPES_H
#define NOTE_TYPES_H

#include "GameConstantsAndTypes.h"
class XNode;

struct TapNoteResult
{
	TapNoteResult() : tns(TNS_None), fTapNoteOffset(0.f), bHidden(false) { }
	
	TapNoteScore	tns;

	/* Offset, in seconds, for a tap grade.  Negative numbers mean the note
	 * was hit early; positive numbers mean it was hit late.  These values are
	 * only meaningful for graded taps (tns >= TNS_W5). */
	float		fTapNoteOffset;

	/* If the whole row has been judged, all taps on the row will be set to hidden. */
	bool		bHidden;

	// XML
	XNode* CreateNode() const;
	void LoadFromNode( const XNode* pNode );
};

struct HoldNoteResult
{
	HoldNoteResult() : hns(HNS_None), fLife(1.f), iLastHeldRow(0), bHeld(false), bActive(false) { }
	float GetLastHeldBeat() const;
	
	HoldNoteScore	hns;

	/* 1.0 means this HoldNote has full life.
	 * 0.0 means this HoldNote is dead
	 * When this value hits 0.0 for the first time, m_HoldScore becomes HNS_LetGo.
	 * If the life is > 0.0 when the HoldNote ends, then m_HoldScore becomes HNS_Held. */
	float	fLife;

	/* Last index where fLife was greater than 0.  If the tap was missed, this will
	 * be the first index of the hold. */
	int		iLastHeldRow;
	bool		bHeld;		// Was button held during last update?
	bool		bActive;	// Is life > 0  &&  overlaps current beat

	// XML
	XNode* CreateNode() const;
	void LoadFromNode( const XNode* pNode );
};


struct TapNote
{
	enum Type
	{ 
		empty, 		// no note here
		tap,		// step on this
		hold_head,	// graded like a tap
		hold_tail,	// in 2sand3s mode, holds are deleted and hold_tail is added
		mine,		// don't step!
		lift,		// up
		attack,
		autoKeysound,
 	};
	enum SubType
	{
		hold_head_hold,
		hold_head_roll,
		SubType_invalid
	};
	enum Source
	{
		original,	// part of the original NoteData
		addition,	// additional note added by a transform
	};
	
	Type		type;
	SubType		subType; // Only used if type is hold_head.
	Source		source;
	TapNoteResult	result;
	PlayerNumber	pn;
	bool		bHopoPossible;
	
	// attack only
	RString		sAttackModifiers;
	float		fAttackDurationSeconds;

	// Index into Song's vector of keysound files if nonnegative.
	int		iKeysoundIndex;

	// hold_head only
	int		iDuration;
	HoldNoteResult	HoldResult;
	
	// XML
	XNode* CreateNode() const;
	void LoadFromNode( const XNode* pNode );

	TapNote() : type(empty), subType(SubType_invalid), source(original), pn(PLAYER_INVALID), bHopoPossible(false),
		fAttackDurationSeconds(0.f), iKeysoundIndex(-1), iDuration(0)  { }
	TapNote( 
		Type type_,
		SubType subType_,
		Source source_, 
		RString sAttackModifiers_,
		float fAttackDurationSeconds_,
		int iKeysoundIndex_ )
	{
		type = type_;
		subType = subType_;
		source = source_;
		sAttackModifiers = sAttackModifiers_;
		fAttackDurationSeconds = fAttackDurationSeconds_;
		iKeysoundIndex = iKeysoundIndex_;
		iDuration = 0;
		pn = PLAYER_INVALID;
	}
	bool operator==( const TapNote &other ) const
	{
#define COMPARE(x)	if(x!=other.x) return false
		COMPARE(type);
		COMPARE(subType);
		COMPARE(source);
		COMPARE(sAttackModifiers);
		COMPARE(fAttackDurationSeconds);
		COMPARE(iKeysoundIndex);
		COMPARE(iDuration);
		COMPARE(pn);
#undef COMPARE
		return true;
	}
	bool operator!=( const TapNote &other ) const { return !operator==( other ); }
};

extern TapNote TAP_EMPTY;			// '0'
extern TapNote TAP_ORIGINAL_TAP;		// '1'
extern TapNote TAP_ORIGINAL_HOLD_HEAD;		// '2'
extern TapNote TAP_ORIGINAL_ROLL_HEAD;		// '4'
extern TapNote TAP_ORIGINAL_MINE;		// 'M'
extern TapNote TAP_ORIGINAL_LIFT;		// 'L'
extern TapNote TAP_ORIGINAL_ATTACK;		// 'A'
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
	NOTE_TYPE_12TH,	// quarter note triplet
	NOTE_TYPE_16TH,	// sixteenth note
	NOTE_TYPE_24TH,	// eighth note triplet
	NOTE_TYPE_32ND,	// thirty-second note
	NOTE_TYPE_48TH, // sixteenth note triplet
	NOTE_TYPE_64TH,	// sixty-fourth note
	NOTE_TYPE_192ND,// sixty-fourth note triplet
	NUM_NoteType,
	NoteType_Invalid
};
const RString& NoteTypeToString( NoteType nt );
const RString& NoteTypeToLocalizedString( NoteType nt );
LuaDeclareType( NoteType );
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
inline int   BeatToNoteRow( float fBeatNum )		{ return lrintf( fBeatNum * ROWS_PER_BEAT ); }	// round
inline int   BeatToNoteRowNotRounded( float fBeatNum )	{ return (int)( fBeatNum * ROWS_PER_BEAT ); }
inline float NoteRowToBeat( int iRow )			{ return iRow / (float)ROWS_PER_BEAT; }

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
