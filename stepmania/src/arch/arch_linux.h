#ifndef LINUX_ARCH_H
#define LINUX_ARCH_H

/* #ifdef HAS_ALSA9 */
#include "Sound/RageSoundDriver_ALSA9.h"
/* #endif HAS_ALSA9 */
#include "Sound/RageSoundDriver_OSS.h"
#endif
/* 
 * Sometimes Alsa and OSS fail to load, so
 * SDL should be a fallback, even though it
 * doesn't work perfectly.
 */
#include "SDL.h"
#include "Sound/RageSoundDriver_SDL.h"

/*
 * Copyright (c) 2002 by the person(s) listed below.  All rights reserved.
 *
 * Glenn Maynard
 */
