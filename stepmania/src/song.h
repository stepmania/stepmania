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

#include "GameInfo.h"	// for definition of GameMode
enum GameMode;	// why is this needed?


struct BPMSegment{
	BPMSegment() { m_fStartBeat = m_fBPM = m_fFreezeSeconds = 0; };
	float m_fStartBeat, m_fBPM, m_fFreezeSeconds;
};


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
	bool LoadSongInfoFromDWIFile( CString sPath );

	void TidyUpData();

public:
	CString GetSongFilePath()	{return m_sSongDir + m_sSongFile; };
	CString GetSongFileDir()	{return m_sSongDir; };
	CString GetMusicPath()		{return m_sSongDir + m_sMusic; };
	CString GetBannerPath()		{return m_sSongDir + m_sBanner; };
	CString GetBackgroundPath()	{return m_sSongDir + m_sBackground; };
//	Steps&  GetStepsAt( int iIndex )  {return arraySteps[iIndex]; };

	bool HasMusic()			{return m_sMusic != ""		&&	DoesFileExist(GetMusicPath()); };
	bool HasBanner()		{return m_sBanner != ""		&&  DoesFileExist(GetBannerPath()); };
	bool HasBackground()	{return m_sBackground != ""	&&  DoesFileExist(GetBackgroundPath()); };


	CString GetTitle()			{return m_sTitle; };
	CString GetArtist()			{return m_sArtist; };
	CString GetCreator()		{return m_sCreator; };
	float GetBeatOffsetInSeconds()	{return m_fOffsetInSeconds; };
	void SetBeatOffsetInSeconds(float fNewOffset)	{m_fOffsetInSeconds = fNewOffset; };
	void GetMinMaxBPM( int &iMinBPM, int &iMaxBPM )
	{
		iMaxBPM = 0;
		iMinBPM = 100000;	// inf
		for( int i=0; i<m_BPMSegments.GetSize(); i++ ) {
			if( m_BPMSegments[i].m_fBPM > iMaxBPM )
				iMaxBPM = (int)m_BPMSegments[i].m_fBPM;
			if( m_BPMSegments[i].m_fBPM < iMinBPM )
				iMinBPM = (int)m_BPMSegments[i].m_fBPM;
		}
	};
	void GetBeatAndBPSFromElapsedTime( float fElapsedTime, float &fBeatOut, float &fBPSOut );
	float GetBPMAtBeat( float fSongBeat )
	{
		for( int i=0; i<m_BPMSegments.GetSize(); i++ ) {
			if( m_BPMSegments[i].m_fStartBeat > fSongBeat || i==m_BPMSegments.GetSize()-1 )
				break;
		}
		return m_BPMSegments[i].m_fBPM;
	};
	void SetBPM( float fNewBPM, float fSongBeat )
	{
		for( int i=0; i<m_BPMSegments.GetSize(); i++ ) {
			if( m_BPMSegments[i].m_fStartBeat > fSongBeat || i==m_BPMSegments.GetSize()-1 )
				break;
		}
		m_BPMSegments[i].m_fBPM = fNewBPM;
	};

	void GetStepsThatMatchGameMode( GameMode gm, CArray<Steps*, Steps*&>& arrayAddTo );
	void GetNumFeet( GameMode gm, int& iDiffEasy, int& iDiffMedium, int& iDiffHard );
	

public:


private:
	CString m_sSongFile;
	CString m_sSongDir;

	bool	m_bChangedSinceSave;
	CString	m_sTitle;
	CString	m_sArtist;
	CString	m_sCreator;
//	float	m_fBPM;
	float	m_fOffsetInSeconds;

	CString	m_sMusic;
	CString	m_sBanner;
	CString	m_sBackground;

	CArray<BPMSegment, BPMSegment&> m_BPMSegments;	// this must be sorted before dancing

public:
	CArray<Steps, Steps&> arraySteps;
};



#endif