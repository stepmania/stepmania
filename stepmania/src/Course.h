#ifndef COURSE_H
#define COURSE_H
/*
-----------------------------------------------------------------------------
 Class: Course

 Desc: A queue of songs and notes.

 Copyright (c) 2001-2002 by the person(s) listed below.  All rights reserved.
	Chris Danford
-----------------------------------------------------------------------------
*/

#include "GameConstantsAndTypes.h"

struct PlayerOptions;
struct SongOptions;
class Song;
struct Notes;

class Course
{
	struct course_entry {
		course_entry( CString dir, Difficulty dc, CString mod )
		{
			songDir = dir;
			difficulty = dc;
			modifiers = mod;			
		}

		CString songDir;	
		Difficulty difficulty;	// the Notes description
		CString modifiers;	// set player and song options from these
	};
	vector<course_entry> m_entries;

public:
	Course();

	bool		m_bIsAutoGen;	// was this created by AutoGen?
	CString		m_sPath;
	CString		m_sName;
	CString		m_sBannerPath;
	CString		m_sCDTitlePath;

	bool		m_bRepeat;	// repeat after last song?  "Endless"
	bool		m_bRandomize;	// play the songs in a random order
	int			m_iLives;	// -1 means use bar life meter
	int			m_iExtra;	// extra stage number...	// not used? -Chris

	int GetEstimatedNumStages() const { return m_entries.size(); }
	bool HasDifficult() const;
	void GetCourseInfo(		// Derefrences course_entries and returns only the playable Songs and Notes
		vector<Song*>& vSongsOut, 
		vector<Notes*>& vNotesOut, 
		vector<CString>& vsModifiersOut, 
		NotesType nt, 
		bool bDifficult ) const;	// like EX's Standard/Difficult option for courses
	bool GetFirstStageInfo(
		Song*& pSongOut, 
		Notes*& pNotesOut, 
		CString& sModifiersOut, 
		NotesType nt ) const;
	RageColor GetColor() const;
	bool IsMysterySong( int stage ) const;
	bool ContainsAnyMysterySongs() const;
	bool GetTotalSeconds( float& fSecondsOut ) const;


	void LoadFromCRSFile( CString sPath );
	void AutoGenEndlessFromGroupAndDifficulty( CString sGroupName, Difficulty dc, vector<Song*> &apSongsInGroup );


	// Statistics
	struct RankingScore
	{
		int iDancePoints;
		float fSurviveTime;
		CString	sName;
	} m_RankingScores[NUM_NOTES_TYPES][NUM_RANKING_LINES];	// sorted highest to lowest by iDancePoints
	struct MemCardScore
	{
		int iNumTimesPlayed;
		int iDancePoints;
		float fSurviveTime;
	} m_MemCardScores[NUM_MEMORY_CARDS][NUM_NOTES_TYPES];
	
	void AddScores( NotesType nt, bool bPlayerEnabled[NUM_PLAYERS], int iDancePoints[NUM_PLAYERS], float fSurviveTime[NUM_PLAYERS], int iRankingIndexOut[NUM_PLAYERS], bool bNewRecordOut[NUM_PLAYERS] );	// iNewRecordIndexOut[p] = -1 if not a new record


private:
	Song *FindSong(CString sSongDir) const;
};


void SortCoursePointerArrayByDifficulty( vector<Course*> &apCourses );


#endif
