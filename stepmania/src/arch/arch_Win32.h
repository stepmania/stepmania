#ifndef WIN32_ARCH_H
#define WIN32_ARCH_H

/* Load drivers for Win32. */
#include "LoadingWindow/LoadingWindow_Win32.h"
#include "ErrorDialog/ErrorDialog_Win32.h"
#include "ArchHooks/ArchHooks_Win32.h"

#include "InputHandler/InputHandler_DirectInput.h"
#include "InputHandler/InputHandler_Win32_Pump.h"
// #include "InputHandler/InputHandler_Win32_Para.h"

#include "Sound/RageSoundDriver_DSound.h"
#include "Sound/RageSoundDriver_DSound_Software.h"
#include "Sound/RageSoundDriver_WaveOut.h"

#undef SUPPORT_SDL_INPUT

#endif

/*
 * Copyright (c) 2002 by the person(s) listed below.  All rights reserved.
 *
 * Glenn Maynard
 */
