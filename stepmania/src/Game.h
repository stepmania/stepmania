/* Game - Holds information about a particular style of a game (e.g. "single", "double"). */

#ifndef GAMEDEF_H
#define GAMEDEF_H

#include "GameInput.h"
#include "MenuInput.h"
#include "GameConstantsAndTypes.h"
#include "RageInputDevice.h"

struct lua_State;

const int MAX_STYLES_PER_GAME = 10;

//
// PrimaryMenuButton and SecondaryMenuButton are used to support using DeviceInputs that only 
// navigate the menus.
// 
// A button being a primary menu button means that this GameButton will generate a the
// corresponding MenuInput IF AND ONLY IF the GameButton corresponding to the pimary input 
// is not mapped.
//
// Example 1:  A user is using an arcade machine as their controller.  Most machines have
// MenuLeft, MenuStart, and MenuRight buttons on the cabinet, so they should be used to navigate menus.
// The user will map these DeviceInputs to the GameButtons "MenuLeft (optional)", "MenuStart", and 
// "MenuRight (optional)".
//
// Example 2:  A user is using PlayStation dance pads to play.  These controllers don't have dedicated
// DeviceInputs for MenuLeft and MenuRight.  The user maps Up, Down, Left, and Right as normal.
// Since the Left and Right GameButtons have the flag FLAG_SECONDARY_MENU_*, they will function as 
// MenuLeft and MenuRight as long as "MenuLeft (optional)" and "MenuRight (optional)" are not mapped.
//

class Style;

#define NO_DEFAULT_KEY DeviceButton_Invalid

class Game
{
public:
	const char	*m_szName;
	const char	*m_szDescription;

	int		m_iNumControllers;
	bool		m_bCountNotesSeparately;	// Count multiple notes in a row as separate notes or as one note
	int		m_iButtonsPerController;
	int		GetNumGameplayButtons() const;
	char		m_szButtonNames[MAX_GAME_BUTTONS][60];	// The name used by the button graphics system.  e.g. "left", "right", "middle C", "snare"
	GameButton	m_DedicatedMenuButton[NUM_MenuButton];
	GameButton	m_SecondaryMenuButton[NUM_MenuButton];
	DeviceButton	m_iDefaultKeyboardKey[MAX_GAME_CONTROLLERS][MAX_GAME_BUTTONS];	// default keyboard keys only have an effect the current game is this game

	GameButton ButtonNameToIndex( const RString &sButtonName ) const;
	MenuButton GameInputToMenuButton( GameInput GameI ) const;
	void MenuInputToGameInput( MenuInput MenuI, PlayerNumber pn, GameInput GameIout[4] ) const;	// looks up current style in GAMESTATE.  Yuck.
	RString ColToButtonName( int col ) const;	// looks up current style.  Yuck.

	TapNoteScore MapTapNoteScore( TapNoteScore tns ) const;
	TapNoteScore	m_mapW1To;
	TapNoteScore	m_mapW2To;
	TapNoteScore	m_mapW3To;
	TapNoteScore	m_mapW4To;
	TapNoteScore	m_mapW5To;

	//
	// Lua
	//
	void PushSelf( lua_State *L );
};

#endif

/*
 * (c) 2001-2002 Chris Danford
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
