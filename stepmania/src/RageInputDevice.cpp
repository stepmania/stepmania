/*
 * Define all of the input devices we know about.
 */

#include "stdafx.h"
#include "RageInputDevice.h"
#include "RageUtil.h"

#include "SDL_keyboard.h"

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
		sReturn = ssprintf("Joy%d ", device - DEVICE_JOY1 + 1 );

		switch( button )
		{
		case JOY_LEFT:		sReturn += "Left";		break;
		case JOY_RIGHT:		sReturn += "Right";		break;
		case JOY_UP:		sReturn += "Up";		break;
		case JOY_DOWN:		sReturn += "Down";		break;
		case JOY_Z_UP:		sReturn += "Z-Up";		break;
		case JOY_Z_DOWN:	sReturn += "Z-Down";	break;
		case JOY_Z_ROT_UP:  sReturn += "ZR-Up";		break;
		case JOY_Z_ROT_DOWN:sReturn += "ZR-Down";	break;
		case JOY_HAT_LEFT:	sReturn += "H-Left";	break;
		case JOY_HAT_RIGHT:	sReturn += "H-Right";	break;
		case JOY_HAT_UP:	sReturn += "H-Up";		break;
		case JOY_HAT_DOWN:	sReturn += "H-Down";	break;
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
		sReturn = ssprintf("Key %s", SDL_GetKeyName((SDLKey)button) );
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
		/* XXX: SDLK_WORLD_* are for international keyboards; we can handle those,
		 * now, by mapping it to Unicode.  However, I can't find any documentation
		 * on those keysyms. */
		return '\0';
	default:
		return '\0';
	}
}

/*
 * Copyright (c) 2001-2002 by the person(s) listed below.  All rights reserved.
 *	Chris Danford
 */
