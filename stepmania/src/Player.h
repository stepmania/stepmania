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

#include "ActorFrame.h"
#include "RandomSample.h"
#include "Judgment.h"
#include "HoldJudgment.h"
#include "Combo.h"
#include "NoteFieldPlus.h"
#include "NoteDataWithScoring.h"
#include "ArrowBackdrop.h"
#include "RageTimer.h"
#include "ProTimingDisplay.h"
#include "RageSound.h"
#include "DancingCharacters.h"

class ScoreDisplay;
class LifeMeter;
class CombinedLifeMeter;
class ScoreKeeper;
class Inventory;

#define	SAMPLE_COUNT	16


class PlayerMinus : public NoteDataWithScoring, public ActorFrame
{
public:
	PlayerMinus();
	~PlayerMinus();

	virtual void Update( float fDeltaTime );
	virtual void DrawPrimitives();

	void Load( PlayerNumber player_no, const NoteData* pNoteData, LifeMeter* pLM, CombinedLifeMeter* pCombinedLM, ScoreDisplay* pScore, Inventory* pInventory, ScoreKeeper* pPrimaryScoreKeeper, ScoreKeeper* pSecondaryScoreKeeper, NoteFieldPlus* pNoteField );
	void CrossedRow( int iNoteRow );
	void Step( int col, RageTimer tm );
	void RandomiseNotes( int iNoteRow );
	void FadeToFail();
	int GetDancingCharacterState() { return m_iDCState; };
	void SetCharacterState(int iDCState) { m_iDCState = iDCState; };
protected:
	void UpdateTapNotesMissedOlderThan( float fMissIfOlderThanThisBeat );
	void OnRowCompletelyJudged( int iStepIndex );
	void HandleTapRowScore( unsigned row );
	void HandleHoldScore( HoldNoteScore holdScore, TapNoteScore tapScore );
	void HandleAutosync(float fNoteOffset);

	int GetClosestNoteDirectional( int col, float fBeat, float fMaxBeatsAhead, int iDirection );
	int GetClosestNote( int col, float fBeat, float fMaxBeatsAhead, float fMaxBeatsBehind );

	static float GetMaxStepDistanceSeconds();

	PlayerNumber	m_PlayerNumber;

	float			m_fOffset[SAMPLE_COUNT];//for AutoAdjust
	int				m_iOffsetSample;		//

	ArrowBackdrop	m_ArrowBackdrop;
	NoteFieldPlus*	m_pNoteField;

	HoldJudgment	m_HoldJudgment[MAX_NOTE_TRACKS];

	Judgment		m_Judgment;
	ProTimingDisplay m_ProTimingDisplay;
	
	Combo			m_Combo;

	int m_iDCState;
//	DancingCharacters* m_pDancingCharacters; // used to adjust the 2D anims dancing states
	LifeMeter*		m_pLifeMeter;
	CombinedLifeMeter*		m_pCombinedLifeMeter;
	ScoreDisplay*	m_pScore;
	ScoreKeeper*	m_pPrimaryScoreKeeper;
	ScoreKeeper*	m_pSecondaryScoreKeeper;
	Inventory*		m_pInventory;

	int				m_iRowLastCrossed;

	RageSound		m_soundMine;
	RageSound		m_soundAttackLaunch;
	RageSound		m_soundAttackEnding;
};

class Player : public PlayerMinus
{
public:
	void Load( PlayerNumber player_no, const NoteData* pNoteData, LifeMeter* pLM, CombinedLifeMeter* pCombinedLM, ScoreDisplay* pScore, Inventory* pInventory, ScoreKeeper* pPrimaryScoreKeeper, ScoreKeeper* pSecondaryScoreKeeper );

protected:
	NoteFieldPlus	m_NoteField;

};

#endif
