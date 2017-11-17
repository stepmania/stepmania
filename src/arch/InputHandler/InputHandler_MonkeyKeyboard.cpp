#include "global.h"
#include "InputHandler_MonkeyKeyboard.h"
#include "RageUtil.h"
#include "PrefsManager.h"


InputHandler_MonkeyKeyboard::InputHandler_MonkeyKeyboard()
{
	m_dbLast = DeviceButton_Invalid;
}

InputHandler_MonkeyKeyboard::~InputHandler_MonkeyKeyboard()
{
}

void InputHandler_MonkeyKeyboard::GetDevicesAndDescriptions( vector<InputDeviceInfo>& vDevicesOut )
{
	vDevicesOut.push_back( InputDeviceInfo(DEVICE_KEYBOARD, "MonkeyKeyboard") );
}

static const DeviceButton g_keys[] =
{
	// Some of the default keys for the dance game type
	DB_KEY_LEFT,		// DANCE_BUTTON_LEFT,
	DB_KEY_RIGHT,		// DANCE_BUTTON_RIGHT,
	DB_KEY_UP,			// DANCE_BUTTON_UP,
	DB_KEY_DOWN,		// DANCE_BUTTON_DOWN,
	DB_KEY_ENTER,		// DANCE_BUTTON_START,
	DB_KEY_ENTER,		// DANCE_BUTTON_START,
	DB_KEY_ENTER,		// DANCE_BUTTON_START,
	DB_KEY_DEL,		// DANCE_BUTTON_MENULEFT
	DB_KEY_PGDN,		// DANCE_BUTTON_MENURIGHT
	DB_KEY_HOME,		// DANCE_BUTTON_MENUUP
	DB_KEY_END,		// DANCE_BUTTON_MENUDOWN
	DB_KEY_F1,			// DANCE_BUTTON_COIN
	DB_KEY_F1,			// DANCE_BUTTON_COIN
	DB_KEY_KP_C4,		// DANCE_BUTTON_LEFT,
	DB_KEY_KP_C6,		// DANCE_BUTTON_RIGHT,
	DB_KEY_KP_C8,		// DANCE_BUTTON_UP,
	DB_KEY_KP_C2,		// DANCE_BUTTON_DOWN,
	DB_KEY_KP_C7,		// DANCE_BUTTON_UPLEFT,
	DB_KEY_KP_C9,		// DANCE_BUTTON_UPRIGHT,
	DB_KEY_KP_ENTER,		// DANCE_BUTTON_START,
	DB_KEY_KP_ENTER,		// DANCE_BUTTON_START,
	DB_KEY_KP_ENTER,		// DANCE_BUTTON_START,
	DB_KEY_KP_SLASH,		// DANCE_BUTTON_MENULEFT
	DB_KEY_KP_ASTERISK,	// DANCE_BUTTON_MENURIGHT
	DB_KEY_KP_HYPHEN,		// DANCE_BUTTON_MENUUP
	DB_KEY_KP_PLUS,		// DANCE_BUTTON_MENUDOWN
};

static DeviceButton GetRandomKeyboardKey()
{
	int index = RandomInt( ARRAYLEN(g_keys) );
	return g_keys[index];
}


void InputHandler_MonkeyKeyboard::Update()
{
	if( !PREFSMAN->m_bMonkeyInput )
	{
		if( m_dbLast != DeviceButton_Invalid )
		{
			// End the previous key
			DeviceInput di = DeviceInput( DEVICE_KEYBOARD, m_dbLast, 0 );
			ButtonPressed( di );
			m_dbLast = DeviceButton_Invalid;
		}
		InputHandler::UpdateTimer();
		return;
	}
	
	float fSecsAgo = m_timerPressButton.Ago();

	if( fSecsAgo > 0.5 )
	{
		if( m_dbLast != DeviceButton_Invalid )
		{
			// End the previous key
			DeviceInput di = DeviceInput( DEVICE_KEYBOARD, m_dbLast, 0 );
			ButtonPressed( di );
		}

		// Choose a new key and send it.
		m_dbLast = GetRandomKeyboardKey();
		DeviceInput di = DeviceInput( DEVICE_KEYBOARD, m_dbLast, 1 );
		ButtonPressed( di );
		m_timerPressButton.Touch();
	}

	InputHandler::UpdateTimer();
}

/*
 * (c) 2002-2004 Chris Danford
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

