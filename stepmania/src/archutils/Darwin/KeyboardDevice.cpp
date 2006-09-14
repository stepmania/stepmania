#include "global.h"
#include "KeyboardDevice.h"

using namespace __gnu_cxx;

bool KeyboardDevice::AddLogicalDevice( int usagePage, int usage )
{
	return usagePage == kHIDPage_GenericDesktop && usage == kHIDUsage_GD_Keyboard;
}

static bool UsbKeyToDeviceButton( UInt8 iUsbKey, DeviceButton &buttonOut )
{
	UInt8 usage = iUsbKey;
	
	if( usage < kHIDUsage_KeyboardA )
		return false;
	
	if( usage <= kHIDUsage_KeyboardZ )
	{
		buttonOut = enum_add2( KEY_Ca, usage - kHIDUsage_KeyboardA );
		return true;
	}
	
	// KEY_C0 = KEY_C1 - 1, kHIDUsage_Keyboard0 = kHIDUsage_Keyboard9 + 1
	if( usage <= kHIDUsage_Keyboard9 )
	{
		buttonOut = enum_add2( KEY_C1, usage - kHIDUsage_Keyboard1 );
		return true;
	}
	
	if( usage >= kHIDUsage_KeyboardF1 && usage <= kHIDUsage_KeyboardF12 )
	{
		buttonOut = enum_add2( KEY_F1, usage - kHIDUsage_KeyboardF1 );
		return true;
	}
	
	if( usage >= kHIDUsage_KeyboardF13 && usage <= kHIDUsage_KeyboardF16 )
	{
		buttonOut = enum_add2( KEY_F13, usage - kHIDUsage_KeyboardF13 );
		return true;
	}
	
	// keypad 0 is again backward
	if( usage >= kHIDUsage_Keypad1 && usage <= kHIDUsage_Keypad9 )
	{
		buttonOut = enum_add2( KEY_KP_C1, usage - kHIDUsage_Keypad1 );
		return true;
	}
	
#define OTHER(n) (enum_add2(KEY_OTHER_0, (n)))
	
	// [0, 8]
	if( usage >= kHIDUsage_KeyboardF17 && usage <= kHIDUsage_KeyboardExecute )
	{
		buttonOut = OTHER( 0 + usage - kHIDUsage_KeyboardF17 );
		return true;
	}
	
	// [9, 19]
	if( usage >= kHIDUsage_KeyboardSelect && usage <= kHIDUsage_KeyboardVolumeDown )
	{
		buttonOut = OTHER( 9 + usage - kHIDUsage_KeyboardSelect );
		return true;
	}
	
	// [20, 41]
	if( usage >= kHIDUsage_KeypadEqualSignAS400 && usage <= kHIDUsage_KeyboardCancel )
	{
		buttonOut = OTHER( 20 + usage - kHIDUsage_KeypadEqualSignAS400 );
		return true;
	}
	
	// [42, 47] 
	// XXX kHIDUsage_KeyboardClearOrAgain
	if( usage >= kHIDUsage_KeyboardSeparator && usage <= kHIDUsage_KeyboardExSel )
	{
		buttonOut = OTHER( 32 + usage - kHIDUsage_KeyboardSeparator );
		return true;
	}
	
#define X(x,y) case x: buttonOut = y; return true
	
	// Time for the special cases
	switch( usage )
	{
		X( kHIDUsage_Keyboard0, KEY_C0 );
		X( kHIDUsage_Keypad0, KEY_KP_C0 );
		X( kHIDUsage_KeyboardReturnOrEnter, KEY_ENTER );
		X( kHIDUsage_KeyboardEscape, KEY_ESC );
		X( kHIDUsage_KeyboardDeleteOrBackspace, KEY_BACK );
		X( kHIDUsage_KeyboardTab, KEY_TAB );
		X( kHIDUsage_KeyboardSpacebar, KEY_SPACE );
		X( kHIDUsage_KeyboardHyphen, KEY_HYPHEN );
		X( kHIDUsage_KeyboardEqualSign, KEY_EQUAL );
		X( kHIDUsage_KeyboardOpenBracket, KEY_LBRACKET );
		X( kHIDUsage_KeyboardCloseBracket, KEY_RBRACKET );
		X( kHIDUsage_KeyboardBackslash, KEY_BACKSLASH );
		X( kHIDUsage_KeyboardNonUSPound, KEY_HASH );
		X( kHIDUsage_KeyboardSemicolon, KEY_SEMICOLON );
		X( kHIDUsage_KeyboardQuote, KEY_SQUOTE );
		X( kHIDUsage_KeyboardGraveAccentAndTilde, KEY_ACCENT );
		X( kHIDUsage_KeyboardComma, KEY_COMMA );
		X( kHIDUsage_KeyboardPeriod, KEY_PERIOD );
		X( kHIDUsage_KeyboardSlash, KEY_SLASH );
		X( kHIDUsage_KeyboardCapsLock, KEY_CAPSLOCK );
		X( kHIDUsage_KeyboardPrintScreen, KEY_PRTSC );
		X( kHIDUsage_KeyboardScrollLock, KEY_SCRLLOCK );
		X( kHIDUsage_KeyboardPause, KEY_PAUSE );
		X( kHIDUsage_KeyboardInsert, KEY_INSERT );
		X( kHIDUsage_KeyboardHome, KEY_HOME );
		X( kHIDUsage_KeyboardPageUp, KEY_PGUP );
		X( kHIDUsage_KeyboardDeleteForward, KEY_DEL );
		X( kHIDUsage_KeyboardEnd, KEY_END );
		X( kHIDUsage_KeyboardPageDown, KEY_PGDN );
		X( kHIDUsage_KeyboardRightArrow, KEY_RIGHT );
		X( kHIDUsage_KeyboardLeftArrow, KEY_LEFT );
		X( kHIDUsage_KeyboardDownArrow, KEY_DOWN );
		X( kHIDUsage_KeyboardUpArrow, KEY_UP );
		X( kHIDUsage_KeypadNumLock, KEY_NUMLOCK );
		X( kHIDUsage_KeypadSlash, KEY_KP_SLASH );
		X( kHIDUsage_KeypadEqualSign, KEY_KP_EQUAL );
		X( kHIDUsage_KeypadAsterisk, KEY_KP_ASTERISK );
		X( kHIDUsage_KeypadHyphen, KEY_KP_HYPHEN );
		X( kHIDUsage_KeypadPlus, KEY_KP_PLUS );
		X( kHIDUsage_KeypadEnter, KEY_KP_ENTER );
		X( kHIDUsage_KeypadPeriod, KEY_KP_PERIOD );
		X( kHIDUsage_KeyboardNonUSBackslash, OTHER(48) );
		X( kHIDUsage_KeyboardApplication, OTHER(49) );
		X( kHIDUsage_KeyboardClear, KEY_NUMLOCK ); // XXX
		X( kHIDUsage_KeyboardHelp, KEY_INSERT );
		X( kHIDUsage_KeyboardMenu, KEY_MENU );
		// XXX kHIDUsage_KeyboardLockingCapsLock
		// XXX kHIDUsage_KeyboardLockingNumLock
		// XXX kHIDUsage_KeyboardLockingScrollLock
		X( kHIDUsage_KeypadComma, KEY_KP_PERIOD ); // XXX
		X( kHIDUsage_KeyboardReturn, KEY_ENTER );
		X( kHIDUsage_KeyboardPrior, OTHER(50) );
		X( kHIDUsage_KeyboardLeftControl, KEY_LCTRL );
		X( kHIDUsage_KeyboardLeftShift, KEY_LSHIFT );
		X( kHIDUsage_KeyboardLeftAlt, KEY_LALT );
		X( kHIDUsage_KeyboardLeftGUI, KEY_LMETA );
		X( kHIDUsage_KeyboardRightControl, KEY_RCTRL );
		X( kHIDUsage_KeyboardRightShift, KEY_RSHIFT );
		X( kHIDUsage_KeyboardRightAlt, KEY_RALT );
		X( kHIDUsage_KeyboardRightGUI, KEY_RMETA );
	}
#undef X
#undef OTHER
	
	return false;
}

