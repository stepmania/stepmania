#ifndef LINUX_ARCH_H
#define LINUX_ARCH_H

#ifdef HAVE_ALSA
#include "Sound/RageSoundDriver_ALSA9.h"
#endif

#ifdef HAVE_OSS
#include "Sound/RageSoundDriver_OSS.h"
#endif

#ifdef HAVE_GTK
#include "LoadingWindow/LoadingWindow_Gtk.h"
#endif

#endif
/*
 * Copyright (c) 2002 by the person(s) listed below.  All rights reserved.
 *
 * Glenn Maynard
 */
