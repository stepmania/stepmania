#ifndef NOTES_LOADER_BMS_H
#define NOTES_LOADER_BMS_H

#include "Song.h"
#include "Notes.h"

class BMSLoader {
	bool LoadFromBMSFile( const CString &sPath, Notes &out );

public:
	bool LoadFromBMSDir( CString sDir, Song &out );
};

#endif
