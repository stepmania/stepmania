//-----------------------------------------------------------------------------
// File: Player.h
//
// Desc: Object that accepts pad input, knocks down ColorArrows that were stepped on, 
//       and keeps score for the player.
//
// Copyright (c) 2001 Chris Danford.  All rights reserved.
//-----------------------------------------------------------------------------

#ifndef _PLAYER_H_
#define _PLAYER_H_


#include "GameOptions.h"
#include "GrayArrows.h"
#include "Steps.h"
#include "ColorArrows.h"
#include "Judgement.h"
#include "Combo.h"
#include "Score.h"
#include "LifeMeter.h"


enum StepScore { no_score, perfect, great, good, boo, miss };


class Player
{
public:
	Player()
	{
		m_fSongBeat = 0.0;

		for( int i=0; i<2; i++ ) {
			for( int j=0; j<4; j++ ) {
				m_fStepCountDown[i][j] = 0.0;
			}
		}

		m_pGA = NULL;
		m_pCA = NULL;	
		m_pJudgement = NULL;
		m_pCombo = NULL;
		m_pScore = NULL;
		m_pLifeMeter = NULL;

		m_iCurCombo = 0;
		m_iMaxCombo = 0;
		m_fScore = 0.0f;
		m_fLife = 0.50f;
	};
	~Player() {};

	void Set( GrayArrows *pGA,	ColorArrows *pCA,
			  Judgement *pJ,	Combo *pC,
			  Score *pS,		LifeMeter *pL,
			  Steps steps,		FLOAT fMaxBeatDifference );

	void SetSongBeat( const FLOAT &fSongBeat ) { m_fSongBeat = fSongBeat; };
	void Update( const FLOAT &fDeltaTime );

	void StepOn( Step player_step );
	void HandleStep();
	int UpdateMissedStepsOlderThan( FLOAT iMissIfOlderThanThisBeat );

	FLOAT GetLife() { return m_fLife; };
	ScoreSummary GetScoreSummary();
private:

	FLOAT		m_fSongBeat;
	FLOAT		m_fMaxBeatDifference;

	// step cache (so player doesn't have to hit buttons 
	// on exact same update in order to hit two arrow notes)
	FLOAT		m_fStepCountDown[2][4];

	GrayArrows		*m_pGA;
	ColorArrows		*m_pCA;

	Judgement		*m_pJudgement;
	Combo			*m_pCombo;
	Score			*m_pScore;
	LifeMeter		*m_pLifeMeter;

	Steps			m_Steps;
	CArray<StepScore, StepScore&>	m_StepScore;

	int		m_iCurCombo;
	int		m_iMaxCombo;

	FLOAT	m_fLife;
	FLOAT	m_fScore;
	
};




#endif