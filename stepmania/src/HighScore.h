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
#include "XmlFile.h"

struct HighScore
{
	CString	sName;
	Grade grade;
	int iScore;
	float fPercentDP;
	float fSurviveSeconds;
	CString	sModifiers;
	time_t time;		// return value of time() when screenshot was taken
	CString sMachineGuid;	// where this screenshot was taken


	HighScore()
	{
		grade = GRADE_NO_DATA;
		iScore = 0;
		fPercentDP = 0;
		fSurviveSeconds = 0;
		time = 0;
	}

	bool operator>=( const HighScore& other ) const;

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
