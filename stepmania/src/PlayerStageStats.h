/* PlayerStageStats - Contains statistics for one stage of play - either one song, or a whole course. */

#ifndef PlayerStageStats_H
#define PlayerStageStats_H

#include "Grade.h"
#include "RadarValues.h"
#include "HighScore.h"
#include <map>
class Steps;
struct lua_State;

class PlayerStageStats
{
public:
	PlayerStageStats() { Init(); }
	void Init();

	void AddStats( const PlayerStageStats& other );		// accumulate

	Grade GetGrade() const;
	float GetPercentDancePoints() const;
	float GetCurMaxPercentDancePoints() const;

	int GetLessonScoreActual() const;
	int GetLessonScoreNeeded() const;
	void ResetScoreForLesson();

	vector<Steps*>  m_vpPlayedSteps;
	vector<Steps*>  m_vpPossibleSteps;
	float		m_fAliveSeconds; // how far into the music did they last before failing?  Updated by Gameplay, scaled by music rate.

	/* Set if the player actually failed at any point during the song.  This is always
	 * false in FAIL_OFF.  If recovery is enabled and two players are playing,
	 * this is only set if both players were failing at the same time. */
	bool		m_bFailed;

	int		m_iPossibleDancePoints;
	int		m_iCurPossibleDancePoints;
	int		m_iActualDancePoints;
	int		m_iPossibleGradePoints;
	int		m_iTapNoteScores[NUM_TapNoteScore];
	int		m_iHoldNoteScores[NUM_HoldNoteScore];
	int		m_iCurCombo;
	int		m_iMaxCombo;
	int		m_iCurMissCombo;
	int		m_iCurScoreMultiplier;
	int		m_iScore;
	int		m_iCurMaxScore;
	int		m_iMaxScore;
	int		m_iBonus;  // bonus to be added on screeneval
	RadarValues	m_radarPossible;	// filled in by ScreenGameplay on start of notes
	RadarValues	m_radarActual;
	/* The number of songs played and passed, respectively. */
	int		m_iSongsPassed;
	int		m_iSongsPlayed;
	float		m_fLifeRemainingSeconds;	// used in survival

	// workout
	float		m_fCaloriesBurned;

	TapNoteScore	m_tnsLast;
	HoldNoteScore	m_hnsLast;

	map<float,float> m_fLifeRecord;
	void	SetLifeRecordAt( float fLife, float fStepsSecond );
	void	GetLifeRecord( float *fLifeOut, int iNumSamples, float fStepsEndSecond ) const;
	float	GetLifeRecordAt( float fStepsSecond ) const;
	float	GetLifeRecordLerpAt( float fStepsSecond ) const;

	struct Combo_t
	{
		/* Start and size of this combo, in the same scale as the combo list mapping and
		 * the life record. */
		float m_fStartSecond, m_fSizeSeconds;

		/* Combo size, in steps. */
		int m_cnt;

		/* Size of the combo that didn't come from this stage (rollover from the last song). 
		 * (This is a subset of cnt.) */
		int m_rollover;

		/* Get the size of the combo that came from this song. */
		int GetStageCnt() const { return m_cnt - m_rollover; }

		Combo_t(): m_fStartSecond(0), m_fSizeSeconds(0), m_cnt(0), m_rollover(0) { }
		bool IsZero() const { return m_fStartSecond < 0; }
	};
	vector<Combo_t> m_ComboList;
	float		m_fFirstSecond;
	float		m_fLastSecond;

	int	GetComboAtStartOfStage() const;
	bool	FullComboOfScore( TapNoteScore tnsAllGreaterOrEqual ) const;
	bool	FullCombo() const { return FullComboOfScore(TNS_W3); }
	bool	SingleDigitsOfScore( TapNoteScore tnsAllGreaterOrEqual ) const;
	bool	OneOfScore( TapNoteScore tnsAllGreaterOrEqual ) const;
	int	GetTotalTaps() const;
	float	GetPercentageOfTaps( TapNoteScore tns ) const;
	void	UpdateComboList( float fSecond, bool rollover );
	Combo_t GetMaxCombo() const;

	float GetSurvivalSeconds() const { return m_fAliveSeconds + m_fLifeRemainingSeconds; }

	// Final results:
	void CalcAwards( PlayerNumber p, bool bGaveUp, bool bUsedAutoplay );
	PerDifficultyAward m_pdaToShow;
	PeakComboAward m_pcaToShow;

	int		m_iPersonalHighScoreIndex;
	int		m_iMachineHighScoreIndex;
	RankingCategory	m_rc;
	HighScore	m_HighScore;

	// Lua
	void PushSelf( lua_State *L );
};


#endif

/*
 * (c) 2001-2004 Chris Danford, Glenn Maynard
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
