/*
-----------------------------------------------------------------------------
 Class: Song

 Desc: Holds data about a piece of music that can be played by one or more
	Games.

 Copyright (c) 2001-2002 by the names listed below.  All rights reserved.
	Chris Danford
-----------------------------------------------------------------------------
*/

#ifndef _SONG_H_
#define _SONG_H_


#include "Pattern.h"

#include "GameTypes.h"
#include "RageUtil.h"
//enum DanceStyle;	// why is this needed?



struct BPMSegment 
{
	BPMSegment() { m_fStartBeat = m_fBPM = -1; };
	float m_fStartBeat;
	float m_fBPM;
};

struct FreezeSegment 
{
	FreezeSegment() { m_fStartBeat = m_fFreezeSeconds = -1; };
	float m_fStartBeat;
	float m_fFreezeSeconds;
};


class Song
{
public:
	Song();

	bool LoadFromSongDir( CString sDir );
	bool LoadSongInfoFromDWIFile( CString sPath );
	bool LoadSongInfoFromBMSDir( CString sDir );

	void SaveOffsetChangeToDisk();

private:

	void TidyUpData();

public:
	CString GetSongFilePath()		{return m_sSongFilePath; };
	CString GetSongFileDir()		{return m_sSongDir; };
	CString GetGroupName()			{return m_sGroupName; };
	CString GetMusicPath()			{return m_sMusicPath; };
	CString GetBannerPath()			{return m_sBannerPath; };
	CString GetBackgroundPath()		{return m_sBackgroundPath; };
	CString GetBackgroundMoviePath(){return m_sBackgroundMoviePath; };


	bool HasMusic()				{return m_sMusicPath != ""			&&	DoesFileExist(GetMusicPath()); };
	bool HasBanner()			{return m_sBannerPath != ""			&&  DoesFileExist(GetBannerPath()); };
	bool HasBackground()		{return m_sBackgroundPath != ""		&&  DoesFileExist(GetBackgroundPath()); };
	bool HasBackgroundMovie()	{return m_sBackgroundMoviePath != ""&&  DoesFileExist(GetBackgroundMoviePath()); };


	CString GetTitle()				{return m_sTitle; };
	CString GetArtist()				{return m_sArtist; };
	CString GetCreator()			{return m_sCreator; };
	float GetBeatOffsetInSeconds()	{return m_fOffsetInSeconds; };
	void SetBeatOffsetInSeconds(float fNewOffset)	{m_fOffsetInSeconds = fNewOffset; };
	void GetMinMaxBPM( float &fMinBPM, float &fMaxBPM )
	{
		fMaxBPM = 0;
		fMinBPM = 100000;	// inf
		for( int i=0; i<m_BPMSegments.GetSize(); i++ ) 
		{
			BPMSegment &seg = m_BPMSegments[i];
			fMaxBPM = max( seg.m_fBPM, fMaxBPM );
			fMinBPM = min( seg.m_fBPM, fMinBPM );
		}
	};
	void GetBeatAndBPSFromElapsedTime( float fElapsedTime, float &fBeatOut, float &fBPSOut );
	void GetPatternsThatMatchStyle( DanceStyle s, CArray<Pattern*, Pattern*>& arrayAddTo );
	void GetNumFeet( DanceStyle s, int& iDiffEasy, int& iDiffMedium, int& iDiffHard );

	int GetNumTimesPlayed()
	{
		int iTotalNumTimesPlayed = 0;
		for( int i=0; i<m_arrayPatterns.GetSize(); i++ )
		{
			iTotalNumTimesPlayed += m_arrayPatterns[i].m_iNumTimesPlayed;
		}
		return iTotalNumTimesPlayed;
	}

	bool HasChangedSinceLastSave()	{ return m_bChangedSinceSave;	}
	void SetChangedSinceLastSave()	{ m_bChangedSinceSave = true;	}

private:
	CString m_sSongFilePath;
	CString m_sSongDir;
	CString m_sGroupName;

	bool	m_bChangedSinceSave;
	CString	m_sTitle;
	CString	m_sArtist;
	CString	m_sCreator;
	float	m_fOffsetInSeconds;

	CString	m_sMusicPath;
	CString	m_sBannerPath;
	CString	m_sBackgroundPath;
	CString	m_sBackgroundMoviePath;

	CArray<BPMSegment, BPMSegment&> m_BPMSegments;	// this must be sorted before dancing
	CArray<FreezeSegment, FreezeSegment&> m_FreezeSegments;	// this must be sorted before dancing

public:
	CArray<Pattern, Pattern&> m_arrayPatterns;
};


void SortSongPointerArrayByTitle( CArray<Song*, Song*> &arraySongPointers );
void SortSongPointerArrayByBPM( CArray<Song*, Song*> &arraySongPointers );
void SortSongPointerArrayByArtist( CArray<Song*, Song*> &arraySongPointers );
void SortSongPointerArrayByGroup( CArray<Song*, Song*> &arraySongPointers );
void SortSongPointerArrayByMostPlayed( CArray<Song*, Song*> &arraySongPointers );



#endif