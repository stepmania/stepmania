#ifndef NOTES_LOADER_SM_H
#define NOTES_LOADER_SM_H

#include "Song.h"
#include "Notes.h"

class SMLoader {
	void LoadFromSMTokens( 
		CString sNotesType, 
		CString sDescription,
		CString sDifficultyClass,
		CString sMeter,
		CString sRadarValues,
		CString sNoteData,		
		Notes &out);

public:
	bool LoadFromSMFile( CString sPath, Song &out );

};

#endif
