#ifndef SDL_ARCH_H
#define SDL_ARCH_H

/* All driver types should have a default, portable implementation, provided
 * here. */

/* Load default fallback drivers; some of these may be overridden by arch-specific drivers. */
#include "LoadingWindow/LoadingWindow_SDL.h"
/* XXX: null, none, Null--pick one */
#include "ErrorDialog/ErrorDialog_null.h"
#include "ArchHooks/ArchHooks_none.h"
#include "Sound/RageSoundDriver_Null.h"
#include "LowLevelWindow/LowLevelWindow_SDL.h"
#include "InputHandler/InputHandler_SDL.h"

#endif

/*
 * Copyright (c) 2002 by the person(s) listed below.  All rights reserved.
 *
 * Glenn Maynard
 */
