/*
-----------------------------------------------------------------------------
 File: Song.h

 Desc: Holds metadata for a song and the song's step data.

 Copyright (c) 2001 Chris Danford.  All rights reserved.
-----------------------------------------------------------------------------
*/

#ifndef _SONG_H_
#define _SONG_H_


#include "Steps.h"
class Steps;	// why is this needed?


class Song
{
public:
	Song();

	bool LoadFromSongDir( CString sDir );

	bool IsUsingMovieBG()
	{
		CString sBGFile = m_sBackground;
		sBGFile.MakeLower();	
		return sBGFile.Right(3) == "avi" || 
				sBGFile.Right(3) == "mpg";
	};

private:
	bool LoadSongInfoFromBMSFile( CString sPath );
	bool LoadSongInfoFromMSDFile( CString sPath );

	void FillEmptyValuesWithDefaults();

public:
	CString GetSongFilePath()	{return m_sSongDir + m_sSongFile; };
	CString GetSongFileDir()	{return m_sSongDir; };
	CString GetMusicPath()		{return m_sSongDir + m_sMusic; };
	CString GetSamplePath()		{return m_sSongDir + m_sSample; };
	CString GetBannerPath()		{return m_sSongDir + m_sBanner; };
	CString GetBackgroundPath()	{return m_sSongDir + m_sBackground; };
//	Steps&  GetStepsAt( int iIndex )  {return arraySteps[iIndex]; };

	CString GetTitle()			{return m_sTitle; };
	CString GetArtist()			{return m_sArtist; };
	CString GetCreator()		{return m_sCreator; };
	float GetBeatOffset()		{return m_fBeatOffset; };
	float GetBPM()				{return m_fBPM; };
	float GetBeatsPerSecond()	{return m_fBPM / 60.0f; };

public:


private:
	CString m_sSongFile;
	CString m_sSongDir;

	CString	m_sTitle;
	CString	m_sArtist;
	CString	m_sCreator;
	float	m_fBPM;
	float	m_fBeatOffset;

	CString	m_sMusic;
	CString	m_sSample;
	CString	m_sBanner;
	CString	m_sBackground;

public:
	CArray<Steps, Steps&> arraySteps;
};



#endif