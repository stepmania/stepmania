#include "global.h"
#include "ScreenTestInput.h"
#include "ScreenManager.h"
#include "RageLog.h"
#include "InputMapper.h"
#include "ThemeManager.h"
#include "ScreenDimensions.h"
#include "PrefsManager.h"
#include "RageInput.h"
#include "InputEventPlus.h"
#include "LocalizedString.h"


class DeviceList: public BitmapText
{
public:
	void Update( float fDeltaTime )
	{
		//
		// Update devices text
		//
		this->SetText( INPUTMAN->GetDisplayDevicesString() );

		BitmapText::Update( fDeltaTime );
	}

	virtual DeviceList *Copy() const;
};

REGISTER_ACTOR_CLASS( DeviceList );

static LocalizedString CONTROLLER	( "ScreenTestInput", "Controller" );
static LocalizedString SECONDARY	( "ScreenTestInput", "secondary" );
static LocalizedString NOT_MAPPED	( "ScreenTestInput", "not mapped" );
class InputList: public BitmapText
{
	virtual InputList *Copy() const;

	void Update( float fDeltaTime )
	{
		//
		// Update input texts
		//
		vector<RString> asInputs;

		DeviceInput di;
		vector<DeviceInput> DeviceInputs;
		INPUTFILTER->GetPressedButtons( DeviceInputs );
		FOREACH( DeviceInput, DeviceInputs, di )
		{
			if( !di->bDown && di->level == 0.0f )
				continue;

			RString sTemp;
			sTemp += INPUTMAN->GetDeviceSpecificInputString(*di);
			
			GameInput gi;
			if( INPUTMAPPER->DeviceToGame(*di,gi) )
			{
				RString sName = GameButtonToLocalizedString( INPUTMAPPER->GetInputScheme(), gi.button );
				sTemp += ssprintf(" - %s %d %s", CONTROLLER.GetValue().c_str(), gi.controller+1, sName.c_str() );

				if( !PREFSMAN->m_bOnlyDedicatedMenuButtons )
				{
					GameButton mb = INPUTMAPPER->GetInputScheme()->GameButtonToMenuButton( gi.button );
					if( mb != GameButton_Invalid && mb != gi.button )
					{
						RString sGameButtonString = GameButtonToLocalizedString( INPUTMAPPER->GetInputScheme(), mb );
						sTemp += ssprintf( " - (%s %s)", sGameButtonString.c_str(), SECONDARY.GetValue().c_str() );
					}
				}
			}
			else
			{
				sTemp += " - "+NOT_MAPPED.GetValue();
			}

			RString sComment = INPUTFILTER->GetButtonComment( *di );
			if( sComment != "" )
				sTemp += " - " + sComment;

			asInputs.push_back( sTemp );
		}

		this->SetText( join( "\n", asInputs ) );

		BitmapText::Update( fDeltaTime );
	}
};

REGISTER_ACTOR_CLASS( InputList );

REGISTER_SCREEN_CLASS( ScreenTestInput );

void ScreenTestInput::Input( const InputEventPlus &input )
{
	RString sMessage = input.DeviceI.ToString();
	switch( input.type )
	{
	case IET_FIRST_PRESS:
	case IET_RELEASE:
		switch( input.type )
		{
		case IET_FIRST_PRESS:	sMessage += "Pressed";	break;
		case IET_RELEASE:		sMessage += "Released";	break;
		}
		MESSAGEMAN->Broadcast( sMessage );
	}

	Screen::Input( input );	// default handler
}

void ScreenTestInput::MenuStart( const InputEventPlus &input )
{
	MenuBack( input );
}

void ScreenTestInput::MenuBack( const InputEventPlus &input )
{
	if( input.type != IET_REPEAT )
		return;	// ignore

	if( !IsTransitioning() )
	{
		SCREENMAN->PlayStartSound();
		StartTransitioningScreen( SM_GoToPrevScreen );		
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
