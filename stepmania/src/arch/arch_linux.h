#ifndef LINUX_ARCH_H
#define LINUX_ARCH_H

#ifdef HAVE_ALSA
#  include "Sound/RageSoundDriver_ALSA9.h"
#endif

#include "Sound/RageSoundDriver_OSS.h"

/* 
 * Sometimes Alsa and OSS fail to load, so
 * SDL should be a fallback, even though it
 * doesn't work perfectly.
 */
/* No, it shouldn't.  If they fail to load, we should receive a bug report
 * and fix our code, not have it silently fall back on a broken driver. -glenn */
// #include "Sound/RageSoundDriver_SDL.h"

#endif
/*
 * Copyright (c) 2002 by the person(s) listed below.  All rights reserved.
 *
 * Glenn Maynard
 */
