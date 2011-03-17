#include "global.h"

#include <dlfcn.h>

#define ALSA_PCM_NEW_HW_PARAMS_API
#define ALSA_PCM_NEW_SW_PARAMS_API
#include <alsa/asoundlib.h>

static void *Handle = NULL;

#include "RageUtil.h"
#include "ALSA9Dynamic.h"

/* foo_f dfoo = NULL */
#define FUNC(ret, name, proto) name##_f d##name = NULL
#include "ALSA9Functions.h"
#undef FUNC

static const RString lib = "libasound.so.2";
RString LoadALSA()
{
	/* If /proc/asound/ doesn't exist, chances are we're on an OSS system.  We shouldn't
	 * touch ALSA at all, since many OSS systems have old, broken versions of ALSA lying
	 * around; we're likely to crash if we go near it.  Do this first, before loading
	 * the ALSA library, since making any ALSA calls may load ALSA core modules.
	 *
	 * It's vaguely possible that a module autoloader would load the entire ALSA module set
	 * on use, and this would prevent that from happening.  I don't know if anyone actually
	 * does that, though: they're often configured to load snd (the core module) if ALSA
	 * devices are accessed, but hardware drivers are typically loaded on boot. */
	if( !IsADirectory("/rootfs/proc/asound/") )
		return "/proc/asound/ does not exist";

	ASSERT( Handle == NULL );

	Handle = dlopen( lib, RTLD_NOW );
	if( Handle == NULL )
		return ssprintf("dlopen(%s): %s", lib.c_str(), dlerror());

	RString error;
	/* Eww.  The "new" HW and SW API functions are really prefixed by __,
	 * eg. __snd_pcm_hw_params_set_rate_near. */
#define FUNC(ret, name, proto) \
	d##name = (name##_f) dlsym(Handle, "__" #name); \
	if( !d##name ) { \
		d##name = (name##_f) dlsym(Handle, #name); \
		if( !d##name ) { \
			error="Couldn't load symbol " #name; \
			goto error; \
		} \
	}
#include "ALSA9Functions.h"
#undef FUNC

	return "";
error:
	UnloadALSA();
	return error;
}

void UnloadALSA()
{
	if( Handle )
		dlclose( Handle );
	Handle = NULL;
#define FUNC(ret, name, proto) d##name = NULL;
#include "ALSA9Functions.h"
#undef FUNC
}

/*
 * (c) 2003-2004 Glenn Maynard
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
