/* SMLoader - Reads a Song from an .SM file. */

#ifndef NOTES_LOADER_SM_H
#define NOTES_LOADER_SM_H

#include "NotesLoader.h"
#include "GameConstantsAndTypes.h"

class MsdFile;
class Song;
class Steps;
class TimingData;

class SMLoader: public NotesLoader
{
	static void LoadFromSMTokens( 
		CString sStepsType, 
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

	void GetApplicableFiles( CString sPath, vector<CString> &out );
	bool LoadFromDir( CString sPath, Song &out );
	void TidyUpData( Song &song, bool cache );
	static bool LoadTimingFromFile( const CString &fn, TimingData &out );
	static void LoadTimingFromSMFile( const MsdFile &msd, TimingData &out );
	static bool LoadEdit( CString sEditFilePath, ProfileSlot slot );
	static bool LoadEditFromBuffer( const CString &sBuffer, CString sEditFilePath, ProfileSlot slot );
private:
	static bool LoadEditFromMsd( const MsdFile &msd, CString sEditFilePath, ProfileSlot slot );
};

#endif

/*
 * (c) 2001-2004 Chris Danford, Glenn Maynard
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
