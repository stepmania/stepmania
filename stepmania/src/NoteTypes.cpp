#include "global.h"
#include "NoteTypes.h"

TapNote TAP_EMPTY				= { TapNote::empty, TapNote::original, 0 };
TapNote TAP_ORIGINAL_TAP		= { TapNote::tap, TapNote::original, 0 };
TapNote TAP_ORIGINAL_HOLD_HEAD	= { TapNote::hold_head, TapNote::original, 0 };		// '2'
TapNote TAP_ORIGINAL_HOLD_TAIL	= { TapNote::hold_tail, TapNote::original, 0 };		// '3'
TapNote TAP_ORIGINAL_HOLD		= { TapNote::hold, TapNote::original, 0 };			// '4'
TapNote TAP_ORIGINAL_MINE		= { TapNote::mine, TapNote::original, 0 };
TapNote TAP_ADDITION_TAP		= { TapNote::tap, TapNote::addition, 0 };
TapNote TAP_ADDITION_MINE		= { TapNote::mine, TapNote::addition, 0 };

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

NoteType GetNoteType( int iNoteIndex )
{ 
	if(      iNoteIndex % (ROWS_PER_MEASURE/4) == 0)	return NOTE_TYPE_4TH;
	else if( iNoteIndex % (ROWS_PER_MEASURE/8) == 0)	return NOTE_TYPE_8TH;
	else if( iNoteIndex % (ROWS_PER_MEASURE/12) == 0)	return NOTE_TYPE_12TH;
	else if( iNoteIndex % (ROWS_PER_MEASURE/16) == 0)	return NOTE_TYPE_16TH;
	else if( iNoteIndex % (ROWS_PER_MEASURE/24) == 0)	return NOTE_TYPE_24TH;
	else if( iNoteIndex % (ROWS_PER_MEASURE/32) == 0)	return NOTE_TYPE_32ND;
	else if( iNoteIndex % (ROWS_PER_MEASURE/48) == 0)	return NOTE_TYPE_48TH;
	else if( iNoteIndex % (ROWS_PER_MEASURE/64) == 0)	return NOTE_TYPE_64TH;
	else												return NOTE_TYPE_INVALID;
};

CString NoteTypeToString( NoteType nt )
{
	switch( nt )
	{
	case NOTE_TYPE_4TH:		return "4th";
	case NOTE_TYPE_8TH:		return "8th";
	case NOTE_TYPE_12TH:	return "12th";
	case NOTE_TYPE_16TH:	return "16th";
	case NOTE_TYPE_24TH:	return "24th";
	case NOTE_TYPE_32ND:	return "32nd";
	case NOTE_TYPE_48TH:	return "48th";
	case NOTE_TYPE_64TH:	return "64th";
	case NOTE_TYPE_192ND:	// fall through
	case NOTE_TYPE_INVALID:	return "192nd";
	default:	ASSERT(0);	return "";
	}
}

NoteType BeatToNoteType( float fBeat )
{ 
	return GetNoteType( BeatToNoteRow(fBeat) );
}

bool IsNoteOfType( int iNoteIndex, NoteType t )
{ 
	return GetNoteType(iNoteIndex) == t;
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
