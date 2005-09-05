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
#include "RageInput.h"
#include "InputEventPlus.h"


REGISTER_SCREEN_CLASS( ScreenTestInput );
ScreenTestInput::ScreenTestInput( CString sClassName ) : ScreenWithMenuElements( sClassName )
{
	LOG->Trace( "ScreenTestInput::ScreenTestInput()" );
}

void ScreenTestInput::Init()
{
	ScreenWithMenuElements::Init();

	m_textDevices.LoadFromFont( THEME->GetPathF("Common","normal") );
	m_textDevices.SetXY( SCREEN_LEFT+20, SCREEN_TOP+80 );
	m_textDevices.SetDiffuse( RageColor(1,1,1,1) );
	m_textDevices.SetZoom( 0.7f );
	m_textDevices.SetHorizAlign( Actor::align_left );
	{
		vector<InputDevice> vDevices;
		vector<CString> vDescriptions;
		INPUTMAN->GetDevicesAndDescriptions( vDevices, vDescriptions );
		FOREACH( CString, vDescriptions, s )
		{
			if( *s == "MonkeyKeyboard" )
			{
				vDescriptions.erase( s );
				break;
			}
		}
		m_textDevices.SetText( join("\n",vDescriptions) );
	}
	this->AddChild( &m_textDevices );

	m_textInputs.LoadFromFont( THEME->GetPathF("Common","normal") );
	m_textInputs.SetXY( SCREEN_CENTER_X-250, SCREEN_CENTER_Y );
	m_textInputs.SetDiffuse( RageColor(1,1,1,1) );
	m_textInputs.SetZoom( 0.7f );
	m_textInputs.SetHorizAlign( Actor::align_left );
	this->AddChild( &m_textInputs );

	this->SortByDrawOrder();
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

	FOREACH_InputDevice( d )
	{
		for( int b=0; b<GetNumDeviceButtons(d); b++ )
		{
			di.device = d;
			di.button = b;

			if( !INPUTFILTER->IsBeingPressed(di) )
				continue;

			CString sTemp;
			sTemp += INPUTMAN->GetDeviceSpecificInputString(di);
			
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

			CString sComment = INPUTFILTER->GetButtonComment( di );
			if( sComment != "" )
				sTemp += " - " + sComment;

			asInputs.push_back( sTemp );
		}
	}

	m_textInputs.SetText( join( "\n", asInputs ) );
}


void ScreenTestInput::Input( const InputEventPlus &input )
{
	CString sMessage = input.DeviceI.toString();
	switch( input.type )
	{
	case IET_FIRST_PRESS:
	case IET_RELEASE:
		{
			switch( input.type )
			{
			case IET_FIRST_PRESS:	sMessage += "Pressed";	break;
			case IET_RELEASE:		sMessage += "Released";	break;
			}
			MESSAGEMAN->Broadcast( sMessage );
		}
	}

	if( input.type != IET_FIRST_PRESS && input.type != IET_SLOW_REPEAT )
		return;	// ignore

	Screen::Input( input );	// default handler
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
