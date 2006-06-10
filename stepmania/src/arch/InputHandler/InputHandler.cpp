#include "global.h"
#include "InputFilter.h"
#include "RageUtil.h"
#include "InputHandler.h"
#include "RageLog.h"

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
	wchar_t c = '\0';
	switch( button )
	{
	default:
		if( button < 127 )
			c = (char) button;
		else if( button >= KEY_KP_C0 && button <= KEY_KP_C9 )
			c =(char) (button - KEY_KP_C0) + '0';
		break;
	case KEY_KP_SLASH:	c ='/';	break;
	case KEY_KP_ASTERISK:	c ='*';	break;
	case KEY_KP_HYPHEN:	c ='-';	break;
	case KEY_KP_PLUS:	c ='+';	break;
	case KEY_KP_PERIOD:	c ='.';	break;
	case KEY_KP_EQUAL:	c ='=';	break;
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
			c = (char)toupper(c);

			switch( c )
			{
			case '`':	c='~';	break;
			case '1':	c='!';	break;
			case '2':	c='@';	break;
			case '3':	c='#';	break;
			case '4':	c='$';	break;
			case '5':	c='%';	break;
			case '6':	c='^';	break;
			case '7':	c='&';	break;
			case '8':	c='*';	break;
			case '9':	c='(';	break;
			case '0':	c=')';	break;
			case '-':	c='_';	break;
			case '=':	c='+';	break;
			case '[':	c='{';	break;
			case ']':	c='}';	break;
			case '\'':	c='"';	break;
			case '\\':	c='|';	break;
			case ';':	c=':';	break;
			case ',':	c='<';	break;
			case '.':	c='>';	break;
			case '/':	c='?';	break;
			}
		}

	}
	
	return c;
}

RString InputHandler::GetDeviceSpecificInputString( const DeviceInput &di )
{
	if( di.device == DEVICE_KEYBOARD )
	{
		wchar_t c = DeviceButtonToChar( di.button, false );
		if( c )
		{
			if( c == ' ' )
				return "space";	// Don't show "Key  " for space.
			else
				return "Key " + WStringToRString(wstring()+c);
		}
		return DeviceButtonToTranslatedString( di.button );
	}

	return di.ToString();
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
