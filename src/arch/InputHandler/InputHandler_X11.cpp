#include "global.h"
#include "InputHandler_X11.h"
#include "RageUtil.h"
#include "RageLog.h"
#include "RageDisplay.h"
#include "InputFilter.h"
#include "archutils/Unix/X11Helper.h"
#include "LinuxInputManager.h"

#include <X11/Xlib.h>
#include <X11/keysym.h>

using namespace X11Helper;

REGISTER_INPUT_HANDLER_CLASS( X11 );

static DeviceButton XSymToDeviceButton( int key )
{
#define DB_KEY_INV DeviceButton_Invalid
	static const DeviceButton ASCIIKeySyms[] =
	{
		DB_KEY_INV       , DB_KEY_INV     , DB_KEY_INV      , DB_KEY_INV     , DB_KEY_INV      , /* 0 - 4 */
		DB_KEY_INV       , DB_KEY_INV     , DB_KEY_INV      , DB_KEY_INV     , DB_KEY_INV      , /* 5 - 9 */
		DB_KEY_INV       , DB_KEY_INV     , DB_KEY_INV      , DB_KEY_INV     , DB_KEY_INV      , /* 10 - 14 */
		DB_KEY_INV       , DB_KEY_INV     , DB_KEY_INV      , DB_KEY_INV     , DB_KEY_INV     , /* 15 - 19 */
		DB_KEY_INV       , DB_KEY_INV     , DB_KEY_INV      , DB_KEY_INV     , DB_KEY_INV      , /* 20 - 24 */
		DB_KEY_INV       , DB_KEY_INV     , DB_KEY_INV      , DB_KEY_INV     , DB_KEY_INV      , /* 25 - 29 */
		DB_KEY_INV       , DB_KEY_INV     , DB_KEY_SPACE    , DB_KEY_EXCL    , DB_KEY_QUOTE    , /* 30 - 34 */
		DB_KEY_HASH      , DB_KEY_DOLLAR  , DB_KEY_PERCENT  , DB_KEY_AMPER   , DB_KEY_SQUOTE   , /* 35 - 39 */
		DB_KEY_LPAREN    , DB_KEY_RPAREN  , DB_KEY_ASTERISK , DB_KEY_PLUS    , DB_KEY_COMMA    , /* 40 - 44 */
		DB_KEY_HYPHEN    , DB_KEY_PERIOD  , DB_KEY_SLASH    , DB_KEY_C0      , DB_KEY_C1       , /* 45 - 49 */
		DB_KEY_C2        , DB_KEY_C3      , DB_KEY_C4       , DB_KEY_C5      , DB_KEY_C6       , /* 50 - 54 */
		DB_KEY_C7        , DB_KEY_C8      , DB_KEY_C9       , DB_KEY_COLON   , DB_KEY_SEMICOLON, /* 55 - 59 */
		DB_KEY_LANGLE    , DB_KEY_EQUAL   , DB_KEY_RANGLE   , DB_KEY_QUESTION, DB_KEY_AT       , /* 60 - 64 */
		DB_KEY_CA        , DB_KEY_CB      , DB_KEY_CC       , DB_KEY_CD      , DB_KEY_CE       , /* 65 - 69 */
		DB_KEY_CF        , DB_KEY_CG      , DB_KEY_CH       , DB_KEY_CI      , DB_KEY_CJ       , /* 70 - 74 */
		DB_KEY_CK        , DB_KEY_CL      , DB_KEY_CM       , DB_KEY_CN      , DB_KEY_CO       , /* 75 - 79 */
		DB_KEY_CP        , DB_KEY_CQ      , DB_KEY_CR       , DB_KEY_CS      , DB_KEY_CT       , /* 80 - 84 */
		DB_KEY_CU        , DB_KEY_CV      , DB_KEY_CW       , DB_KEY_CX      , DB_KEY_CY       , /* 85 - 89 */
		DB_KEY_CZ        , DB_KEY_LBRACKET, DB_KEY_BACKSLASH, DB_KEY_RBRACKET, DB_KEY_CARAT    , /* 90 - 94 */
		DB_KEY_UNDERSCORE, DB_KEY_ACCENT  , DB_KEY_Ca       , DB_KEY_Cb      , DB_KEY_Cc       , /* 95 - 99 */
		DB_KEY_Cd        , DB_KEY_Ce      , DB_KEY_Cf       , DB_KEY_Cg      , DB_KEY_Ch       , /* 100 - 104 */
		DB_KEY_Ci        , DB_KEY_Cj      , DB_KEY_Ck       , DB_KEY_Cl      , DB_KEY_Cm       , /* 105 - 109 */
		DB_KEY_Cn        , DB_KEY_Co      , DB_KEY_Cp       , DB_KEY_Cq      , DB_KEY_Cr       , /* 110 - 114 */
		DB_KEY_Cs        , DB_KEY_Ct      , DB_KEY_Cu       , DB_KEY_Cv      , DB_KEY_Cw       , /* 115 - 119 */
		DB_KEY_Cx        , DB_KEY_Cy      , DB_KEY_Cz       , DB_KEY_LBRACE  , DB_KEY_PIPE     , /* 120 - 124 */
		DB_KEY_RBRACE    , DB_KEY_INV     , DB_KEY_DEL                                     /* 125 - 127 */
	};

	/* 32...127: */
	if( key < int(ARRAYLEN(ASCIIKeySyms)))
		return ASCIIKeySyms[key];

	/* XK_KP_0 ... XK_KP_9 to DB_KEY_KP_C0 ... DB_KEY_KP_C9 */
	if( key >= XK_KP_0 && key <= XK_KP_9 )
		return enum_add2(DB_KEY_KP_C0, key - XK_KP_0);
	
	switch( key )
	{
	/* These are needed because of the way X registers the keypad. */
	case XK_BackSpace:	return DB_KEY_BACK;
	case XK_Tab:		return DB_KEY_TAB;
	case XK_Pause:		return DB_KEY_PAUSE;
	case XK_Escape:		return DB_KEY_ESC;
	case XK_KP_Insert:	return DB_KEY_KP_C0;
	case XK_KP_End:		return DB_KEY_KP_C1;
	case XK_KP_Down:	return DB_KEY_KP_C2;
	case XK_KP_Page_Down:	return DB_KEY_KP_C3;
	case XK_KP_Left:	return DB_KEY_KP_C4;
	case XK_KP_Begin:	return DB_KEY_KP_C5;
	case XK_KP_Right:	return DB_KEY_KP_C6;
	case XK_KP_Home:	return DB_KEY_KP_C7;
	case XK_KP_Up:		return DB_KEY_KP_C8;
	case XK_KP_Page_Up:	return DB_KEY_KP_C9;
	case XK_KP_Decimal:	return DB_KEY_KP_PERIOD;
	case XK_KP_Divide:	return DB_KEY_KP_SLASH;
	case XK_KP_Multiply:	return DB_KEY_KP_ASTERISK;
	case XK_KP_Subtract:	return DB_KEY_KP_HYPHEN;
	case XK_KP_Add:		return DB_KEY_KP_PLUS;
	case XK_KP_Equal:	return DB_KEY_KP_EQUAL;
	case XK_KP_Enter:	return DB_KEY_KP_ENTER;
	case XK_Up:		return DB_KEY_UP;
	case XK_Down:		return DB_KEY_DOWN;
	case XK_Right:		return DB_KEY_RIGHT;
	case XK_Left:		return DB_KEY_LEFT;
	case XK_Insert:		return DB_KEY_INSERT;
	case XK_Home:		return DB_KEY_HOME;
	case XK_Delete:		return DB_KEY_DEL;
	case XK_End:		return DB_KEY_END;
	case XK_Page_Up:	return DB_KEY_PGUP;
	case XK_Page_Down:	return DB_KEY_PGDN;
	case XK_F1:		return DB_KEY_F1;
	case XK_F2:		return DB_KEY_F2;
	case XK_F3:		return DB_KEY_F3;
	case XK_F4:		return DB_KEY_F4;
	case XK_F5:		return DB_KEY_F5;
	case XK_F6:		return DB_KEY_F6;
	case XK_F7:		return DB_KEY_F7;
	case XK_F8:		return DB_KEY_F8;
	case XK_F9:		return DB_KEY_F9;
	case XK_F10:		return DB_KEY_F10;
	case XK_F11:		return DB_KEY_F11;
	case XK_F12:		return DB_KEY_F12;
	case XK_F13:		return DB_KEY_F13;
	case XK_F14:		return DB_KEY_F14;
	case XK_F15:		return DB_KEY_F15;

	case XK_Num_Lock:	return DB_KEY_NUMLOCK;
	case XK_Caps_Lock:	return DB_KEY_CAPSLOCK;
	case XK_Scroll_Lock:	return DB_KEY_SCRLLOCK;
	case XK_Return: 	return DB_KEY_ENTER;
	case XK_Sys_Req:	return DB_KEY_PRTSC;
	case XK_Print:		return DB_KEY_PRTSC;
	case XK_Shift_R:	return DB_KEY_RSHIFT;
	case XK_Shift_L:	return DB_KEY_LSHIFT;
	case XK_Control_R:	return DB_KEY_RCTRL;
	case XK_Control_L:	return DB_KEY_LCTRL;
	case XK_Alt_R:		return DB_KEY_RALT;
	case XK_Alt_L:		return DB_KEY_LALT;
	case XK_Meta_R: 	return DB_KEY_RMETA;
	case XK_Meta_L: 	return DB_KEY_LMETA;
	case XK_Super_L:	return DB_KEY_LSUPER;
	case XK_Super_R:	return DB_KEY_RSUPER;
	case XK_Menu:		return DB_KEY_MENU;

	// mouse
	case XK_Pointer_Button1: return MOUSE_LEFT;
	case XK_Pointer_Button2: return MOUSE_MIDDLE;
	case XK_Pointer_Button3: return MOUSE_RIGHT;
	case XK_Pointer_Button4: return MOUSE_WHEELUP;
	case XK_Pointer_Button5: return MOUSE_WHEELDOWN;
	}

	return DeviceButton_Invalid;
}

