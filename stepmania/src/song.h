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

	void TidyUpData();

public:
	CString GetSongFilePath()	{return m_sSongDir + m_sSongFile; };
	CString GetSongFileDir()	{return m_sSongDir; };
	CString GetMusicPath()		{return m_sSongDir + m_sMusic; };
	CString GetBannerPath()		{return m_sSongDir + m_sBanner; };
	CString GetBackgroundPath()	{return m_sSongDir + m_sBackground; };
//	Steps&  GetStepsAt( int iIndex )  {return arraySteps[iIndex]; };

	CString GetTitle()			{return m_sTitle; };
	CString GetArtist()			{return m_sArtist; };
	CString GetCreator()		{return m_sCreator; };
//	float GetBeatOffset()		{return m_fBeatOffset; };
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
	void GetBeatAndBPSFromElapsedTime( float fElapsedTime, float &fBeatOut, float &fBPSOut )
	{
		if( fElapsedTime > 50 )
			fElapsedTime = fElapsedTime;
		fElapsedTime += m_fOffsetInSeconds;

		for( int i=0; i<m_BPMSegments.GetSize(); i++ ) {
			int iStartBeatThisSegment = m_BPMSegments[i].m_iStartBeat;
			bool bIsLastBPMSegment = i==m_BPMSegments.GetSize()-1;
			int iStartBeatNextSegment = bIsLastBPMSegment ? 40000/*inf*/ : m_BPMSegments[i+1].m_iStartBeat; 
			int iBeatsInThisSegment = iStartBeatNextSegment - iStartBeatThisSegment;
			float fBPM = m_BPMSegments[i].m_fBPM;
			float fBPS = fBPM / 60.0f;
			float fSecondsInThisSegment =  iBeatsInThisSegment / fBPS;
			if( fElapsedTime > fSecondsInThisSegment )
				fElapsedTime -= fSecondsInThisSegment;
			else {
				fBeatOut = iStartBeatThisSegment + fElapsedTime*fBPS;
				fBPSOut = fBPS;
				return;
			}
		}
	};

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

	struct BPMSegment{ int m_iStartBeat; float m_fBPM; };
	CArray<BPMSegment, BPMSegment&> m_BPMSegments;

public:
	CArray<Steps, Steps&> arraySteps;
};



#endif