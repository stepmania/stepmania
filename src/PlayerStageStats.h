#ifndef PlayerStageStats_H
#define PlayerStageStats_H

#include "Grade.h"
#include "RadarValues.h"
#include "HighScore.h"
#include "PlayerNumber.h"

#include <map>
#include <vector>


class Steps;
class Style;
struct lua_State;
/** @brief Contains statistics for one stage of play - either one song, or a whole course. */
class PlayerStageStats
{
public:
	/** @brief Set up the PlayerStageStats with default values. */
	PlayerStageStats() { InternalInit(); }
	void InternalInit();
	void Init(PlayerNumber pn);
	void Init(MultiPlayer pn);

	/**
	 * @brief Add stats from one PlayerStageStats to another.
	 * @param other the other stats to add to this one. */
	void AddStats( const PlayerStageStats& other );		// accumulate

	Grade GetGrade() const;
	static float MakePercentScore( int iActual, int iPossible );
	static RString FormatPercentScore( float fPercentScore );
	float GetPercentDancePoints() const;
	float GetCurMaxPercentDancePoints() const;

	int GetLessonScoreActual() const;
	int GetLessonScoreNeeded() const;
	void ResetScoreForLesson();

	bool m_for_multiplayer;
	PlayerNumber m_player_number;
	MultiPlayer m_multiplayer_number;
	const Style*	m_pStyle;

	bool		m_bJoined;
  bool    m_bPlayerCanAchieveFullCombo;
	std::vector<Steps*>  m_vpPossibleSteps;
	int		m_iStepsPlayed; // how many of m_vpPossibleStepshow many of m_vpPossibleSteps were played
	/**
	 * @brief How far into the music did the Player last before failing?
	 *
	 * This is updated by Gameplay, and scaled by the music rate. */
	float		m_fAliveSeconds;

	/**
	 * @brief Have the Players failed at any point during the song?
	 *
	 * If FAIL_OFF is in use, this is always false.
	 *
	 * If health recovery is possible after failing (requires two players),
	 * this is only set if both players were failing at the same time. */
	bool		m_bFailed;

	int		m_iPossibleDancePoints;
	int		m_iCurPossibleDancePoints;
	int		m_iActualDancePoints;
	int		m_iPossibleGradePoints;
	int		m_iTapNoteScores[NUM_TapNoteScore];
	int		m_iHoldNoteScores[NUM_HoldNoteScore];
	/** @brief The Player's current combo. */
	unsigned int		m_iCurCombo;
	/** @brief The Player's max combo. */
	unsigned int		m_iMaxCombo;
	/** @brief The Player's current miss combo. */
	unsigned int		m_iCurMissCombo;
	int		m_iCurScoreMultiplier;
	/** @brief The player's current score. */
	unsigned int		m_iScore;
	/** @brief The theoretically highest score the Player could have at this point. */
	unsigned int		m_iCurMaxScore;
	/** @brief The maximum score the Player can get this goaround. */
	unsigned int		m_iMaxScore;

	/**
	 * @brief The possible RadarValues for a song.
	 *
	 * This is filled in by ScreenGameplay on the start of the notes. */
	RadarValues	m_radarPossible;
	RadarValues	m_radarActual;
	/** @brief How many songs were passed by the Player? */
	int		m_iSongsPassed;
	/** @brief How many songs were played by the Player? */
	int		m_iSongsPlayed;
	/**
	 * @brief How many seconds were left for the Player?
	 *
	 * This is used in the Survival mode. */
	float		m_fLifeRemainingSeconds;

	// workout
	float		m_iNumControllerSteps;
	float		m_fCaloriesBurned;

	std::map<float,float> m_fLifeRecord;
	void	SetLifeRecordAt( float fLife, float fStepsSecond );
	void	GetLifeRecord( float *fLifeOut, int iNumSamples, float fStepsEndSecond ) const;
	float	GetLifeRecordAt( float fStepsSecond ) const;
	float	GetLifeRecordLerpAt( float fStepsSecond ) const;
	float	GetCurrentLife() const;

	struct Combo_t
	{
		// Update GetComboList in PlayerStageStats.cpp when adding new members that should be visible from the Lua side.
		/**
		 * @brief The start time of the combo.
		 *
		 * This uses the same scale as the combo list mapping. */
		float m_fStartSecond;
		/**
		 * @brief The size time of the combo.
		 *
		 * This uses the same scale as the life record. */
		float m_fSizeSeconds;

		/** @brief The size of the Combo, in steps. */
		int m_cnt;

		/**
		 * @brief The size of the combo that didn't come from this stage.
		 *
		 * This is generally rolled over from the last song.
		 * It is also a subset of m_cnt. */
		int m_rollover;

		/**
		 * @brief Retrieve the size of the combo that came from this song.
		 * @return this song's combo size. */
		int GetStageCnt() const { return m_cnt - m_rollover; }

		Combo_t(): m_fStartSecond(0), m_fSizeSeconds(0), m_cnt(0), m_rollover(0) { }
		bool IsZero() const { return m_fStartSecond < 0; }
	};
	std::vector<Combo_t> m_ComboList;
	float	m_fFirstSecond;
	float	m_fLastSecond;

	int	GetComboAtStartOfStage() const;
	bool	FullComboOfScore( TapNoteScore tnsAllGreaterOrEqual ) const;
	bool	FullCombo() const { return FullComboOfScore(TNS_W3); }
	TapNoteScore GetBestFullComboTapNoteScore() const;
	bool	SingleDigitsOfScore( TapNoteScore tnsAllGreaterOrEqual ) const;
	bool	OneOfScore( TapNoteScore tnsAllGreaterOrEqual ) const;
	int		GetTotalTaps() const;
	float	GetPercentageOfTaps( TapNoteScore tns ) const;
	void	UpdateComboList( float fSecond, bool rollover );
	Combo_t GetMaxCombo() const;

	float GetSurvivalSeconds() const { return m_fAliveSeconds + m_fLifeRemainingSeconds; }

	// Final results:
	void CalcAwards( PlayerNumber p, bool bGaveUp, bool bUsedAutoplay );
	StageAward m_StageAward;
	PeakComboAward m_PeakComboAward;

	int		m_iPersonalHighScoreIndex;
	int		m_iMachineHighScoreIndex;
	bool	m_bDisqualified;
	bool	IsDisqualified() const;

	RankingCategory	m_rc;
	HighScore	m_HighScore;

	// Lua
	void PushSelf( lua_State *L );
};

#endif

/**
 * @file
 * @author Chris Danford, Glenn Maynard (c) 2001-2004
 * @section LICENSE
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
