#ifndef DSOUND_HELPERS
#define DSOUND_HELPERS 1

#define DIRECTSOUND_VERSION 0x0800
#include <mmsystem.h>
#include <dsound.h>
#include "SDL.h"

/* Create a DS buffer of the given format. */
IDirectSoundBuffer8 *CreateBuf(IDirectSound8 *ds8, 
							  int channels, int samplerate, int bits,
							  Uint32 buffersize, bool Hardware);

#endif

/*
 * Copyright (c) 2002 by the person(s) listed below.  All rights reserved.
 *
 * Glenn Maynard
 */
