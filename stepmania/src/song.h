//-----------------------------------------------------------------------------
// File: Song.h
//
// Desc: Holds metadata for a song and the song's step data.
//
// Copyright (c) 2001 Chris Danford.  All rights reserved.
//-----------------------------------------------------------------------------

#ifndef _SONG_H_
#define _SONG_H_


#include <afxtempl.h>
#include "Steps.h"


#define BEATS_IN_MEASURE	4


class Song
{
public:
	Song() :
	  m_fBPM(0.0),
	  m_fBeatOffset(0.0)
	{
	};
	Song( Song &from )
	{
		Copy( from );
	};
	Song& operator=( const Song &from )
	{
		if (this != &from)
        {
			Copy( from );
        }
        return *this;
	};
	void Copy( const Song &from )
	{
		m_sSongFile		= from.m_sSongFile;
		m_sSongDir		= from.m_sSongDir;
		m_sTitle		= from.m_sTitle;
		m_sArtist		= from.m_sArtist;
		m_sCreator		= from.m_sCreator;
		m_fBPM			= from.m_fBPM;
		m_fBeatOffset	= from.m_fBeatOffset;

		m_sMusic		= from.m_sMusic;
		m_sSample		= from.m_sSample;
		m_sBanner		= from.m_sBanner;
		m_sBackground	= from.m_sBackground;

		arraySteps.Copy( from.arraySteps );
	};

	bool LoadFromSongDir( CString sDir );

	bool IsUsingBitmapBG()
	{
		CString sBGFile = m_sBackground;
		sBGFile.MakeLower();	
		return sBGFile.Right(3) == "bmp";
	};

private:
	BOOL LoadFromBMSFile( CString sPath );
	BOOL LoadFromMSDFile( CString sPath );

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
	FLOAT GetBeatOffset()		{return m_fBeatOffset; };
	FLOAT GetBPM()				{return m_fBPM; };
	FLOAT GetBeatsPerSecond()	{return m_fBPM / 60.0f; };

public:


private:
	CString m_sSongFile;
	CString m_sSongDir;

	CString	m_sTitle;
	CString	m_sArtist;
	CString	m_sCreator;
	FLOAT	m_fBPM;
	FLOAT	m_fBeatOffset;

	CString	m_sMusic;
	CString	m_sSample;
	CString	m_sBanner;
	CString	m_sBackground;

public:
	CArray<Steps, Steps&> arraySteps;
};



#endif