#ifndef NOTES_LOADER_SM_H
#define NOTES_LOADER_SM_H

#include "song.h"
#include "Steps.h"
#include "NotesLoader.h"

class SMLoader: public NotesLoader  {
	static void LoadFromSMTokens( 
		CString sNotesType, 
		CString sDescription,
		CString sDifficulty,
		CString sMeter,
		CString sRadarValues,
		CString sNoteData,		
		Steps &out);

	bool FromCache;

public:
	SMLoader() { FromCache = false; }
	bool LoadFromSMFile( CString sPath, Song &out );
	bool LoadFromSMFile( CString sPath, Song &out, bool cache )
	{
		FromCache=cache;
		return LoadFromSMFile( sPath, out );
	}

	void GetApplicableFiles( CString sPath, CStringArray &out );
	bool LoadFromDir( CString sPath, Song &out );
};

#endif
