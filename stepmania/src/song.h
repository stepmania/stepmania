#pragma once
/*
-----------------------------------------------------------------------------
 Class: Song

 Desc: Holds data about a piece of music that can be played by one or more
	Games.

 Copyright (c) 2001-2002 by the person(s) listed below.  All rights reserved.
	Chris Danford
-----------------------------------------------------------------------------
*/


#include "Notes.h"

#include "GameConstantsAndTypes.h"
#include "RageUtil.h"
//enum DanceStyle;	// why is this needed?



struct BPMSegment 
{
	BPMSegment() { m_fStartBeat = m_fBPM = -1; };
	BPMSegment( float s, float b ) { m_fStartBeat = s; m_fBPM = b; };
	float m_fStartBeat;
	float m_fBPM;
};

struct FreezeSegment 
{
	FreezeSegment() { m_fStartBeat = m_fFreezeSeconds = -1; };
	FreezeSegment( float s, float f ) { m_fStartBeat = s; m_fFreezeSeconds = f; };
	float m_fStartBeat;
	float m_fFreezeSeconds;
};


class Song
{
public:
	Song();
	~Song();

	bool LoadFromSongDir( CString sDir );	// calls one of the loads below
	void Save()			{ SaveToSMDir(); SaveToCacheFile(); }; 

	bool LoadFromCacheFile( bool bLoadNoteData );

protected:
	bool LoadFromDWIFile( CString sPath );
	bool LoadFromBMSDir( CString sDir );
	bool LoadFromSMDir( CString sDir );
	void TidyUpData();	// call after loading to clean up invalid data

	void SaveToSMDir();	// saves to StepMania song and notes files
	void SaveToCacheFile();	// saves to cache file

	void DeleteCacheFile();

public:
	CString GetSongFilePath()		{return m_sSongFilePath; };
	CString GetCacheFilePath();
	CString GetSongFileDir()		{return m_sSongDir; };
	CString GetGroupName()			{return m_sGroupName; };
	CString GetMusicPath()			{return m_sMusicPath; };
	float GetMusicLengthSeconds()	{return m_fMusicLength; };
	void GetMusicSampleRange( float &fStartSec, float &fEndSec ) { fStartSec = m_fMusicSampleStartSeconds; fEndSec = m_fMusicSampleStartSeconds + m_fMusicSampleLengthSeconds; };
	CString GetBannerPath()			{return m_sBannerPath; };
	CString GetBackgroundPath()		{return m_sBackgroundPath; };
	CString GetBackgroundMoviePath(){return m_sBackgroundMoviePath; };
	CString GetCDTitlePath()		{return m_sCDTitlePath; };


	bool HasMusic()				{return m_sMusicPath != ""			&&	DoesFileExist(GetMusicPath()); };
	bool HasBanner()			{return m_sBannerPath != ""			&&  DoesFileExist(GetBannerPath()); };
	bool HasBackground()		{return m_sBackgroundPath != ""		&&  DoesFileExist(GetBackgroundPath()); };
	bool HasBackgroundMovie()	{return m_sBackgroundMoviePath != ""&&  DoesFileExist(GetBackgroundMoviePath()); };
	bool HasCDTitle()			{return m_sCDTitlePath != ""		&&  DoesFileExist(GetCDTitlePath()); };


	CString GetMainTitle()		{return m_sMainTitle; };
	CString GetSubTitle()		{return m_sSubTitle; };
	CString GetFullTitle()		{return m_sMainTitle + " " + m_sSubTitle; };
	void GetMainAndSubTitlesFromFullTitle( CString sFullTitle, CString &sMainTitleOut, CString &sSubTitleOut );

	CString GetArtist()				{return m_sArtist; };
	CString GetCredit()				{return m_sCredit; };
	float GetBeatOffsetInSeconds()	{return m_fOffsetInSeconds; };
	void SetBeatOffsetInSeconds(float fNewOffset)	{m_fOffsetInSeconds = fNewOffset; SetChangedSinceLastSave(); };
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
	void GetNotesThatMatch( NotesType nt, CArray<Notes*, Notes*>& arrayAddTo );

	int GetNumTimesPlayed()
	{
		int iTotalNumTimesPlayed = 0;
		for( int i=0; i<m_arrayNotes.GetSize(); i++ )
		{
			iTotalNumTimesPlayed += m_arrayNotes[i]->m_iNumTimesPlayed;
		}
		return iTotalNumTimesPlayed;
	}
	bool IsNew() { return GetNumTimesPlayed()==0; };

	bool HasChangedSinceLastSave()	{ return m_bChangedSinceSave;	}

	Grade GetGradeForDifficultyClass( NotesType nt, DifficultyClass dc );


private:
	void SetChangedSinceLastSave()	{ m_bChangedSinceSave = true;	}

	CString m_sSongFilePath;
	CString m_sSongDir;
	CString m_sGroupName;

	bool	m_bChangedSinceSave;
	CString	m_sMainTitle;
	CString	m_sSubTitle;
	CString	m_sArtist;
	CString	m_sCredit;
	float	m_fOffsetInSeconds;

	CString	m_sMusicPath;
	DWORD	m_iMusicBytes;
	float	m_fMusicLength;
	float	m_fMusicSampleStartSeconds, m_fMusicSampleLengthSeconds;
	CString	m_sBannerPath;
	CString	m_sBackgroundPath;
	CString	m_sBackgroundMoviePath;
	CString	m_sCDTitlePath;

	void AddBPMSegment( BPMSegment seg );
	void AddFreezeSegment( FreezeSegment seg );

	CArray<BPMSegment, BPMSegment&> m_BPMSegments;	// this must be sorted before gameplay
	CArray<FreezeSegment, FreezeSegment&> m_FreezeSegments;	// this must be sorted before gameplay

public:
	CArray<Notes*, Notes*> m_arrayNotes;
};


void SortSongPointerArrayByTitle( CArray<Song*, Song*> &arraySongPointers );
void SortSongPointerArrayByBPM( CArray<Song*, Song*> &arraySongPointers );
void SortSongPointerArrayByArtist( CArray<Song*, Song*> &arraySongPointers );
void SortSongPointerArrayByGroup( CArray<Song*, Song*> &arraySongPointers );
void SortSongPointerArrayByMostPlayed( CArray<Song*, Song*> &arraySongPointers );


