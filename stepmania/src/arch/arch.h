#ifndef ARCH_H
#define ARCH_H

/* Include this file if you need to create an instance of a driver object.  */
class ErrorDialog;
class LoadingWindow;
class RageSoundDriver;
class ArchHooks;
class InputHandler;
class LowLevelWindow;

ErrorDialog *MakeErrorDialog();
LoadingWindow *MakeLoadingWindow();
ArchHooks *MakeArchHooks();
LowLevelWindow *MakeLowLevelWindow();

void MakeInputHandlers(vector<InputHandler *> &Add);
RageSoundDriver *MakeRageSoundDriver(CString drivers);

/* Define the default list of sound drivers for each arch.  It's
 * OK to list drivers that may not be available. */
#if defined(LINUX)
#define DEFAULT_SOUND_DRIVER_LIST "ALSA9,OSS,Null"
#elif defined(DARWIN)
#define DEFAULT_SOUND_DRIVER_LIST "Null" /* XXX */
#elif defined(WIN32)
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
