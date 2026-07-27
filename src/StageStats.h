#ifndef StageStats_H
#define StageStats_H

#include "PlayerNumber.h"
#include "PlayerStageStats.h"

#include <vector>


class Song;
class Style;
struct lua_State;

/**
 * @brief Contains statistics for one stage of play.
 *
 * This is either one song, or a whole course. */
class StageStats
{
public:
	StageStats();
	void Init();

	/**
	 * @brief Ensure that the Player is valid.
	 * @param pn the PlayerNumber to check. */
	void AssertValid( PlayerNumber pn ) const;

	/**
	 * @brief Ensure that the Player is valid.
	 * @param mp the Multiplayer to check. */
	void AssertValid( MultiPlayer mp ) const;

	void AddStats( const StageStats& other );		// accumulate

	bool OnePassed() const;
	bool AllFailed() const;

	int		GetAverageMeter( PlayerNumber pn ) const;

	Stage		m_Stage;
	int		m_iStageIndex;
	PlayMode	m_playMode;
	std::vector<Song*>	m_vpPlayedSongs;
	std::vector<Song*>	m_vpPossibleSongs;

	/** @brief Was an extra stage earned this goaround? */
	EarnedExtraStage m_EarnedExtraStage;
	/** @brief Was the gameplay exited by the Player giving up? */
	bool	m_bGaveUp;
	/** @brief Did the PLayer use Autoplay at any point during gameplay? */
	bool	m_bUsedAutoplay;

	// TODO: These are updated in ScreenGameplay::Update based on fDelta.
	// They should be made more accurate.
	/**
	 * @brief How many seconds were there before gameplay ended?
	 *
	 * This is updated by Gameplay, and not scaled by the music rate. */
	float	m_fGameplaySeconds;
	/**
	 * @brief How many seconds are we in a song?
	 *
	 * This is equivalent to m_fGameplaySeconds unless the song has steps past the end. */
	float	m_fStepsSeconds;
	/** @brief How fast was the music going compared to normal? */
	float	m_fMusicRate;

	// Total number of seconds between first beat and last beat for every song.
	float GetTotalPossibleStepsSeconds() const;

	PlayerStageStats m_player[NUM_PLAYERS];
	PlayerStageStats m_multiPlayer[NUM_MultiPlayer];

	void FinalizeScores( bool bSummary );
	/**
	 * @brief Determine if the PlayerNumber has a high score.
	 * @param pn the PlayerNumber in question.
	 * @return true if the PlayerNumber has a high score, false otherwise. */
	bool PlayerHasHighScore( PlayerNumber pn ) const;
	unsigned int GetMinimumMissCombo() const;

	// Lua
	void PushSelf( lua_State *L );

private:
	// TODO: Implement the copy and assignment operators on our own.
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
