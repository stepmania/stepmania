#ifndef DIFFICULTY_LIST
#define DIFFICULTY_LIST

#include "ActorFrame.h"
#include "ActorUtil.h"
#include "PlayerNumber.h"
#include "BitmapText.h"
#include "DifficultyMeter.h"

class Song;
class Steps;

class DifficultyList: public ActorFrame
{
public:
	DifficultyList();
	~DifficultyList();

	void Load();
	void SetFromGameState();
	void TweenOnScreen();
	void TweenOffScreen();
	void Hide();
	void Show();

private:
	void UpdatePositions();
	void PositionItems();
	int GetCurrentRowIndex( PlayerNumber pn ) const;
	void HideRows();
	CString GetDifficultyString( Difficulty d ) const;

	AutoActor		m_Cursors[NUM_PLAYERS];
	ActorFrame		m_CursorFrames[NUM_PLAYERS];

	struct Line
	{
		DifficultyMeter m_Meter;
		BitmapText		m_Description;
		BitmapText		m_Number;
	};
	vector<Line>	m_Lines;

	Song			*m_CurSong;
	bool			m_bShown;

	struct Row
	{
		Row()
		{
			m_Steps = NULL;
			m_dc = DIFFICULTY_INVALID;
			m_fY = 0;
			m_bHidden = false;
		}
		
		Steps *m_Steps;
		Difficulty m_dc;
		float m_fY;
		bool m_bHidden; // currently off screen
	};
	
	vector<Row>		m_Rows;
};

#endif
/*
 * Copyright (c) 2003-2004 by the person(s) listed below.  All rights reserved.
 *	Glenn Maynard
 */
