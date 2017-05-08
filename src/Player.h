#ifndef PLAYER_H
#define PLAYER_H

#include "ActorFrame.h"
#include "AutoActor.h"
#include "NoteDataWithScoring.h"
#include "RageSound.h"
#include "AttackDisplay.h"
#include "NoteData.h"
#include "ScreenMessage.h"
#include "ThemeMetric.h"
#include "InputEventPlus.h"
#include "TimingData.h"

class ScoreDisplay;
class LifeMeter;
class CombinedLifeMeter;
class ScoreKeeper;
class Inventory;
class RageTimer;
struct NoteField;
class PlayerStageStats;
class JudgedRows;

// todo: replace these with a Message and MESSAGEMAN? -aj
AutoScreenMessage( SM_100Combo );
AutoScreenMessage( SM_200Combo );
AutoScreenMessage( SM_300Combo );
AutoScreenMessage( SM_400Combo );
AutoScreenMessage( SM_500Combo );
AutoScreenMessage( SM_600Combo );
AutoScreenMessage( SM_700Combo );
AutoScreenMessage( SM_800Combo );
AutoScreenMessage( SM_900Combo );
AutoScreenMessage( SM_1000Combo );
AutoScreenMessage( SM_ComboStopped );
AutoScreenMessage( SM_ComboContinuing );

/** @brief Accepts input, knocks down TapNotes that were stepped on, and keeps score for the player. */
class Player: public ActorFrame
{
public:
	// The passed in NoteData isn't touched until Load() is called.
	Player( NoteData &nd, bool bVisibleParts = true );
	~Player();

	virtual void Update( float fDeltaTime );
	virtual bool EarlyAbortDraw() const;
	virtual void DrawPrimitives();
	// DrawNoteFieldBoard exists so that the board can be drawn underneath
	// theme elements like score. -Kyz
	void DrawNoteFieldBoard();
	void SetNoteFieldToEditMode();
	void update_displayed_time();
	void disable_defective_mode();
	NoteField* get_note_field_because_i_really_need_it_for_edit_mode()
	{ return m_note_field; }

	struct TrackRowTapNote
	{
		int iTrack;
		int iRow;
		TapNote *pTN;
	};
	void UpdateHoldNotes( int iSongRow, float fDeltaTime, std::vector<TrackRowTapNote> &vTN );

	void Init(
		const std::string &sType,
		PlayerState* pPlayerState,
		PlayerStageStats* pPlayerStageStats,
		LifeMeter* pLM,
		CombinedLifeMeter* pCombinedLM,
		ScoreDisplay* pScoreDisplay,
		ScoreDisplay* pSecondaryScoreDisplay,
		Inventory* pInventory,
		ScoreKeeper* pPrimaryScoreKeeper,
		ScoreKeeper* pSecondaryScoreKeeper );
	void Load();
	double get_field_width();
	void set_gameplay_zoom(double zoom);
	float calc_read_bpm();
	void CrossedRows( int iLastRowCrossed, const RageTimer &now );
	bool IsOniDead() const;

	/**
	 * @brief Retrieve the Player's TimingData.
	 *
	 * This is primarily for a lua hook.
	 * @return the TimingData in question. */
	TimingData GetPlayerTimingData() const
	{
		return *(this->m_Timing);
	}

	void ScoreAllActiveHoldsLetGo();
	void DoTapScoreNone(bool bStepped);

	void Step( int col, int row, const RageTimer &tm, bool bHeld, bool bRelease );

	void FadeToFail();
	TapNoteScore GetLastTapNoteScore() const { return m_LastTapNoteScore; }
	void ApplyWaitingTransforms();
	void SetPaused( bool bPaused ) { m_bPaused = bPaused; }

	static float GetMaxStepDistanceSeconds();
	static float GetWindowSeconds( TimingWindow tw );
	const NoteData &GetNoteData() const { return m_NoteData; }
	bool HasVisibleParts() const { return m_note_field != nullptr; }

	void SetActorWithJudgmentPosition( Actor *pActor ) { m_pActorWithJudgmentPosition = pActor; }
	void SetActorWithComboPosition( Actor *pActor ) { m_pActorWithComboPosition = pActor; }

	void SetSendJudgmentAndComboMessages( bool b ) { m_bSendJudgmentAndComboMessages = b; }

	// Lua
	virtual void PushSelf( lua_State *L );

	PlayerState * GetPlayerState() { return this->m_pPlayerState; }
	void ChangeLife(float delta);
	void SetLife(float value);
	bool m_inside_lua_set_life;

protected:
	void UpdateTapNotesMissedOlderThan( float fMissIfOlderThanThisBeat );
	void UpdateJudgedRows();
	void FlashGhostRow( int iRow );
	void HandleTapRowScore( unsigned row );
	void HandleHoldScore( const TapNote &tn );
	void HandleHoldCheckpoint( int iRow, int iNumHoldsHeldThisRow, int iNumHoldsMissedThisRow, const std::vector<int> &viColsWithHold );
	void DrawTapJudgments();
	void SendComboMessages( unsigned int iOldCombo, unsigned int iOldMissCombo );
	void PlayKeysound( const TapNote &tn, TapNoteScore score );

