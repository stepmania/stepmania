#ifndef NOTES_LOADER_KSF_H
#define NOTES_LOADER_KSF_H

#include "Song.h"
#include "Notes.h"

class KSFLoader {
	bool LoadFromKSFFile( const CString &sPath, Notes &out );

public:
	bool LoadFromKSFDir( CString sDir, Song &out );

};

#endif
