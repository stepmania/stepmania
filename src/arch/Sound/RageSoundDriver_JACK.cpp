#include "global.h"
#include "RageSoundDriver_JACK.h"
#include "RageLog.h"
#include "RageUtil.h"
#include "PrefsManager.h"
#include "ProductInfo.h"

#include <cstdint>
#include <vector>


REGISTER_SOUND_DRIVER_CLASS( JACK );

RageSoundDriver_JACK::RageSoundDriver_JACK() :
	RageSoundDriver()
{
	client = nullptr;
	port_l = nullptr;
	port_r = nullptr;
}

RageSoundDriver_JACK::~RageSoundDriver_JACK()
{
	// If Init failed, it cleaned up already and set client to nullptr
	if (client == nullptr)
		return;

	// Clean up and shut down client
	jack_deactivate(client);
	jack_port_unregister(client, port_r);
	jack_port_unregister(client, port_l);
	jack_client_close(client);
}

RString RageSoundDriver_JACK::Init()
{
	jack_status_t status;
	RString error;

	// Open JACK client and call it "StepMania" or whatever
	client = jack_client_open(PRODUCT_FAMILY, JackNoStartServer, &status);
	if (client == nullptr)
		return "Couldn't connect to JACK server";

	sample_rate = jack_get_sample_rate(client);
	LOG->Trace("JACK connected at %u Hz", sample_rate);

	// Start this before callbacks
	StartDecodeThread();

	// Set callback for processing audio
	if (jack_set_process_callback(client, ProcessTrampoline, this))
	{
		error = "Couldn't set JACK process callback";
		goto out_close;
	}

	if (jack_set_sample_rate_callback(client, SampleRateTrampoline, this))
	{
		error = "Couldn't set JACK sample-rate callback";
		goto out_close;
	}

	// TODO Set a jack_on_shutdown callback as well?  Probably just stop
	// caring about sound altogether if that happens.

	// Create output ports
	port_l = jack_port_register(client, "out_l", JACK_DEFAULT_AUDIO_TYPE,
			JackPortIsOutput, 0);
	if (port_l == nullptr)
	{
		error = "Couldn't create JACK port out_l";
		goto out_close;
	}

	port_r = jack_port_register(client, "out_r", JACK_DEFAULT_AUDIO_TYPE,
			JackPortIsOutput, 0);
	if (port_r == nullptr)
	{
		error = "Couldn't create JACK port out_r";
		goto out_unreg_l;
	}

	// Go!
	if (jack_activate(client))
	{
		error = "Couldn't activate JACK client";
		goto out_unreg_r;
	}

	error = ConnectPorts();
	if (!error.empty())
		// Eh. Not fatal. JACK *is* running and we successfully created
		// our source ports, so it's unlkely any other driver will
		// function.
		LOG->Warn( "RageSoundDriver_JACK: Couldn't connect ports: %s", error.c_str() );

	// Success!
	LOG->Trace("JACK sound driver started successfully");
	return RString();


	// Not success!
out_unreg_r:
	jack_port_unregister(client, port_r);
out_unreg_l:
	jack_port_unregister(client, port_l);
out_close:
	jack_client_close(client);
	client = nullptr;
	return error;
}

RString RageSoundDriver_JACK::ConnectPorts()
{
	std::vector<RString> portNames;
	split(PREFSMAN->m_iSoundDevice.Get(), ",", portNames, true);

	const char *port_out_l = nullptr, *port_out_r = nullptr;
	const char **ports = nullptr;
	if( portNames.size() == 0 )
	{
		// The user has NOT specified any ports to connect to. Search
		// for all physical sinks and use the first two.
		ports = jack_get_ports( client, nullptr, nullptr, JackPortIsInput | JackPortIsPhysical );
		if( ports == nullptr )
			return "Couldn't get JACK ports";
		if( ports[0] == nullptr )
		{
			jack_free( ports );
			return "No physical sinks!";
		}
		port_out_l = ports[0];

		if( ports[1] == nullptr )
			// Only one physical sink. We're going mono!
			port_out_r = ports[0];
		else
			port_out_r = ports[1];
	}
	else
	{
		// The user has specified ports to connect to. Loop through
		// them to find two that are valid, then use them. If we find
		// only one that is valid, connect both channels to it.
		// Use jack_port_by_name to ensure ports exist, then
		// jack_port_name to use their canonical name.  (I'm not sure
		// if that second step is necessary, I've seen something about
		// "aliases" in the docs.)
		for ( RString const &portName : portNames )
		{
			jack_port_t *out = jack_port_by_name( client, portName );
			// Make sure the port is a sink.
			if( ! ( jack_port_flags( out ) & JackPortIsInput ) )
				continue;

			if( out != nullptr )
			{
				if( port_out_l == nullptr )
					port_out_l = jack_port_name( out );
				else
				{
					port_out_r = jack_port_name( out );
					break;
				}
			}
		}
		if( port_out_l == nullptr )
			return "All specified sinks are invalid.";

		if( port_out_r == nullptr )
			// Only found one valid sink. Going mono!
			port_out_r = port_out_l;
	}

	RString ret = RString();

	if( jack_connect( client, jack_port_name(port_l), port_out_l ) != 0 )
		ret = "Couldn't connect left JACK port";
	else if( jack_connect( client, jack_port_name(port_r), port_out_r ) != 0 )
		ret = "Couldn't connect right JACK port";

	if( ports != nullptr )
		jack_free( ports );

	return ret;
}

std::int64_t RageSoundDriver_JACK::GetPosition() const
{
	return jack_frame_time(client);
}

int RageSoundDriver_JACK::GetSampleRate() const
{
	// For now, let's pretend there isn't a race condition between this and
	// SampleRateCallback().
	return sample_rate;
}

int RageSoundDriver_JACK::ProcessCallback(jack_nframes_t nframes)
{
	jack_default_audio_sample_t *bufs[2];

	bufs[0] = (jack_default_audio_sample_t *) jack_port_get_buffer(port_l, nframes);
	bufs[1] = (jack_default_audio_sample_t *) jack_port_get_buffer(port_r, nframes);

	jack_time_t now = jack_last_frame_time(client);

	MixDeinterlaced( bufs, 2, nframes, now, now + nframes * 2 );

	return 0;
}

int RageSoundDriver_JACK::SampleRateCallback(jack_nframes_t nframes)
{
	// For now, let's pretend there isn't a race condition between this and
	// GetSampleRate().
	sample_rate = jack_get_sample_rate(client);
	return 0;
}

// Static callback trampoline
int RageSoundDriver_JACK::ProcessTrampoline(jack_nframes_t nframes, void *arg)
{
	return ((RageSoundDriver_JACK *) arg)->ProcessCallback(nframes);
}

int RageSoundDriver_JACK::SampleRateTrampoline(jack_nframes_t nframes, void *arg)
{
	return ((RageSoundDriver_JACK *) arg)->SampleRateCallback(nframes);
}

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
