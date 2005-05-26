/*
 * Define all of the input devices we know about.  This is the public
 * interface for describing input devices.
 */

#include "global.h"
#include "RageInputDevice.h"
#include "RageUtil.h"
#include "EnumHelper.h"


CString RageKeySymToString( RageKeySym key )
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
	default:				return "unknown";
	}
}

RageKeySym StringToRageKeySym( const CString& s )
{
	for( int i=0; i<NUM_KEYS; i++ )
	{
		if( RageKeySymToString((RageKeySym)i) == s )
			return (RageKeySym)i;
	}
	return KEY_INVALID;
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
	"Para1",
};
XToString( InputDevice, NUM_INPUT_DEVICES );
StringToX( InputDevice );


static const CString JoystickButtonNames[] = {
	"Left",
	"Right",
	"Up",
	"Down",
	"Left2",
	"Right2",	
	"Up2",		
	"Down2",		
	"Z-Up",		
	"Z-Down",	
	"R-Up",		
	"R-Down",	
	"R-Left",	
	"R-Right",	
	"ZR-Up",		
	"ZR-Down",	
	"H-Left",	
	"H-Right",	
	"H-Up",		
	"H-Down",	
	"Aux1",		
	"Aux2",	
	"Aux3",	
	"Aux4",	
	"1",		
	"2",		
	"3",		
	"4",		
	"5",		
	"6",		
	"7",		
	"8",		
	"9",		
	"10",	
	"11",	
	"12",	
	"13",	
	"14",	
	"15",	
	"16",	
	"17",	
	"18",	
	"19",	
	"20",	
	"21",	
	"22",	
	"23",	
	"24",	
	"25",	
	"26",	
	"27",	
	"28",	
	"29",	
	"30",	
	"31",	
	"32",
};
XToString( JoystickButton, NUM_JOYSTICK_BUTTONS );
StringToX( JoystickButton );
	
	
static const CString PumpPadButtonNames[] = {
	"UL",
	"UR",
	"MID",
	"DL",
	"DR",
	"Esc",
	"P2 UL",
	"P2 UR",
	"P2 MID",
	"P2 DL",
	"P2 DR"
};
XToString( PumpPadButton, NUM_PUMP_PAD_BUTTONS );
StringToX( PumpPadButton );


static const CString ParaPadButtonNames[] = {
	"L",
	"UL",
	"U",
	"UR",
	"R",
	"SELECT",
	"START",
	"MENU_LEFT",
	"MENU_RIGHT",
};
XToString( ParaPadButton, NUM_PARA_PAD_BUTTONS );
StringToX( ParaPadButton );


CString MidiButtonToString( DeviceButton i )
{
	return ssprintf( "Midi %d", i );
}

DeviceButton StringToMidiButton( const CString &s )
{
	DeviceButton ret;
	sscanf( s, "Midi %d", &ret );
	return ret;
}

CString DeviceButtonToString( InputDevice device, DeviceButton i )
{
	switch( device )
	{
	case DEVICE_KEYBOARD:	return RageKeySymToString( (RageKeySym)i );
	case DEVICE_JOY1:
	case DEVICE_JOY2:
	case DEVICE_JOY3:
	case DEVICE_JOY4:
	case DEVICE_JOY5:
	case DEVICE_JOY6:
	case DEVICE_JOY7:
	case DEVICE_JOY8:
	case DEVICE_JOY9:
	case DEVICE_JOY10:
	case DEVICE_JOY11:
	case DEVICE_JOY12:
	case DEVICE_JOY13:
	case DEVICE_JOY14:
	case DEVICE_JOY15:
	case DEVICE_JOY16:		return JoystickButtonToString( (JoystickButton)i );
	case DEVICE_PUMP1:
	case DEVICE_PUMP2:		return PumpPadButtonToString( (PumpPadButton)i );
	case DEVICE_MIDI:		return MidiButtonToString( i );
	case DEVICE_PARA1:		return ParaPadButtonToString( (ParaPadButton)i );
	case DEVICE_NONE:		return "";
	default:	ASSERT(0);	return "";
	}
}

DeviceButton StringToDeviceButton( InputDevice device, const CString& s )
{
	switch( device )
	{
	case DEVICE_KEYBOARD:	return (DeviceButton)StringToRageKeySym( s );
	case DEVICE_JOY1:
	case DEVICE_JOY2:
	case DEVICE_JOY3:
	case DEVICE_JOY4:
	case DEVICE_JOY5:
	case DEVICE_JOY6:
	case DEVICE_JOY7:
	case DEVICE_JOY8:
	case DEVICE_JOY9:
	case DEVICE_JOY10:
	case DEVICE_JOY11:
	case DEVICE_JOY12:
	case DEVICE_JOY13:
	case DEVICE_JOY14:
	case DEVICE_JOY15:
	case DEVICE_JOY16:		return (DeviceButton)StringToJoystickButton( s );
	case DEVICE_PUMP1:
	case DEVICE_PUMP2:		return (DeviceButton)StringToPumpPadButton( s );
	case DEVICE_MIDI:		return (DeviceButton)StringToMidiButton( s );
	case DEVICE_PARA1:		return (DeviceButton)StringToParaPadButton( s );
	case DEVICE_NONE:		return DEVICE_BUTTON_INVALID;
	default:	ASSERT(0);	return DEVICE_BUTTON_INVALID;
	}
}

CString DeviceInput::toString() 
{
	if( device == DEVICE_NONE )
		return "";

	CString s = InputDeviceToString(device) + "_" + DeviceButtonToString(device,button);
	return s;
}

bool DeviceInput::fromString( const CString &s )
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
	switch( device )
	{
	case DEVICE_KEYBOARD:
		if( button < 128 )
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
	default:
		return '\0';
	}
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
