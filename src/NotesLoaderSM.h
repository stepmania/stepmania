#ifndef NotesLoaderSM_H
#define NotesLoaderSM_H

#include "GameConstantsAndTypes.h"
#include "BackgroundUtil.h"
#include "MsdFile.h" // we require the struct from here.

class Song;
class Steps;
class TimingData;

/**
 * @brief The highest allowable speed before Warps come in.
 *
 * This was brought in from StepMania 4's recent betas. */
const float FAST_BPM_WARP = 9999999.f;

/** @brief Reads a Song from an .SM file. */
struct SMLoader
{
	virtual ~SMLoader() {}
	
	bool LoadFromDir( const RString &sPath, Song &out );
	/**
	 * @brief Perform some cleanup on the loaded song.
	 * @param song a reference to the song that may need cleaning up.
	 * @param bFromCache a flag to determine if this song is loaded from a cache file.
	 */
	virtual void TidyUpData( Song &song, bool bFromCache );

	bool LoadFromSMFile( const RString &sPath, Song &out, bool bFromCache = false );
	void GetApplicableFiles( const RString &sPath, vector<RString> &out );
	virtual bool LoadEditFromFile( RString sEditFilePath, ProfileSlot slot, bool bAddStepsToSong );
	virtual bool LoadEditFromBuffer( const RString &sBuffer, const RString &sEditFilePath, ProfileSlot slot );
	virtual bool LoadEditFromMsd( const MsdFile &msd, const RString &sEditFilePath, ProfileSlot slot, bool bAddStepsToSong );
	virtual bool LoadFromBGChangesString(BackgroundChange &change, 
					     const RString &sBGChangeExpression );
	
	/**
	 * @brief Process the BPM Segments from the string.
	 * @param out the TimingData being modified.
	 * @param line the string in question.
	 * @param rowsPerBeat the number of rows per beat for this purpose.
	 * @return true if there was at least one segment found, false otherwise. */
	bool ProcessBPMs(TimingData & out,
			 const RString line,
			 const int rowsPerBeat = -1);
	/**
	 * @brief Process the Stop Segments from the string.
	 * @param out the TimingData being modified.
	 * @param line the string in question.
	 * @param rowsPerBeat the number of rows per beat for this purpose. */
	void ProcessStops(TimingData & out,
			  const RString line,
			  const int rowsPerBeat = -1);
	/**
	 * @brief Process the Delay Segments from the string.
	 * @param out the TimingData being modified.
	 * @param line the string in question.
	 * @param rowsPerBeat the number of rows per beat for this purpose. */
	void ProcessDelays(TimingData & out,
			  const RString line,
			  const int rowsPerBeat = -1);
	/**
	 * @brief Process the Time Signature Segments from the string.
	 * @param out the TimingData being modified.
	 * @param line the string in question.
	 * @param rowsPerBeat the number of rows per beat for this purpose. */
	void ProcessTimeSignatures(TimingData & out,
			   const RString line,
			   const int rowsPerBeat = -1);
	/**
	 * @brief Process the Tickcount Segments from the string.
	 * @param out the TimingData being modified.
	 * @param line the string in question.
	 * @param rowsPerBeat the number of rows per beat for this purpose. */
	void ProcessTickcounts(TimingData & out,
				   const RString line,
				   const int rowsPerBeat = -1);
	
	/**
	 * @brief Process the Speed Segments from the string.
	 * @param out the TimingData being modified.
	 * @param line the string in question.
	 * @param rowsPerBeat the number of rows per beat for this purpose. */
	virtual void ProcessSpeeds(TimingData & out,
				   const RString line,
				   const int rowsPerBeat = -1);
	
	virtual void ProcessCombos(TimingData & out,
				   const RString line,
				   const int rowsPerBeat = -1) {}
	
	virtual void ProcessBGChanges( Song &out, const RString &sValueName, 
			      const RString &sPath, const RString &sParam );
	void ProcessAttacks( Song &out, MsdFile::value_t sParams );
	void ProcessInstrumentTracks( Song &out, const RString &sParam );
	
	/**
	 * @brief Convert a row value to the proper beat value.
	 * 
	 * This is primarily used for assistance with converting SMA files.
	 * @param line The line that contains the value.
	 * @param rowsPerBeat the number of rows per beat according to the original file.
	 * @return the converted beat value. */
	float RowToBeat(RString line, const int rowsPerBeat);
	
protected:
	/**
	 * @brief Process the different tokens we have available to get NoteData.
	 * @param stepsType The current StepsType.
	 * @param description The description of the chart.
	 * @param difficulty The difficulty (in words) of the chart.
	 * @param meter the difficulty (in numbers) of the chart.
	 * @param radarValues the calculated radar values.
	 * @param noteData the note data itself.
	 * @param out the Steps getting the data. */
	virtual void LoadFromTokens(RString sStepsType, 
				    RString sDescription,
				    RString sDifficulty,
				    RString sMeter,
				    RString sRadarValues,
				    RString sNoteData,
				    Steps &out);
};

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
