#ifndef LYRICS_LOADER_H
#define LYRICS_LOADER_H
/*
 * Loads lyrics from an LRC file.
 */

#include "song.h"

class LyricsLoader {
public:
	bool LoadFromLRCFile( CString sPath, Song &out );
};

#endif
/*
-----------------------------------------------------------------------------
 Copyright (c) 2003 by the person(s) listed below.  All rights reserved.
	Kevin Slaughter
	Glenn Maynard
-----------------------------------------------------------------------------
*/
