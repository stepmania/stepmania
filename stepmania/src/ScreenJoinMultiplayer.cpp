/*
 * Join players in perparation for ScreenGameplayMultiplayer.
 */

#include "global.h"
#include "ScreenJoinMultiplayer.h"
#include "RageInput.h"
class Style;
#include "GameManager.h"
#include "GameState.h"
#include "InputMapper.h"
#include "GameSoundManager.h"
#include "GameCommand.h"
#include "ScreenPrompt.h"
#include "ScreenManager.h"


static const CString MultiPlayerStatusNames[] = {
	"Joined",
	"NotJoined",
	"Unplugged",
	"MissingMultitap",
};
XToString( MultiPlayerStatus, NUM_MultiPlayerStatus );


REGISTER_SCREEN_CLASS( ScreenJoinMultiplayer );
ScreenJoinMultiplayer::ScreenJoinMultiplayer( CString sClassName ) : ScreenWithMenuElements( sClassName )
{
	FOREACH_MultiPlayer( p )
	{
		m_InputDeviceState[p] = InputDeviceState_INVALID;
		m_MultiPlayerStatus[p] = MultiPlayerStatus_INVALID;
	}
}

void ScreenJoinMultiplayer::Init()
{
	ScreenWithMenuElements::Init();

	GAMESTATE->m_bMultiplayer = true;

	// Set Style to the first style (should be a one player style)
	vector<const Style*> v;
	GAMEMAN->GetStylesForGame( GAMESTATE->m_pCurGame, v, false );
	GAMESTATE->m_pCurStyle.Set( v[0] );

	GAMESTATE->m_PlayMode.Set( PLAY_MODE_REGULAR );

	GAMESTATE->m_bTemporaryEventMode = true;

	FOREACH_MultiPlayer( p )
	{
		GameCommand gc;
		gc.m_iIndex = p;

		Lua *L = LUA->Get();
		gc.PushSelf( L );
		lua_setglobal( L, "ThisGameCommand" );
		LUA->Release( L );

		m_sprPlayer[p].Load( THEME->GetPathG(m_sName, "player") );
		this->AddChild( m_sprPlayer[p] );

		LUA->UnsetGlobal( "ThisGameCommand" );
	}

	m_exprOnCommandFunction.SetFromExpression( THEME->GetMetric(m_sName,"TranformFunction") );

	FOREACH_MultiPlayer( p )
	{
		PositionItem( m_sprPlayer[p], p, NUM_MultiPlayer );
	}

	m_soundPlugIn.Load(	THEME->GetPathS(m_sName,"PlugIn") );
	m_soundUnplug.Load(	THEME->GetPathS(m_sName,"Unplug") );
	m_soundJoin.Load(	THEME->GetPathS(m_sName,"Join") );
	m_soundUnjoin.Load(	THEME->GetPathS(m_sName,"Unjoin") );

	UpdatePlayerStatus( true );
}

void ScreenJoinMultiplayer::PositionItem( Actor *pActor, int iItemIndex, int iNumItems )
{
	Lua *L = LUA->Get();
	m_exprOnCommandFunction.PushSelf( L );
	ASSERT( !lua_isnil(L, -1) );
	pActor->PushSelf( L );
	LuaHelpers::Push( iItemIndex, L );
	LuaHelpers::Push( iNumItems, L );
	lua_call( L, 3, 0 ); // 3 args, 0 results
	LUA->Release(L);
}

void ScreenJoinMultiplayer::HandleScreenMessage( const ScreenMessage SM )
{
	ScreenWithMenuElements::HandleScreenMessage( SM );
}

