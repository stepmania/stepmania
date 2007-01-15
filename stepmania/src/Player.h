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
#include "ScreenMessage.h"
//#include "RageLog.h"

class ScoreDisplay;
class LifeMeter;
class CombinedLifeMeter;
class ScoreKeeper;
class Inventory;
class RageTimer;
class NoteField;
class PlayerStageStats;

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

// Helper class to ensure that each row is only judged once without taking too much memory.
class JudgedRows
{
	char	*m_pRows;
	int 	m_iStart;
	int	m_iOffset;
	int	m_iLen;
	
	void Resize( int iMin )
	{
		char *p = m_pRows;
		int newSize = max( m_iLen*2, iMin );
		//LOG->Trace( "Old size %d, new size %d.", m_iLen, newSize );
		m_pRows = new char[newSize];
		int i = 0;
		if( p )
		{
			for( ; i < m_iLen; ++i )
				m_pRows[i] = p[(i+m_iOffset)%m_iLen];
			delete[] p;
		}
		m_iOffset = 0;
		m_iLen = newSize;
		memset( m_pRows + i, 0, newSize - i );
	}
public:
	JudgedRows() : m_pRows(NULL), m_iStart(0), m_iOffset(0), m_iLen(0) { Resize( 32 ); }
	~JudgedRows() { delete[] m_pRows; }
	bool operator[]( int iRow )
	{
		if( iRow < m_iStart ) return true;
		if( iRow >= m_iStart+m_iLen ) Resize( iRow+1-m_iStart );
		const bool ret = m_pRows[(iRow-m_iStart+m_iOffset)%m_iLen] != 0;
		m_pRows[(iRow-m_iStart+m_iOffset)%m_iLen] = 1;
		while( m_pRows[m_iOffset] )
		{
			m_pRows[m_iOffset] = 0;
			++m_iStart;
			if( ++m_iOffset >= m_iLen ) m_iOffset -= m_iLen;
		}
		return ret;
	}
	void Reset( int iStart )
	{
		m_iStart = iStart;
		m_iOffset = 0;
		memset( m_pRows, 0, m_iLen );
	}
};

class Player: public ActorFrame
{
public:
	// The passed in NoteData isn't touched until Load() is called.
	Player( NoteData &nd, bool bShowNoteField = true, bool bShowJudgment = true );
	~Player();

	virtual void Update( float fDeltaTime );
	virtual void DrawPrimitives();

	struct TrackRowTapNote
	{
		int iTrack;
		int iRow;
		TapNote *pTN;
	};
	void UpdateHoldNotes( int iSongRow, float fDeltaTime, vector<TrackRowTapNote> &vTN );

