#ifndef ARCH_H
#define ARCH_H

/* Include this file if you need to create an instance of a driver object.  */
class ErrorDialog;
class LoadingWindow;
class RageSoundDriver;

ErrorDialog *MakeErrorDialog();
LoadingWindow *MakeLoadingWindow();
RageSoundDriver *MakeRageSoundDriver(CString drivers);

/* Define the default list of sound drivers for each arch. */
#if defined(WIN32)
#define DEFAULT_SOUND_DRIVER_LIST "DirectSound,DirectSound-sw,WaveOut"
#endif

#endif

/*
 * Copyright (c) 2002 by the person(s) listed below.  All rights reserved.
 *
 * Glenn Maynard
 */
