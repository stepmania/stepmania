#include "InputHandler_X11.h"

#include "global.h"
#include "InputHandler_Xlib.h"
#include "RageUtil.h"
#include "RageLog.h"
#include "RageDisplay.h"

#include <X11/Xlib.h>
#include <X11/keysym.h>

static RageKeySym XSymToKeySym( int key )
{
#define KEY_INV KEY_INVALID
	const RageKeySym ASCIIKeySyms[] =
	{
		KEY_INV,	KEY_INV,	KEY_INV,	KEY_INV,       /* 0-3 */
		KEY_INV,	KEY_INV,	KEY_INV,	KEY_INV,       /* 4-7 */
		KEY_BACK,	KEY_TAB,	KEY_INV,	KEY_INV,       /* 8-11 */
		KEY_INV,	KEY_ENTER,	KEY_INV,	KEY_INV,       /* 12-15 */
		KEY_INV,	KEY_INV,	KEY_INV,	KEY_PAUSE,     /* 16-19 */
		KEY_INV,	KEY_INV,	KEY_INV,	KEY_INV,       /* 20-23 */
		KEY_INV,	KEY_INV,	KEY_INV,	KEY_ESC,       /* 24-27 */
		KEY_INV,	KEY_INV,	KEY_INV,	KEY_INV,       /* 28-31 */
		KEY_SPACE,	KEY_EXCL,	KEY_QUOTE,	KEY_HASH,      /* 32-35 */
		KEY_DOLLAR,	KEY_PERCENT,KEY_AMPER,	KEY_SQUOTE,	/* 36-39 */
		KEY_LPAREN,	KEY_RPAREN,	KEY_ASTERISK,KEY_PLUS,	/* 40-43 */
		KEY_COMMA,	KEY_HYPHEN,	KEY_PERIOD,	KEY_SLASH,     /* 44-47 */
		KEY_C0, 	KEY_C1, 	KEY_C2, 	KEY_C3,        /* 48-51 */
		KEY_C4, 	KEY_C5, 	KEY_C6, 	KEY_C7,        /* 52-55 */
		KEY_C8, 	KEY_C9, 	KEY_COLON,	KEY_SEMICOLON, /* 56-59 */
		KEY_LANGLE,	KEY_EQUAL,	KEY_RANGLE,	KEY_QUESTION,  /* 60-63 */
		KEY_AT, KEY_CA, KEY_CB, KEY_CC, KEY_CD, KEY_CE, KEY_CF, KEY_CG, KEY_CH, /* 64-72 */
		KEY_CI, KEY_CJ, KEY_CK, KEY_CL, KEY_CM, KEY_CN, KEY_CO, KEY_CP, KEY_CQ, /* 73-81 */
		KEY_CR, KEY_CS, KEY_CT, KEY_CU, KEY_CV, KEY_CW, KEY_CX, KEY_CY, KEY_CZ, /* 82-90 */
		KEY_LBRACKET, KEY_BACKSLASH, KEY_RBRACKET, KEY_CARAT, /* 91-94 */
		KEY_UNDERSCORE, KEY_ACCENT, KEY_Ca, KEY_Cb,		/* 95-98 */
		KEY_Cc, KEY_Cd, KEY_Ce, KEY_Cf, KEY_Cg, KEY_Ch, KEY_Ci, KEY_Cj, KEY_Ck, /* 99-107 */
		KEY_Cl, KEY_Cm, KEY_Cn, KEY_Co, KEY_Cp, KEY_Cq, KEY_Cr, KEY_Cs, KEY_Ct, /* 108-116 */
		KEY_Cu, KEY_Cv, KEY_Cw, KEY_Cx, KEY_Cy, KEY_Cz, /* 117-122 */
		KEY_LBRACE, KEY_PIPE, KEY_RBRACE, KEY_INV, KEY_DEL /* 123-127 */
	};

	/* 32...127: */
	if( key < int(ARRAYSIZE(ASCIIKeySyms)))
		return ASCIIKeySyms[key];

	/* XK_KP_0 ... XK_KP_9 to KEY_KP_C0 ... KEY_KP_C9 */
	if( key >= XK_KP_0 && key <= XK_KP_9 )
		return (RageKeySym) (key - XK_KP_0 + KEY_KP_C0);
	
	switch( key )
	{
	case XK_KP_Decimal:		return KEY_KP_PERIOD;
	case XK_KP_Divide:		return KEY_KP_SLASH;
	case XK_KP_Multiply:	return KEY_KP_ASTERISK;
	case XK_KP_Subtract:	return KEY_KP_HYPHEN;
	case XK_KP_Add: 		return KEY_KP_PLUS;
	case XK_KP_Equal:		return KEY_KP_EQUAL;
	case XK_KP_Enter:		return KEY_KP_ENTER;
	case XK_Up:				return KEY_UP;
	case XK_Down:			return KEY_DOWN;
	case XK_Right:			return KEY_RIGHT;
	case XK_Left:			return KEY_LEFT;
	case XK_Insert: 		return KEY_INSERT;
	case XK_Home:			return KEY_HOME;
	case XK_End:			return KEY_END;
	case XK_Page_Up:		return KEY_PGUP;
	case XK_Page_Down:		return KEY_PGDN;
	case XK_F1:				return KEY_F1;
	case XK_F2:				return KEY_F2;
	case XK_F3:				return KEY_F3;
	case XK_F4:				return KEY_F4;
	case XK_F5:				return KEY_F5;
	case XK_F6:				return KEY_F6;
	case XK_F7:				return KEY_F7;
	case XK_F8:				return KEY_F8;
	case XK_F9:				return KEY_F9;
	case XK_F10:			return KEY_F10;
	case XK_F11:			return KEY_F11;
	case XK_F12:			return KEY_F12;
	case XK_F13:			return KEY_F13;
	case XK_F14:			return KEY_F14;
	case XK_F15:			return KEY_F15;

	case XK_Num_Lock:		return KEY_NUMLOCK;
	case XK_Caps_Lock:		return KEY_CAPSLOCK;
	case XK_Scroll_Lock:	return KEY_SCRLLOCK;
	case XK_Return: 		return KEY_ENTER;
	case XK_Sys_Req:		return KEY_PRTSC;
	case XK_Print:			return KEY_PRTSC;
	case XK_Shift_R:		return KEY_RSHIFT;
	case XK_Shift_L:		return KEY_LSHIFT;
	case XK_Control_R:		return KEY_RCTRL;
	case XK_Control_L:		return KEY_LCTRL;
	case XK_Alt_R:			return KEY_RALT;
	case XK_Alt_L:			return KEY_LALT;
	case XK_Meta_R: 		return KEY_RMETA;
	case XK_Meta_L: 		return KEY_LMETA;
	case XK_Super_L:		return KEY_LSUPER;
	case XK_Super_R:		return KEY_RSUPER;
	case XK_Menu:			return KEY_MENU;
	}

	/* 0...31: */
	if( key - 0xFF00 < 0x0D)
		return ASCIIKeySyms[key - 0xFF00];

	return KEY_INVALID;
}

