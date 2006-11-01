/*
 * Define all of the input devices we know about.  This is the public
 * interface for describing input devices.
 */

#include "global.h"
#include "RageInputDevice.h"
#include "RageUtil.h"
#include "Foreach.h"

static map<DeviceButton,RString> g_mapNamesToString;
static map<RString,DeviceButton> g_mapStringToNames;
static void InitNames()
{
	if( !g_mapNamesToString.empty() )
		return;

	g_mapNamesToString[KEY_PERIOD] = "period"; 
	g_mapNamesToString[KEY_COMMA] = "comma"; 
	g_mapNamesToString[KEY_SPACE] = "space"; 
	g_mapNamesToString[KEY_DEL] = "delete"; 

	g_mapNamesToString[KEY_BACK] = "backspace"; 
	g_mapNamesToString[KEY_TAB] = "tab"; 
	g_mapNamesToString[KEY_ENTER] = "enter"; 
	g_mapNamesToString[KEY_PAUSE] = "pause"; 
	g_mapNamesToString[KEY_ESC] = "escape"; 

	g_mapNamesToString[KEY_F1] = "F1"; 
	g_mapNamesToString[KEY_F2] = "F2"; 
	g_mapNamesToString[KEY_F3] = "F3"; 
	g_mapNamesToString[KEY_F4] = "F4"; 
	g_mapNamesToString[KEY_F5] = "F5"; 
	g_mapNamesToString[KEY_F6] = "F6"; 
	g_mapNamesToString[KEY_F7] = "F7"; 
	g_mapNamesToString[KEY_F8] = "F8"; 
	g_mapNamesToString[KEY_F9] = "F9"; 
	g_mapNamesToString[KEY_F10] = "F10"; 
	g_mapNamesToString[KEY_F11] = "F11"; 
	g_mapNamesToString[KEY_F12] = "F12"; 
	g_mapNamesToString[KEY_F13] = "F13"; 
	g_mapNamesToString[KEY_F14] = "F14"; 
	g_mapNamesToString[KEY_F15] = "F15"; 
	g_mapNamesToString[KEY_F16] = "F16"; 

	g_mapNamesToString[KEY_LCTRL] = "left ctrl"; 
	g_mapNamesToString[KEY_RCTRL] = "right ctrl"; 
	g_mapNamesToString[KEY_LSHIFT] = "left shift"; 
	g_mapNamesToString[KEY_RSHIFT] = "right shift"; 
	g_mapNamesToString[KEY_LALT] = "left alt"; 
	g_mapNamesToString[KEY_RALT] = "right alt"; 
	g_mapNamesToString[KEY_LMETA] = "left meta"; 
	g_mapNamesToString[KEY_RMETA] = "right meta"; 
	g_mapNamesToString[KEY_LSUPER] = "left wnd"; 
	g_mapNamesToString[KEY_RSUPER] = "right wnd"; 
	g_mapNamesToString[KEY_MENU] = "menu"; 

	g_mapNamesToString[KEY_NUMLOCK] = "num lock"; 
	g_mapNamesToString[KEY_SCRLLOCK] = "scroll lock"; 
	g_mapNamesToString[KEY_CAPSLOCK] = "caps lock"; 
	g_mapNamesToString[KEY_PRTSC] = "prtsc"; 

	g_mapNamesToString[KEY_UP] = "up"; 
	g_mapNamesToString[KEY_DOWN] = "down"; 
	g_mapNamesToString[KEY_LEFT] = "left"; 
	g_mapNamesToString[KEY_RIGHT] = "right"; 

	g_mapNamesToString[KEY_INSERT] = "insert"; 
	g_mapNamesToString[KEY_HOME] = "home"; 
	g_mapNamesToString[KEY_END] = "end"; 
	g_mapNamesToString[KEY_PGUP] = "pgup"; 
	g_mapNamesToString[KEY_PGDN] = "pgdn"; 

	g_mapNamesToString[KEY_KP_C0] = "KP 0"; 
	g_mapNamesToString[KEY_KP_C1] = "KP 1"; 
	g_mapNamesToString[KEY_KP_C2] = "KP 2"; 
	g_mapNamesToString[KEY_KP_C3] = "KP 3"; 
	g_mapNamesToString[KEY_KP_C4] = "KP 4"; 
	g_mapNamesToString[KEY_KP_C5] = "KP 5"; 
	g_mapNamesToString[KEY_KP_C6] = "KP 6"; 
	g_mapNamesToString[KEY_KP_C7] = "KP 7"; 
	g_mapNamesToString[KEY_KP_C8] = "KP 8"; 
	g_mapNamesToString[KEY_KP_C9] = "KP 9"; 
	g_mapNamesToString[KEY_KP_SLASH] = "KP /"; 
	g_mapNamesToString[KEY_KP_ASTERISK] = "KP *"; 
	g_mapNamesToString[KEY_KP_HYPHEN] = "KP -"; 
	g_mapNamesToString[KEY_KP_PLUS] = "KP +"; 
	g_mapNamesToString[KEY_KP_PERIOD] = "KP ."; 
	g_mapNamesToString[KEY_KP_EQUAL] = "KP ="; 
	g_mapNamesToString[KEY_KP_ENTER] = "KP enter"; 

	g_mapNamesToString[JOY_LEFT] = "Left1";
	g_mapNamesToString[JOY_RIGHT] = "Right1";
	g_mapNamesToString[JOY_UP] = "Up1";
	g_mapNamesToString[JOY_DOWN] = "Down1";

	/* Secondary sticks: */
	g_mapNamesToString[JOY_LEFT_2] = "Left2";
	g_mapNamesToString[JOY_RIGHT_2] = "Right2";
	g_mapNamesToString[JOY_UP_2] = "Up2";
	g_mapNamesToString[JOY_DOWN_2] = "Down2";


	g_mapNamesToString[JOY_Z_UP] = "Z-Up";
	g_mapNamesToString[JOY_Z_DOWN] = "Z-Down";
	g_mapNamesToString[JOY_ROT_UP] = "R-Up";
	g_mapNamesToString[JOY_ROT_DOWN] = "R-Down";
	g_mapNamesToString[JOY_ROT_LEFT] = "R-Left";
	g_mapNamesToString[JOY_ROT_RIGHT] = "R-Right";
	g_mapNamesToString[JOY_ROT_Z_UP] = "ZR-Up";
	g_mapNamesToString[JOY_ROT_Z_DOWN] = "ZR-Down";
	g_mapNamesToString[JOY_HAT_LEFT] = "H-Left";
	g_mapNamesToString[JOY_HAT_RIGHT] = "H-Right";
	g_mapNamesToString[JOY_HAT_UP] = "H-Up";
	g_mapNamesToString[JOY_HAT_DOWN] = "H-Down";
	g_mapNamesToString[JOY_AUX_1] = "Aux1";
	g_mapNamesToString[JOY_AUX_2] = "Aux2";
	g_mapNamesToString[JOY_AUX_3] = "Aux3";
	g_mapNamesToString[JOY_AUX_4] = "Aux4";
	FOREACHM( DeviceButton, RString, g_mapNamesToString, m )
		g_mapStringToNames[m->second] = m->first;
}

