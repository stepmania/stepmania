#ifndef NOTES_WRITER_DWI_H
#define NOTES_WRITER_DWI_H

#include "song.h"
#include "Steps.h"
#include "NoteTypes.h"

class NotesWriterDWI
{
	CString NotesToDWIString( TapNote cNoteCol1, TapNote cNoteCol2, TapNote cNoteCol3, TapNote cNoteCol4, TapNote cNoteCol5, TapNote cNoteCol6 );
	CString NotesToDWIString( TapNote cNoteCol1, TapNote cNoteCol2, TapNote cNoteCol3, TapNote cNoteCol4 );
	void WriteDWINotesField( FILE* fp, const Steps &out, int start );
	bool WriteDWINotesTag( FILE* fp, const Steps &out );
	CString NotesToDWIString( const TapNote cNoteCols[6] );
	CString OptimizeDWIString( CString holds, CString taps );
	char OptimizeDWIPair( char c1, char c2 );

public:
	bool Write( CString sPath, const Song &out );
};

#endif
