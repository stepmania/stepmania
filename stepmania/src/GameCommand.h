/* GameCommand */

#ifndef GameCommand_H
#define GameCommand_H

#include "GameConstantsAndTypes.h"
#include "PlayerNumber.h"
#include "Difficulty.h"
#include "LuaReference.h"
#include "Command.h"
#include <map>

class Song;
class Steps;
class Course;
class Trail;
class Character;
class Style;
class Game;
struct lua_State;

class GameCommand
{
public:
	GameCommand() { Init(); }
	void Init();

	void Load( int iIndex, const Commands& cmds );
	void LoadOne( const Command& cmd );
	
	void ApplyToAllPlayers() const;
	void Apply( PlayerNumber pn ) const;
private:
	void Apply( const vector<PlayerNumber> &vpns ) const;
	void ApplySelf( const vector<PlayerNumber> &vpns ) const;
public:

	bool DescribesCurrentMode( PlayerNumber pn ) const;
	bool DescribesCurrentModeForAllPlayers() const;
	bool IsPlayable( CString *why = NULL ) const;
	bool IsZero() const;
	CString GetAndClearScreen();

	// Same as what was passed to Load.  We need to keep the original commands
	// so that we know the order of commands when it comes time to Apply.
	Commands	m_Commands;

	CString		m_sName;	// choice name
	CString		m_sText;	// display text
	bool		m_bInvalid;
	CString		m_sInvalidReason;
	int			m_iIndex;
	MultiPlayer	m_MultiPlayer;
	const Game*	m_pGame;
	const Style*	m_pStyle;
	PlayMode	m_pm;
	Difficulty	m_dc;
	CourseDifficulty	m_CourseDifficulty;
	CString		m_sAnnouncer;
	CString		m_sModifiers;
	CString		m_sScreen;
	LuaExpression	m_LuaFunction;
	Song*		m_pSong;
	Steps*		m_pSteps;
	Course*		m_pCourse;
	Trail*		m_pTrail;
	Character*	m_pCharacter;
	std::map<CString,CString> m_SetEnv;
	CString		m_sSongGroup;
	SortOrder	m_SortOrder;
	int			m_iUnlockIndex;	// -1 for no unlock
	CString		m_sSoundPath;	// "" for no sound
	vector<CString>	m_vsScreensToPrepare;
	int			m_iWeightPounds;	// -1 == none specified
	int			m_iGoalCalories;	// -1 == none specified
	GoalType	m_GoalType;
	CString		m_sProfileID;

	bool m_bInsertCredit;
	bool m_bStopMusic;
	bool m_bApplyDefaultOptions;

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
