#include "global.h"
#include "Game.h"
#include "RageLog.h"
#include "RageUtil.h"
#include "IniFile.h"
#include "Style.h"
#include "RageException.h"
#include "GameState.h"
#include "InputMapper.h"
#include "PrefsManager.h"

int	Game::GetNumGameplayButtons() const
{
	int iIndexOfStart = ButtonNameToIndex( "Start" );
	ASSERT( iIndexOfStart != GAME_BUTTON_INVALID );
	return iIndexOfStart;
}

GameButton Game::ButtonNameToIndex( const CString &sButtonName ) const
{
	for( int i=0; i<m_iButtonsPerController; i++ ) 
		if( stricmp(m_szButtonNames[i], sButtonName) == 0 )
			return i;

	return GAME_BUTTON_INVALID;
}

MenuInput Game::GameInputToMenuInput( GameInput GameI ) const
{
	PlayerNumber pn;

	Style::StyleType type = Style::TWO_PLAYERS_TWO_CREDITS;
	if( GAMESTATE->GetCurrentStyle() )
		type = GAMESTATE->GetCurrentStyle()->m_StyleType;
	switch( type )
	{
	case Style::ONE_PLAYER_ONE_CREDIT:
	case Style::TWO_PLAYERS_TWO_CREDITS:
		pn = (PlayerNumber)GameI.controller;
		break;
	case Style::ONE_PLAYER_TWO_CREDITS:
		pn = GAMESTATE->m_MasterPlayerNumber;
		break;
	default:
		ASSERT(0);	return MenuInput(); // invalid m_StyleType
	};
	
	int i;
	for( i=0; i<NUM_MENU_BUTTONS; i++ )
		if( m_DedicatedMenuButton[i] == GameI.button )
			return MenuInput( pn, (MenuButton)i );

	if( !PREFSMAN->m_bOnlyDedicatedMenuButtons )
	{
		for( i=0; i<NUM_MENU_BUTTONS; i++ )
			if( m_SecondaryMenuButton[i] == GameI.button )
				return MenuInput( pn, (MenuButton)i );
	}

	return MenuInput();	// invalid GameInput
}

void Game::MenuInputToGameInput( MenuInput MenuI, GameInput GameIout[4] ) const
{
	ASSERT( MenuI.IsValid() );

	GameIout[0].MakeInvalid();	// initialize
	GameIout[1].MakeInvalid();	
	GameIout[2].MakeInvalid();	
	GameIout[3].MakeInvalid();	

	GameController controller[2];
	Style::StyleType type = Style::TWO_PLAYERS_TWO_CREDITS;
	if( GAMESTATE->GetCurrentStyle() )
		type = GAMESTATE->GetCurrentStyle()->m_StyleType;

	int iNumSidesUsing = 1;
	switch( type )
	{
	case Style::ONE_PLAYER_ONE_CREDIT:
	case Style::TWO_PLAYERS_TWO_CREDITS:
		controller[0] = (GameController)MenuI.player;
		iNumSidesUsing = 1;
		break;
	case Style::ONE_PLAYER_TWO_CREDITS:
		controller[0] = GAME_CONTROLLER_1;
		controller[1] = GAME_CONTROLLER_2;
		iNumSidesUsing = 2;
		break;
	default:
		ASSERT(0);	// invalid m_StyleType
	};

	GameButton button[2] = { m_DedicatedMenuButton[MenuI.button], m_SecondaryMenuButton[MenuI.button] };
	int iNumButtonsUsing = PREFSMAN->m_bOnlyDedicatedMenuButtons ? 1 : 2;

	for( int i=0; i<iNumSidesUsing; i++ )
	{
		for( int j=0; j<iNumButtonsUsing; j++ )
		{
			GameIout[i*2+j].controller = controller[i];
			GameIout[i*2+j].button = button[j];
		}
	}
}

CString Game::ColToButtonName( int col ) const
{
	const Style* pStyle = GAMESTATE->GetCurrentStyle();

	StyleInput SI( PLAYER_1, col );
	GameInput GI = pStyle->StyleInputToGameInput( SI );

	return m_szButtonNames[GI.button];
}

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
