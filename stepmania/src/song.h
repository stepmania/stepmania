#ifndef SONG_H
#define SONG_H
/*
-----------------------------------------------------------------------------
 Class: Song

 Desc: See header.

 Copyright (c) 2001-2002 by the person(s) listed below.  All rights reserved.
	Chris Danford
-----------------------------------------------------------------------------
*/

#include "PlayerNumber.h"
#include "GameConstantsAndTypes.h"
#include "Grade.h"
#include "TimingData.h"

class Steps;
class StyleDef;
class NotesLoader;
class LyricsLoader;


extern const int FILE_CACHE_VERSION;


struct BackgroundChange 
{
	BackgroundChange() { m_fStartBeat=-1; m_fRate=1; m_bFadeLast=false; m_bRewindMovie=false; m_bLoop=true; };
	BackgroundChange( float s, CString n, float r=1.f, bool f=false, bool m=false, bool l=true ) { m_fStartBeat=s; m_sBGName=n; m_fRate=r; m_bFadeLast=f; m_bRewindMovie=m; m_bLoop=l; };
	float m_fStartBeat;
	CString m_sBGName;
	float m_fRate;
	bool m_bFadeLast;
	bool m_bRewindMovie;
	bool m_bLoop;
};

void SortBackgroundChangesArray( vector<BackgroundChange> &arrayBackgroundChanges );

struct LyricSegment
{
	float	m_fStartTime;
	CString m_sLyric;
	RageColor m_Color;
};


class Song
{
	CString m_sSongDir;

public:
	/* Set when this song should be displayed in the music wheel: */
	enum { SHOW_ALWAYS,		/* all the time */
		   SHOW_ROULETTE,	/* only when rouletting */
		   SHOW_NEVER }		/* never (unless song hiding is turned off) */
		m_SelectionDisplay;

	Song();
	~Song();
	void Reset();

	NotesLoader *MakeLoader( CString sDir ) const;

	bool LoadFromSongDir( CString sDir );
	void RevertFromDisk();

	void TidyUpData();	// call after loading to clean up invalid data
	void ReCalculateRadarValuesAndLastBeat();	// called by TidyUpData, and after saving
	void TranslateTitles();	// called by TidyUpData

	void SaveToSMFile( CString sPath, bool bSavingCache );
	void Save();	// saves SM and DWI
	void SaveToCacheFile();
	void SaveToDWIFile();

	const CString &GetSongFilePath() const;
	CString GetCacheFilePath() const;

	void AddAutoGenNotes();
	void AutoGen( StepsType ntTo, StepsType ntFrom );	// create Steps of type ntTo from Steps of type ntFrom
	void RemoveAutoGenNotes();

	/* Directory this song data came from: */
	const CString &GetSongDir() const { return m_sSongDir; }

	/* Filename associated with this file.  This will always have
	 * an .SM extension.  If we loaded an .SM, this will point to 
	 * it, but if we loaded any other type, this will point to a
	 * generated .SM filename. */
	CString m_sSongFileName;

	CString m_sGroupName;


	bool	m_bChangedSinceSave;
	bool	m_bIsSymLink;

	CString	m_sMainTitle, m_sSubTitle, m_sArtist;
	CString m_sMainTitleTranslit, m_sSubTitleTranslit, m_sArtistTranslit;

	/* If PREFSMAN->m_bShowNative is off, these are the same as GetTranslit* below.
	 * Otherwise, they return the main titles. */
	CString GetDisplayMainTitle() const;
	CString GetDisplaySubTitle() const;
	CString GetDisplayArtist() const;

	/* Returns the transliterated titles, if any; otherwise returns the main titles. */
	CString GetTranslitMainTitle() const { return m_sMainTitleTranslit.size()? m_sMainTitleTranslit: m_sMainTitle; }
	CString GetTranslitSubTitle() const { return m_sSubTitleTranslit.size()? m_sSubTitleTranslit: m_sSubTitle; }
	CString GetTranslitArtist() const { return m_sArtistTranslit.size()? m_sArtistTranslit:m_sArtist; }

	/* "title subtitle" */
	CString GetFullDisplayTitle() const;
	CString GetFullTranslitTitle() const;

	/* This is read and saved, but never actually used. */
	CString	m_sCredit;

	CString	m_sMusicFile;
	float	m_fMusicLengthSeconds;
	float	m_fFirstBeat;
	float	m_fLastBeat;
	float	m_fMusicSampleStartSeconds;
	float	m_fMusicSampleLengthSeconds;
	enum { DISPLAY_ACTUAL, DISPLAY_SPECIFIED, DISPLAY_RANDOM } m_DisplayBPMType;
	float		m_fSpecifiedBPMMin;
	float		m_fSpecifiedBPMMax;	// if a range, then Min != Max

	CString	m_sBannerFile;
	CString m_sLyricsFile;
	CString	m_sBackgroundFile;
	CString	m_sCDTitleFile;

	CString GetMusicPath() const;
	CString GetBannerPath() const;
	CString	GetLyricsPath() const;
	CString GetBackgroundPath() const;
	CString GetCDTitlePath() const;

