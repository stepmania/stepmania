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

CString LoadALSA();
void UnloadALSA();

#endif
