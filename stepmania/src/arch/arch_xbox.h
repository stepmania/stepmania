#ifndef XBOX_ARCH_H
#define XBOX_ARCH_H

/* Load drivers for Xbox. */
#include "ErrorDialog/ErrorDialog_null.h"
#include "ArchHooks/ArchHooks_none.h"

/* Err, is SDL input working on Xbox?  arch_default comments indicated no.
 * (Best to use a custom driver for threaded input, anyway.) */
#include "InputHandler/InputHandler_SDL.h"

#include "Sound/RageSoundDriver_DSound.h"

#define SUPPORT_D3D
#undef SUPPORT_OPENGL
/* Undef this if you need no SDL input. */
// #undef SUPPORT_SDL_INPUT

#endif

/*
 * Copyright (c) 2002 by the person(s) listed below.  All rights reserved.
 *
 * Glenn Maynard
 * Chris Danford
 */
