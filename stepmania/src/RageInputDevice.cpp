/*
 * Define all of the input devices we know about.  This is the public
 * interface for describing input devices.
 */

#include "global.h"
#include "RageInputDevice.h"
#include "RageUtil.h"
#include "EnumHelper.h"


CString DeviceButtonToString( DeviceButton key )
{
	if( key >= 33 && key < 127 &&
		!(key >= 'A' && key <= 'Z' ) )
	{
		/* All printable ASCII except for uppercase alpha characters line up. */
		return ssprintf( "%c", key );
	}

	if( key >= KEY_OTHER_0 && key < KEY_LAST_OTHER )
	{
		return ssprintf( "unk %i", key-KEY_OTHER_0 );
	}

	if( key >= JOY_BUTTON_1 && key <= JOY_BUTTON_32 )
	{
		return ssprintf( "%i", key-JOY_BUTTON_1+1 );
	}

	if( key >= MIDI_FIRST && key <= MIDI_LAST )
	{
		return ssprintf( "Midi %d", key-MIDI_FIRST );
	}

	switch( key )
	{
	case KEY_SPACE:			return "space"; 
	case KEY_DEL:			return "delete"; 

	case KEY_BACK:			return "backspace"; 
	case KEY_TAB:			return "tab"; 
	case KEY_ENTER:			return "enter"; 
	case KEY_PAUSE:			return "pause"; 
	case KEY_ESC:			return "escape"; 

	case KEY_F1:			return "F1"; 
	case KEY_F2:			return "F2"; 
	case KEY_F3:			return "F3"; 
	case KEY_F4:			return "F4"; 
	case KEY_F5:			return "F5"; 
	case KEY_F6:			return "F6"; 
	case KEY_F7:			return "F7"; 
	case KEY_F8:			return "F8"; 
	case KEY_F9:			return "F9"; 
	case KEY_F10:			return "F10"; 
	case KEY_F11:			return "F11"; 
	case KEY_F12:			return "F12"; 
	case KEY_F13:			return "F13"; 
	case KEY_F14:			return "F14"; 
	case KEY_F15:			return "F15"; 
	case KEY_F16:			return "F16"; 

	case KEY_LCTRL:			return "left ctrl"; 
	case KEY_RCTRL:			return "right ctrl"; 
	case KEY_LSHIFT:		return "left shift"; 
	case KEY_RSHIFT:		return "right shift"; 
	case KEY_LALT:			return "left alt"; 
	case KEY_RALT:			return "right alt"; 
	case KEY_LMETA:			return "left meta"; 
	case KEY_RMETA:			return "right meta"; 
	case KEY_LSUPER:		return "left wnd"; 
	case KEY_RSUPER:		return "right wnd"; 
	case KEY_MENU:			return "menu"; 

	case KEY_NUMLOCK:		return "num lock"; 
	case KEY_SCRLLOCK:		return "scroll lock"; 
	case KEY_CAPSLOCK:		return "caps lock"; 
	case KEY_PRTSC:			return "prtsc"; 

	case KEY_UP:			return "up"; 
	case KEY_DOWN:			return "down"; 
	case KEY_LEFT:			return "left"; 
	case KEY_RIGHT:			return "right"; 

	case KEY_INSERT:		return "insert"; 
	case KEY_HOME:			return "home"; 
	case KEY_END:			return "end"; 
	case KEY_PGUP:			return "pgup"; 
	case KEY_PGDN:			return "pgdn"; 

	case KEY_KP_C0:			return "KP 0"; 
	case KEY_KP_C1:			return "KP 1"; 
	case KEY_KP_C2:			return "KP 2"; 
	case KEY_KP_C3:			return "KP 3"; 
	case KEY_KP_C4:			return "KP 4"; 
	case KEY_KP_C5:			return "KP 5"; 
	case KEY_KP_C6:			return "KP 6"; 
	case KEY_KP_C7:			return "KP 7"; 
	case KEY_KP_C8:			return "KP 8"; 
	case KEY_KP_C9:			return "KP 9"; 
	case KEY_KP_SLASH:		return "KP /"; 
	case KEY_KP_ASTERISK:	return "KP *"; 
	case KEY_KP_HYPHEN:		return "KP -"; 
	case KEY_KP_PLUS:		return "KP +"; 
	case KEY_KP_PERIOD:		return "KP ."; 
	case KEY_KP_EQUAL:		return "KP ="; 
	case KEY_KP_ENTER:		return "KP enter"; 

	case JOY_LEFT:			return "Left1";
	case JOY_RIGHT:			return "Right1";
	case JOY_UP:			return "Up1";
	case JOY_DOWN:			return "Down1";

	/* Secondary sticks: */
	case JOY_LEFT_2:		return "Left2";
	case JOY_RIGHT_2:		return "Right2";
	case JOY_UP_2:			return "Up2";
	case JOY_DOWN_2:		return "Down2";


	case JOY_Z_UP:			return "Z-Up";
	case JOY_Z_DOWN:		return "Z-Down";
	case JOY_ROT_UP:		return "R-Up";
	case JOY_ROT_DOWN:		return "R-Down";
	case JOY_ROT_LEFT:		return "R-Left";
	case JOY_ROT_RIGHT:		return "R-Right";
	case JOY_ROT_Z_UP:		return "ZR-Up";
	case JOY_ROT_Z_DOWN:	return "ZR-Down";
	case JOY_HAT_LEFT:		return "H-Left";
	case JOY_HAT_RIGHT:		return "H-Right";
	case JOY_HAT_UP:		return "H-Up";
	case JOY_HAT_DOWN:		return "H-Down";
	case JOY_AUX_1:			return "Aux1";
	case JOY_AUX_2:			return "Aux2";
	case JOY_AUX_3:			return "Aux3";
	case JOY_AUX_4:			return "Aux4";

	default:				return "unknown";
	}
}