void ScreenJoinMultiplayer::Input( const DeviceInput& DeviceI, const InputEventType type, const GameInput &GameI, const MenuInput &MenuI, const StyleInput &StyleI )
{
	if( this->IsTransitioning() )
		return;

	// Translate input and sent to the appropriate player.  Assume that all 
	// joystick devices are mapped the same as the master player.
	if( type == IET_FIRST_PRESS  &&
		DeviceI.device >= DEVICE_JOY1  &&  
		DeviceI.device < DEVICE_JOY1 + NUM_MultiPlayer )
	{
		DeviceInput di = DeviceI;
		di.device = DEVICE_JOY1;
		GameInput gi;
		INPUTMAPPER->DeviceToGame( di, gi );

		if( GameI.IsValid() )
		{
			MenuInput mi;
			INPUTMAPPER->GameToMenu( gi, mi );

			MultiPlayer p = (MultiPlayer)(DeviceI.device - DEVICE_JOY1);

			// testing hack
			if( INPUTFILTER->IsBeingPressed( DeviceInput(DEVICE_KEYBOARD,KEY_LSHIFT) ) )
				p = (MultiPlayer)(p + 1);
			if( INPUTFILTER->IsBeingPressed( DeviceInput(DEVICE_KEYBOARD,KEY_LCTRL) ) )
				p = (MultiPlayer)(p + 2);
			if( INPUTFILTER->IsBeingPressed( DeviceInput(DEVICE_KEYBOARD,KEY_LALT) ) )
				p = (MultiPlayer)(p + 4);

			ASSERT( p>=0 && p<NUM_MultiPlayer );
			
			MultiPlayerStatus old = m_MultiPlayerStatus[p];

			switch( mi.button )
			{
			case MENU_BUTTON_UP:
				if( old == MultiPlayerStatus_NotJoined )
				{
					m_soundJoin.Play();
					m_MultiPlayerStatus[p] = MultiPlayerStatus_Joined;
					m_sprPlayer[p]->PlayCommand( MultiPlayerStatusToString(m_MultiPlayerStatus[p]) );
				}
				return;	// input handled
			case MENU_BUTTON_DOWN:
				if( old == MultiPlayerStatus_Joined )
				{
					m_soundUnjoin.Play();
					m_MultiPlayerStatus[p] = MultiPlayerStatus_NotJoined;
					m_sprPlayer[p]->PlayCommand( MultiPlayerStatusToString(m_MultiPlayerStatus[p]) );
				}
				return;	// input handled
			case MENU_BUTTON_BACK:
				MenuBack( GAMESTATE->m_MasterPlayerNumber );
				return;	// input handled
			case MENU_BUTTON_START:
				MenuStart( GAMESTATE->m_MasterPlayerNumber );
				return;	// input handled
			}
		}
	}

	ScreenWithMenuElements::Input( DeviceI, type, GameI, MenuI, StyleI );
}

void ScreenJoinMultiplayer::Update( float fDeltaTime )
{
	ScreenWithMenuElements::Update(fDeltaTime);

	UpdatePlayerStatus( false );
}

void ScreenJoinMultiplayer::DrawPrimitives()
{
	ScreenWithMenuElements::DrawPrimitives();
}

void ScreenJoinMultiplayer::UpdatePlayerStatus( bool bFirstUpdate )
{
	FOREACH_MultiPlayer( p )
	{
		InputDevice id = (InputDevice)p;
	
		InputDeviceState idsOld = m_InputDeviceState[p];
		bool bWasConnected = idsOld == InputDeviceState_Connected;
		
		InputDeviceState idsNew = INPUTMAN->GetInputDeviceState(id);
		if( INPUTFILTER->IsBeingPressed(DeviceInput(DEVICE_KEYBOARD,KEY_LSHIFT)) )
			idsNew = InputDeviceState_Disconnected;
		bool bIsConnected = idsNew == InputDeviceState_Connected;

		MultiPlayerStatus &mps = m_MultiPlayerStatus[p];

		if( bFirstUpdate  ||  idsOld != idsNew )
		{
			switch( idsNew )
			{
			default:
				ASSERT(0);
			case InputDeviceState_Connected:
				mps = MultiPlayerStatus_NotJoined;
				break;
			case InputDeviceState_INVALID:
			case InputDeviceState_Disconnected:
				mps = MultiPlayerStatus_Unplugged;
				break;
			case InputDeviceState_MissingMultitap:
				mps = MultiPlayerStatus_MissingMultitap;
				break;
			}

			m_sprPlayer[p]->PlayCommand( MultiPlayerStatusToString(mps) );

			if( idsOld == InputDeviceState_INVALID )
			{
				m_sprPlayer[p]->FinishTweening();
			}
			else
			{
				if( !bWasConnected && bIsConnected )
					m_soundPlugIn.Play();
				else if( bWasConnected && !bIsConnected )
					m_soundUnplug.Play();
			}
		}

		m_InputDeviceState[p] = idsNew;
	}
}

void ScreenJoinMultiplayer::MenuBack( PlayerNumber pn )
{
	SOUND->StopMusic();
	Cancel( SM_GoToPrevScreen );
}

void ScreenJoinMultiplayer::MenuStart( PlayerNumber pn )
{
	int iNumJoinedPlayers = 0;
	FOREACH_MultiPlayer( p )
	{
		bool bJoined = m_MultiPlayerStatus[p] == MultiPlayerStatus_Joined;
		GAMESTATE->m_bIsMultiPlayerJoined[p] = bJoined;
		if( bJoined )
			iNumJoinedPlayers++;
	}

	SCREENMAN->PlayStartSound();

	if( iNumJoinedPlayers < 2 )
	{
		ScreenPrompt::Prompt( SM_None, "You must join 2 or more players before continuing." );
		return;
	}
	
	this->StartTransitioning( SM_GoToNextScreen );
}


/*
 * (c) 2005 Chris Danford
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
