#ifndef NOTES_LOADER_BMS_H
#define NOTES_LOADER_BMS_H

#include "song.h"
#include "Notes.h"
#include "NotesLoader.h"

class BMSLoader: public NotesLoader {
	bool LoadFromBMSFile( const CString &sPath, Notes &out1 );
	void mapBMSTrackToDanceNote( int iBMSTrack, int &iDanceColOut, char &cNoteCharOut );

public:
	void GetApplicableFiles( CString sPath, CStringArray &out );
	bool LoadFromDir( CString sDir, Song &out );
};

#endif
