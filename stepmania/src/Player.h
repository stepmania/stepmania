/* Player - Accepts input, knocks down TapNotes that were stepped on, and keeps score for the player. */

#ifndef PLAYER_H
#define PLAYER_H

#include "ActorFrame.h"
#include "Judgment.h"
#include "HoldJudgment.h"
#include "Combo.h"
#include "NoteDataWithScoring.h"
#include "RageTimer.h"
#include "ProTimingDisplay.h"
#include "RageSound.h"
#include "AttackDisplay.h"
#include "NoteField.h"

class ScoreDisplay;
class LifeMeter;
class CombinedLifeMeter;
class ScoreKeeper;
class Inventory;
struct PlayerStageStats;

#define	SAMPLE_COUNT	16

class Player: public ActorFrame
{
public:
	Player();
	~Player();

	virtual void Update( float fDeltaTime );
	virtual void DrawPrimitives();

	void Init( 
		PlayerState* pPlayerState, 
		PlayerStageStats* pPlayerStageStats,
		LifeMeter* pLM, 
		CombinedLifeMeter* pCombinedLM, 
		ScoreDisplay* pScoreDisplay, 
		ScoreDisplay* pSecondaryScoreDisplay, 
		Inventory* pInventory, 
		ScoreKeeper* pPrimaryScoreKeeper, 
		ScoreKeeper* pSecondaryScoreKeeper );
	void Load( const NoteData& noteData );
	void CrossedRow( int iNoteRow );
	void CrossedMineRow( int iNoteRow );
	void Step( int col, RageTimer tm );
	void RandomizeNotes( int iNoteRow );
	void FadeToFail();
	int GetDancingCharacterState() const { return m_iDCState; };
	void SetCharacterState(int iDCState) { m_iDCState = iDCState; };
	void ApplyWaitingTransforms();

	static float GetMaxStepDistanceSeconds();

	void CacheAllUsedNoteSkins( bool bDeleteUnused ) { m_pNoteField->CacheAllUsedNoteSkins(bDeleteUnused); }

	NoteData m_NoteData;

protected:
	void UpdateTapNotesMissedOlderThan( float fMissIfOlderThanThisBeat );
	void OnRowCompletelyJudged( int iStepIndex );
	void HandleTapRowScore( unsigned row );
	void HandleHoldScore( HoldNoteScore holdScore, TapNoteScore tapScore );
	void HandleAutosync(float fNoteOffset);
	void DrawTapJudgments();
	void DrawHoldJudgments();

	int GetClosestNoteDirectional( int col, int iStartRow, int iMaxRowsAhead, bool bAllowGraded, bool bForward ) const;
	int GetClosestNote( int col, float fBeat, float fMaxBeatsAhead, float fMaxBeatsBehind, bool bAllowGraded ) const;

	PlayerState*	m_pPlayerState;
	PlayerStageStats*	m_pPlayerStageStats;
	float			m_fNoteFieldHeight;

	float			m_fOffset[SAMPLE_COUNT]; // for AutoSync
	int				m_iOffsetSample;

	NoteField*		m_pNoteField;

	HoldJudgment	m_HoldJudgment[MAX_NOTE_TRACKS];

	Judgment		m_Judgment;
	ProTimingDisplay m_ProTimingDisplay;
	
	Combo			m_Combo;

	AttackDisplay	m_AttackDisplay;

	int m_iDCState;
	LifeMeter*			m_pLifeMeter;
	CombinedLifeMeter*	m_pCombinedLifeMeter;
	ScoreDisplay*		m_pScoreDisplay;
	ScoreDisplay*		m_pSecondaryScoreDisplay;
	ScoreKeeper*		m_pPrimaryScoreKeeper;
	ScoreKeeper*		m_pSecondaryScoreKeeper;
	Inventory*			m_pInventory;

	int			m_iRowLastCrossed;
	int			m_iMineRowLastCrossed;

	RageSound	m_soundMine;
	RageSound	m_soundAttackLaunch;
	RageSound	m_soundAttackEnding;

	vector<RageSound> m_vKeysounds;
};

#endif

/*
 * (c) 2001-2004 Chris Danford
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
