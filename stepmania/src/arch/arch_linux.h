#ifndef LINUX_ARCH_H
#define LINUX_ARCH_H

#ifdef HAVE_ALSA
#include "Sound/RageSoundDriver_ALSA9.h"
#include "Sound/RageSoundDriver_ALSA9_Software.h"
#endif

#ifdef HAVE_OSS
#include "Sound/RageSoundDriver_OSS.h"
#endif

#ifdef HAVE_GTK
#include "LoadingWindow/LoadingWindow_Gtk.h"
#endif

/* Load this even if we have GTK, since we can fall back if GTK is missing. */
#include "LoadingWindow/LoadingWindow_SDL.h"

#include "ArchHooks/ArchHooks_Unix.h"

#include "MemoryCard/MemoryCardDriverThreaded_Linux.h"

#endif
/*
 * Copyright (c) 2002-2003 by the person(s) listed below.  All rights reserved.
 *
 * Glenn Maynard
 */
