#include "stdafx.h"
/*
-----------------------------------------------------------------------------
 Class: GameDef

 Desc: See header.

 Copyright (c) 2001-2002 by the person(s) listed below.  All rights reserved.
	Chris Danford
-----------------------------------------------------------------------------
*/

#include "GameDef.h"
#include "RageLog.h"
#include "RageUtil.h"
#include "IniFile.h"
#include "StyleDef.h"
#include "RageException.h"
#include "GameState.h"
#include "InputMapper.h"
#include "PrefsManager.h"

GameButton GameDef::ButtonNameToIndex( const CString &sButtonName )
{
	for( int i=0; i<m_iButtonsPerController; i++ ) 
		if( stricmp(m_szButtonNames[i], sButtonName) == 0 )
			return i;

	return GAME_BUTTON_INVALID;
}

MenuInput GameDef::GameInputToMenuInput( GameInput GameI ) const
{
	PlayerNumber pn;

	StyleDef::StyleType type = StyleDef::TWO_PLAYERS_TWO_CREDITS;
	if( GAMESTATE->m_CurStyle != STYLE_NONE )
		type = GAMESTATE->GetCurrentStyleDef()->m_StyleType;
	switch( type )
	{
	case StyleDef::ONE_PLAYER_ONE_CREDIT:
	case StyleDef::TWO_PLAYERS_TWO_CREDITS:
		pn = (PlayerNumber)GameI.controller;
		break;
	case StyleDef::ONE_PLAYER_TWO_CREDITS:
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

void GameDef::MenuInputToGameInput( MenuInput MenuI, GameInput GameIout[4] ) const
{
	ASSERT( MenuI.IsValid() );

	GameIout[0].MakeInvalid();	// initialize
	GameIout[1].MakeInvalid();	
	GameIout[2].MakeInvalid();	
	GameIout[3].MakeInvalid();	

	GameController controller[2];
	int iNumSidesUsing = 1;
	switch( GAMESTATE->GetCurrentStyleDef()->m_StyleType )
	{
	case StyleDef::ONE_PLAYER_ONE_CREDIT:
	case StyleDef::TWO_PLAYERS_TWO_CREDITS:
		controller[0] = (GameController)MenuI.player;
		iNumSidesUsing = 1;
		break;
	case StyleDef::ONE_PLAYER_TWO_CREDITS:
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
