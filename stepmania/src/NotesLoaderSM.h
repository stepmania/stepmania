#ifndef NOTES_LOADER_SM_H
#define NOTES_LOADER_SM_H

#include "Song.h"
#include "Notes.h"
#include "NotesLoader.h"

class SMLoader: public NotesLoader  {
	void LoadFromSMTokens( 
		CString sNotesType, 
		CString sDescription,
		CString sDifficulty,
		CString sMeter,
		CString sRadarValues,
		CString sNoteData,		
		Notes &out);

public:
	bool LoadFromSMFile( CString sPath, Song &out );

	void GetApplicableFiles( CString sPath, CStringArray &out );
	bool LoadFromDir( CString sPath, Song &out );
};

#endif
