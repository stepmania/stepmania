#include "global.h"
#include "NoteTypes.h"
#include "RageUtil.h"
#include "LuaManager.h"
#include "LuaFunctions.h"
#include "XmlFile.h"

TapNote TAP_EMPTY					( TapNote::empty,		TapNote::SubType_invalid,	TapNote::original, "", 0, false, 0 );
TapNote TAP_ORIGINAL_TAP			( TapNote::tap,			TapNote::SubType_invalid,	TapNote::original, "", 0, false, 0 );
TapNote TAP_ORIGINAL_HOLD_HEAD		( TapNote::hold_head,	TapNote::hold_head_hold,	TapNote::original, "", 0, false, 0 );
TapNote TAP_ORIGINAL_ROLL_HEAD		( TapNote::hold_head,	TapNote::hold_head_roll,	TapNote::original, "", 0, false, 0 );
TapNote TAP_ORIGINAL_MINE			( TapNote::mine,		TapNote::SubType_invalid,	TapNote::original, "", 0, false, 0 );
TapNote TAP_ORIGINAL_ATTACK			( TapNote::attack,		TapNote::SubType_invalid,	TapNote::original, "", 0, false, 0 );
TapNote TAP_ORIGINAL_AUTO_KEYSOUND	( TapNote::autoKeysound,TapNote::SubType_invalid,	TapNote::original, "", 0, false, 0 );
TapNote TAP_ADDITION_TAP			( TapNote::tap,			TapNote::SubType_invalid,	TapNote::addition, "", 0, false, 0 );
TapNote TAP_ADDITION_MINE			( TapNote::mine,		TapNote::SubType_invalid,	TapNote::addition, "", 0, false, 0 );

static const char *NoteTypeNames[] = {
	"4th",
	"8th",
	"12th",
	"16th",
	"24th",
	"32nd",
	"48th",
	"64th",
	"192nd",
};
XToString( NoteType, NUM_NOTE_TYPES );
LuaXToString( NoteType )
LuaXType( NoteType, NUM_NOTE_TYPES , "NOTE_TYPE_", true )

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
	case NOTE_TYPE_48TH:	return 1.0f/12;
	case NOTE_TYPE_64TH:	return 1.0f/16;
	// MD 11/03/03 - NOTE_TYPE_INVALID should be treated as equivalent to
	//               NOTE_TYPE_192ND; NOTE_TYPE_96TH should not exist.
	// MD 11/12/03 - And, really, since NOTE_TYPE_192ND had to be added, we'll
	//				 keep the behavior of NOTE_TYPE_INVALID being the same, but
	//				 it shouldn't ever come up anyway.
	case NOTE_TYPE_192ND:	return 1.0f/48;
	default:	ASSERT(0); // and fall through
	case NOTE_TYPE_INVALID:	return 1.0f/48;
	}
}

NoteType GetNoteType( int row )
{ 
	if(      row % (ROWS_PER_MEASURE/4) == 0)	return NOTE_TYPE_4TH;
	else if( row % (ROWS_PER_MEASURE/8) == 0)	return NOTE_TYPE_8TH;
	else if( row % (ROWS_PER_MEASURE/12) == 0)	return NOTE_TYPE_12TH;
	else if( row % (ROWS_PER_MEASURE/16) == 0)	return NOTE_TYPE_16TH;
	else if( row % (ROWS_PER_MEASURE/24) == 0)	return NOTE_TYPE_24TH;
	else if( row % (ROWS_PER_MEASURE/32) == 0)	return NOTE_TYPE_32ND;
	else if( row % (ROWS_PER_MEASURE/48) == 0)	return NOTE_TYPE_48TH;
	else if( row % (ROWS_PER_MEASURE/64) == 0)	return NOTE_TYPE_64TH;
	else												return NOTE_TYPE_INVALID;
};

NoteType BeatToNoteType( float fBeat )
{ 
	return GetNoteType( BeatToNoteRow(fBeat) );
}

bool IsNoteOfType( int row, NoteType t )
{ 
	return GetNoteType(row) == t;
}


XNode* TapNoteResult::CreateNode() const
{
	XNode *p = new XNode;
	p->m_sName = "TapNoteResult";

	p->AppendAttr( "TapNoteScore", TapNoteScoreToString(tns) );
	p->AppendAttr( "TapNoteOffset", fTapNoteOffset );

	return p;
}

void TapNoteResult::LoadFromNode( const XNode* pNode )
{
	ASSERT(0);
}

XNode* HoldNoteResult::CreateNode() const
{
	XNode *p = new XNode;
	p->m_sName = "HoldNoteResult";

	return p;
}

void HoldNoteResult::LoadFromNode( const XNode* pNode )
{
	ASSERT(0);
}

XNode* TapNote::CreateNode() const
{
	XNode *p = new XNode;
	p->m_sName = "TapNote";

	p->AppendChild( result.CreateNode() );
	p->AppendChild( HoldResult.CreateNode() );

	return p;
}

void TapNote::LoadFromNode( const XNode* pNode )
{
	ASSERT(0);
}

float HoldNoteResult::GetLastHeldBeat() const
{
	return NoteRowToBeat(iLastHeldRow);
}

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
