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

static const CString lib = "libasound.so.2";
CString LoadALSA()
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
	if( !IsADirectory("/proc/asound/") )
		return "/proc/asound/ does not exist";

	ASSERT( Handle == NULL );

	Handle = dlopen( lib, RTLD_NOW );
	if( Handle == NULL )
		return ssprintf("dlopen(%s): %s", lib.c_str(), dlerror());

	CString error;
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