	/* For loading only: */
	bool m_bHasMusic, m_bHasBanner;

	bool HasMusic() const;
	bool HasBanner() const;
	bool HasBackground() const;
	bool HasCDTitle() const;
	bool HasMovieBackground() const;
	bool HasBGChanges() const;
	bool HasLyrics() const;

	bool Matches(CString sGroup, CString sSong) const;

	TimingData					m_Timing;
	vector<BackgroundChange>	m_BackgroundChanges;	// this must be sorted before gameplay
	vector<LyricSegment>		m_LyricSegments;	// same

	void AddBPMSegment( const BPMSegment &seg ) { m_Timing.AddBPMSegment( seg ); }
	void AddStopSegment( const StopSegment &seg ) { m_Timing.AddStopSegment( seg ); }
	void AddBackgroundChange( BackgroundChange seg );
	void AddLyricSegment( LyricSegment seg );

	void GetDisplayBPM( float &fMinBPMOut, float &fMaxBPMOut ) const;
	CString GetBackgroundAtBeat( float fBeat ) const;

	float GetBPMAtBeat( float fBeat ) const { return m_Timing.GetBPMAtBeat( fBeat ); }
	void SetBPMAtBeat( float fBeat, float fBPM ) { m_Timing.SetBPMAtBeat( fBeat, fBPM ); }
	BPMSegment& GetBPMSegmentAtBeat( float fBeat ) { return m_Timing.GetBPMSegmentAtBeat( fBeat ); }
	void GetBeatAndBPSFromElapsedTime( float fElapsedTime, float &fBeatOut, float &fBPSOut, bool &bFreezeOut ) const { m_Timing.GetBeatAndBPSFromElapsedTime( fElapsedTime, fBeatOut, fBPSOut, bFreezeOut ); }
	float GetBeatFromElapsedTime( float fElapsedTime ) const { return m_Timing.GetBeatFromElapsedTime( fElapsedTime ); }
	float GetElapsedTimeFromBeat( float fBeat ) const { return m_Timing.GetElapsedTimeFromBeat( fBeat ); }
	
	
	
	vector<Steps*> m_apNotes;

	bool SongCompleteForStyle( const StyleDef *st ) const;
	bool SongHasNotesType( StepsType nt ) const;
	bool SongHasNotesTypeAndDifficulty( StepsType nt, Difficulty dc ) const;
	const vector<Steps*>& GetAllSteps() const { return m_apNotes; }
	void GetSteps( vector<Steps*>& arrayAddTo, StepsType nt, Difficulty dc = DIFFICULTY_INVALID, int iMeterLow = -1, int iMeterHigh = -1, const CString &sDescription = "", bool bIncludeAutoGen = true, int Max = -1 ) const;
	Steps* GetStepsByDifficulty( StepsType nt, Difficulty dc, bool bIncludeAutoGen = true ) const;
	Steps* GetStepsByMeter( StepsType nt, int iMeterLow, int iMeterHigh, bool bIncludeAutoGen = true ) const;
	Steps* GetStepsByDescription( StepsType nt, CString sDescription, bool bIncludeAutoGen = true ) const;
	Steps* GetClosestNotes( StepsType nt, Difficulty dc, bool bIncludeAutoGen = true ) const;
	void GetEdits( vector<Steps*>& arrayAddTo, StepsType nt, bool bIncludeAutoGen = true ) const;
	int GetNumTimesPlayed( MemoryCard card ) const;
	bool IsNew() const;
	bool IsEasy( StepsType nt ) const;
	bool HasEdits( StepsType nt ) const;
	bool NormallyDisplayed() const;
	bool RouletteDisplayed() const;
	int	GetNumNotesWithGrade( Grade g ) const;

	void AddNotes( Steps* pNotes );		// we are responsible for deleting the memory pointed to by pNotes!
	void RemoveNotes( const Steps* pNotes );
};

CString MakeSortString( CString s );
void SortSongPointerArrayByDifficulty( vector<Song*> &arraySongPointers );
void SortSongPointerArrayByTitle( vector<Song*> &arraySongPointers );
void SortSongPointerArrayByBPM( vector<Song*> &arraySongPointers );
void SortSongPointerArrayByGrade( vector<Song*> &arraySongPointers );
void SortSongPointerArrayByArtist( vector<Song*> &arraySongPointers );
void SortSongPointerArrayByGroupAndDifficulty( vector<Song*> &arraySongPointers );
void SortSongPointerArrayByGroupAndTitle( vector<Song*> &arraySongPointers );
void SortSongPointerArrayByMostPlayed( vector<Song*> &arraySongPointers, MemoryCard card );
void SortSongPointerArrayByMeter( vector<Song*> &arraySongPointers, Difficulty dc );
CString GetSectionNameFromSongAndSort( const Song* pSong, SongSortOrder so );
void SortSongPointerArrayBySectionName( vector<Song*> &arraySongPointers, SongSortOrder so );



#endif
