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
#elif defined(WIN32)
#include "arch_Win32.h"
#endif

LoadingWindow *MakeLoadingWindow() { return new ARCH_LOADING_WINDOW; }
ErrorDialog *MakeErrorDialog() { return new ARCH_ERROR_DIALOG; }
ArchHooks *MakeArchHooks() { return new ARCH_HOOKS; }
LowLevelWindow *MakeLowLevelWindow() { return new ARCH_LOW_LEVEL_WINDOW; }

void MakeInputHandlers(vector<InputHandler *> &Add)
{
#if defined(WIN32) && !defined(_XBOX)
	Add.push_back(new InputHandler_DInput);
#else
	Add.push_back(new InputHandler_SDL);
#endif

#if defined(WIN32)
	Add.push_back(new InputHandler_Win32_Pump);
//	Add.push_back(new InputHandler_Win32_Para);
#endif
}

//#if defined(LINUX)
// #define HAVE_OSS
//#define HAVE_ALSA
//#endif

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

#ifdef WIN32
			if(!DriversToTry[i].CompareNoCase("DirectSound")) ret = new RageSound_DSound;
			if(!DriversToTry[i].CompareNoCase("DirectSound-sw")) ret = new RageSound_DSound_Software;
			if(!DriversToTry[i].CompareNoCase("WaveOut")) ret = new RageSound_WaveOut;
#endif
#ifdef HAVE_ALSA
			if(!DriversToTry[i].CompareNoCase("ALSA9")) ret = new RageSound_ALSA9;
#endif		
#ifdef HAVE_OSS
			if(!DriversToTry[i].CompareNoCase("OSS")) ret = new RageSound_OSS;
#endif
#ifdef RAGE_SOUND_SDL
            if(!DriversToTry[i].CompareNoCase("SDL")) ret = new RageSound_SDL;
#endif
			if(!DriversToTry[i].CompareNoCase("Null")) ret = new RageSound_Null;
			if( !ret )
				LOG->Warn("Unknown sound driver name: %s", DriversToTry[i].c_str());
		}
		catch(const RageException &e) {
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
