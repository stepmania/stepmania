#ifndef PLAYER_H
#define PLAYER_H
/*
-----------------------------------------------------------------------------
 Class: Player

 Desc: Object that accepts pad input, knocks down ColorNotes that were stepped on, 
		and keeps score for the player.

 Copyright (c) 2001-2002 by the person(s) listed below.  All rights reserved.
	Chris Danford
-----------------------------------------------------------------------------
*/

#include "PrefsManager.h"	// for GameplayStatistics
#include "Sprite.h"
#include "BitmapText.h"

#include "GrayArrow.h"
#include "GhostArrow.h"
#include "GhostArrowBright.h"
#include "HoldGhostArrow.h"
#include "ActorFrame.h"
#include "RandomSample.h"
#include "Judgment.h"
#include "HoldJudgment.h"
#include "Combo.h"
#include "NoteField.h"
#include "GrayArrowRow.h"
#include "GhostArrowRow.h"
#include "NoteDataWithScoring.h"
class ScoreDisplay;
class LifeMeter;
class ScoreKeeper;
class Inventory;

#define	SAMPLE_COUNT	16

class Player : public NoteDataWithScoring, public ActorFrame
{
public:
	Player();

	virtual void Update( float fDeltaTime );
	virtual void DrawPrimitives();
	~Player();

	void Load( PlayerNumber player_no, NoteData* pNoteData, LifeMeter* pLM, ScoreDisplay* pScore, Inventory* pInventory, ScoreKeeper* pScoreKeeper );
	void CrossedRow( int iNoteRow );
	void Step( int col );

	void	FadeToFail();
	
protected:
	int UpdateTapNotesMissedOlderThan( float fMissIfOlderThanThisBeat );
	void OnRowDestroyed( int iStepIndex );
	void HandleTapRowScore( TapNoteScore scoreOfLastTap, int iNumTapsInRow );
	void HandleHoldScore( HoldNoteScore holdScore, TapNoteScore tapScore );
	void HandleAutosync(float fNoteOffset);

	int GetClosestNoteDirectional( int col, float fBeat, float fMaxBeatsAhead, int iDirection );
	int GetClosestNote( int col, float fBeat, float fMaxBeatsAhead, float fMaxBeatsBehind );

	static float GetMaxStepDistanceSeconds();

	PlayerNumber	m_PlayerNumber;

	float			m_fOffset[SAMPLE_COUNT];//for AutoAdjust
	int				m_iOffsetSample;		//

	GrayArrowRow	m_GrayArrowRow;
	NoteField		m_NoteField;
	GhostArrowRow	m_GhostArrowRow;

	HoldJudgment	m_HoldJudgment[MAX_NOTE_TRACKS];

	Judgment		m_Judgment;
	
	Combo			m_Combo;

	LifeMeter*		m_pLifeMeter;
	ScoreDisplay*	m_pScore;
	ScoreKeeper*	m_pScoreKeeper;
	Inventory*		m_pInventory;
};

#endif
