#ifndef XBOX_ARCH_H
#define XBOX_ARCH_H

/* Load drivers for Xbox. */

#include "LoadingWindow/LoadingWindow_SDL.h"

#include "ArchHooks/ArchHooks_Xbox.h"

/* Err, is SDL input working on Xbox?  arch_default comments indicated no.
 * (Best to use a custom driver for threaded input, anyway.) */
/* I believe it is working, and it's the only thing we depend on SDL for.
 * I'll write an InputHandler so we can ditch the SDLXbox mess. -Chris */
#include "InputHandler/InputHandler_SDL.h"

#include "Sound/RageSoundDriver_DSound.h"

/* Undef this if you need no SDL input. */
// #undef SUPPORT_SDL_INPUT

#endif

/*
 * (c) 2002 Glenn Maynard, Chris Danford
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

