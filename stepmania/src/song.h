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

#include "Grade.h"




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
	void GetStepsThatMatchGameMode( GameMode gm, CArray<Steps*, Steps*&>& arrayAddTo );
	void GetNumFeet( GameMode gm, int& iDiffEasy, int& iDiffMedium, int& iDiffHard );
	

public:
	int m_iNumTimesPlayed;


private:
	CString m_sSongFilePath;
	CString m_sSongDir;
	CString m_sGroupName;

	bool	m_bChangedSinceSave;
	CString	m_sTitle;
	CString	m_sArtist;
	CString	m_sCreator;
//	float	m_fBPM;
	float	m_fOffsetInSeconds;

	CString	m_sMusicPath;
	CString	m_sBannerPath;
	CString	m_sBackgroundPath;
	CString	m_sBackgroundMoviePath;

	CArray<BPMSegment, BPMSegment&> m_BPMSegments;	// this must be sorted before dancing
	CArray<FreezeSegment, FreezeSegment&> m_FreezeSegments;	// this must be sorted before dancing

public:
	CArray<Steps, Steps&> arraySteps;
};


void SortSongPointerArrayByTitle( CArray<Song*, Song*&> &arraySongPointers );
void SortSongPointerArrayByBPM( CArray<Song*, Song*&> &arraySongPointers );
void SortSongPointerArrayByArtist( CArray<Song*, Song*&> &arraySongPointers );
void SortSongPointerArrayByGroup( CArray<Song*, Song*&> &arraySongPointers );
void SortSongPointerArrayByMostPlayed( CArray<Song*, Song*&> &arraySongPointers );



#endif