/*
 * This file provides functions to create driver objects.
 */

#include "global.h"
#include "RageLog.h"
#include "RageUtil.h"

#include "PrefsManager.h"
#include "arch.h"

/* Load default drivers. */
#include "arch_default.h"

/* Override them with arch-specific drivers, as available. */
#if defined(LINUX)
#include "arch_linux.h"
#elif defined(DARWIN)
#include "arch_darwin.h"
#elif defined(_XBOX)
#include "arch_xbox.h"
#elif defined(_WINDOWS)
#include "arch_Win32.h"
#endif

LoadingWindow *MakeLoadingWindow()
{
	/* Don't load NULL by default.  On most systems, if we can't load the SDL
	 * loading window, we won't be able to init OpenGL, either, so don't bother. */
	CString drivers = "win32,cocoa,gtk,sdl";
	CStringArray DriversToTry;
	split(drivers, ",", DriversToTry, true);

	ASSERT( DriversToTry.size() != 0 );

	CString Driver;
	LoadingWindow *ret = NULL;

	for(unsigned i = 0; ret == NULL && i < DriversToTry.size(); ++i)
	{
		try {
			Driver = DriversToTry[i];

#ifdef HAVE_LOADING_WINDOW_WIN32
			if(!DriversToTry[i].CompareNoCase("Win32")) ret = new LoadingWindow_Win32;
#endif
#ifdef HAVE_LOADING_WINDOW_GTK
			if(!DriversToTry[i].CompareNoCase("Gtk")) ret = new LoadingWindow_Gtk;
#endif
#ifdef HAVE_LOADING_WINDOW_COCOA
			if(!DriversToTry[i].CompareNoCase("Cocoa")) ret = new LoadingWindow_Cocoa;
#endif
#ifdef HAVE_LOADING_WINDOW_SDL
			if(!DriversToTry[i].CompareNoCase("SDL")) ret = new LoadingWindow_SDL;
#endif
#ifdef HAVE_LOADING_WINDOW_NULL
			if(!DriversToTry[i].CompareNoCase("Null")) ret = new LoadingWindow_Null;
#endif
		} catch(const RageException &e) {
			LOG->Info("Couldn't load driver %s: %s", DriversToTry[i].c_str(), e.what());
		}
	}
	
	if(ret)
		LOG->Info("Loading window: %s", Driver.c_str());
	
	return ret;
}

ErrorDialog *MakeErrorDialog() { return new ARCH_ERROR_DIALOG; }
ArchHooks *MakeArchHooks() { return new ARCH_HOOKS; }
#if defined(SUPPORT_OPENGL)
LowLevelWindow *MakeLowLevelWindow() { return new ARCH_LOW_LEVEL_WINDOW; }
#endif

void MakeInputHandlers(vector<InputHandler *> &Add)
{
#if defined(_WINDOWS)
	Add.push_back(new InputHandler_DInput);
	Add.push_back(new InputHandler_Win32_Pump);
//	Add.push_back(new InputHandler_Win32_Para);
#elif defined(_XBOX)
	// Add.push_back(new InputHandler_DInput);
#endif

#if defined(SUPPORT_SDL_INPUT)
	Add.push_back(new InputHandler_SDL);
#endif
}

/* Err, this is ugly--breaks arch encapsulation. Hmm. */
RageSoundDriver *MakeRageSoundDriver(CString drivers)
{
	CStringArray DriversToTry;
	split(drivers, ",", DriversToTry, true);

	ASSERT( DriversToTry.size() != 0 );

	CString Driver;
	RageSoundDriver *ret = NULL;

	for(unsigned i = 0; ret == NULL && i < DriversToTry.size(); ++i)
	{
		try {
			Driver = DriversToTry[i];
			LOG->Trace("Initializing driver: %s", DriversToTry[i].c_str());

#ifdef _WINDOWS
			if(!DriversToTry[i].CompareNoCase("DirectSound")) ret = new RageSound_DSound;
			if(!DriversToTry[i].CompareNoCase("DirectSound-sw")) ret = new RageSound_DSound_Software;
			if(!DriversToTry[i].CompareNoCase("WaveOut")) ret = new RageSound_WaveOut;
#endif
#ifdef _XBOX
			if(!DriversToTry[i].CompareNoCase("DirectSound")) ret = new RageSound_DSound;
#endif
#ifdef HAVE_ALSA
			if(!DriversToTry[i].CompareNoCase("ALSA9")) ret = new RageSound_ALSA9;
#endif		
#ifdef HAVE_OSS
			if(!DriversToTry[i].CompareNoCase("OSS")) ret = new RageSound_OSS;
#endif
#ifdef RAGE_SOUND_CA
			if(!DriversToTry[i].CompareNoCase("CoreAudio")) ret = new RageSound_CA;
#endif
#ifdef RAGE_SOUND_QT
			if(!DriversToTry[i].CompareNoCase("QT")) ret = new RageSound_QT;
#endif
#ifdef RAGE_SOUND_QT1
			if(!DriversToTry[i].CompareNoCase("QT1")) ret = new RageSound_QT1;
#endif
			if(!DriversToTry[i].CompareNoCase("Null")) ret = new RageSound_Null;
			if( !ret )
				LOG->Warn("Unknown sound driver name: %s", DriversToTry[i].c_str());
		} catch(const RageException &e) {
			LOG->Info("Couldn't load driver %s: %s", DriversToTry[i].c_str(), e.what());
		}
	}
	
	if(ret)
		LOG->Info("Sound driver: %s", Driver.c_str());
	
	return ret;
}

/*
 * Copyright (c) 2002 by the person(s) listed below.  All rights reserved.
 *
 * Glenn Maynard
 */
