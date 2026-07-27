/* GameCommand */

#ifndef GameCommand_H
#define GameCommand_H

#include "GameConstantsAndTypes.h"
#include "PlayerNumber.h"
#include "Difficulty.h"
#include "LuaReference.h"
#include "Command.h"

#include <map>
#include <vector>


class Song;
class Steps;
class Course;
class Trail;
class Character;
class Style;
struct Game;
struct lua_State;

int GetNumCreditsPaid();
int GetCreditsRequiredToPlayStyle( const Style *style );

class GameCommand
{
public:
	GameCommand(): m_Commands(), m_sName(""), m_sText(""),
		m_bInvalid(true), m_sInvalidReason(""),
		m_iIndex(-1), m_MultiPlayer(MultiPlayer_Invalid),
		m_pStyle(nullptr), m_pm(PlayMode_Invalid),
		m_dc(Difficulty_Invalid),
		m_CourseDifficulty(Difficulty_Invalid),
		m_sAnnouncer(""), m_sPreferredModifiers(""),
		m_sStageModifiers(""), m_sScreen(""), m_LuaFunction(),
		m_pSong(nullptr), m_pSteps(nullptr), m_pCourse(nullptr),
		m_pTrail(nullptr), m_pCharacter(nullptr), m_SetEnv(), m_SetPref(),
		m_sSongGroup(""), m_SortOrder(SortOrder_Invalid),
		m_sSoundPath(""), m_vsScreensToPrepare(), m_iWeightPounds(-1),
		m_iGoalCalories(-1), m_GoalType(GoalType_Invalid),
		m_sProfileID(""), m_sUrl(""), m_bUrlExits(true),
		m_bInsertCredit(false), m_bClearCredits(false),
		m_bStopMusic(false), m_bApplyDefaultOptions(false),
		m_bFadeMusic(false), m_fMusicFadeOutVolume(-1),
		m_fMusicFadeOutSeconds(-1), m_bApplyCommitsScreens(true)
	{
		m_LuaFunction.Unset();
	}
	void Init();

	void Load( int iIndex, const Commands& cmds );
	void LoadOne( const Command& cmd );

	void ApplyToAllPlayers() const;
	void Apply( PlayerNumber pn ) const;
private:
	void Apply( const std::vector<PlayerNumber> &vpns ) const;
	void ApplySelf( const std::vector<PlayerNumber> &vpns ) const;
public:

	bool DescribesCurrentMode( PlayerNumber pn ) const;
	bool DescribesCurrentModeForAllPlayers() const;
	bool IsPlayable( RString *why = nullptr ) const;
	bool IsZero() const;

	/* If true, Apply() will apply m_sScreen. If false, it won't, and you need
	 * to do it yourself. */
	void ApplyCommitsScreens( bool bOn ) { m_bApplyCommitsScreens = bOn; }

	// Same as what was passed to Load. We need to keep the original commands
	// so that we know the order of commands when it comes time to Apply.
	Commands	m_Commands;

	RString		m_sName;	// choice name
	RString		m_sText;	// display text
	bool		m_bInvalid;
	RString		m_sInvalidReason;
	int		m_iIndex;
	MultiPlayer	m_MultiPlayer;
	const Style*	m_pStyle;
	PlayMode	m_pm;
	Difficulty	m_dc;
	CourseDifficulty	m_CourseDifficulty;
	RString		m_sAnnouncer;
	RString		m_sPreferredModifiers;
	RString		m_sStageModifiers;
	RString		m_sScreen;
	LuaReference	m_LuaFunction;
	Song*		m_pSong;
	Steps*		m_pSteps;
	Course*		m_pCourse;
	Trail*		m_pTrail;
	Character*	m_pCharacter;
	std::map<RString,RString> m_SetEnv;
	std::map<RString,RString> m_SetPref;
	RString		m_sSongGroup;
	SortOrder	m_SortOrder;
	RString		m_sSoundPath;	// "" for no sound
	std::vector<RString>	m_vsScreensToPrepare;
	/**
	 * @brief What is the player's weight in pounds?
	 *
	 * If this value is -1, then no weight was specified. */
	int		m_iWeightPounds;
	int		m_iGoalCalories;	// -1 == none specified
	GoalType	m_GoalType;
	RString		m_sProfileID;
	RString		m_sUrl;
	// sm-ssc adds:
	bool		m_bUrlExits;	// for making stepmania not exit on url

	bool m_bInsertCredit;
	bool m_bClearCredits;
	bool m_bStopMusic;
	bool m_bApplyDefaultOptions;
	// sm-ssc also adds:
	bool m_bFadeMusic;
	float m_fMusicFadeOutVolume;
	// currently, GameSoundManager uses consts for fade out/in times, so this
	// is kind of pointless, but I want to have it working eventually. -aj
	float m_fMusicFadeOutSeconds;

	// Lua
	void PushSelf( lua_State *L );

private:
	bool		m_bApplyCommitsScreens;
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