InputHandler_X11::InputHandler_X11()
{
	if( Dpy == NULL  || Win == None )
		return;
	XWindowAttributes winAttrib;

	XGetWindowAttributes( Dpy, Win, &winAttrib );
	// todo: add ButtonMotionMask, Button(1-5)MotionMask,
	// (EnterWindowMask/LeaveWindowMask?) -aj
	long eventMask( winAttrib.your_event_mask
		| ButtonPressMask | ButtonReleaseMask | PointerMotionMask
		| FocusChangeMask );
	if(LINUXINPUT == NULL) LINUXINPUT = new LinuxInputManager;
	if( LINUXINPUT->X11IgnoreKeyboard() == false )
	{
		eventMask |= KeyPressMask | KeyReleaseMask;
	}
	XSelectInput( Dpy, Win, eventMask );
}

InputHandler_X11::~InputHandler_X11()
{
	if( Dpy == NULL || Win == None )
		return;
	// TODO: Determine if we even need to set this back (or is the window
	// destroyed just after this?)
	XWindowAttributes winAttrib;

	XGetWindowAttributes( Dpy, Win, &winAttrib );
	XSelectInput( Dpy, Win,
		winAttrib.your_event_mask &
		~(KeyPressMask|KeyReleaseMask|ButtonPressMask|ButtonReleaseMask|PointerMotionMask|FocusChangeMask)
	);
}