/* Return a reversible representation of a DeviceButton.  This is not affected by InputDrivers,
 * localization or the keyboard language. */
RString DeviceButtonToString( DeviceButton key )
{
	InitNames();

	/* All printable ASCII except for uppercase alpha characters line up. */
	if( key >= 33 && key < 127 &&
		!(key >= 'A' && key <= 'Z' ) )
		return ssprintf( "%c", key );

	if( key >= KEY_OTHER_0 && key < KEY_LAST_OTHER )
		return ssprintf( "unk %i", key-KEY_OTHER_0 );

	if( key >= JOY_BUTTON_1 && key <= JOY_BUTTON_32 )
		return ssprintf( "B%i", key-JOY_BUTTON_1+1 );

	if( key >= MIDI_FIRST && key <= MIDI_LAST )
		return ssprintf( "Midi %d", key-MIDI_FIRST );

	map<DeviceButton,RString>::const_iterator it = g_mapNamesToString.find( key );
	if( it != g_mapNamesToString.end() )
		return it->second;

	return "unknown";
}

DeviceButton StringToDeviceButton( const RString& s )
{
	InitNames();

	if( s.size() == 1 )
		return (DeviceButton) s[0];

	int i;
	if( sscanf(s, "unk %i", &i) == 1 )
		return enum_add2( KEY_OTHER_0, i );

	if( sscanf(s, "B%i", &i) == 1 )
		return enum_add2( JOY_BUTTON_1, i-1 );

	if( sscanf(s, "Midi %i", &i) == 1 )
		return enum_add2( MIDI_FIRST, i );

	map<RString,DeviceButton>::const_iterator it = g_mapStringToNames.find( s );
	if( it != g_mapStringToNames.end() )
		return it->second;

	return DeviceButton_Invalid;
}

	
static const char *InputDeviceNames[] = {
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
	"Joy17",
	"Joy18",
	"Joy19",
	"Joy20",
	"Joy21",
	"Joy22",
	"Joy23",
	"Joy24",
	"Joy25",
	"Joy26",
	"Joy27",
	"Joy28",
	"Joy29",
	"Joy30",
	"Joy31",
	"Joy32",
	"Pump1",
	"Pump2",
	"Midi",
};
XToString( InputDevice );
StringToX( InputDevice );

/* Return a reversible representation of a DeviceInput.  This is not affected by InputDrivers,
 * localization or the keyboard language. */
RString DeviceInput::ToString() const
{
	if( device == InputDevice_Invalid )
		return RString();

	RString s = InputDeviceToString(device) + "_" + DeviceButtonToString(button);
	return s;
}

bool DeviceInput::FromString( const RString &s )
{
	char szDevice[32] = "";
	char szButton[32] = "";

	if( 2 != sscanf( s, "%31[^_]_%31[^_]", szDevice, szButton ) )
	{
		device = InputDevice_Invalid;
		return false;
	}

	device = StringToInputDevice( szDevice );
	button = StringToDeviceButton( szButton );
	return true;
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
