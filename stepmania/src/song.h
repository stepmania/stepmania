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

extern const int FILE_CACHE_VERSION;

struct BPMSegment 
{
	BPMSegment() { m_fStartBeat = m_fBPM = -1; };
	BPMSegment( float s, float b ) { m_fStartBeat = s; m_fBPM = b; };
	float m_fStartBeat;
	float m_fBPM;
};

struct StopSegment 
{
	StopSegment() { m_fStartBeat = m_fStopSeconds = -1; };
	StopSegment( float s, float f ) { m_fStartBeat = s; m_fStopSeconds = f; };
	float m_fStartBeat;
	float m_fStopSeconds;
};

struct BackgroundChange 
{
	BackgroundChange() { m_fStartBeat = -1; };
	BackgroundChange( float s, CString sBGName ) { m_fStartBeat = s; m_sBGName = sBGName; };
	float m_fStartBeat;
	CString m_sBGName;
};


class Song
{
	/* Set when this song should be displayed in the music wheel: */
	enum { SHOW_ALWAYS,		/* all the time */
		   SHOW_ROULETTE,	/* only when rouletting */
		   SHOW_NEVER }		/* never (unless song hiding is turned off) */
		m_SelectionDisplay;
public:
	Song();
	~Song();
	
	bool LoadWithoutCache( CString sDir );

	bool LoadFromSongDir( CString sDir );	// calls one of the loads below

	bool LoadFromDWIFile( CString sPath );
	bool LoadFromBMSDir( CString sDir );
	bool LoadFromSMFile( CString sPath );
	bool LoadFromKSFDir( CString sDir );

	void TidyUpData();	// call after loading to clean up invalid data

	void SaveToSMFile( CString sPath, bool bSavingCache );
	void SaveToSongFileAndDWI();	// saves SM, then saves DWI so that the SM is the master copy and NoteTypes not supported by DWI are not lost
	void SaveToCacheFile();
	void SaveToSongFile();	// no path means save to SongFilePath

	const CString &GetSongFilePath() const;
	CString GetCacheFilePath() const;

	void AddAutoGenNotes();

	/* Directory this song data came from: */
	CString m_sSongDir;

	/* Filename associated with this file.  This will always have
	 * an .SM extension.  If we loaded an .SM, this will point to 
	 * it, but if we loaded any other type, this will point to a
	 * generated .SM filename. */
	CString m_sSongFileName;

	CString m_sGroupName;


	bool	m_bChangedSinceSave;

	CString	m_sMainTitle;
	CString	m_sSubTitle;
	CString	m_sArtist;
	CString	m_sCredit;

	CString GetFullTitle() const { return m_sMainTitle + (m_sSubTitle.GetLength()? (" " + m_sSubTitle):""); }
	static void GetMainAndSubTitlesFromFullTitle( const CString sFullTitle, CString &sMainTitleOut, CString &sSubTitleOut );


	CString	m_sMusicFile;
	DWORD	m_iMusicBytes;
	float	m_fBeat0OffsetInSeconds;
	float	m_fMusicLengthSeconds;
	float	m_fFirstBeat;
	float	m_fLastBeat;
	float	m_fMusicSampleStartSeconds;
	float	m_fMusicSampleLengthSeconds;

	float GetMusicStartBeat() const;

	CString	m_sBannerFile;
	CString	m_sBackgroundFile;
	CString	m_sCDTitleFile;
	CString	m_sMovieBackgroundFile;

	CString GetMusicPath() const	{return m_sMusicFile.Find('.')==0 ? m_sMusicFile : m_sSongDir+m_sMusicFile; };
	CString GetBannerPath() const	{return m_sSongDir+m_sBannerFile; };
	CString GetBackgroundPath() const {return m_sSongDir+m_sBackgroundFile; };
	CString GetCDTitlePath() const	{return m_sCDTitleFile.Find('.')==0 ? m_sCDTitleFile : m_sSongDir+m_sCDTitleFile; };
	CString GetMovieBackgroundPath() const {return m_sSongDir+m_sMovieBackgroundFile; };


