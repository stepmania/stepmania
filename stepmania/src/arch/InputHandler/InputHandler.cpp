#include "global.h"
#include "InputFilter.h"
#include "RageUtil.h"
#include "InputHandler.h"

#include "Selector_InputHandler.h"

void MakeInputHandlers(CString drivers, vector<InputHandler *> &Add)
{
	CStringArray DriversToTry;
	split(drivers, ",", DriversToTry, true);

	ASSERT( DriversToTry.size() != 0 );

	CString Driver;

	for(unsigned i = 0; i < DriversToTry.size(); ++i)
	{
#ifdef USE_INPUT_HANDLER_DIRECTINPUT
		if(!DriversToTry[i].CompareNoCase("DirectInput") )	Add.push_back(new InputHandler_DInput);
#endif
#ifdef USE_INPUT_HANDLER_LINUX_JOYSTICK
		if(!DriversToTry[i].CompareNoCase("Linux_Joystick") )	Add.push_back(new InputHandler_Linux_Joystick);
#endif
#ifdef USE_INPUT_HANDLER_LINUX_TTY
		if(!DriversToTry[i].CompareNoCase("Linux_tty") )	Add.push_back(new InputHandler_Linux_tty);
#endif
#ifdef USE_INPUT_HANDLER_MONKEY_KEYBOARD
		if(!DriversToTry[i].CompareNoCase("MonkeyKeyboard") )	Add.push_back(new InputHandler_MonkeyKeyboard);
#endif
#ifdef USE_INPUT_HANDLER_SDL
		if(!DriversToTry[i].CompareNoCase("SDL") )		Add.push_back(new InputHandler_SDL);
#endif
#ifdef USE_INPUT_HANDLER_WIN32_PARA
		if(!DriversToTry[i].CompareNoCase("Para") )		Add.push_back(new InputHandler_Win32_Para);
#endif
#ifdef USE_INPUT_HANDLER_WIN32_PUMP
		if(!DriversToTry[i].CompareNoCase("Pump") )		Add.push_back(new InputHandler_Win32_Pump);
#endif
#ifdef USE_INPUT_HANDLER_X11
		if(!DriversToTry[i].CompareNoCase("X11") )		Add.push_back(new InputHandler_X11);
#endif
#ifdef USE_INPUT_HANDLER_XBOX
		if(!DriversToTry[i].CompareNoCase("Xbox") )		Add.push_back(new InputHandler_Xbox);
#endif
	}
}

void InputHandler::UpdateTimer()
{
	m_LastUpdate.Touch();
}

void InputHandler::ButtonPressed( DeviceInput di, bool Down )
{
	if( di.ts.IsZero() )
		di.ts = m_LastUpdate.Half();

	INPUTFILTER->ButtonPressed( di, Down );
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
