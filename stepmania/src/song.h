#pragma once
/*
-----------------------------------------------------------------------------
 Class: Song

 Desc: See header.

 Copyright (c) 2001-2002 by the person(s) listed below.  All rights reserved.
	Chris Danford
-----------------------------------------------------------------------------
*/

#include "GameConstantsAndTypes.h"
#include "Grade.h"

struct Notes;
class StyleDef;
class NotesLoader;

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
public:
	/* Set when this song should be displayed in the music wheel: */
	enum { SHOW_ALWAYS,		/* all the time */
		   SHOW_ROULETTE,	/* only when rouletting */
		   SHOW_NEVER }		/* never (unless song hiding is turned off) */
		m_SelectionDisplay;

	Song();
	~Song();
	
	bool LoadWithoutCache( CString sDir );
	NotesLoader *MakeLoader( CString sDir ) const;

	bool LoadFromSongDir( CString sDir );

	void TidyUpData();	// call after loading to clean up invalid data

	void SaveToSMFile( CString sPath, bool bSavingCache );
	void Save();	// saves SM and DWI
	void SaveToCacheFile();
	void SaveToDWIFile();

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

	CString GetMusicPath() const;
	CString GetBannerPath() const;
	CString GetBackgroundPath() const;
	CString GetCDTitlePath() const;
	CString GetMovieBackgroundPath() const;


	bool HasMusic() const;
	bool HasBanner() const;
	bool HasBackground() const;
	bool HasCDTitle() const;
	bool HasMovieBackground() const;
	bool HasBGChanges() const;

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

	bool SongCompleteForStyle( const StyleDef *st ) const;
	bool SongHasNotesType( NotesType nt ) const;
	bool SongHasNotesTypeAndDifficulty( NotesType nt, DifficultyClass dc ) const;
	void GetNotesThatMatch( const StyleDef *s, int p, CArray<Notes*, Notes*>& arrayAddTo, bool UseFallbacks = true) const;
	int GetNotesThatMatch( NotesType nt, CArray<Notes*, Notes*>& arrayAddTo ) const;
	int GetNumTimesPlayed() const;
	bool IsNew() const;
	bool IsEasy( NotesType nt ) const;
	Grade GetGradeForDifficultyClass( const StyleDef *s, int p, DifficultyClass dc ) const;
	bool NormallyDisplayed() const;
	bool RouletteDisplayed() const;
};


void SortSongPointerArrayByTitle( CArray<Song*, Song*> &arraySongPointers );
void SortSongPointerArrayByBPM( CArray<Song*, Song*> &arraySongPointers );
void SortSongPointerArrayByArtist( CArray<Song*, Song*> &arraySongPointers );
void SortSongPointerArrayByGroup( CArray<Song*, Song*> &arraySongPointers );
void SortSongPointerArrayByMostPlayed( CArray<Song*, Song*> &arraySongPointers );


