/* SSCLoader - Reads a Song and its Steps from a .SSC file. */

#ifndef NotesLoaderSSC_H
#define NotesLoaderSSC_H

#include "GameConstantsAndTypes.h"

class MsdFile;
class Song;
class Steps;
class TimingData;

enum SSCLoadingStates
{
	GETTING_SONG_INFO,
	GETTING_STEP_INFO,
	GETTING_STEP_TIMING_INFO,
	GETTING_NOTE_INFO,
	NUM_SSCLoadingStates
};

namespace SSCLoader
{
	bool LoadFromDir( const RString &sPath, Song &out );
	
	bool LoadFromSSCFile( const RString &sPath, Song &out, bool bFromCache = false );
	void GetApplicableFiles( const RString &sPath, vector<RString> &out );
	bool LoadEditFromFile( RString sEditFilePath, ProfileSlot slot, bool bAddStepsToSong );
	bool LoadEditFromMsd( const MsdFile &msd, const RString &sEditFilePath, ProfileSlot slot, bool bAddStepsToSong );
	void TidyUpData( Song &song, bool bFromCache );
	
}

#endif

/*
 * (c) 2011 Jason Felds
 * All rights reserved.
 * 
 * Permission is hereby granted, free of charge, to any person obtaining a
 * copy of this software and associated documentation files (the
 * "Software"), to deal in the Software without restriction, including
 * without limitation the rights to use, copy, modify, merge, publish,
 * distribute, and/or sell copies of the Software, and to permit persons to
 * whom the Software is furnished to do so, provided that the above
 * copyright notice(s) and this permission notice appear in all copies of
 * the Software and that both the above copyright notice(s) and this
 * permission notice appear in supporting documentation.
 * 
 * THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS
 * OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF
 * MERCHANTABILITY, FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT OF
 * THIRD PARTY RIGHTS. IN NO EVENT SHALL THE COPYRIGHT HOLDER OR HOLDERS
 * INCLUDED IN THIS NOTICE BE LIABLE FOR ANY CLAIM, OR ANY SPECIAL INDIRECT
 * OR CONSEQUENTIAL DAMAGES, OR ANY DAMAGES WHATSOEVER RESULTING FROM LOSS
 * OF USE, DATA OR PROFITS, WHETHER IN AN ACTION OF CONTRACT, NEGLIGENCE OR
 * OTHER TORTIOUS ACTION, ARISING OUT OF OR IN CONNECTION WITH THE USE OR
 * PERFORMANCE OF THIS SOFTWARE.
 */
