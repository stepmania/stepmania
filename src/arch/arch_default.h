#ifndef ARCH_DEFAULT_H
#define ARCH_DEFAULT_H

/* Define the default driver sets. */
#if defined(WINDOWS)
#include "ArchHooks/ArchHooks_Win32.h"
#include "LoadingWindow/LoadingWindow_Win32.h"
#include "LowLevelWindow/LowLevelWindow_Win32.h"
#include "MemoryCard/MemoryCardDriverThreaded_Windows.h"
#include "USB/USBDriver_Impl_Libusb.h"
#define DEFAULT_INPUT_DRIVER_LIST "DirectInput,Pump,Para"
#define DEFAULT_MOVIE_DRIVER_LIST "FFMpeg,DShow,Null"
#define DEFAULT_SOUND_DRIVER_LIST "WaveOut,DirectSound-sw,WDMKS,Null"


#elif defined(MACOSX)
#include "ArchHooks/ArchHooks_MacOSX.h"
#include "LoadingWindow/LoadingWindow_MacOSX.h"
#include "LowLevelWindow/LowLevelWindow_MacOSX.h"
#include "MemoryCard/MemoryCardDriverThreaded_MacOSX.h"
#define DEFAULT_INPUT_DRIVER_LIST "HID"
#define DEFAULT_MOVIE_DRIVER_LIST "FFMpeg,Null"
#define DEFAULT_SOUND_DRIVER_LIST "AudioUnit,Null"


#elif defined(UNIX)
#include "ArchHooks/ArchHooks_Unix.h"
#include "LowLevelWindow/LowLevelWindow_X11.h"

#if defined(LINUX)
#include "MemoryCard/MemoryCardDriverThreaded_Linux.h"
#endif

#if defined(HAVE_GTK)
#include "LoadingWindow/LoadingWindow_Gtk.h"
#endif
#if defined(LINUX)
#include "USB/USBDriver_Impl_Libusb.h"
#define DEFAULT_INPUT_DRIVER_LIST "X11,LinuxEvent,LinuxJoystick"
#else
#define DEFAULT_INPUT_DRIVER_LIST "X11"
#endif
#define DEFAULT_MOVIE_DRIVER_LIST "FFMpeg,Null"
// ALSA comes first, as the system may have OSS compat but we don't want to use
// it if it's actually an ALSA wrapper.
// Then try OSS before daemon drivers so we're going direct instead of
// unwittingly starting a daemon.
// JACK gives us an explicit option to NOT start a daemon, so try it third,
// as PulseAudio will successfully Init() but not actually work if the
// PulseAudio daemon has been suspended by/for jackd.
#define DEFAULT_SOUND_DRIVER_LIST "ALSA-sw,OSS,JACK,Pulse,Null"
#else
#error Which arch?
#endif

/* All use these. */
#include "LoadingWindow/LoadingWindow_Null.h"
#include "MemoryCard/MemoryCardDriver_Null.h"

#endif

/*
 * (c) 2002-2006 Glenn Maynard, Ben Anderson, Steve Checkoway
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
