#include "global.h"
#include "InputHandler_MonkeyKeyboard.h"
#include "RageUtil.h"
#include "PrefsManager.h"


InputHandler_MonkeyKeyboard::InputHandler_MonkeyKeyboard()
{
}

InputHandler_MonkeyKeyboard::~InputHandler_MonkeyKeyboard()
{
}

void InputHandler_MonkeyKeyboard::GetDevicesAndDescriptions(vector<InputDevice>& vDevicesOut, vector<CString>& vDescriptionsOut)
{
	vDevicesOut.push_back( InputDevice(DEVICE_KEYBOARD) );
	vDescriptionsOut.push_back( "MonkeyKeyboard" );
}

static const int g_keys[] =
{
	// Some of the default keys for the dance game type
	KEY_LEFT,				// DANCE_BUTTON_LEFT,
	KEY_RIGHT,				// DANCE_BUTTON_RIGHT,
	KEY_UP,				// DANCE_BUTTON_UP,
	KEY_DOWN,				// DANCE_BUTTON_DOWN,
	KEY_ENTER,			// DANCE_BUTTON_START,
	KEY_ENTER,			// DANCE_BUTTON_START,
	KEY_ENTER,			// DANCE_BUTTON_START,
	KEY_DEL,			// DANCE_BUTTON_MENULEFT
	KEY_PGDN,			// DANCE_BUTTON_MENURIGHT
	KEY_HOME,				// DANCE_BUTTON_MENUUP
	KEY_END,				// DANCE_BUTTON_MENUDOWN
	KEY_F1,				// DANCE_BUTTON_COIN
	KEY_F1,				// DANCE_BUTTON_COIN
	KEY_KP_C4,				// DANCE_BUTTON_LEFT,
	KEY_KP_C6,				// DANCE_BUTTON_RIGHT,
	KEY_KP_C8,				// DANCE_BUTTON_UP,
	KEY_KP_C2,				// DANCE_BUTTON_DOWN,
	KEY_KP_C7,				// DANCE_BUTTON_UPLEFT,
	KEY_KP_C9,				// DANCE_BUTTON_UPRIGHT,
	KEY_KP_ENTER,			// DANCE_BUTTON_START,
	KEY_KP_ENTER,			// DANCE_BUTTON_START,
	KEY_KP_ENTER,			// DANCE_BUTTON_START,
	KEY_KP_SLASH,			// DANCE_BUTTON_MENULEFT
	KEY_KP_ASTERISK,		// DANCE_BUTTON_MENURIGHT
	KEY_KP_HYPHEN,			// DANCE_BUTTON_MENUUP
	KEY_KP_PLUS,			// DANCE_BUTTON_MENUDOWN
};

static int GetRandomKeyboardKey()
{
	int index = rand()%ARRAYSIZE(g_keys);
	return g_keys[index];
}


void InputHandler_MonkeyKeyboard::Update()
{
	if( !PREFSMAN->m_bMonkeyInput )
	{
		if( m_diLast.IsValid() )
		{
			// End the previous key
			ButtonPressed(m_diLast, false);
			m_diLast.MakeInvalid();
		}
		InputHandler::UpdateTimer();
		return;
	}
	
	float fSecsAgo = m_timerPressButton.Ago();

	if( fSecsAgo > 0.5 )
	{
		// End the previous key
		ButtonPressed(m_diLast, false);

		// Choose a new key and send it.
		m_diLast = DeviceInput(DEVICE_KEYBOARD,GetRandomKeyboardKey());
		ButtonPressed(m_diLast, true);

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

