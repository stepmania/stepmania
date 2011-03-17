#ifndef ALSA9_DYNAMIC_H

#include <alsa/asoundlib.h>

/* typedef int (*foo_f)(char c) */
#define FUNC(ret, name, proto) typedef ret (*name##_f) proto
#include "ALSA9Functions.h"
#undef FUNC

/* extern foo_f dfoo */
#define FUNC(ret, name, proto) extern name##_f d##name
#include "ALSA9Functions.h"
#undef FUNC

#define dsnd_pcm_hw_params_alloca(ptr) { assert(ptr); *ptr = (snd_pcm_hw_params_t *) alloca(dsnd_pcm_hw_params_sizeof()); memset(*ptr, 0, dsnd_pcm_hw_params_sizeof()); }
#define dsnd_pcm_sw_params_alloca(ptr) { assert(ptr); *ptr = (snd_pcm_sw_params_t *) alloca(dsnd_pcm_sw_params_sizeof()); memset(*ptr, 0, dsnd_pcm_sw_params_sizeof()); }
#define dsnd_pcm_info_alloca(ptr) { assert(ptr); *ptr = (snd_pcm_info_t *) alloca(dsnd_pcm_info_sizeof()); memset(*ptr, 0, dsnd_pcm_info_sizeof()); }
#define dsnd_ctl_card_info_alloca(ptr) { assert(ptr); *ptr = (snd_ctl_card_info_t *) alloca(dsnd_ctl_card_info_sizeof()); memset(*ptr, 0, dsnd_ctl_card_info_sizeof()); }
#define dsnd_pcm_status_alloca(ptr) do { assert(ptr); *ptr = (snd_pcm_status_t *) alloca(dsnd_pcm_status_sizeof()); memset(*ptr, 0, dsnd_pcm_status_sizeof()); } while (0)

RString LoadALSA();
void UnloadALSA();

#endif

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
