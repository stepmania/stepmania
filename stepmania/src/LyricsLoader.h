#ifndef LYRICS_LOADER_H
#define LYRICS_LOADER_H
/*
	Declares for lyrics loader
	By: Kevin Slaughter
*/

#include "song.h"
//#include "GameInput.h"

class LyricsLoader {
	//bool LoadFromLRCFile( CString sPath, Song &out );
public:
	void GetApplicableFiles( CString sPath, CStringArray &out );
	bool Loadable( CString sPath );
	bool LoadFromDir( CString sPath, Song &out );
	bool LoadFromLRCFile( CString sPath, Song &out );
private:
	LyricSegment	LySeg;
	float			fSecs;
};

#endif
