#ifndef SDL_ARCH_H
#define SDL_ARCH_H

/* All driver types should have a default, portable implementation, provided
 * here. */

/* Load default fallback drivers; some of these may be overridden by arch-specific drivers. */
#include "LoadingWindow/LoadingWindow_SDL.h"
#include "ErrorDialog/ErrorDialog_stdout.h"

/* no default sound driver */

#endif

/*
 * Copyright (c) 2002 by the person(s) listed below.  All rights reserved.
 *
 * Glenn Maynard
 */