	void send_judge_message(Message& msg);
	void SetMineJudgment( TapNoteScore tns , int iTrack );
	void SetJudgment( int iRow, int iFirstTrack, const TapNote &tn ) { SetJudgment( iRow, iFirstTrack, tn, tn.result.tns, tn.result.fTapNoteOffset ); }
	void SetJudgment( int iRow, int iFirstTrack, const TapNote &tn, TapNoteScore tns, float fTapNoteOffset );	// -1 if no track as in TNS_Miss
	void SetHoldJudgment( TapNote &tn, int iTrack );
	void SetCombo( unsigned int iCombo, unsigned int iMisses );
	void IncrementComboOrMissCombo( bool bComboOrMissCombo );
	void IncrementCombo() { IncrementComboOrMissCombo(true); };
	void IncrementMissCombo() { IncrementComboOrMissCombo(false); };

	void ChangeLife( TapNoteScore tns );
	void ChangeLife( HoldNoteScore hns, TapNoteScore tns );
	void ChangeLifeRecord();

	int GetClosestNoteDirectional( int col, int iStartRow, int iMaxRowsAhead, bool bAllowGraded, bool bForward ) const;
	int GetClosestNote( int col, int iNoteRow, int iMaxRowsAhead, int iMaxRowsBehind, bool bAllowGraded ) const;
	int GetClosestNonEmptyRowDirectional( int iStartRow, int iMaxRowsAhead, bool bAllowGraded, bool bForward ) const;
	int GetClosestNonEmptyRow( int iNoteRow, int iMaxRowsAhead, int iMaxRowsBehind, bool bAllowGraded ) const;

	std::string ApplyRandomAttack();

	inline void HideNote( int col, int row )
	{
		NoteData::iterator iter = m_NoteData.FindTapNote( col, row );
		if( iter != m_NoteData.end(col) )
			iter->second.result.bHidden = true;
	}

	bool			m_bLoaded;

	/** @brief The player's present state. */
	PlayerState		*m_pPlayerState;
	/** @brief The player's present stage stats. */
	PlayerStageStats	*m_pPlayerStageStats;
	TimingData      *m_Timing;

	bool			m_bPaused;
	bool			m_bDelay;

	NoteData		&m_NoteData;
	NoteField		*m_note_field;

	AutoActor		m_sprJudgment;
	AutoActor		m_sprCombo;
	Actor			*m_pActorWithJudgmentPosition;
	Actor			*m_pActorWithComboPosition;

	AttackDisplay		*m_pAttackDisplay;

	TapNoteScore		m_LastTapNoteScore;
	LifeMeter		*m_pLifeMeter;
	CombinedLifeMeter	*m_pCombinedLifeMeter;
	ScoreDisplay		*m_pScoreDisplay;
	ScoreDisplay		*m_pSecondaryScoreDisplay;
	ScoreKeeper		*m_pPrimaryScoreKeeper;
	ScoreKeeper		*m_pSecondaryScoreKeeper;
	Inventory		*m_pInventory;

	int			m_iFirstUncrossedRow;	// used by hold checkpoints logic
	NoteData::all_tracks_iterator *m_pIterNeedsTapJudging;
	NoteData::all_tracks_iterator *m_pIterNeedsHoldJudging;
	NoteData::all_tracks_iterator *m_pIterUncrossedRows;
	NoteData::all_tracks_iterator *m_pIterUnjudgedRows;
	NoteData::all_tracks_iterator *m_pIterUnjudgedMineRows;
	unsigned int	m_iLastSeenCombo;
	bool	m_bSeenComboYet;
	JudgedRows		*m_pJudgedRows;

	RageSound		m_soundMine;
	RageSound		m_soundAttackLaunch;
	RageSound		m_soundAttackEnding;

	float			m_fActiveRandomAttackStart;

	std::vector<bool>	m_vbFretIsDown;

	std::vector<RageSound>	m_vKeysounds;

	ThemeMetric2D<float>	ATTACK_DISPLAY_X;
	ThemeMetric<float>	ATTACK_DISPLAY_Y;
	ThemeMetric<float>	ATTACK_DISPLAY_Y_REVERSE;
	ThemeMetric<float>	HOLD_JUDGMENT_Y_STANDARD;
	ThemeMetric<float>	HOLD_JUDGMENT_Y_REVERSE;
	ThemeMetric<int>	BRIGHT_GHOST_COMBO_THRESHOLD;

#define NUM_REVERSE 2
#define NUM_CENTERED 2
	TweenState		m_tsJudgment[NUM_REVERSE][NUM_CENTERED];
	TweenState		m_tsCombo[NUM_REVERSE][NUM_CENTERED];

	bool m_bSendJudgmentAndComboMessages;
	bool m_bTickHolds;
	// This exists so that the board can be drawn underneath theme elements. -Kyz
	bool m_drawing_notefield_board;
};

class PlayerPlus
{
	Player *m_pPlayer;
	NoteData m_NoteData;
public:
	PlayerPlus() { m_pPlayer = new Player(m_NoteData); }
	~PlayerPlus() { delete m_pPlayer; }
	void Load( const NoteData &nd ) { m_NoteData = nd; m_pPlayer->Load(); }
	Player *operator->() { return m_pPlayer; }
	const Player *operator->() const { return m_pPlayer; }
	operator Player*() { return m_pPlayer; }
	operator const Player*() const { return m_pPlayer; }
};

#endif

/*
 * (c) 2001-2006 Chris Danford, Steve Checkoway
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
