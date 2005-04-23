#include "global.h"
#include "ScreenTestInput.h"
#include "ScreenManager.h"
#include "RageLog.h"
#include "InputMapper.h"
#include "GameState.h"
#include "GameSoundManager.h"
#include "ThemeManager.h"
#include "Game.h"
#include "ScreenDimensions.h"
#include "GameManager.h"
#include "PrefsManager.h"


REGISTER_SCREEN_CLASS( ScreenTestInput );
ScreenTestInput::ScreenTestInput( CString sClassName ) : ScreenWithMenuElements( sClassName )
{
	LOG->Trace( "ScreenTestInput::ScreenTestInput()" );
}

void ScreenTestInput::Init()
{
	ScreenWithMenuElements::Init();

	m_textInputs.LoadFromFont( THEME->GetPathF("Common","normal") );
	m_textInputs.SetXY( SCREEN_CENTER_X-250, SCREEN_CENTER_Y );
	m_textInputs.SetDiffuse( RageColor(1,1,1,1) );
	m_textInputs.SetZoom( 0.7f );
	m_textInputs.SetHorizAlign( Actor::align_left );
	this->AddChild( &m_textInputs );

	SOUND->PlayMusic( THEME->GetPathS("ScreenTestInput","music") );
}



ScreenTestInput::~ScreenTestInput()
{
	LOG->Trace( "ScreenTestInput::~ScreenTestInput()" );
}


void ScreenTestInput::Update( float fDeltaTime )
{
	Screen::Update( fDeltaTime );

	CStringArray asInputs;

	DeviceInput di;

	for( int d=0; d<NUM_INPUT_DEVICES; d++ )
	{
		for( int b=0; b<NUM_DEVICE_BUTTONS[d]; b++ )
		{
			di.device = (InputDevice)d;
			di.button = b;

			if( INPUTFILTER->IsBeingPressed(di) )
			{
				CString sTemp;
				sTemp += di.GetDescription();
				
				GameInput gi;
				if( INPUTMAPPER->DeviceToGame(di,gi) )
				{
					CString sName = GAMESTATE->GetCurrentGame()->m_szButtonNames[gi.button];
					sTemp += ssprintf(" - Controller %d %s", gi.controller+1, sName.c_str() );

					if( !PREFSMAN->m_bOnlyDedicatedMenuButtons )
					{
						CString sSecondary = GAMEMAN->GetMenuButtonSecondaryFunction( GAMESTATE->GetCurrentGame(), gi.button );
						if( !sSecondary.empty() )
							sTemp += ssprintf(" - (%s secondary)", sSecondary.c_str() );
					}
				}
				else
				{
					sTemp += " - not mapped";
				}

				asInputs.push_back( sTemp );
			}
		}
	}

	m_textInputs.SetText( join( "\n", asInputs ) );
}


void ScreenTestInput::DrawPrimitives()
{
	Screen::DrawPrimitives();
}

void ScreenTestInput::Input( const DeviceInput& DeviceI, const InputEventType type, const GameInput &GameI, const MenuInput &MenuI, const StyleInput &StyleI )
{
	if( type != IET_FIRST_PRESS && type != IET_SLOW_REPEAT )
		return;	// ignore

	Screen::Input( DeviceI, type, GameI, MenuI, StyleI );	// default handler
}

void ScreenTestInput::HandleScreenMessage( const ScreenMessage SM )
{
	switch( SM )
	{
	case SM_GoToNextScreen:
	case SM_GoToPrevScreen:
		SCREENMAN->SetNewScreen( "ScreenOptionsMenu" );
		break;
	}
}

void ScreenTestInput::MenuStart( PlayerNumber pn )
{
	MenuBack(pn);
}

void ScreenTestInput::MenuBack( PlayerNumber pn )
{
	if(!IsTransitioning())
	{
		SCREENMAN->PlayStartSound();
		StartTransitioning( SM_GoToPrevScreen );		
	}
}

/*
 * (c) 2003-2004 Chris Danford
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
