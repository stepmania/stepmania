FUNC(size_t, snd_pcm_hw_params_sizeof, (void));
FUNC(size_t, snd_pcm_sw_params_sizeof, (void));
FUNC(size_t, snd_pcm_info_sizeof, (void));
FUNC(size_t, snd_ctl_card_info_sizeof, (void));
FUNC(int, snd_ctl_card_info, (snd_ctl_t *ctl, snd_ctl_card_info_t *info));
FUNC(int, snd_card_next, (int *card));
FUNC(const char *, snd_ctl_card_info_get_id, (const snd_ctl_card_info_t *obj));
FUNC(const char *, snd_ctl_card_info_get_name, (const snd_ctl_card_info_t *obj));
FUNC(snd_pcm_state_t, snd_pcm_state, (snd_pcm_t *pcm));
FUNC(const char *,snd_strerror, (int errnum));
FUNC(int, snd_ctl_close, (snd_ctl_t *ctl));
FUNC(int, snd_ctl_open, (snd_ctl_t **ctl, const char *name, int mode));
FUNC(int, snd_lib_error_set_handler, (snd_lib_error_handler_t handler));
FUNC(int, snd_output_buffer_open, (snd_output_t **outputp));
FUNC(size_t, snd_output_buffer_string, (snd_output_t *output, char **buf));
FUNC(int, snd_output_close, (snd_output_t *output));
FUNC(int, snd_output_flush, (snd_output_t *output));
FUNC(snd_pcm_sframes_t, snd_pcm_avail_update, (snd_pcm_t *pcm));
FUNC(int, snd_pcm_close, (snd_pcm_t *pcm));
FUNC(int, snd_pcm_delay, (snd_pcm_t *pcm, snd_pcm_sframes_t *delayp));
FUNC(int, snd_pcm_drop, (snd_pcm_t *pcm));
FUNC(int, snd_pcm_dump, (snd_pcm_t *pcm, snd_output_t *out));
FUNC(snd_pcm_sframes_t, snd_pcm_forward, (snd_pcm_t *pcm, snd_pcm_uframes_t frames));
FUNC(int, snd_pcm_hw_free, (snd_pcm_t *pcm));
FUNC(int, snd_pcm_hw_params, (snd_pcm_t *pcm, snd_pcm_hw_params_t *params));
FUNC(int, snd_pcm_hw_params_any, (snd_pcm_t *pcm, snd_pcm_hw_params_t *params));
FUNC(int, snd_pcm_hw_params_set_access, (snd_pcm_t *pcm, snd_pcm_hw_params_t *params, snd_pcm_access_t access));
FUNC(int, snd_pcm_hw_params_set_channels, (snd_pcm_t *pcm, snd_pcm_hw_params_t *params, unsigned int val));
FUNC(int, snd_pcm_hw_params_set_format, (snd_pcm_t *pcm, snd_pcm_hw_params_t *params, snd_pcm_format_t val));
FUNC(int, snd_pcm_hw_params_set_rate_near, (snd_pcm_t *pcm, snd_pcm_hw_params_t *params, unsigned int *val, int *dir));
FUNC(int, snd_pcm_hw_params_set_buffer_size_near, (snd_pcm_t *pcm, snd_pcm_hw_params_t *params, snd_pcm_uframes_t *val));
FUNC(int, snd_pcm_hw_params_set_period_size_near, (snd_pcm_t *pcm, snd_pcm_hw_params_t *params, snd_pcm_uframes_t *val, int *dir));
FUNC(int, snd_pcm_status, (snd_pcm_t *pcm, snd_pcm_status_t *status));
FUNC(snd_pcm_uframes_t, snd_pcm_status_get_avail, (const snd_pcm_status_t *obj));
FUNC(size_t, snd_pcm_status_sizeof, (void));
FUNC(int, snd_pcm_hwsync, (snd_pcm_t *pcm));
FUNC(int, snd_ctl_pcm_next_device, (snd_ctl_t *ctl, int *device));
FUNC(int, snd_ctl_pcm_info, (snd_ctl_t *ctl, snd_pcm_info_t * info));
FUNC(const char *,snd_pcm_info_get_id, (const snd_pcm_info_t *obj));
FUNC(const char *, snd_pcm_info_get_name, (const snd_pcm_info_t *obj));
FUNC(unsigned int, snd_pcm_info_get_subdevices_avail, (const snd_pcm_info_t *obj));
FUNC(unsigned int, snd_pcm_info_get_subdevices_count, (const snd_pcm_info_t *obj));
FUNC(void, snd_pcm_info_set_device, (snd_pcm_info_t *obj, unsigned int val));
FUNC(void, snd_pcm_info_set_stream, (snd_pcm_info_t *obj, snd_pcm_stream_t val));
FUNC(snd_pcm_sframes_t, snd_pcm_mmap_writei, (snd_pcm_t *pcm, const void *buffer, snd_pcm_uframes_t size));
FUNC(int, snd_pcm_open, (snd_pcm_t **pcm, const char *name, snd_pcm_stream_t stream, int mode));
FUNC(int, snd_pcm_prepare, (snd_pcm_t *pcm));
FUNC(int, snd_pcm_resume, (snd_pcm_t *pcm));
FUNC(int, snd_pcm_wait, (snd_pcm_t *pcm, int timeout));
FUNC(int, snd_pcm_sw_params, (snd_pcm_t *pcm, snd_pcm_sw_params_t *params));
FUNC(int, snd_pcm_sw_params_current, (snd_pcm_t *pcm, snd_pcm_sw_params_t *params));
FUNC(int, snd_pcm_sw_params_get_boundary, (const snd_pcm_sw_params_t *params, snd_pcm_uframes_t *val));
FUNC(int, snd_pcm_sw_params_set_xfer_align, (snd_pcm_t *pcm, snd_pcm_sw_params_t *params, snd_pcm_uframes_t val));
FUNC(int, snd_pcm_sw_params_set_stop_threshold, (snd_pcm_t *pcm, snd_pcm_sw_params_t *params, snd_pcm_uframes_t val));
FUNC(int, snd_pcm_sw_params_get_avail_min, (snd_pcm_sw_params_t *params, snd_pcm_uframes_t *val));
FUNC(int, snd_pcm_sw_params_set_avail_min, (snd_pcm_t *pcm, snd_pcm_sw_params_t *params, snd_pcm_uframes_t val));

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
