/*
 * Define all of the input devices we know about.  This is the public
 * interface for describing input devices.
 */

#include "global.h"
#include "RageInputDevice.h"
#include "RageUtil.h"

static const char *PumpButtonNames[] = {
	"UL", "UR", "MID", "DL", "DR", "Esc",
	"P2 UL", "P2 UR", "P2 MID", "P2 DL", "P2 DR"
};

int DeviceInput::NumButtons(InputDevice device)
{
	switch( device )
	{
	case DEVICE_KEYBOARD:	
		return NUM_KEYBOARD_BUTTONS;	
	case DEVICE_JOY1:
	case DEVICE_JOY2:
	case DEVICE_JOY3:
	case DEVICE_JOY4:
	case DEVICE_JOY5:
	case DEVICE_JOY6:
		return NUM_JOYSTICK_BUTTONS;	
	case DEVICE_PUMP1:
	case DEVICE_PUMP2:
		return NUM_PUMP_PAD_BUTTONS;	
	default:
		ASSERT(0);	// invalid device
	}
	return -1; /* quiet compiler */
}

CString DeviceInput::GetDescription() 
{
	CString sReturn;

	switch( device )
	{
	case DEVICE_NONE:	// blank
		ASSERT( false );	// called GetDescription on a blank DeviceInput!
		break;

	case DEVICE_JOY1:	// joystick
	case DEVICE_JOY2:
	case DEVICE_JOY3:
	case DEVICE_JOY4:
	case DEVICE_JOY5:
	case DEVICE_JOY6:
		sReturn = ssprintf("Joy%d ", device - DEVICE_JOY1 + 1 );

		switch( button )
		{
		case JOY_LEFT:		sReturn += "Left";		break;
		case JOY_RIGHT:		sReturn += "Right";		break;
		case JOY_UP:		sReturn += "Up";		break;
		case JOY_DOWN:		sReturn += "Down";		break;
		case JOY_LEFT_2:	sReturn += "Left2";		break;
		case JOY_RIGHT_2:	sReturn += "Right2";	break;
		case JOY_UP_2:		sReturn += "Up2";		break;
		case JOY_DOWN_2:	sReturn += "Down2";		break;
		case JOY_Z_UP:		sReturn += "Z-Up";		break;
		case JOY_Z_DOWN:	sReturn += "Z-Down";	break;
		case JOY_ROT_UP:  sReturn += "R-Up";		break;
		case JOY_ROT_DOWN:sReturn += "R-Down";		break;
		case JOY_ROT_LEFT:  sReturn += "R-Left";	break;
		case JOY_ROT_RIGHT:sReturn += "R-Right";	break;
		case JOY_ROT_Z_UP:  sReturn += "ZR-Up";		break;
		case JOY_ROT_Z_DOWN:sReturn += "ZR-Down";	break;
		case JOY_HAT_LEFT:	sReturn += "H-Left";	break;
		case JOY_HAT_RIGHT:	sReturn += "H-Right";	break;
		case JOY_HAT_UP:	sReturn += "H-Up";		break;
		case JOY_HAT_DOWN:	sReturn += "H-Down";	break;
		case JOY_AUX_1:	sReturn += "Aux1";	break;
		case JOY_AUX_2:	sReturn += "Aux2";	break;
		case JOY_AUX_3:	sReturn += "Aux3";	break;
		case JOY_AUX_4:	sReturn += "Aux4";	break;
		case JOY_1:		sReturn += "1";		break;
		case JOY_2:		sReturn += "2";		break;
		case JOY_3:		sReturn += "3";		break;
		case JOY_4:		sReturn += "4";		break;
		case JOY_5:		sReturn += "5";		break;
		case JOY_6:		sReturn += "6";		break;
		case JOY_7:		sReturn += "7";		break;
		case JOY_8:		sReturn += "8";		break;
		case JOY_9:		sReturn += "9";		break;
		case JOY_10:	sReturn += "10";	break;
		case JOY_11:	sReturn += "11";	break;
		case JOY_12:	sReturn += "12";	break;
		case JOY_13:	sReturn += "13";	break;
		case JOY_14:	sReturn += "14";	break;
		case JOY_15:	sReturn += "15";	break;
		case JOY_16:	sReturn += "16";	break;
		case JOY_17:	sReturn += "17";	break;
		case JOY_18:	sReturn += "18";	break;
		case JOY_19:	sReturn += "19";	break;
		case JOY_20:	sReturn += "20";	break;
		case JOY_21:	sReturn += "21";	break;
		case JOY_22:	sReturn += "22";	break;
		case JOY_23:	sReturn += "23";	break;
		case JOY_24:	sReturn += "24";	break;
		case JOY_25:	sReturn += "25";	break;
		case JOY_26:	sReturn += "26";	break;
		case JOY_27:	sReturn += "27";	break;
		case JOY_28:	sReturn += "28";	break;
		case JOY_29:	sReturn += "29";	break;
		case JOY_30:	sReturn += "30";	break;
		case JOY_31:	sReturn += "31";	break;
		case JOY_32:	sReturn += "32";	break;
		}
		
		break;

	case DEVICE_PUMP1:
	case DEVICE_PUMP2:
		/* There is almost always only one Pump device, even if there are
		 * two pads, so only enumerate the second and higher. */
		if(device == DEVICE_PUMP1)
			sReturn = ssprintf("PIU ");
		else
			sReturn = ssprintf("PIU#%d ", device - DEVICE_PUMP1 + 1 );

		if(button < NUM_PUMP_PAD_BUTTONS)
			sReturn += PumpButtonNames[button];
		else 
			/* This can happen if the INI is corrupt and has an invalid
			 * button number.  Crashing with an assertion failure because
			 * we got something unexpected is no good. */
			sReturn += "???";

		break;

	case DEVICE_KEYBOARD:		// keyboard
		sReturn = "Key ";
		if( button >= 33 && button < 127 &&
			!(button >= 'A' && button <= 'Z' ) )
		{
			/* All printable ASCII except for uppercase alpha characters line up. */
			sReturn += ssprintf( "%c", button );
			break;
		}

		if( button >= KEY_OTHER_0 && button < KEY_LAST_OTHER )
		{
			sReturn += ssprintf( "unk %i", button-KEY_OTHER_0 );
			break;
		}

		switch( button )
		{
		case KEY_SPACE:			sReturn += "space"; break;
		case KEY_DEL:			sReturn += "delete"; break;

		case KEY_BACK:			sReturn += "backspace"; break;
		case KEY_TAB:			sReturn += "tab"; break;
		case KEY_ENTER:			sReturn += "enter"; break;
		case KEY_PAUSE:			sReturn += "pause"; break;
		case KEY_ESC:			sReturn += "escape"; break;

		case KEY_F1:			sReturn += "F1"; break;
		case KEY_F2:			sReturn += "F2"; break;
		case KEY_F3:			sReturn += "F3"; break;
		case KEY_F4:			sReturn += "F4"; break;
		case KEY_F5:			sReturn += "F5"; break;
		case KEY_F6:			sReturn += "F6"; break;
		case KEY_F7:			sReturn += "F7"; break;
		case KEY_F8:			sReturn += "F8"; break;
		case KEY_F9:			sReturn += "F9"; break;
		case KEY_F10:			sReturn += "F10"; break;
		case KEY_F11:			sReturn += "F11"; break;
		case KEY_F12:			sReturn += "F12"; break;
		case KEY_F13:			sReturn += "F13"; break;
		case KEY_F14:			sReturn += "F14"; break;
		case KEY_F15:			sReturn += "F15"; break;
		case KEY_F16:			sReturn += "F16"; break;

		case KEY_LCTRL:			sReturn += "left ctrl"; break;
		case KEY_RCTRL:			sReturn += "right ctrl"; break;
		case KEY_LSHIFT:		sReturn += "left shift"; break;
		case KEY_RSHIFT:		sReturn += "right shift"; break;
		case KEY_LALT:			sReturn += "left alt"; break;
		case KEY_RALT:			sReturn += "right alt"; break;
		case KEY_LMETA:			sReturn += "left meta"; break;
		case KEY_RMETA:			sReturn += "right meta"; break;
		case KEY_LSUPER:		sReturn += "left wnd"; break;
		case KEY_RSUPER:		sReturn += "right wnd"; break;
		case KEY_MENU:			sReturn += "menu"; break;

		case KEY_NUMLOCK:		sReturn += "num lock"; break;
		case KEY_SCRLLOCK:		sReturn += "scroll lock"; break;
		case KEY_CAPSLOCK:		sReturn += "caps lock"; break;
		case KEY_PRTSC:			sReturn += "prtsc"; break;

		case KEY_UP:			sReturn += "up"; break;
		case KEY_DOWN:			sReturn += "down"; break;
		case KEY_LEFT:			sReturn += "left"; break;
		case KEY_RIGHT:			sReturn += "right"; break;

		case KEY_INSERT:		sReturn += "insert"; break;
		case KEY_HOME:			sReturn += "home"; break;
		case KEY_END:			sReturn += "end"; break;
		case KEY_PGUP:			sReturn += "pgup"; break;
		case KEY_PGDN:			sReturn += "pgdn"; break;

		case KEY_KP_C0:			sReturn += "KP 0"; break;
		case KEY_KP_C1:			sReturn += "KP 1"; break;
		case KEY_KP_C2:			sReturn += "KP 2"; break;
		case KEY_KP_C3:			sReturn += "KP 3"; break;
		case KEY_KP_C4:			sReturn += "KP 4"; break;
		case KEY_KP_C5:			sReturn += "KP 5"; break;
		case KEY_KP_C6:			sReturn += "KP 6"; break;
		case KEY_KP_C7:			sReturn += "KP 7"; break;
		case KEY_KP_C8:			sReturn += "KP 8"; break;
		case KEY_KP_C9:			sReturn += "KP 9"; break;
		case KEY_KP_SLASH:		sReturn += "KP /"; break;
		case KEY_KP_ASTERISK:	sReturn += "KP *"; break;
		case KEY_KP_HYPHEN:		sReturn += "KP -"; break;
		case KEY_KP_PLUS:		sReturn += "KP +"; break;
		case KEY_KP_PERIOD:		sReturn += "KP ."; break;
		case KEY_KP_EQUAL:		sReturn += "KP ="; break;
		case KEY_KP_ENTER:		sReturn += "KP enter"; break;
		default:				sReturn += "unknown"; break;
		}
		break;
		
	default:
		ASSERT(0);	// what device is this?
	}

	return sReturn;
}

CString DeviceInput::toString() 
{
	if( device == DEVICE_NONE )
		return "";
	else
		return ssprintf("%d-%d", device, button );
}

bool DeviceInput::fromString( const CString &s )
{ 
	CStringArray a;
	split( s, "-", a);

	if( a.size() != 2 ) {
		device = DEVICE_NONE;
		return false;
	}

	device = (InputDevice)atoi( a[0] );
	button = atoi( a[1] );
	return true;
}


char DeviceInput::ToChar() const
{
	switch( device )
	{
	case DEVICE_KEYBOARD:
		if( button < 128 )
			return (char) button;
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
