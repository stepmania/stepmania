#ifndef ARCH_DEFAULT_H
#define ARCH_DEFAULT_H

#include "arch/arch_platform.h"

/* Define the default driver sets. */

/* InputHandler drivers */
#if defined (HAVE_XDK)
 #define DEFAULT_INPUT_DRIVER_LIST "Xbox"
#elif defined(HAVE_DIRECTX)
 #define DEFAULT_INPUT_DRIVER_LIST "DirectInput,Pump,Para"
#elif defined(HAVE_X11) // Prefer X11 over SDL
 #define DEFAULT_INPUT_DRIVER_LIST "X11,Joystick"
#elif defined(MACOSX)
# define DEFAULT_INPUT_DRIVER_LIST "Carbon"
#elif defined(LINUX)
 #define DEFAULT_INPUT_DRIVER_LIST "Joystick"
#else
 #define DEFAULT_INPUT_DRIVER_LIST "Null"
#endif

/* MovieTexture drivers */
#define DEFAULT_MOVIE_DRIVER_LIST "Theora,FFMpeg,DShow,Null"

/* RageSoundDrivers */
#define DEFAULT_SOUND_DRIVER_LIST "ALSA,DirectSound,ALSA-sw,DirectSound-sw,CoreAudio,OSS,WaveOut,Null"

#endif

/*
 * (c) 2002-2005 Glenn Maynard, Ben Anderson
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
