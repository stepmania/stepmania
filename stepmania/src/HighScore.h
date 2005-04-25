/* HighScore - Player scoring data that persists between sessions. */

#ifndef HighScore_H
#define HighScore_H

#include "Grade.h"
#include "GameConstantsAndTypes.h"
#include "RadarValues.h"
#include "DateTime.h"

struct XNode;

struct HighScore
{
	CString	sName;	// name that shows in the machine's ranking screen
	Grade grade;
	int iScore;
	float fPercentDP;
	float fSurviveSeconds;
	CString	sModifiers;
	DateTime dateTime;		// return value of time() when screenshot was taken
	CString sPlayerGuid;	// who made this high score
	CString sMachineGuid;	// where this high score was made
	int	iProductID;
	int		iTapNoteScores[NUM_TAP_NOTE_SCORES];
	int		iHoldNoteScores[NUM_HOLD_NOTE_SCORES];
	RadarValues radarValues;

	HighScore() { Unset(); }
	void Unset()
	{
		sName = "";
		grade = GRADE_NO_DATA;
		iScore = 0;
		fPercentDP = 0;
		fSurviveSeconds = 0;
		sModifiers = "";
		dateTime.Init();
		sPlayerGuid = "";
		sMachineGuid = "";
		iProductID = 0;
		ZERO( iTapNoteScores );
		ZERO( iHoldNoteScores );
		radarValues.MakeUnknown();
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
		COMPARE( dateTime );
		COMPARE( sPlayerGuid );
		COMPARE( sMachineGuid );
		COMPARE( iProductID );
		FOREACH_TapNoteScore( tns )
			COMPARE( iTapNoteScores[tns] );
		FOREACH_HoldNoteScore( hns )
			COMPARE( iHoldNoteScores[hns] );
		COMPARE( radarValues );
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

	void AddHighScore( HighScore hs, int &iIndexOut, bool bIsMachine );

	const HighScore& GetTopScore() const;

	XNode* CreateNode() const;
	void LoadFromNode( const XNode* pNode );

	void RemoveAllButOneOfEachName();
	void ClampSize( bool bIsMachine );
};

struct Screenshot
{
	CString sFileName;	// no directory part - just the file name
	CString sMD5;		// MD5 hash of the screenshot file
	HighScore highScore;

	XNode* CreateNode() const;
	void LoadFromNode( const XNode* pNode );
};

#endif

/*
 * (c) 2004 Chris Danford
 * All rights reserved.
 * 
 * Permission is hereby granted, free of charge, to any person obtaining a
 * copy of this software and associated documentation files (the
 * "Software"), to deal in the Software without restriction, including
 * without limitation the rights to use, copy, modify, merge, publish,
 * distribute, and/or sell copies of the Software, and to permit persons to
 * whom the Software is furnished to do so, provided that the above
 * copyright notice(s) and this permission notice appear in all copies of
 * the Software and that both the above copyright notice(s) and this
 * permission notice appear in supporting documentation.
 * 
 * THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS
 * OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF
 * MERCHANTABILITY, FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT OF
 * THIRD PARTY RIGHTS. IN NO EVENT SHALL THE COPYRIGHT HOLDER OR HOLDERS
 * INCLUDED IN THIS NOTICE BE LIABLE FOR ANY CLAIM, OR ANY SPECIAL INDIRECT
 * OR CONSEQUENTIAL DAMAGES, OR ANY DAMAGES WHATSOEVER RESULTING FROM LOSS
 * OF USE, DATA OR PROFITS, WHETHER IN AN ACTION OF CONTRACT, NEGLIGENCE OR
 * OTHER TORTIOUS ACTION, ARISING OUT OF OR IN CONNECTION WITH THE USE OR
 * PERFORMANCE OF THIS SOFTWARE.
 */