	bool HasMusic() const 		{return m_sMusicFile != ""			&&	IsAFile(GetMusicPath()); };
	bool HasBanner() const 		{return m_sBannerFile != ""			&&  IsAFile(GetBannerPath()); };
	bool HasBackground() const 	{return m_sBackgroundFile != ""		&&  IsAFile(GetBackgroundPath()); };
	bool HasCDTitle() const 	{return m_sCDTitleFile != ""		&&  IsAFile(GetCDTitlePath()); };
	bool HasMovieBackground() const {return m_sMovieBackgroundFile != ""&&  IsAFile(GetMovieBackgroundPath()); };
	bool HasBGChanges() const 	{return m_BackgroundChanges.GetSize() > 0; };

	CArray<BPMSegment, BPMSegment&> m_BPMSegments;	// this must be sorted before gameplay
	CArray<StopSegment, StopSegment&> m_StopSegments;	// this must be sorted before gameplay
	CArray<BackgroundChange, BackgroundChange&> m_BackgroundChanges;	// this must be sorted before gameplay

	void AddBPMSegment( BPMSegment seg );
	void AddStopSegment( StopSegment seg );
	void AddBackgroundChange( BackgroundChange seg );

	void GetMinMaxBPM( float &fMinBPM, float &fMaxBPM ) const
	{
		fMaxBPM = 0;
		fMinBPM = 100000;	// inf
		for( int i=0; i<m_BPMSegments.GetSize(); i++ ) 
		{
			const BPMSegment &seg = m_BPMSegments[i];
			fMaxBPM = max( seg.m_fBPM, fMaxBPM );
			fMinBPM = min( seg.m_fBPM, fMinBPM );
		}
	};
	float GetBPMAtBeat( float fBeat ) const
	{
		for( int i=0; i<m_BPMSegments.GetSize()-1; i++ )
			if( m_BPMSegments[i+1].m_fStartBeat > fBeat )
				break;
		return m_BPMSegments[i].m_fBPM;
	};
	BPMSegment& GetBPMSegmentAtBeat( float fBeat )
	{
		for( int i=0; i<m_BPMSegments.GetSize()-1; i++ )
			if( m_BPMSegments[i+1].m_fStartBeat > fBeat )
				break;
		return m_BPMSegments[i];
	};
	CString GetBackgroundAtBeat( float fBeat )
	{
		for( int i=0; i<m_BackgroundChanges.GetSize()-1; i++ )
			if( m_BackgroundChanges[i+1].m_fStartBeat > fBeat )
				break;
		return m_BackgroundChanges[i].m_sBGName;
	};
	void GetBeatAndBPSFromElapsedTime( float fElapsedTime, float &fBeatOut, float &fBPSOut, bool &bFreezeOut ) const;
	float GetElapsedTimeFromBeat( float fBeat ) const;
	
	
	
	CArray<Notes*, Notes*> m_apNotes;

	bool SongHasNotesType( NotesType nt ) const;
	bool SongHasNotesTypeAndDifficulty( NotesType nt, DifficultyClass dc ) const;
	void GetNotesThatMatch( NotesType nt, CArray<Notes*, Notes*>& arrayAddTo ) const;
	int GetNumTimesPlayed() const
	{
		int iTotalNumTimesPlayed = 0;
		for( int i=0; i<m_apNotes.GetSize(); i++ )
		{
			iTotalNumTimesPlayed += m_apNotes[i]->m_iNumTimesPlayed;
		}
		return iTotalNumTimesPlayed;
	}
	bool IsNew() const;
	bool IsEasy( NotesType nt ) const;
	Grade GetGradeForDifficultyClass( NotesType nt, DifficultyClass dc ) const;
	bool NormallyDisplayed() const;
	bool RouletteDisplayed() const;
};


void SortSongPointerArrayByTitle( CArray<Song*, Song*> &arraySongPointers );
void SortSongPointerArrayByBPM( CArray<Song*, Song*> &arraySongPointers );
void SortSongPointerArrayByArtist( CArray<Song*, Song*> &arraySongPointers );
void SortSongPointerArrayByGroup( CArray<Song*, Song*> &arraySongPointers );
void SortSongPointerArrayByMostPlayed( CArray<Song*, Song*> &arraySongPointers );