DeviceButton StringToDeviceButton( const CString& s )
{
	for( int i=0; i<NUM_DeviceButton; i++ )
	{
		// XXX: yuck
		if( DeviceButtonToString((DeviceButton)i) == s )
			return (DeviceButton)i;
	}
	return DeviceButton_Invalid;
}

	
static const CString InputDeviceNames[] = {
	"Key",
	"Joy1",
	"Joy2",
	"Joy3",
	"Joy4",
	"Joy5",
	"Joy6",
	"Joy7",
	"Joy8",
	"Joy9",
	"Joy10",
	"Joy11",
	"Joy12",
	"Joy13",
	"Joy14",
	"Joy15",
	"Joy16",
	"Pump1",
	"Pump2",
	"Midi",
};
XToString( InputDevice, NUM_INPUT_DEVICES );
StringToX( InputDevice );


// XXX remove
CString DeviceButtonToString( InputDevice device, DeviceButton i )
{
	return DeviceButtonToString( i );
}

DeviceButton StringToDeviceButton( InputDevice device, const CString& s )
{
	return StringToDeviceButton( s );
}

int GetNumDeviceButtons( InputDevice device )
{
	return NUM_DeviceButton;
};

CString DeviceInput::ToString() const
{
	if( device == DEVICE_NONE )
		return CString();

	CString s = InputDeviceToString(device) + "_" + DeviceButtonToString(device,button);
	return s;
}

bool DeviceInput::FromString( const CString &s )
{
	char szDevice[32] = "";
	char szButton[32] = "";

	if( 2 != sscanf( s, "%31[^_]_%31[^_]", szDevice, szButton ) )
	{
		device = DEVICE_NONE;
		return false;
	}

	device = StringToInputDevice( szDevice );
	button = StringToDeviceButton( device, szButton );
	return true;
}


char DeviceInput::ToChar() const
{
	if( button < 127 )
		return (char) button;

	if( button >= KEY_KP_C0 && button <= KEY_KP_C9 )
		return (char) (button - KEY_KP_C0) + '0';

	switch( button )
	{
	case KEY_KP_SLASH: return '/';
	case KEY_KP_ASTERISK: return '*';
	case KEY_KP_HYPHEN: return '-';
	case KEY_KP_PLUS: return '+';
	case KEY_KP_PERIOD: return '.';
	case KEY_KP_EQUAL: return '=';
	}

	return '\0';
}

/*
 * Copyright (c) 2001-2002 Chris Danford
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