InputHandler_X11::InputHandler_Xlib()
{
	X11Helper::Go();
	X11Helper::OpenMask(KeyPressMask); X11Helper::OpenMask(KeyReleaseMask);
}

InputHandler_X11::~InputHandler_Xlib()
{
	X11Helper::CloseMask(KeyPressMask); X11Helper::CloseMask(KeyReleaseMask);
	X11Helper::Stop();
}

void InputHandler_X11::Update(float fDeltaTime)
{
	XEvent event;
	while(XCheckTypedEvent(X11Helper::Dpy(), KeyPress, &event)
		|| XCheckTypedEvent(X11Helper::Dpy(), KeyRelease, &event) )
	{
		LOG->Trace("key: sym %i, key %i, state %i",
			event.xkey.keycode, XSymToKeySym(event.key.keycode),
event.key.state );
		DeviceInput di( DEVICE_KEYBOARD, XSymToKeySym(event.key.keycode) );
		ButtonPressed(di, event.key.state);
	}

	InputHandler::UpdateTimer();
}


void InputHandler_X11::GetDevicesAndDescriptions(vector<InputDevice>&
vDevicesOut, vector<CString>& vDescriptionsOut)
{
	vDevicesOut.push_back( DEVICE_KEYBOARD );
	vDescriptionsOut.push_back( "Keyboard" );
}

/*
 * (c) 2005 Sean Burke, Ben Anderson
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