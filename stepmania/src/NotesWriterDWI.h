#ifndef NOTES_WRITER_DWI_H
#define NOTES_WRITER_DWI_H

#include "Song.h"
#include "Notes.h"

class NotesWriterDWI {
	char NotesToDWIChar( bool bCol1, bool bCol2, bool bCol3, bool bCol4, bool bCol5, bool bCol6 );
	char NotesToDWIChar( bool bCol1, bool bCol2, bool bCol3, bool bCol4 );
	CString NotesToDWIString( char cNoteCol1, char cNoteCol2, char cNoteCol3, char cNoteCol4, char cNoteCol5, char cNoteCol6 );
	CString NotesToDWIString( char cNoteCol1, char cNoteCol2, char cNoteCol3, char cNoteCol4 );
	void WriteDWINotesField( FILE* fp, const Notes &out, int start );
	bool WriteDWINotesTag( FILE* fp, const Notes &out );

public:
	bool Write( CString sPath, const Song &out );
};

#endif
