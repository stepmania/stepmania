#include "global.h"
#include "InputFilter.h"
#include "RageUtil.h"
#include "InputHandler.h"
#include "RageLog.h"
#include "LocalizedString.h"

void InputHandler::UpdateTimer()
{
	m_LastUpdate.Touch();
	m_iInputsSinceUpdate = 0;
}

void InputHandler::ButtonPressed( DeviceInput di, bool Down )
{
	if( di.ts.IsZero() )
	{
		di.ts = m_LastUpdate.Half();
		++m_iInputsSinceUpdate;
	}

	INPUTFILTER->ButtonPressed( di, Down );

	if( m_iInputsSinceUpdate >= 1000 )
	{
		/*
		 * We havn't received an update in a long time, so warn about it.  We expect to receive
		 * input events before the first UpdateTimer call only on the first update.  Leave
		 * m_iInputsSinceUpdate where it is, so we only warn once.  Only updates that didn't provide
		 * a timestamp are counted; if the driver provides its own timestamps, UpdateTimer is
		 * optional.
		 */
		LOG->Warn( "InputHandler::ButtonPressed: Driver sent many updates without calling UpdateTimer" );
		FAIL_M("x");
	}
}

wchar_t InputHandler::DeviceButtonToChar( DeviceButton button, bool bUseCurrentKeyModifiers )
{
	wchar_t c = L'\0';
	switch( button )
	{
	default:
		if( button < 127 )
			c = (wchar_t) button;
		else if( button >= KEY_KP_C0 && button <= KEY_KP_C9 )
			c =(wchar_t) (button - KEY_KP_C0) + '0';
		break;
	case KEY_KP_SLASH:	c = L'/';	break;
	case KEY_KP_ASTERISK:	c = L'*';	break;
	case KEY_KP_HYPHEN:	c = L'-';	break;
	case KEY_KP_PLUS:	c = L'+';	break;
	case KEY_KP_PERIOD:	c = L'.';	break;
	case KEY_KP_EQUAL:	c = L'=';	break;
	}

	// Handle some default US keyboard modifiers for derived InputHandlers that 
	// don't implement DeviceButtonToChar.
	if( bUseCurrentKeyModifiers )
	{
		bool bHoldingShift = 
			INPUTFILTER->IsBeingPressed(DeviceInput(DEVICE_KEYBOARD, KEY_LSHIFT)) ||
			INPUTFILTER->IsBeingPressed(DeviceInput(DEVICE_KEYBOARD, KEY_RSHIFT));

		bool bHoldingCtrl = 
			INPUTFILTER->IsBeingPressed(DeviceInput(DEVICE_KEYBOARD, KEY_LCTRL)) ||
			INPUTFILTER->IsBeingPressed(DeviceInput(DEVICE_KEYBOARD, KEY_RCTRL));
		
		if( bHoldingShift && !bHoldingCtrl )
		{
			MakeUpper( &c, 1 );

			switch( c )
			{
			case '`':	c = L'~';	break;
			case '1':	c = L'!';	break;
			case '2':	c = L'@';	break;
			case '3':	c = L'#';	break;
			case '4':	c = L'$';	break;
			case '5':	c = L'%';	break;
			case '6':	c = L'^';	break;
			case '7':	c = L'&';	break;
			case '8':	c = L'*';	break;
			case '9':	c = L'(';	break;
			case '0':	c = L')';	break;
			case '-':	c = L'_';	break;
			case '=':	c = L'+';	break;
			case '[':	c = L'{';	break;
			case ']':	c = L'}';	break;
			case '\'':	c = L'"';	break;
			case '\\':	c = L'|';	break;
			case ';':	c = L':';	break;
			case ',':	c = L'<';	break;
			case '.':	c = L'>';	break;
			case '/':	c = L'?';	break;
			}
		}

	}
	
	return c;
}

static LocalizedString HOME		( "DeviceButton", "Home" );
static LocalizedString END		( "DeviceButton", "End" );
static LocalizedString UP		( "DeviceButton", "Up" );
static LocalizedString DOWN		( "DeviceButton", "Down" );
static LocalizedString SPACE		( "DeviceButton", "Space" );
static LocalizedString SHIFT		( "DeviceButton", "Shift" );
static LocalizedString CTRL		( "DeviceButton", "Ctrl" );
static LocalizedString ALT		( "DeviceButton", "Alt" );
static LocalizedString INSERT		( "DeviceButton", "Insert" );
static LocalizedString DEL		( "DeviceButton", "Delete" );
static LocalizedString PGUP		( "DeviceButton", "PgUp" );
static LocalizedString PGDN		( "DeviceButton", "PgDn" );
static LocalizedString BACKSLASH	( "DeviceButton", "Backslash" );

RString InputHandler::GetDeviceSpecificInputString( const DeviceInput &di )
{
	if( di.device == InputDevice_Invalid )
		return RString();

	if( di.device == DEVICE_KEYBOARD )
	{
		wchar_t c = DeviceButtonToChar( di.button, false );
		if( c && c != L' ' )				// Don't show "Key  " for space.
			return InputDeviceToString( di.device ) + " " + Capitalize( WStringToRString(wstring()+c) );
	}

	RString s = DeviceButtonToString( di.button );
	if( di.device != DEVICE_KEYBOARD )
		s = InputDeviceToString( di.device ) + " " + s;
	return s;
}

RString InputHandler::GetLocalizedInputString( const DeviceInput &di )
{
	switch( di.button )
	{
	case KEY_HOME:				return HOME.GetValue();
	case KEY_END:				return END.GetValue();
	case KEY_UP:				return UP.GetValue();
	case KEY_DOWN:				return DOWN.GetValue();
	case KEY_SPACE:				return SPACE.GetValue();
	case KEY_LSHIFT: case KEY_RSHIFT:	return SHIFT.GetValue();
	case KEY_LCTRL:	 case KEY_RCTRL:	return CTRL.GetValue();
	case KEY_LALT:	 case KEY_RALT:		return ALT.GetValue();
	case KEY_INSERT:			return INSERT.GetValue();
	case KEY_DEL:				return DEL.GetValue();
	case KEY_PGUP:				return PGUP.GetValue();
	case KEY_PGDN:				return PGDN.GetValue();
	case KEY_BACKSLASH:			return BACKSLASH.GetValue();
	default:
		wchar_t c = DeviceButtonToChar( di.button, false );
		if( c && c != L' ' ) // Don't show "Key  " for space.
			return Capitalize( WStringToRString(wstring()+c) );

		return DeviceButtonToString( di.button );
	}
}

/*
 * (c) 2003-2004 Glenn Maynard
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
