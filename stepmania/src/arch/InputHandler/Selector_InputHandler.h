#ifndef SELECTOR_INPUT_HANDLER_H
#define SELECTOR_INPUT_HANDLER_H

#include "arch/arch_platform.h"

/* InputHandler drivers selector. */
#if defined(HAVE_DIRECTX) && !defined(HAVE_XDK)
#include "InputHandler_DirectInput.h"
#endif

#if defined(LINUX)
#include "InputHandler_Linux_Joystick.h"
#endif

#include "InputHandler_MonkeyKeyboard.h"

// NOTE: If X11 is available, we don't use LLW_SDL, which IH_SDL depends on.
#if defined(HAVE_X11)
#include "InputHandler_X11.h"
#elif defined(HAVE_CARBON)
#include "InputHandler_Carbon.h"
#elif defined(HAVE_SDL)
#include "InputHandler_SDL.h"
#endif

#if defined(WINDOWS)
#include "InputHandler_Win32_Pump.h"
#include "InputHandler_Win32_Para.h"
#include "InputHandler_Win32_MIDI.h"
#endif

#ifdef HAVE_XDK
#include "InputHandler_Xbox.h"
#endif

#endif

/*
 * (c) 2005 Ben Anderson.
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
