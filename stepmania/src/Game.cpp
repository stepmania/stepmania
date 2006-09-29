#include "global.h"
#include "Game.h"
#include "RageLog.h"
#include "RageUtil.h"
#include "Style.h"
#include "GameState.h"
#include "PrefsManager.h"

int	Game::GetNumGameplayButtons() const
{
	int iIndexOfStart = ButtonNameToIndex( "Start" );
	ASSERT( iIndexOfStart != GameButton_Invalid );
	return iIndexOfStart;
}

GameButton Game::ButtonNameToIndex( const RString &sButtonName ) const
{
	for( int i=0; i<m_iButtonsPerController; i++ ) 
		if( stricmp(m_szButtonNames[i], sButtonName) == 0 )
			return i;

	return GameButton_Invalid;
}

MenuButton Game::GameInputToMenuButton( GameInput GameI ) const
{
	FOREACH_MenuButton(i)
		if( m_DedicatedMenuButton[i] == GameI.button )
			return i;

	if( !PREFSMAN->m_bOnlyDedicatedMenuButtons )
	{
		FOREACH_MenuButton(i)
			if( m_SecondaryMenuButton[i] == GameI.button )
				return i;
	}

	return MenuButton_INVALID;	// invalid GameInput
}

void Game::MenuButtonToGameInputs( MenuButton MenuI, PlayerNumber pn, GameInput GameIout[4] ) const
{
	ASSERT( MenuI != MenuButton_INVALID );

	GameIout[0].MakeInvalid();	// initialize
	GameIout[1].MakeInvalid();	
	GameIout[2].MakeInvalid();	
	GameIout[3].MakeInvalid();	

	GameController controller[2];
	StyleType type = TWO_PLAYERS_TWO_SIDES;
	if( GAMESTATE->GetCurrentStyle() )
		type = GAMESTATE->GetCurrentStyle()->m_StyleType;

	int iNumSidesUsing = 1;
	switch( type )
	{
	case ONE_PLAYER_ONE_SIDE:
	case TWO_PLAYERS_TWO_SIDES:
		controller[0] = (GameController)pn;
		iNumSidesUsing = 1;
		break;
	case ONE_PLAYER_TWO_SIDES:
	case TWO_PLAYERS_SHARED_SIDES:
		controller[0] = GAME_CONTROLLER_1;
		controller[1] = GAME_CONTROLLER_2;
		iNumSidesUsing = 2;
		break;
	default:
		ASSERT(0);	// invalid m_StyleType
	};

	GameButton button[2] = { m_DedicatedMenuButton[MenuI], m_SecondaryMenuButton[MenuI] };
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

RString Game::ColToButtonName( int iCol ) const
{
	const Style *pStyle = GAMESTATE->GetCurrentStyle();
	const char *pzColumnName = pStyle->m_ColumnInfo[PLAYER_1][iCol].pzName;
	if( pzColumnName != NULL )
		return pzColumnName;

	GameInput GI = pStyle->StyleInputToGameInput( iCol, PLAYER_1 );

	return m_szButtonNames[GI.button];
}

TapNoteScore Game::MapTapNoteScore( TapNoteScore tns ) const
{
	switch( tns )
	{
	case TNS_W1: return m_mapW1To;
	case TNS_W2: return m_mapW2To;
	case TNS_W3: return m_mapW3To;
	case TNS_W4: return m_mapW4To;
	case TNS_W5: return m_mapW5To;
	default: return tns;
	}
}

// lua start
#include "LuaBinding.h"

class LunaGame: public Luna<Game>
{
public:
	static int GetName( T* p, lua_State *L )			{ lua_pushstring( L, p->m_szName ); return 1; }

	LunaGame()
	{
		ADD_METHOD( GetName );
	}
};

LUA_REGISTER_CLASS( Game )
// lua end

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