void KeyboardDevice::AddElement( int usagePage, int usage, int cookie, const CFDictionaryRef properties )
{
	if( usagePage != kHIDPage_KeyboardOrKeypad )
		return;
	
	DeviceButton button;
	if( UsbKeyToDeviceButton(usage,button) )
		m_Mapping[cookie] = button;
}

void KeyboardDevice::Open()
{
	for( hash_map<int,DeviceButton>::const_iterator i = m_Mapping.begin(); i != m_Mapping.end(); ++i )
		AddElementToQueue( i->first );
}

void KeyboardDevice::GetButtonPresses( vector<pair<DeviceInput, bool> >& vPresses, int cookie,
				       int value, const RageTimer& now ) const
{
	hash_map<int, DeviceButton>::const_iterator iter = m_Mapping.find( cookie );
	
	if( iter != m_Mapping.end() )
		vPresses.push_back( pair<DeviceInput, bool>(DeviceInput(DEVICE_KEYBOARD, iter->second, value, now), value) );
	
}
	
void KeyboardDevice::GetDevicesAndDescriptions( vector<InputDeviceInfo>& vDevices ) const
{
	if( vDevices.size() && vDevices[0].id == DEVICE_KEYBOARD )
		return;
	vDevices.insert( vDevices.begin(), InputDeviceInfo(DEVICE_KEYBOARD, "Keyboard") );
}


