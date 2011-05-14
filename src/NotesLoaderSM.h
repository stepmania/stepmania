#ifndef NotesLoaderSM_H
#define NotesLoaderSM_H

#include "GameConstantsAndTypes.h"
#include "BackgroundUtil.h"
#include "MsdFile.h" // we require the struct from here.

class Song;
class Steps;
class TimingData;
/** @brief Reads a Song from an .SM file. */
namespace SMLoader
{
	void LoadFromSMTokens( RString sStepsType, RString sDescription, RString sDifficulty,
			      RString sMeter, RString sRadarValues, RString sNoteData, Steps &out );
	
	bool LoadFromDir( const RString &sPath, Song &out );
	void TidyUpData( Song &song, bool bFromCache );

	bool LoadFromSMFile( const RString &sPath, Song &out, bool bFromCache = false );
	void GetApplicableFiles( const RString &sPath, vector<RString> &out );
	bool LoadTimingFromFile( const RString &fn, TimingData &out );
	void LoadTimingFromSMFile( const MsdFile &msd, TimingData &out );
	bool LoadEditFromFile( RString sEditFilePath, ProfileSlot slot, bool bAddStepsToSong );
	bool LoadEditFromBuffer( const RString &sBuffer, const RString &sEditFilePath, ProfileSlot slot );
	bool LoadEditFromMsd( const MsdFile &msd, const RString &sEditFilePath, ProfileSlot slot, bool bAddStepsToSong );
	bool LoadFromBGChangesString( BackgroundChange &change, const RString &sBGChangeExpression );
	
	
	bool ProcessBPMs( TimingData &, const RString );
	void ProcessStops( TimingData &, const RString );
	void ProcessDelays( TimingData &, const RString );
	void ProcessTimeSignatures( TimingData &, const RString );
	void ProcessTickcounts( TimingData &, const RString );
	void ProcessBGChanges( Song &out, const RString &sValueName, 
			      const RString &sPath, const RString &sParam );
	void ProcessAttacks( Song &out, MsdFile::value_t sParams );
}

#endif

/**
 * @file
 * @author Chris Danford, Glenn Maynard (c) 2001-2004
 * @section LICENSE
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
