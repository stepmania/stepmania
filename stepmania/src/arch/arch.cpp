/*
 * This file provides functions to create driver objects.
 */

#include "../stdafx.h"
#include "../RageLog.h"
#include "../RageUtil.h"

#include "../PrefsManager.h"
#include "arch.h"

/* Load default drivers. */
#include "arch_default.h"

/* Override them with arch-specific drivers, as available. */
#if defined(WIN32)
#include "arch_Win32.h"
#endif

LoadingWindow *MakeLoadingWindow() { return new ARCH_LOADING_WINDOW; }
ErrorDialog *MakeErrorDialog() { return new ARCH_ERROR_DIALOG; }

/* Err, this is ugly--breaks arch encapsulation. Hmm. */
RageSoundDriver *MakeRageSoundDriver(CString drivers)
{
	CStringArray DriversToTry;
	split(drivers, ",", DriversToTry, true);

	for(unsigned i = 0; i < DriversToTry.size(); ++i)
	{
		try {
			LOG->Trace("Initializing driver: %s", DriversToTry[i].GetString());

#if defined(WIN32)
			if(DriversToTry[i] == "DirectSound") return new RageSound_DSound;
			if(DriversToTry[i] == "DirectSound-sw") return new RageSound_DSound_Software;
#endif

			LOG->Warn("Unknown sound driver name: %s", DriversToTry[i].GetString());
		}
		catch(const char *e) {
			LOG->Trace("Couldn't load driver %s: %s", DriversToTry[i].GetString(), e);
		}
	}

	return NULL;
}

/*
 * Copyright (c) 2002 by the person(s) listed below.  All rights reserved.
 *
 * Glenn Maynard
 */
