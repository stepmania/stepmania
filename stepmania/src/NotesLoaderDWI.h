#ifndef NOTES_LOADER_DWI_H
#define NOTES_LOADER_DWI_H

#include "Song.h"
#include "Notes.h"
#include "GameInput.h"
#include "NotesLoader.h"

#include "Song.h"
#include "Notes.h"

class DWILoader {
	void DWIcharToNote( char c, GameController i, DanceNote &note1Out, DanceNote &note2Out );

	bool LoadFromDWITokens( 
		CString sMode, CString sDescription, CString sNumFeet, CString sStepData1, 
		CString sStepData2,
		Notes &out, Notes &out2);

public:
	bool LoadFromDWIFile( CString sPath, Song &out );
};

#endif
