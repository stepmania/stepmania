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
class Notes;
class StyleDef;
class NotesLoader;
class LyricsLoader;


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

	bool LoadWithoutCache( CString sDir );
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
	void AutoGen( NotesType ntTo, NotesType ntFrom );	// create Notes of type ntTo from Notes of type ntFrom
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
	unsigned m_iMusicBytes;
	float	m_fBeat0OffsetInSeconds;
	float	m_fMusicLengthSeconds;
	float	m_fFirstBeat;
	float	m_fLastBeat;
	float   GetFirstBeat() const;
	float   GetLastBeat() const;
	float	m_fMusicSampleStartSeconds;
	float	m_fMusicSampleLengthSeconds;
	enum { DISPLAY_ACTUAL, DISPLAY_SPECIFIED, DISPLAY_RANDOM } m_DisplayBPMType;
	float		m_fSpecifiedBPMMin;
	float		m_fSpecifiedBPMMax;	// if a range, then Min != Max

	float GetMusicStartBeat() const;

	CString	m_sBannerFile;
	CString m_sLyricsFile;
	CString	m_sBackgroundFile;
	CString	m_sCDTitleFile;

	CString GetMusicPath() const;
	CString GetBannerPath() const;
	CString	GetLyricsPath() const;
	CString GetBackgroundPath() const;
	CString GetCDTitlePath() const;


	bool HasMusic() const;
	bool HasBanner() const;
	bool HasBackground() const;
	bool HasCDTitle() const;
	bool HasMovieBackground() const;
	bool HasBGChanges() const;
	bool HasLyrics() const;
	bool m_bIsLocked;

	bool Matches(CString sGroup, CString sSong) const;

	vector<BPMSegment>			m_BPMSegments;	// this must be sorted before gameplay
	vector<StopSegment>			m_StopSegments;	// this must be sorted before gameplay
	vector<BackgroundChange>	m_BackgroundChanges;	// this must be sorted before gameplay
	vector<LyricSegment>		m_LyricSegments;	// same

	void AddBPMSegment( BPMSegment seg );
	void AddStopSegment( StopSegment seg );
	void AddBackgroundChange( BackgroundChange seg );
	void AddLyricSegment( LyricSegment seg );

	void GetDisplayBPM( float &fMinBPMOut, float &fMaxBPMOut ) const
	{
		if( m_DisplayBPMType == DISPLAY_SPECIFIED )
		{
			fMinBPMOut = m_fSpecifiedBPMMin;
			fMaxBPMOut = m_fSpecifiedBPMMax;
		}
		else
		{
			GetActualBPM( fMinBPMOut, fMaxBPMOut );
		}
	}
	void GetActualBPM( float &fMinBPMOut, float &fMaxBPMOut ) const
	{
		fMaxBPMOut = 0;
		fMinBPMOut = 100000;	// inf
		for( unsigned i=0; i<m_BPMSegments.size(); i++ ) 
		{
			const BPMSegment &seg = m_BPMSegments[i];
			fMaxBPMOut = max( seg.m_fBPM, fMaxBPMOut );
			fMinBPMOut = min( seg.m_fBPM, fMinBPMOut );
		}
	}
	float GetBPMAtBeat( float fBeat ) const
	{
		unsigned i;
		for( i=0; i<m_BPMSegments.size()-1; i++ )
			if( m_BPMSegments[i+1].m_fStartBeat > fBeat )
				break;
		return m_BPMSegments[i].m_fBPM;
	}
	BPMSegment& GetBPMSegmentAtBeat( float fBeat )
	{
		unsigned i;
		for( i=0; i<m_BPMSegments.size()-1; i++ )
			if( m_BPMSegments[i+1].m_fStartBeat > fBeat )
				break;
		return m_BPMSegments[i];
	};
	CString GetBackgroundAtBeat( float fBeat )
	{
		unsigned i;
		for( i=0; i<m_BackgroundChanges.size()-1; i++ )
			if( m_BackgroundChanges[i+1].m_fStartBeat > fBeat )
				break;
		return m_BackgroundChanges[i].m_sBGName;
	}
	void GetBeatAndBPSFromElapsedTime( float fElapsedTime, float &fBeatOut, float &fBPSOut, bool &bFreezeOut ) const;
	float GetBeatFromElapsedTime( float fElapsedTime ) const	// shortcut for places that care only about the beat
	{
		float fBeat, fThrowAway;
		bool bThrowAway;
		GetBeatAndBPSFromElapsedTime( fElapsedTime, fBeat, fThrowAway, bThrowAway );
		return fBeat;
	}
	float GetElapsedTimeFromBeat( float fBeat ) const;
	
	
	
	vector<Notes*> m_apNotes;

	bool SongCompleteForStyle( const StyleDef *st ) const;
	bool SongHasNotesType( NotesType nt ) const;
	bool SongHasNotesTypeAndDifficulty( NotesType nt, Difficulty dc ) const;
	void GetNotes( vector<Notes*>& arrayAddTo, NotesType nt, Difficulty dc = DIFFICULTY_INVALID, int iMeterLow = -1, int iMeterHigh = -1, CString sDescription = "", bool bIncludeAutoGen = true ) const;
	Notes* GetNotes( NotesType nt, Difficulty dc, bool bIncludeAutoGen = true ) const;
	Notes* GetNotes( NotesType nt, int iMeterLow, int iMeterHigh, bool bIncludeAutoGen = true ) const;
	Notes* GetNotes( NotesType nt, CString sDescription, bool bIncludeAutoGen = true ) const;
	Notes* GetClosestNotes( NotesType nt, Difficulty dc, bool bIncludeAutoGen = true ) const;
	void GetEdits( vector<Notes*>& arrayAddTo, NotesType nt, bool bIncludeAutoGen = true ) const;
	int GetNumTimesPlayed() const;
	bool IsNew() const;
	bool IsEasy( NotesType nt ) const;
	bool HasEdits( NotesType nt ) const;
	Grade GetGradeForDifficulty( const StyleDef *s, PlayerNumber pn, Difficulty dc ) const;
	bool NormallyDisplayed() const;
	bool RouletteDisplayed() const;
	int	GetNumNotesWithGrade( Grade g ) const;

	void AddNotes( Notes* pNotes );		// we are responsible for deleting the memory pointed to by pNotes!
	void RemoveNotes( Notes* pNotes );
};

CString MakeSortString( CString s );
void SortSongPointerArrayByDifficulty( vector<Song*> &arraySongPointers );
void SortSongPointerArrayByTitle( vector<Song*> &arraySongPointers );
void SortSongPointerArrayByBPM( vector<Song*> &arraySongPointers );
void SortSongPointerArrayByGrade( vector<Song*> &arraySongPointers );
void SortSongPointerArrayByArtist( vector<Song*> &arraySongPointers );
void SortSongPointerArrayByGroupAndDifficulty( vector<Song*> &arraySongPointers );
void SortSongPointerArrayByGroupAndTitle( vector<Song*> &arraySongPointers );
void SortSongPointerArrayByMostPlayed( vector<Song*> &arraySongPointers );
void SortSongPointerArrayByMeter( vector<Song*> &arraySongPointers, Difficulty dc );



#endif
