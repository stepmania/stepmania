#ifndef RAGE_SOUND_JACK
#define RAGE_SOUND_JACK

#include "RageSoundDriver.h"

#include <cstdint>

#include <jack/jack.h>

#define USE_RAGE_SOUND_JACK

class RageSoundDriver_JACK: public RageSoundDriver
{
public:
	RageSoundDriver_JACK();
	~RageSoundDriver_JACK();

	RString Init();

	int GetSampleRate() const;
	std::int64_t GetPosition() const;

private:
	jack_client_t *client;
	jack_port_t *port_l;
	jack_port_t *port_r;

	int sample_rate;

	// Helper for Init()
	RString ConnectPorts();

	// JACK callbacks and trampolines
	int ProcessCallback(jack_nframes_t nframes);
	static int ProcessTrampoline(jack_nframes_t nframes, void *arg);
	int SampleRateCallback(jack_nframes_t nframes);
	static int SampleRateTrampoline(jack_nframes_t nframes, void *arg);
};

#endif

/*
 * (c) 2013 Devin J. Pohly
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
