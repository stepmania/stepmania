#ifndef ARCH_H
#define ARCH_H

/* Include this file if you need to create an instance of a driver object.  */
class ErrorDialog;
class LoadingWindow;
class RageSoundDriver;
class ArchHooks;

ErrorDialog *MakeErrorDialog();
LoadingWindow *MakeLoadingWindow();
RageSoundDriver *MakeRageSoundDriver(CString drivers);
ArchHooks *MakeArchHooks();

/* Define the default list of sound drivers for each arch. */
#if defined(WIN32)
#define DEFAULT_SOUND_DRIVER_LIST "DirectSound,DirectSound-sw,WaveOut"
#else
#define DEFAULT_SOUND_DRIVER_LIST "Null"
#endif

#endif

/*
 * Copyright (c) 2002-2003 by the person(s) listed below.  All rights reserved.
 *
 * Glenn Maynard
 */
