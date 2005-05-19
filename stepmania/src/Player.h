/* Player - Accepts input, knocks down TapNotes that were stepped on, and keeps score for the player. */

#ifndef PLAYER_H
#define PLAYER_H

#include "ActorFrame.h"
#include "Judgment.h"
#include "HoldJudgment.h"
#include "Combo.h"
#include "NoteDataWithScoring.h"
#include "RageSound.h"
#include "AttackDisplay.h"
#include "NoteData.h"

class ScoreDisplay;
class LifeMeter;
class CombinedLifeMeter;
class ScoreKeeper;
class Inventory;
class RageTimer;
class NoteField;
struct PlayerStageStats;

#define	SAMPLE_COUNT	32

class Player: public ActorFrame
{
public:
	Player();
	~Player();

	virtual void Update( float fDeltaTime );
	virtual void DrawPrimitives();
	virtual void HandleMessage( const CString& sMessage );

	void Init( 
		const CString &sType,
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
	void Step( int col, const RageTimer &tm, bool bHeld = false );
	void RandomizeNotes( int iNoteRow );
	void FadeToFail();
	TapNoteScore GetLastTapNoteScore() const { return m_LastTapNoteScore; }
	void ApplyWaitingTransforms();
	void SetPaused( bool bPaused ) { m_bPaused = bPaused; }

	float GetMaxStepDistanceSeconds();

	NoteData m_NoteData;

protected:
	void HandleStep( int col, const RageTimer &tm, bool bHeld );
	void UpdateTapNotesMissedOlderThan( float fMissIfOlderThanThisBeat );
	void DisplayJudgedRow( int iIndexThatWasSteppedOn, TapNoteScore score, int iTrack );
	void OnRowCompletelyJudged( int iStepIndex );
	void HandleTapRowScore( unsigned row );
	void HandleHoldScore( HoldNoteScore holdScore, TapNoteScore tapScore );
	void HandleAutosync(float fNoteOffset);
	void DrawTapJudgments();
	void DrawHoldJudgments();

	int GetClosestNoteDirectional( int col, int iStartRow, int iMaxRowsAhead, bool bAllowGraded, bool bForward ) const;
	int GetClosestNote( int col, int iNoteRow, int iMaxRowsAhead, int iMaxRowsBehind, bool bAllowGraded ) const;

	bool IsPlayingBeginner() const;

	PlayerState*	m_pPlayerState;
	PlayerStageStats*	m_pPlayerStageStats;
	float			m_fNoteFieldHeight;

	bool			m_bPaused;
	float			m_fOffset[SAMPLE_COUNT]; // for AutoSync
	int				m_iOffsetSample;

	NoteField*		m_pNoteField;

	HoldJudgment	m_HoldJudgment[MAX_NOTE_TRACKS];

	Judgment		m_Judgment;
	
	Combo			m_Combo;

	AttackDisplay	m_AttackDisplay;

	TapNoteScore		m_LastTapNoteScore;
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

	CString m_sMessageToSendOnStep;

	ThemeMetric<float>		GRAY_ARROWS_Y_STANDARD;
	ThemeMetric<float>		GRAY_ARROWS_Y_REVERSE;
	ThemeMetric2D<float>	JUDGMENT_X;
	ThemeMetric<float>		JUDGMENT_Y;
	ThemeMetric<float>		JUDGMENT_Y_REVERSE;
	ThemeMetric<float>		JUDGMENT_CENTERED_ADDY;
	ThemeMetric<float>		JUDGMENT_CENTERED_ADDY_REVERSE;
	ThemeMetric2D<float>	COMBO_X;
	ThemeMetric<float>		COMBO_Y;
	ThemeMetric<float>		COMBO_Y_REVERSE;
	ThemeMetric<float>		COMBO_CENTERED_ADDY;
	ThemeMetric<float>		COMBO_CENTERED_ADDY_REVERSE;
	ThemeMetric2D<float>	ATTACK_DISPLAY_X;
	ThemeMetric<float>		ATTACK_DISPLAY_Y;
	ThemeMetric<float>		ATTACK_DISPLAY_Y_REVERSE;
	ThemeMetric<float>		HOLD_JUDGMENT_Y_STANDARD;
	ThemeMetric<float>		HOLD_JUDGMENT_Y_REVERSE;
	ThemeMetric<int>		BRIGHT_GHOST_COMBO_THRESHOLD;
	ThemeMetric<bool>		TAP_JUDGMENTS_UNDER_FIELD;
	ThemeMetric<bool>		HOLD_JUDGMENTS_UNDER_FIELD;
	ThemeMetric<int>		START_DRAWING_AT_PIXELS;
	ThemeMetric<int>		STOP_DRAWING_AT_PIXELS;
	ThemeMetric<int>		MAX_PRO_TIMING_ERROR;
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