	void Init( 
		const RString &sType,
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
	void CrossedRow( int iNoteRow, const RageTimer &now );
	void CrossedMineRow( int iNoteRow, const RageTimer &now );
	bool IsOniDead() const;

	// Called when a fret, step, or strum type button changes
	void Fret( int col, int row, const RageTimer &tm, bool bHeld, bool bRelease );
	enum ButtonType { ButtonType_Step, ButtonType_Strum, ButtonType_Hopo };
	void StepStrumHopo( int col, int row, const RageTimer &tm, bool bHeld, bool bRelease, ButtonType gbt );
	void Step( int col, int row, const RageTimer &tm, bool bHeld, bool bRelease )	{ StepStrumHopo(col, row, tm, bHeld, bRelease, ButtonType_Step); }
	void Strum( int col, int row, const RageTimer &tm, bool bHeld, bool bRelease )	{ StepStrumHopo(col, row, tm, bHeld, bRelease, ButtonType_Strum); }
	
	// called by Fret for Hammer-ons and Pull-offs
	void Hopo( int col, int row, const RageTimer &tm, bool bHeld, bool bRelease )	{ StepStrumHopo(col, row, tm, bHeld, bRelease, ButtonType_Hopo); }
	
	void RandomizeNotes( int iNoteRow );
	void FadeToFail();
	void CacheAllUsedNoteSkins();
	TapNoteScore GetLastTapNoteScore() const { return m_LastTapNoteScore; }
	void ApplyWaitingTransforms();
	void SetPaused( bool bPaused ) { m_bPaused = bPaused; }

	static float GetMaxStepDistanceSeconds();
	static float GetWindowSeconds( TimingWindow tw );
	const NoteData &GetNoteData() const { return m_NoteData; }
	bool HasNoteField() const { return m_pNoteField != NULL; }

protected:
	void UpdateTapNotesMissedOlderThan( float fMissIfOlderThanThisBeat );
	void UpdateJudgedRows();
	void FlashGhostRow( int iRow, PlayerNumber pn );
	void HandleTapRowScore( unsigned row );
	void HandleHoldScore( const TapNote &tn );
	void DrawTapJudgments();
	void DrawHoldJudgments();
	void SendComboMessages( int iOldCombo, int iOldMissCombo );

	void SetJudgment( TapNoteScore tns, bool bEarly );
	void SetHoldJudgment( TapNoteScore tns, HoldNoteScore hns, int iTrack );

	void ChangeLife( TapNoteScore tns );
	void ChangeLife( HoldNoteScore hns, TapNoteScore tns );
	void ChangeLifeRecord();

	int GetClosestNoteDirectional( int col, int iStartRow, int iMaxRowsAhead, bool bAllowGraded, bool bForward ) const;
	int GetClosestNote( int col, int iNoteRow, int iMaxRowsAhead, int iMaxRowsBehind, bool bAllowGraded ) const;
	int GetClosestNonEmptyRowDirectional( int iStartRow, int iMaxRowsAhead, bool bAllowGraded, bool bForward ) const;
	int GetClosestNonEmptyRow( int iNoteRow, int iMaxRowsAhead, int iMaxRowsBehind, bool bAllowGraded ) const;

	bool IsPlayingBeginner() const;
	inline void HideNote( int col, int row )
	{
		NoteData::iterator iter = m_NoteData.FindTapNote( col, row );
		if( iter != m_NoteData.end(col) )
			iter->second.result.bHidden = true;
	}

	bool			m_bLoaded;

	PlayerState		*m_pPlayerState;
	PlayerStageStats	*m_pPlayerStageStats;
	float			m_fNoteFieldHeight;

	bool			m_bPaused;

	NoteData		&m_NoteData;
	NoteField		*m_pNoteField;

	vector<HoldJudgment*>	m_vHoldJudgment;

	Judgment		*m_pJudgment;
	AutoActor		m_sprJudgmentFrame;
		
	Combo			*m_pCombo;

	AttackDisplay		*m_pAttackDisplay;

	TapNoteScore		m_LastTapNoteScore;
	LifeMeter		*m_pLifeMeter;
	CombinedLifeMeter	*m_pCombinedLifeMeter;
	ScoreDisplay		*m_pScoreDisplay;
	ScoreDisplay		*m_pSecondaryScoreDisplay;
	ScoreKeeper		*m_pPrimaryScoreKeeper;
	ScoreKeeper		*m_pSecondaryScoreKeeper;
	Inventory		*m_pInventory;

	int			m_iRowLastCrossed;
	int			m_iMineRowLastCrossed;
	int			m_iRowLastJudged; // Everything up to and including this row has been judged.
	int			m_iMineRowLastJudged;
	JudgedRows		m_JudgedRows;

	RageSound		m_soundMine;
	RageSound		m_soundAttackLaunch;
	RageSound		m_soundAttackEnding;

	vector<bool>	m_vbFretIsDown;

	vector<RageSound>	m_vKeysounds;

	RString			m_sMessageToSendOnStep;

	ThemeMetric<float>	GRAY_ARROWS_Y_STANDARD;
	ThemeMetric<float>	GRAY_ARROWS_Y_REVERSE;
	ThemeMetric2D<float>	COMBO_X;
	ThemeMetric<float>	COMBO_Y;
	ThemeMetric<float>	COMBO_Y_REVERSE;
	ThemeMetric<float>	COMBO_CENTERED_ADDY;
	ThemeMetric<float>	COMBO_CENTERED_ADDY_REVERSE;
	ThemeMetric2D<float>	ATTACK_DISPLAY_X;
	ThemeMetric<float>	ATTACK_DISPLAY_Y;
	ThemeMetric<float>	ATTACK_DISPLAY_Y_REVERSE;
	ThemeMetric<float>	HOLD_JUDGMENT_Y_STANDARD;
	ThemeMetric<float>	HOLD_JUDGMENT_Y_REVERSE;
	ThemeMetric<int>	BRIGHT_GHOST_COMBO_THRESHOLD;
	ThemeMetric<bool>	TAP_JUDGMENTS_UNDER_FIELD;
	ThemeMetric<bool>	HOLD_JUDGMENTS_UNDER_FIELD;
	ThemeMetric<int>	DRAW_DISTANCE_AFTER_TARGET_PIXELS;
	ThemeMetric<int>	DRAW_DISTANCE_BEFORE_TARGET_PIXELS;
	
#define NUM_REVERSE 2
#define NUM_CENTERED 2
	TweenState		m_tsJudgment[NUM_REVERSE][NUM_CENTERED];
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
