/* NotesWriterDWI - Writes a Song to a .DWI file. */

#ifndef NOTES_WRITER_DWI_H
#define NOTES_WRITER_DWI_H

#include "NoteTypes.h"

class RageFile;
class Song;
class Steps;
class NotesWriterDWI
{
	RString NotesToDWIString( TapNote cNoteCol1, TapNote cNoteCol2, TapNote cNoteCol3, TapNote cNoteCol4, TapNote cNoteCol5, TapNote cNoteCol6 );
	RString NotesToDWIString( TapNote cNoteCol1, TapNote cNoteCol2, TapNote cNoteCol3, TapNote cNoteCol4 );
	void WriteDWINotesField( RageFile &f, const Steps &out, int start );
	bool WriteDWINotesTag( RageFile &f, const Steps &out );
	RString NotesToDWIString( const TapNote cNoteCols[6] );
	RString OptimizeDWIString( RString holds, RString taps );
	char OptimizeDWIPair( char c1, char c2 );

public:
	bool Write( RString sPath, const Song &out );
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
