#ifndef NOTES_LOADER_SMA_H
#define NOTES_LOADER_SMA_H

#include "GameConstantsAndTypes.h"
#include "NotesLoaderSM.h"
#include "BackgroundUtil.h"

class MsdFile;
class Song;
class Steps;
class TimingData;

/**
 * @brief The various states while parsing a .sma file.
 */
enum SMALoadingStates
{
	SMA_GETTING_SONG_INFO, /**< Retrieving song information. */
	SMA_GETTING_STEP_INFO, /**< Retrieving step information. */
	NUM_SMALoadingStates /**< The number of states used. */
};

/** @brief Reads a Song from a .SMA file. */
struct SMALoader : public SMLoader
{	
	bool LoadFromDir( const RString &sPath, Song &out );
	void TidyUpData( Song &song, bool bFromCache );
	
	bool LoadFromSMAFile( const RString &sPath, Song &out );
	void GetApplicableFiles( const RString &sPath, vector<RString> &out );
	
	bool LoadEditFromFile( RString sEditFilePath, ProfileSlot slot, bool bAddStepsToSong );
	bool LoadEditFromBuffer( const RString &sBuffer, const RString &sEditFilePath, ProfileSlot slot );
	bool LoadEditFromMsd( const MsdFile &msd, const RString &sEditFilePath, ProfileSlot slot, bool bAddStepsToSong );
	bool LoadFromBGChangesString( BackgroundChange &change, const RString &sBGChangeExpression );
	
	void ProcessBeatsPerMeasure( TimingData &out, const RString sParam );
	void ProcessTickcounts( TimingData &out, const int iRowsPerBeat, const RString sParam );
	void ProcessMultipliers( TimingData &out, const int iRowsPerBeat, const RString sParam );
	void ProcessSpeeds( TimingData &out, const int iRowsPerBeat, const RString sParam );
	void ProcessFakes( TimingData &out, const int iRowsPerBeat, const RString sParam );
};

#endif

/**
 * @file
 * @author Aldo Fregoso, Jason Felds (c) 2009-2011
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
