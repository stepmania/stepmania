#pragma once
/*
-----------------------------------------------------------------------------
 Class: Song

 Desc: See header.

 Copyright (c) 2001-2002 by the person(s) listed below.  All rights reserved.
	Chris Danford
-----------------------------------------------------------------------------
*/

#include "Notes.h"

#include "GameConstantsAndTypes.h"
#include "RageUtil.h"


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

	bool LoadFromDWIFile( CString sPath );
	bool LoadFromBMSDir( CString sDir );
	bool LoadFromSMFile( CString sPath );

	void TidyUpData();	// call after loading to clean up invalid data

	void SaveToSMFile( CString sPath = "" );	// no path means save to SongFilePath
	void SaveToCacheFile();

	CString GetSongFilePath();
	CString GetCacheFilePath();

public:
	CString m_sSongDir;
	CString m_sGroupName;


	bool	m_bChangedSinceSave;

	CString	m_sMainTitle;
	CString	m_sSubTitle;
	CString	m_sArtist;
	CString	m_sCredit;

	CString GetFullTitle()		{return m_sMainTitle + " " + m_sSubTitle; };
	static void GetMainAndSubTitlesFromFullTitle( CString sFullTitle, CString &sMainTitleOut, CString &sSubTitleOut );


	CString	m_sMusicFile;
	DWORD	m_iMusicBytes;
	float	m_fOffsetInSeconds;
	float	m_fMusicLengthSeconds;
	float	m_fMusicSampleStartSeconds;
	float	m_fMusicSampleLengthSeconds;
	CString	m_sBannerFile;
	CString	m_sBackgroundFile;
	CString	m_sBackgroundMovieFile;
	CString	m_sCDTitleFile;

	CString GetMusicPath()			{return m_sSongDir+m_sMusicFile; };
	CString GetBannerPath()			{return m_sSongDir+m_sBannerFile; };
	CString GetBackgroundPath()		{return m_sSongDir+m_sBackgroundFile; };
	CString GetBackgroundMoviePath(){return m_sSongDir+m_sBackgroundMovieFile; };
	CString GetCDTitlePath()		{return m_sSongDir+m_sCDTitleFile; };


	bool HasMusic()				{return m_sMusicFile != ""			&&	DoesFileExist(GetMusicPath()); };
	bool HasBanner()			{return m_sBannerFile != ""			&&  DoesFileExist(GetBannerPath()); };
	bool HasBackground()		{return m_sBackgroundFile != ""		&&  DoesFileExist(GetBackgroundPath()); };
	bool HasBackgroundMovie()	{return m_sBackgroundMovieFile != ""&&  DoesFileExist(GetBackgroundMoviePath()); };
	bool HasCDTitle()			{return m_sCDTitleFile != ""		&&  DoesFileExist(GetCDTitlePath()); };


	CArray<BPMSegment, BPMSegment&> m_BPMSegments;	// this must be sorted before gameplay
	CArray<FreezeSegment, FreezeSegment&> m_FreezeSegments;	// this must be sorted before gameplay

	void AddBPMSegment( BPMSegment seg );
	void AddFreezeSegment( FreezeSegment seg );
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
	float GetBPMAtBeat( float fBeat )
	{
		for( int i=0; i<m_BPMSegments.GetSize()-1; i++ )
			if( m_BPMSegments[i+1].m_fStartBeat > fBeat )
				break;
		return m_BPMSegments[i].m_fBPM;
	};
	void GetBeatAndBPSFromElapsedTime( float fElapsedTime, float &fBeatOut, float &fBPSOut, bool &bFreezeOut );
	float GetElapsedTimeFromBeat( float fBeat );
	
	
	
	CArray<Notes*, Notes*> m_arrayNotes;

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
	Grade GetGradeForDifficultyClass( NotesType nt, DifficultyClass dc );
};


void SortSongPointerArrayByTitle( CArray<Song*, Song*> &arraySongPointers );
void SortSongPointerArrayByBPM( CArray<Song*, Song*> &arraySongPointers );
void SortSongPointerArrayByArtist( CArray<Song*, Song*> &arraySongPointers );
void SortSongPointerArrayByGroup( CArray<Song*, Song*> &arraySongPointers );
void SortSongPointerArrayByMostPlayed( CArray<Song*, Song*> &arraySongPointers );


