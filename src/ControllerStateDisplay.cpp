#include "global.h"
#include "ControllerStateDisplay.h"
#include "EnumHelper.h"
#include "RageUtil.h"
#include "RageInputDevice.h"
#include "ThemeManager.h"
#include "InputMapper.h"
#include "InputFilter.h"
#include "RageLog.h"
#include "LuaBinding.h"

static const char *ControllerStateButtonNames[] = {
	"UpLeft",
	"UpRight",
	"Center",
	"DownLeft",
	"DownRight",
};
XToString( ControllerStateButton );

// TODO: Generalize for all game types
static const GameButton ControllerStateButtonToGameButton[] = {
	PUMP_BUTTON_UPLEFT,
	PUMP_BUTTON_UPRIGHT,
	PUMP_BUTTON_CENTER,
	PUMP_BUTTON_DOWNLEFT,
	PUMP_BUTTON_DOWNRIGHT,
};

REGISTER_ACTOR_CLASS( ControllerStateDisplay );

ControllerStateDisplay::ControllerStateDisplay()
{
	m_bIsLoaded = false;
	m_mp = MultiPlayer_Invalid;
	m_idsLast = InputDeviceState_Invalid;
}

void ControllerStateDisplay::LoadMultiPlayer( RString sType, MultiPlayer mp )
{
	LoadInternal( sType, mp, GameController_1 );
}

void ControllerStateDisplay::LoadGameController( RString sType, GameController gc )
{
	LoadInternal( sType, MultiPlayer_Invalid, gc );
}

void ControllerStateDisplay::LoadInternal( RString sType, MultiPlayer mp, GameController gc )
{
	ASSERT( !m_bIsLoaded );
	m_bIsLoaded = true;
	m_mp = mp;

	LuaThreadVariable varElement( "MultiPlayer", LuaReference::Create(m_mp) );
	m_sprFrame.Load( THEME->GetPathG(sType, "frame") );
	this->AddChild( m_sprFrame );

	FOREACH_ENUM( ControllerStateButton, b )
	{
		Button &button = m_Buttons[ b ];

		RString sPath = THEME->GetPathG( sType, ControllerStateButtonToString(b) );
		button.spr.Load( sPath );
		this->AddChild( m_Buttons[b].spr );
		
		button.gi = GameInput( gc, ControllerStateButtonToGameButton[b] );
	}
}

void ControllerStateDisplay::Update( float fDelta )
{
	ActorFrame::Update( fDelta );

	if( m_mp != MultiPlayer_Invalid )
	{
		InputDevice id = InputMapper::MultiPlayerToInputDevice( m_mp );
		InputDeviceState ids = INPUTMAN->GetInputDeviceState(id);
		if( ids != m_idsLast )
		{
			PlayCommand( InputDeviceStateToString(ids) );
		}
		m_idsLast = ids;
	}

	FOREACH_ENUM( ControllerStateButton, b )
	{
		Button &button = m_Buttons[ b ];
		if( !button.spr.IsLoaded() )
			continue;

		bool bVisible = INPUTMAPPER->IsBeingPressed( button.gi, m_mp );

		button.spr->SetVisible( bVisible );
	}
}


/** @brief Allow Lua to have access to the ControllerStateDisplay. */ 
class LunaControllerStateDisplay: public Luna<ControllerStateDisplay>
{
public:
	static int LoadGameController( T* p, lua_State *L )	{ p->LoadGameController( SArg(1), Enum::Check<GameController>(L, 2) ); COMMON_RETURN_SELF; }
	static int LoadMultiPlayer( T* p, lua_State *L )	{ p->LoadMultiPlayer( SArg(1), Enum::Check<MultiPlayer>(L, 2) ); COMMON_RETURN_SELF; }

	LunaControllerStateDisplay() 
	{
		ADD_METHOD( LoadGameController );
		ADD_METHOD( LoadMultiPlayer );
	}
};

LUA_REGISTER_DERIVED_CLASS( ControllerStateDisplay, ActorFrame )

/*
 * (c) 2001-2004 Chris Danford
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
