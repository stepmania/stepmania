/* KSFLoader - reads a Song from a set of .BMS files. */

#ifndef NOTES_LOADER_BMS_H
#define NOTES_LOADER_BMS_H

#include "NotesLoader.h"
#include <map>

class Song;
class Steps;

class BMSLoader: public NotesLoader
{
	void SlideDuplicateDifficulties( Song &p );

	typedef multimap<RString, RString> NameToData_t;
	bool ReadBMSFile( const RString &sPath, BMSLoader::NameToData_t &mapNameToData );
	bool LoadFromBMSFile( const RString &sPath, const NameToData_t &mapNameToData, Steps &out1 );
	void ReadGlobalTags( const NameToData_t &mapNameToData, Song &out );
	static bool GetTagFromMap( const BMSLoader::NameToData_t &mapNameToData, const RString &sName, RString &sOut );
	static bool GetCommonTagFromMapList( const vector<NameToData_t> &aBMSData, const RString &sName, RString &out );
	void SearchForDifficulty( RString sTag, Steps *pOut );

	typedef map<int, float> MeasureToTimeSig_t;
	void ReadTimeSigs( const NameToData_t &mapNameToData, MeasureToTimeSig_t &out );
	float GetBeatsPerMeasure( const MeasureToTimeSig_t &sigs, int iMeasure );
	int GetMeasureStartRow( const MeasureToTimeSig_t &sigs, int iMeasureNo );
	void SetTimeSigAdjustments( const MeasureToTimeSig_t &sigs, Song *pOut );
	MeasureToTimeSig_t m_TimeSigAdjustments;

	RString m_sDir;
	map<RString,int> m_mapWavIdToKeysoundIndex;

public:
	void GetApplicableFiles( RString sPath, vector<RString> &out );
	bool LoadFromDir( RString sDir, Song &out );
};

#endif

/*
 * (c) 2001-2003 Chris Danford
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
