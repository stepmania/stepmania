#ifndef NOTES_LOADER_KSF_H
#define NOTES_LOADER_KSF_H

#include "Song.h"
#include "Notes.h"
#include "NotesLoader.h"

class KSFLoader: public NotesLoader {
	bool LoadFromKSFFile( const CString &sPath, Notes &out );

public:
	void GetApplicableFiles( CString sPath, CStringArray &out );
	bool LoadFromDir( CString sDir, Song &out );

};

#endif
