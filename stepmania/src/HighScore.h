#ifndef HighScore_H
#define HighScore_H
/*
-----------------------------------------------------------------------------
 Class: HighScore

 Desc: Player data that persists between sessions.  Can be stored on a local
	disk or on a memory card.

 Copyright (c) 2001-2002 by the person(s) listed below.  All rights reserved.
	Chris Danford
-----------------------------------------------------------------------------
*/

#include "Grade.h"
#include "GameConstantsAndTypes.h"

struct XNode;
struct HighScore
{
	CString	sName;	// name that shows in the machine's ranking screen
	Grade grade;
	int iScore;
	float fPercentDP;
	float fSurviveSeconds;
	CString	sModifiers;
	time_t time;		// return value of time() when screenshot was taken
	CString sPlayerGuid;	// who made this high score
	CString sMachineGuid;	// where this high score was made
	int	iProductID;
	int		iTapNoteScores[NUM_TAP_NOTE_SCORES];
	int		iHoldNoteScores[NUM_HOLD_NOTE_SCORES];
	float	fRadarActual[NUM_RADAR_CATEGORIES];

	HighScore() { Unset(); }
	void Unset()
	{
		sName = "";
		grade = GRADE_NO_DATA;
		iScore = 0;
		fPercentDP = 0;
		fSurviveSeconds = 0;
		sModifiers = "";
		time = 0;
		sPlayerGuid = "";
		sMachineGuid = "";
		iProductID = 0;
		ZERO( iTapNoteScores );
		ZERO( iHoldNoteScores );
		ZERO( fRadarActual );
	}

	bool operator>=( const HighScore& other ) const;
	bool operator==( const HighScore& other ) const 
	{
#define COMPARE(x)	if( x!=other.x )	return false;
		COMPARE( sName );
		COMPARE( grade );
		COMPARE( iScore );
		COMPARE( fPercentDP );
		COMPARE( fSurviveSeconds );
		COMPARE( sModifiers );
		COMPARE( time );
		COMPARE( sPlayerGuid );
		COMPARE( sMachineGuid );
		COMPARE( iProductID );
		FOREACH_TapNoteScore( tns )
			COMPARE( iTapNoteScores[tns] );
		FOREACH_HoldNoteScore( hns )
			COMPARE( iHoldNoteScores[hns] );
		FOREACH_RadarCategory( rc )
			COMPARE( fRadarActual[rc] );
#undef COMPARE
		return true;
	}

	XNode* CreateNode() const;
	void LoadFromNode( const XNode* pNode );

	CString GetDisplayName() const;
};

struct HighScoreList
{
	int iNumTimesPlayed;
	vector<HighScore> vHighScores;

	
	HighScoreList()
	{
		iNumTimesPlayed = 0;
	}
	
	void Init();

	void AddHighScore( HighScore hs, int &iIndexOut );

	const HighScore& GetTopScore() const;

	XNode* CreateNode() const;
	void LoadFromNode( const XNode* pNode );
};


#endif