// http://lists.apple.com/archives/carbon-dev/2005/Feb/msg00071.html
// index represents USB keyboard usage value, content is Mac virtual keycode
static UInt8 g_iUsbKeyToMacVirtualKey[256] = 
{  
	0xFF, 	/* 00 no event */		
	0xFF,	/* 01 ErrorRollOver */	
	0xFF,	/* 02 POSTFail */	
	0xFF,	/* 03 ErrorUndefined */	
	0x00,	/* 04 A */
	0x0B,	/* 05 B */
	0x08,	/* 06 C */
	0x02,	/* 07 D */
	0x0E,	/* 08 E */
	0x03,	/* 09 F */
	0x05,	/* 0A G */
	0x04,	/* 0B H */
	0x22,	/* 0C I */
	0x26,	/* 0D J */
	0x28,	/* 0E K */
	0x25,	/* 0F L */

	0x2E, 	/* 10 M */		
	0x2D,	/* 11 N */	
	0x1F,	/* 12 O */	
	0x23,	/* 13 P */	
	0x0C,	/* 14 Q */
	0x0F,	/* 15 R */
	0x01,	/* 16 S */
	0x11,	/* 17 T */
	0x20,	/* 18 U */
	0x09,	/* 19 V */
	0x0D,	/* 1A W */
	0x07,	/* 1B X */
	0x10,	/* 1C Y */
	0x06,	/* 1D Z */
	0x12,	/* 1E 1/! */
	0x13,	/* 1F 2/@ */

	0x14, 	/* 20 3 # */		
	0x15,	/* 21 4 $ */	
	0x17,	/* 22 5 % */	
	0x16,	/* 23 6 ^ */	
	0x1A,	/* 24 7 & */
	0x1C,	/* 25 8 * */
	0x19,	/* 26 9 ( */
	0x1D,	/* 27 0 ) */
	0x24,	/* 28 Return (Enter) */
	0x35,	/* 29 ESC */
	0x33,	/* 2A Delete (Backspace) */
	0x30,	/* 2B Tab */
	0x31,	/* 2C Spacebar */
	0x1B,	/* 2D - _ */
	0x18,	/* 2E = + */
	0x21,	/* 2F [ { */

	0x1E, 	/* 30 ] } */		
	0x2A,	/* 31 \ | */	
	0xFF,	/* 32 Non-US # and ~ (what?!!!) */	
	0x29,	/* 33 ; : */	
	0x27,	/* 34 ' " */
	0x32,	/* 35 ` ~ */
	0x2B,	/* 36 , < */
	0x2F,	/* 37 . > */
	0x2C,	/* 38 / ? */
	0x39,	/* 39 Caps Lock */
	0x7A,	/* 3A F1 */
	0x78,	/* 3B F2 */
	0x63,	/* 3C F3 */
	0x76,	/* 3D F4 */
	0x60,	/* 3E F5 */
	0x61,	/* 3F F6 */

	0x62, 	/* 40 F7 */		
	0x64,	/* 41 F8 */	
	0x65,	/* 42 F9 */	
	0x6D,	/* 43 F10 */	
	0x67,	/* 44 F11 */
	0x6F,	/* 45 F12 */
	0x69,	/* 46 F13/PrintScreen */
	0x6B,	/* 47 F14/ScrollLock */
	0x71,	/* 48 F15/Pause */				
	0x72,	/* 49 Insert */
	0x73,	/* 4A Home */
	0x74,	/* 4B PageUp */
	0x75,	/* 4C Delete Forward */
	0x77,	/* 4D End */
	0x79,	/* 4E PageDown */
	0x7C,	/* 4F RightArrow */

	0x7B, 	/* 50 LeftArrow */		
	0x7D,	/* 51 DownArrow */	
	0x7E,	/* 52 UpArrow */	
	0x47,	/* 53 NumLock/Clear */	
	0x4B,	/* 54 Keypad / */
	0x43,	/* 55 Keypad * */
	0x4E,	/* 56 Keypad - */
	0x45,	/* 57 Keypad + */
	0x4C,	/* 58 Keypad Enter */
	0x53,	/* 59 Keypad 1 */
	0x54,	/* 5A Keypad 2 */
	0x55,	/* 5B Keypad 3 */
	0x56,	/* 5C Keypad 4 */
	0x57,	/* 5D Keypad 5 */
	0x58,	/* 5E Keypad 6 */
	0x59,	/* 5F Keypad 7 */

	0x5B, 	/* 60 Keypad 8 */		
	0x5C,	/* 61 Keypad 9 */	
	0x52,	/* 62 Keypad 0 */	
	0x41,	/* 63 Keypad . */	
	0xFF,	/* 64 Non-US \ and  | (what ??!!) */
	0x6E,	/* 65 ApplicationKey (not on a mac!)*/
	0x7F,	/* 66 PowerKey  */
	0x51,	/* 67 Keypad = */
	0x69,	/* 68 F13 */
	0x6B,	/* 69 F14 */
	0x71,	/* 6A F15 */
	0xFF,	/* 6B F16 */
	0xFF,	/* 6C F17 */
	0xFF,	/* 6D F18 */
	0xFF,	/* 6E F19 */
	0xFF,	/* 6F F20 */

	0x5B, 	/* 70 F21 */		
	0x5C,	/* 71 F22 */	
	0x52,	/* 72 F23 */	
	0x41,	/* 73 F24 */	
	0xFF,	/* 74 Execute */
	0xFF,	/* 75 Help */
	0x7F,	/* 76 Menu */
	0x4C,	/* 77 Select */
	0x69,	/* 78 Stop */
	0x6B,	/* 79 Again */
	0x71,	/* 7A Undo */
	0xFF,	/* 7B Cut */
	0xFF,	/* 7C Copy */
	0xFF,	/* 7D Paste */
	0xFF,	/* 7E Find */
	0xFF,	/* 7F Mute */
	
	0xFF, 	/* 80 no event */		
	0xFF,	/* 81 no event */	
	0xFF,	/* 82 no event */	
	0xFF,	/* 83 no event */	
	0xFF,	/* 84 no event */
	0xFF,	/* 85 no event */
	0xFF,	/* 86 no event */
	0xFF,	/* 87 no event */
	0xFF,	/* 88 no event */
	0xFF,	/* 89 no event */
	0xFF,	/* 8A no event */
	0xFF,	/* 8B no event */
	0xFF,	/* 8C no event */
	0xFF,	/* 8D no event */
	0xFF,	/* 8E no event */
	0xFF,	/* 8F no event */

	0xFF, 	/* 90 no event */		
	0xFF,	/* 91 no event */	
	0xFF,	/* 92 no event */	
	0xFF,	/* 93 no event */	
	0xFF,	/* 94 no event */
	0xFF,	/* 95 no event */
	0xFF,	/* 96 no event */
	0xFF,	/* 97 no event */
	0xFF,	/* 98 no event */
	0xFF,	/* 99 no event */
	0xFF,	/* 9A no event */
	0xFF,	/* 9B no event */
	0xFF,	/* 9C no event */
	0xFF,	/* 9D no event */
	0xFF,	/* 9E no event */
	0xFF,	/* 9F no event */

	0xFF, 	/* A0 no event */		
	0xFF,	/* A1 no event */	
	0xFF,	/* A2 no event */	
	0xFF,	/* A3 no event */	
	0xFF,	/* A4 no event */
	0xFF,	/* A5 no event */
	0xFF,	/* A6 no event */
	0xFF,	/* A7 no event */
	0xFF,	/* A8 no event */
	0xFF,	/* A9 no event */
	0xFF,	/* AA no event */
	0xFF,	/* AB no event */
	0xFF,	/* AC no event */
	0xFF,	/* AD no event */
	0xFF,	/* AE no event */
	0xFF,	/* AF no event */

	0xFF, 	/* B0 no event */		
	0xFF,	/* B1 no event */	
	0xFF,	/* B2 no event */	
	0xFF,	/* B3 no event */	
	0xFF,	/* B4 no event */
	0xFF,	/* B5 no event */
	0xFF,	/* B6 no event */
	0xFF,	/* B7 no event */
	0xFF,	/* B8 no event */
	0xFF,	/* B9 no event */
	0xFF,	/* BA no event */
	0xFF,	/* BB no event */
	0xFF,	/* BC no event */
	0xFF,	/* BD no event */
	0xFF,	/* BE no event */
	0xFF,	/* BF no event */

	0xFF, 	/* C0 no event */		
	0xFF,	/* C1 no event */	
	0xFF,	/* C2 no event */	
	0xFF,	/* C3 no event */	
	0xFF,	/* C4 no event */
	0xFF,	/* C5 no event */
	0xFF,	/* C6 no event */
	0xFF,	/* C7 no event */
	0xFF,	/* C8 no event */
	0xFF,	/* C9 no event */
	0xFF,	/* CA no event */
	0xFF,	/* CB no event */
	0xFF,	/* CC no event */
	0xFF,	/* CD no event */
	0xFF,	/* CE no event */
	0xFF,	/* CF no event */

	0xFF, 	/* D0 no event */		
	0xFF,	/* D1 no event */	
	0xFF,	/* D2 no event */	
	0xFF,	/* D3 no event */	
	0xFF,	/* D4 no event */
	0xFF,	/* D5 no event */
	0xFF,	/* D6 no event */
	0xFF,	/* D7 no event */
	0xFF,	/* D8 no event */
	0xFF,	/* D9 no event */
	0xFF,	/* DA no event */
	0xFF,	/* DB no event */
	0xFF,	/* DC no event */
	0xFF,	/* DD no event */
	0xFF,	/* DE no event */
	0xFF,	/* DF no event */

	0x3B, 	/* E0 left control key */		
	0x38,	/* E1 left shift key key */	
	0x3A,	/* E2 left alt/option key */	
	0x37,	/* E3 left GUI (windows/cmd) key */	
	
	0x3B,	/* E4 right control key */ 
	0x38,	/* E5 right shift key key */ 
	0x3A,	/* E6 right alt/option key */ 
	0x37,	/* E7 right GUI (windows/cmd) key */
	0xFF,	/* E8 no event */
	0xFF,	/* E9 no event */
	0xFF,	/* EA no event */
	0xFF,	/* EB no event */
	0xFF,	/* EC no event */
	0xFF,	/* ED no event */
	0xFF,	/* EE no event */
	0xFF,	/* EF no event */
	
	0xFF, 	/* F0 no event */		
	0xFF,	/* F1 no event */	
	0xFF,	/* F2 no event */	
	0xFF,	/* F3 no event */	
	0xFF,	/* F4 no event */
	0xFF,	/* F5 no event */
	0xFF,	/* F6 no event */
	0xFF,	/* F7 no event */
	0xFF,	/* F8 no event */
	0xFF,	/* F9 no event */
	0xFF,	/* FA no event */
	0xFF,	/* FB no event */
	0xFF,	/* FC no event */
	0xFF,	/* FD no event */
	0xFF,	/* FE no event */
	0xFF,	/* FF no event */
};

static UInt8 g_iDeviceButtonToMacVirtualKey[KEY_OTHER_0];

bool KeyboardDevice::DeviceButtonToMacVirtualKey( DeviceButton button, UInt8 &iMacVKOut )
{
	static bool bInited = false;
	
	if( !bInited )
	{
		memset( g_iDeviceButtonToMacVirtualKey, 0xFF, sizeof(g_iDeviceButtonToMacVirtualKey) );
		for( int iUsbKey = 0; iUsbKey < 256; ++iUsbKey )
		{
			DeviceButton button2;
			if( UsbKeyToDeviceButton(iUsbKey, button2) && size_t(button2) < sizeof(g_iDeviceButtonToMacVirtualKey) )
				g_iDeviceButtonToMacVirtualKey[button2] = g_iUsbKeyToMacVirtualKey[iUsbKey];
		}
		bInited = true;
	}
	iMacVKOut = g_iDeviceButtonToMacVirtualKey[button];
	return iMacVKOut != 0xFF;
}

		
/*
 * (c) 2005-2006 Steve Checkoway
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
