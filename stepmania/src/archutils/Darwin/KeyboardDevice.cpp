#include "global.h"
#include "KeyboardDevice.h"

using namespace __gnu_cxx;

bool KeyboardDevice::AddLogicalDevice( int usagePage, int usage )
{
	return usagePage == kHIDPage_GenericDesktop && usage == kHIDUsage_GD_Keyboard;
}

void KeyboardDevice::AddElement( int usagePage, int usage, int cookie, const CFDictionaryRef dict )
{
	if( usagePage != kHIDPage_KeyboardOrKeypad )
		return;
	
	if( usage < kHIDUsage_KeyboardA )
		return;
	
	if( usage <= kHIDUsage_KeyboardZ )
	{
		m_Mapping[cookie] = enum_add2( KEY_Ca, usage - kHIDUsage_KeyboardA );
		return;
	}
	
	// KEY_C0 = KEY_C1 - 1, kHIDUsage_Keyboard0 = kHIDUsage_Keyboard9 + 1
	if( usage <= kHIDUsage_Keyboard9 )
	{
		m_Mapping[cookie] = enum_add2( KEY_C1, usage - kHIDUsage_Keyboard1 );
		return;
	}
	
	if( usage >= kHIDUsage_KeyboardF1 && usage <= kHIDUsage_KeyboardF12 )
	{
		m_Mapping[cookie] = enum_add2( KEY_F1, usage - kHIDUsage_KeyboardF1 );
		return;
	}
	
	if( usage >= kHIDUsage_KeyboardF13 && usage <= kHIDUsage_KeyboardF16 )
	{
		m_Mapping[cookie] = enum_add2( KEY_F13, usage - kHIDUsage_KeyboardF13 );
		return;
	}
	
	// keypad 0 is again backward
	if( usage >= kHIDUsage_Keypad1 && usage <= kHIDUsage_Keypad9 )
	{
		m_Mapping[cookie] = enum_add2( KEY_KP_C1, usage - kHIDUsage_Keypad1 );
		return;
	}
	
#define OTHER(n) (enum_add2(KEY_OTHER_0, (n)))
	
	// [0, 8]
	if( usage >= kHIDUsage_KeyboardF17 && usage <= kHIDUsage_KeyboardExecute )
	{
		m_Mapping[cookie] = OTHER( 0 + usage - kHIDUsage_KeyboardF17 );
		return;
	}
	
	// [9, 19]
	if( usage >= kHIDUsage_KeyboardSelect && usage <= kHIDUsage_KeyboardVolumeDown )
	{
		m_Mapping[cookie] = OTHER( 9 + usage - kHIDUsage_KeyboardSelect );
		return;
	}
	
	// [20, 31]
	if( usage >= kHIDUsage_KeypadEqualSignAS400 && usage <= kHIDUsage_KeyboardCancel )
	{
		m_Mapping[cookie] = OTHER( 20 + usage - kHIDUsage_KeypadEqualSignAS400 );
		return;
	}
	
	// [32, 37] 
	// XXX kHIDUsage_KeyboardClearOrAgain
	if( usage >= kHIDUsage_KeyboardSeparator && usage <= kHIDUsage_KeyboardExSel )
	{
		m_Mapping[cookie] = OTHER( 32 + usage - kHIDUsage_KeyboardSeparator );
		return;
	}
	
#define X(x,y) case x: m_Mapping[cookie] = y; return
	
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
		X( kHIDUsage_KeyboardPause, OTHER(38) );
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
		X( kHIDUsage_KeyboardNonUSBackslash, OTHER(39) );
		X( kHIDUsage_KeyboardApplication, OTHER(40) );
		X( kHIDUsage_KeyboardClear, KEY_NUMLOCK ); // XXX
		X( kHIDUsage_KeyboardHelp, KEY_INSERT );
		X( kHIDUsage_KeyboardMenu, KEY_MENU );
		// XXX kHIDUsage_KeyboardLockingCapsLock
		// XXX kHIDUsage_KeyboardLockingNumLock
		// XXX kHIDUsage_KeyboardLockingScrollLock
		X( kHIDUsage_KeypadComma, KEY_KP_PERIOD ); // XXX
		X( kHIDUsage_KeyboardReturn, KEY_ENTER );
		X( kHIDUsage_KeyboardPrior, OTHER(41) );
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
	
void KeyboardDevice::GetDevicesAndDescriptions( vector<InputDevice>& dev, vector<RString>& desc ) const
{
	if( dev.size() && dev[0] == DEVICE_KEYBOARD )
		return;
	dev.insert( dev.begin(), DEVICE_KEYBOARD );
	desc.insert( desc.begin(), "Keyboard" );
}
		

