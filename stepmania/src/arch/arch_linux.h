#ifndef LINUX_ARCH_H
#define LINUX_ARCH_H

#include "global.h"

#ifdef HAVE_ALSA
#  include "Sound/RageSoundDriver_ALSA9.h"
#endif

#include "Sound/RageSoundDriver_OSS.h"

#ifdef HAVE_GTK
#  include "LoadingWindow/LoadingWindow_Gtk.h"
#endif
#include "InputHandler/InputHandler_SDL.h"
#include "LowLevelWindow/LowLevelWindow_SDL.h"

#endif
/*
 * Copyright (c) 2002 by the person(s) listed below.  All rights reserved.
 *
 * Glenn Maynard
 */
