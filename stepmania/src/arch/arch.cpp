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
#elif defined(WIN32)
#include "arch_Win32.h"
#endif


LoadingWindow *MakeLoadingWindow() { return new ARCH_LOADING_WINDOW; }
ErrorDialog *MakeErrorDialog() { return new ARCH_ERROR_DIALOG; }
ArchHooks *MakeArchHooks() { return new ARCH_HOOKS; }
LowLevelWindow *MakeLowLevelWindow() { return new ARCH_LOW_LEVEL_WINDOW; }

void MakeInputHandlers(vector<InputHandler *> &Add)
{
#if defined(WIN32)
	Add.push_back(new InputHandler_Win32_Pump);
#endif
}

/* Err, this is ugly--breaks arch encapsulation. Hmm. */
RageSoundDriver *MakeRageSoundDriver(CString drivers)
{
	CStringArray DriversToTry;
	split(drivers, ",", DriversToTry, true);

	CString Driver;
	RageSoundDriver *ret = NULL;

	for(unsigned i = 0; ret == NULL && i < DriversToTry.size(); ++i)
	{
		try {
			Driver = DriversToTry[i];
			LOG->Trace("Initializing driver: %s", DriversToTry[i].GetString());

#ifdef WIN32
			if(!DriversToTry[i].CompareNoCase("DirectSound")) ret = new RageSound_DSound;
			if(!DriversToTry[i].CompareNoCase("DirectSound-sw")) ret = new RageSound_DSound_Software;
			if(!DriversToTry[i].CompareNoCase("WaveOut")) ret = new RageSound_WaveOut;
#endif
#ifdef LINUX	/* should use some define akin to HAS_ALSA9 */
			if(!DriversToTry[i].CompareNoCase("ALSA9")) ret = new RageSound_ALSA9;
#endif		
			if(!DriversToTry[i].CompareNoCase("Null")) ret = new RageSound_Null;
			if( !ret )
				LOG->Warn("Unknown sound driver name: %s", DriversToTry[i].GetString());
		}
		catch(const RageException &e) {
			LOG->Info("Couldn't load driver %s: %s", DriversToTry[i].GetString(), e.what());
		}
	}

	if(ret)
		LOG->Info("Sound driver: %s", Driver.GetString());
	
	return ret;
}

/*
 * Copyright (c) 2002 by the person(s) listed below.  All rights reserved.
 *
 * Glenn Maynard
 */