void InputHandler_X11::Update()
{
	if( Dpy == NULL || Win == None )
	{
		InputHandler::UpdateTimer();
		return;
	}

	XEvent event, lastEvent;
	DeviceButton lastDB = DeviceButton_Invalid;
	lastEvent.type = 0;

	// We use XCheckWindowEvent instead of XNextEvent because
	// other things (most notably ArchHooks_Unix) may be looking for
	// events that we'd pick up and discard.
	// todo: add other masks? (like the ones for drag'n drop) -aj
	while( XCheckWindowEvent(Dpy, Win,
			KeyPressMask | KeyReleaseMask
			| ButtonPressMask | ButtonReleaseMask | PointerMotionMask
			| FocusChangeMask,
			&event) )
	{
		const bool bKeyPress = event.type == KeyPress;
		//const bool bMousePress = event.type == ButtonPress;

		if( event.type == MotionNotify )
		{
			INPUTFILTER->UpdateCursorLocation(event.xbutton.x,event.xbutton.y);
		}

		if( lastEvent.type != 0 )
		{
			if( bKeyPress && event.xkey.time == lastEvent.xkey.time &&
			    event.xkey.keycode == lastEvent.xkey.keycode )
			{
				// This is a repeat event so ignore it.
				lastEvent.type = 0;
				continue;
			}
			// This is a new event so the last release was not a repeat.
			ButtonPressed( DeviceInput(DEVICE_KEYBOARD, lastDB, 0) );
			lastEvent.type = 0;
		}

		if( event.type == FocusOut )
		{
			// Release all buttons
			INPUTFILTER->Reset();
		}

		// Get the first defined keysym for this event's key
		lastDB = XSymToDeviceButton( XLookupKeysym(&event.xkey, 0) );

		if( lastDB == DeviceButton_Invalid )
			continue;

		if( bKeyPress )
			ButtonPressed( DeviceInput(DEVICE_KEYBOARD, lastDB, 1) );
		/*
		else if( bMousePress )
			ButtonPressed( DeviceInput(DEVICE_MOUSE, lastDB, 1) );
		*/
		else
			lastEvent = event;
	}

	// Handle any last releases.
	if( lastEvent.type != 0 )
	{
		if( lastEvent.type == (KeyPress|KeyRelease) )
			ButtonPressed( DeviceInput(DEVICE_KEYBOARD, lastDB, 0) );
		/*
		if( lastEvent.type == (ButtonPress|ButtonRelease) )
			ButtonPressed( DeviceInput(DEVICE_MOUSE, lastDB, 0) );
		*/
	}

	InputHandler::UpdateTimer();
}


void InputHandler_X11::GetDevicesAndDescriptions( vector<InputDeviceInfo>& vDevicesOut )
{
	if( Dpy && Win )
	{
		vDevicesOut.push_back( InputDeviceInfo(DEVICE_KEYBOARD,"Keyboard") );
		vDevicesOut.push_back( InputDeviceInfo(DEVICE_MOUSE,"Mouse") );
	}
}

/*
 * (c) 2005, 2006 Sean Burke, Ben Anderson, Steve Checkoway
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
