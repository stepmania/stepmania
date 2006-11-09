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

	vector<Steps*>  vpPlayedSteps;
	vector<Steps*>  vpPossibleSteps;
	float		fAliveSeconds; // how far into the music did they last before failing?  Updated by Gameplay, scaled by music rate.

	/* Set if the player actually failed at any point during the song.  This is always
	 * false in FAIL_OFF.  If recovery is enabled and two players are playing,
	 * this is only set if both players were failing at the same time. */
	bool		bFailed;

	int		iPossibleDancePoints;
	int		iCurPossibleDancePoints;
	int		iActualDancePoints;
	int		iPossibleGradePoints;
	int		iTapNoteScores[NUM_TapNoteScore];
	int		iHoldNoteScores[NUM_HoldNoteScore];
	int		iCurCombo;
	int		iMaxCombo;
	int		iCurMissCombo;
	int		iScore;
	int		iCurMaxScore;
	int		iMaxScore;
	int		iBonus;  // bonus to be added on screeneval
	RadarValues	radarPossible;	// filled in by ScreenGameplay on start of notes
	RadarValues	radarActual;
	/* The number of songs played and passed, respectively. */
	int		iSongsPassed;
	int		iSongsPlayed;
	float		fLifeRemainingSeconds;	// used in survival

	// workout
	float		fCaloriesBurned;

	TapNoteScore	tnsLast;
	HoldNoteScore	hnsLast;

	map<float,float> fLifeRecord;
	void	SetLifeRecordAt( float fLife, float fStepsSecond );
	void	GetLifeRecord( float *fLifeOut, int iNumSamples, float fStepsEndSecond ) const;
	float	GetLifeRecordAt( float fStepsSecond ) const;
	float	GetLifeRecordLerpAt( float fStepsSecond ) const;

	struct Combo_t
	{
		/* Start and size of this combo, in the same scale as the combo list mapping and
		 * the life record. */
		float fStartSecond, fSizeSeconds;

		/* Combo size, in steps. */
		int cnt;

		/* Size of the combo that didn't come from this stage (rollover from the last song). 
		 * (This is a subset of cnt.) */
		int rollover;

		/* Get the size of the combo that came from this song. */
		int GetStageCnt() const { return cnt - rollover; }

		Combo_t(): fStartSecond(0), fSizeSeconds(0), cnt(0), rollover(0) { }
		bool IsZero() const { return fStartSecond < 0; }
	};
	vector<Combo_t> ComboList;
	float		fFirstSecond;
	float		fLastSecond;

	int	GetComboAtStartOfStage() const;
	bool	FullComboOfScore( TapNoteScore tnsAllGreaterOrEqual ) const;
	bool	FullCombo() const { return FullComboOfScore(TNS_W3); }
	bool	SingleDigitsOfScore( TapNoteScore tnsAllGreaterOrEqual ) const;
	bool	OneOfScore( TapNoteScore tnsAllGreaterOrEqual ) const;
	int	GetTotalTaps() const;
	float	GetPercentageOfTaps( TapNoteScore tns ) const;
	void	UpdateComboList( float fSecond, bool rollover );
	Combo_t GetMaxCombo() const;

	float GetSurvivalSeconds() const { return fAliveSeconds + fLifeRemainingSeconds; }

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
