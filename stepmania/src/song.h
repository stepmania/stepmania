#pragma once
/*
-----------------------------------------------------------------------------
 Class: Song

 Desc: Holds data about a piece of music that can be played by one or more
	Games.

 Copyright (c) 2001-2002 by the persons listed below.  All rights reserved.
	Chris Danford
-----------------------------------------------------------------------------
*/


#include "NoteMetadata.h"

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
	bool LoadFromDWIFile( CString sPath );
	bool LoadFromBMSDir( CString sDir );
	bool LoadFromSMDir( CString sDir );

	void Save();

private:

	void TidyUpData();	// call after loading to clean up invalid data

public:
	CString GetSongFilePath()		{return m_sSongFilePath; };
	CString GetSongFileDir()		{return m_sSongDir; };
	CString GetGroupName()			{return m_sGroupName; };
	CString GetMusicPath()			{return m_sMusicPath; };
	void GetMusicSampleRange( float &fStartBeatOut, float &fEndBeatOut ) { fStartBeatOut = m_fMusicSampleStartBeat; fEndBeatOut = m_fMusicSampleEndBeat; };
	CString GetBannerPath()			{return m_sBannerPath; };
	CString GetBackgroundPath()		{return m_sBackgroundPath; };
	CString GetBackgroundMoviePath(){return m_sBackgroundMoviePath; };
	CString GetCDTitlePath()		{return m_sCDTitlePath; };


	bool HasMusic()				{return m_sMusicPath != ""			&&	DoesFileExist(GetMusicPath()); };
	bool HasBanner()			{return m_sBannerPath != ""			&&  DoesFileExist(GetBannerPath()); };
	bool HasBackground()		{return m_sBackgroundPath != ""		&&  DoesFileExist(GetBackgroundPath()); };
	bool HasBackgroundMovie()	{return m_sBackgroundMoviePath != ""&&  DoesFileExist(GetBackgroundMoviePath()); };
	bool HasCDTitle()			{return m_sCDTitlePath != ""		&&  DoesFileExist(GetCDTitlePath()); };


	CString GetFullTitle()		{return m_sTitle; };
	void GetMainTitleAndSubTitle( CString &sMainTitleOut, CString &sSubTitleOut )
	{
		char szSeps[] = { '-', '~' };
		for( int i=0; i<sizeof(szSeps); i++ )
		{
			const char c = szSeps[i];
			int iBeginIndex = m_sTitle.Find( c );
			if( iBeginIndex == -1 )
				continue;
			int iEndIndex = m_sTitle.Find( c, iBeginIndex+1 );	
			if( iEndIndex == -1 )
				continue;
			sMainTitleOut = m_sTitle.Left( iBeginIndex-1 );
			sSubTitleOut = m_sTitle.Mid( iBeginIndex, iEndIndex-iBeginIndex+1 );
			return;
		}
		sMainTitleOut = m_sTitle; 
		sSubTitleOut = ""; 
	};	
	CString GetMainTitle()
	{
		CString sMainTitle, sSubTitle;
		GetMainTitleAndSubTitle( sMainTitle, sSubTitle );
		return sMainTitle;
	};
	CString GetSubTitle()
	{
		CString sMainTitle, sSubTitle;
		GetMainTitleAndSubTitle( sMainTitle, sSubTitle );
		return sSubTitle;
	};
	CString GetArtist()				{return m_sArtist; };
	CString GetCredit()				{return m_sCredit; };
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
	float GetElapsedTimeFromBeat( float fBeat );
	void GetNoteMetadatasThatMatchGameAndStyle( CString sGame, CString sStyle, CArray<NoteMetadata*, NoteMetadata*>& arrayAddTo );

	int GetNumTimesPlayed()
	{
		int iTotalNumTimesPlayed = 0;
		for( int i=0; i<m_arrayNoteMetadatas.GetSize(); i++ )
		{
			iTotalNumTimesPlayed += m_arrayNoteMetadatas[i].m_iNumTimesPlayed;
		}
		return iTotalNumTimesPlayed;
	}

	bool HasChangedSinceLastSave()	{ return m_bChangedSinceSave;	}
	void SetChangedSinceLastSave()	{ m_bChangedSinceSave = true;	}

	Grade GetGradeForDifficultyClass( CString sGame, CString sStyle, DifficultyClass dc );


private:
	CString m_sSongFilePath;
	CString m_sSongDir;
	CString m_sGroupName;

	bool	m_bChangedSinceSave;
	CString	m_sTitle;
	CString	m_sArtist;
	CString	m_sCredit;
	float	m_fOffsetInSeconds;

	CString	m_sMusicPath;
	DWORD	m_dwMusicBytes;
	float	m_fMusicSampleStartBeat, m_fMusicSampleEndBeat;
	CString	m_sBannerPath;
	CString	m_sBackgroundPath;
	CString	m_sBackgroundMoviePath;
	CString	m_sCDTitlePath;

	CArray<BPMSegment, BPMSegment&> m_BPMSegments;	// this must be sorted before dancing
	CArray<FreezeSegment, FreezeSegment&> m_FreezeSegments;	// this must be sorted before dancing

public:
	CArray<NoteMetadata, NoteMetadata&> m_arrayNoteMetadatas;
};


void SortSongPointerArrayByTitle( CArray<Song*, Song*> &arraySongPointers );
void SortSongPointerArrayByBPM( CArray<Song*, Song*> &arraySongPointers );
void SortSongPointerArrayByArtist( CArray<Song*, Song*> &arraySongPointers );
void SortSongPointerArrayByGroup( CArray<Song*, Song*> &arraySongPointers );
void SortSongPointerArrayByMostPlayed( CArray<Song*, Song*> &arraySongPointers );


