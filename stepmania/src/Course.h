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

#include "PlayerNumber.h"
#include "GameConstantsAndTypes.h"

struct PlayerOptions;
struct SongOptions;
class Song;
class Notes;

class Course
{
	struct Entry {
		enum Type { fixed, random, random_within_group, players_best, players_worst } type;
		Song* pSong;			// used in type=fixed
		CString group_name;		// used in type=random_within_group
		Difficulty difficulty;	// = DIFFICULTY_INVALID if no difficulty specified
		int low_meter;			// = -1 if no meter range specified
		int high_meter;			// = -1 if no meter range specified
		int players_index;		// ignored if type isn't a players_*
		CString modifiers;		// set player and song options using these

		Entry()
		{
			difficulty = DIFFICULTY_INVALID;
			low_meter = -1;
			high_meter = -1;
			players_index = -1;
		}
	};
	vector<Entry> m_entries;

public:
	Course();

	bool		m_bIsAutogen;	// was this created by AutoGen?
	CString		m_sPath;
	CString		m_sName;
	CString		m_sBannerPath;
	CString		m_sCDTitlePath;

	bool		m_bRepeat;	// repeat after last song?  "Endless"
	bool		m_bRandomize;	// play the songs in a random order
	bool		m_bDifficult; // only make something difficult once
	int			m_iLives;	// -1 means use bar life meter
	int			m_iExtra;	// extra stage number...	// not used? -Chris

	int GetEstimatedNumStages() const { return m_entries.size(); }
	bool HasDifficult() const;
	bool IsPlayableIn( NotesType nt ) const;
	void GetStageInfo(		// Derefrences course_entries and returns only the playable Songs and Notes
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
	Difficulty GetDifficulty( int stage ) const;
	void GetMeterRange( int stage, int& iMeterLowOut, int& iMeterHighOut ) const;
	bool ContainsAnyMysterySongs() const;
	bool GetTotalSeconds( float& fSecondsOut ) const;


	void LoadFromCRSFile( CString sPath );
	void AutogenEndlessFromGroup( CString sGroupName, vector<Song*> &apSongsInGroup );
	void AutogenNonstopFromGroup( CString sGroupName, vector<Song*> &apSongsInGroup );


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
	Song *FindSong(CString sGroup, CString sSong) const;
};


void SortCoursePointerArrayByDifficulty( vector<Course*> &apCourses );


#endif
