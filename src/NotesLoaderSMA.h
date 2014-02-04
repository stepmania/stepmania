#ifndef NOTES_LOADER_SMA_H
#define NOTES_LOADER_SMA_H

#include "GameConstantsAndTypes.h"
#include "NotesLoaderSM.h"
#include "BackgroundUtil.h"

class MsdFile;
class Song;
class Steps;
class TimingData;

/** @brief Reads a Song from a .SMA file. */
struct SMALoader : public SMLoader
{	
	SMALoader() : SMLoader(".sma") {}
	
	virtual bool LoadFromSimfile( const RString &sPath, Song &out, bool bFromCache = false );

	
	void ProcessBeatsPerMeasure( TimingData &out, const RString sParam );
	void ProcessMultipliers( TimingData &out, const int iRowsPerBeat, const RString sParam );
	/**
	 * @brief Process the Speed Segments from the string.
	 * @param out the TimingData being modified.
	 * @param line the string in question.
	 * @param rowsPerBeat the number of rows per beat for this purpose. */
	virtual void ProcessSpeeds( TimingData &out, const RString line, const int rowsPerBeat );
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
