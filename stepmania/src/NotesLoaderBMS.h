#ifndef NOTES_LOADER_BMS_H
#define NOTES_LOADER_BMS_H

#include "Song.h"
#include "Notes.h"
#include "NotesLoader.h"

class BMSLoader {
	bool LoadFromBMSFile( const CString &sPath, Notes &out1, Notes &out2 );
	void mapBMSTrackToDanceNote( int iBMSTrack, int &iDanceColOut, char &cNoteCharOut );

public:
	bool LoadFromBMSDir( CString sDir, Song &out );
};

#endif
