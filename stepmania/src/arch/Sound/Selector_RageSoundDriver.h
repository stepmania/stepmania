#ifndef SELECTOR_RAGE_SOUND_DRIVER_H
#define SELECTOR_RAGE_SOUND_DRIVER_H

#include "arch/arch_platform.h"

/* RageSoundDriver selector. */
#ifdef HAVE_ALSA
#include "RageSoundDriver_ALSA9.h"
#include "RageSoundDriver_ALSA9_Software.h"
#endif

#if defined(MACOSX)
#include "RageSoundDriver_CA.h"
#endif

#ifdef HAVE_DIRECTX
#include "RageSoundDriver_DSound.h"
#include "RageSoundDriver_DSound_Software.h"
#endif

#include "RageSoundDriver_Null.h"

#ifdef HAVE_OSS
#include "RageSoundDriver_OSS.h"
#endif

#if defined(WIN32)
#include "RageSoundDriver_WaveOut.h"
#endif

#endif

/*
 * (c) 2005 Ben Anderson
 * All rights reserved.
 * 
 * Permission is hereby granted, free of charge, to any person obtaining a
 * copy of this software and associated documentation files (the
 * "Software"), to deal in the Software without restriction, including
 * without limitation the rights to use, copy, modify, merge, publish,
 * distribute, and/or sell copies of the Software, and to permit persons to
 * whom the Software is furnished to do so, provided that the above
 * copyright notice(s) and this permission notice appear in all copies of
 * the Software and that both the above copyright notice(s) and this
 * permission notice appear in supporting documentation.
 * 
 * THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS
 * OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF
 * MERCHANTABILITY, FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT OF
 * THIRD PARTY RIGHTS. IN NO EVENT SHALL THE COPYRIGHT HOLDER OR HOLDERS
 * INCLUDED IN THIS NOTICE BE LIABLE FOR ANY CLAIM, OR ANY SPECIAL INDIRECT
 * OR CONSEQUENTIAL DAMAGES, OR ANY DAMAGES WHATSOEVER RESULTING FROM LOSS
 * OF USE, DATA OR PROFITS, WHETHER IN AN ACTION OF CONTRACT, NEGLIGENCE OR
 * OTHER TORTIOUS ACTION, ARISING OUT OF OR IN CONNECTION WITH THE USE OR
 * PERFORMANCE OF THIS SOFTWARE.
 */
