#ifndef NOTES_LOADER_KSF_H
#define NOTES_LOADER_KSF_H

#include "song.h"
#include "Steps.h"
#include "NotesLoader.h"

class KSFLoader: public NotesLoader {
	bool LoadFromKSFFile( const CString &sPath, Steps &out, const Song &song );
	bool LoadGlobalData( const CString &sPath, Song &out );
	void RemoveHoles( NoteData &out, const Song &song );
	void LoadTags( const CString &str, Song &out );

public:
	void GetApplicableFiles( CString sPath, CStringArray &out );
	bool LoadFromDir( CString sDir, Song &out );

};

#endif
