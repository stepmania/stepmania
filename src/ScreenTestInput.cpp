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

using std::vector;

class DeviceList: public BitmapText
{
public:
	void Update( float fDeltaTime )
	{
		// Update devices text
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
		// Update input texts
		vector<std::string> asInputs;

		vector<DeviceInput> DeviceInputs;
		INPUTFILTER->GetPressedButtons( DeviceInputs );
		for (auto &di: DeviceInputs)
		{
			if( !di.bDown && di.level == 0.0f )
			{
				continue;
			}
			// TODO: Utilize a string stream.
			std::string sTemp = INPUTMAN->GetDeviceSpecificInputString(di);
			if( di.level == 1.0f )
			{
				sTemp += fmt::sprintf(" - 1 " );
			}
			else
			{
				sTemp += fmt::sprintf(" - %.3f ", di.level );
			}
			GameInput gi;
			if( INPUTMAPPER->DeviceToGame(di,gi) )
			{
				auto sName = GameButtonToLocalizedString( INPUTMAPPER->GetInputScheme(), gi.button );
				sTemp += fmt::sprintf(" - %s %d %s", CONTROLLER.GetValue().c_str(), gi.controller+1, sName.c_str() );

				if( !PREFSMAN->m_bOnlyDedicatedMenuButtons )
				{
					GameButton mb = INPUTMAPPER->GetInputScheme()->GameButtonToMenuButton( gi.button );
					if( mb != GameButton_Invalid && mb != gi.button )
					{
						auto sGameButtonString = GameButtonToLocalizedString( INPUTMAPPER->GetInputScheme(), mb );
						sTemp += fmt::sprintf( " - (%s %s)", sGameButtonString.c_str(), SECONDARY.GetValue().c_str() );
					}
				}
			}
			else
			{
				sTemp += " - "+NOT_MAPPED.GetValue();
			}

			auto sComment = INPUTFILTER->GetButtonComment( di );
			if( sComment != "" )
			{
				sTemp += " - " + sComment;
			}
			asInputs.push_back( sTemp );
		}

		this->SetText( Rage::join( "\n", asInputs ) );

		BitmapText::Update( fDeltaTime );
	}
};

REGISTER_ACTOR_CLASS( InputList );

REGISTER_SCREEN_CLASS( ScreenTestInput );

bool ScreenTestInput::Input( const InputEventPlus &input )
{
	std::string sMessage = input.DeviceI.ToString();
	bool bHandled = false;
	switch( input.type )
	{
		case IET_FIRST_PRESS:
		case IET_RELEASE:
		{
			switch( input.type )
			{
				case IET_FIRST_PRESS:	sMessage += "Pressed";	break;
				case IET_RELEASE:	sMessage += "Released";	break;
				default: break;
			}
			MESSAGEMAN->Broadcast( sMessage );
			bHandled = true;
			break;
		}
		default: break;
	}

	return Screen::Input( input ) || bHandled;	// default handler
}

bool ScreenTestInput::MenuStart( const InputEventPlus &input )
{
	return MenuBack( input );
}

bool ScreenTestInput::MenuBack( const InputEventPlus &input )
{
	if( input.type != IET_REPEAT )
		return false;	// ignore

	if( IsTransitioning() )
		return false;
	SCREENMAN->PlayStartSound();
	StartTransitioningScreen( SM_GoToPrevScreen );
	return true;
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
