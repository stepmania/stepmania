#include "global.h"
#include "RageLog.h"
#include "RageUtil.h"
#include "ALSA9Helpers.h"
#include "SDL_utils.h"

/* int err; must be defined before using this macro */
#define ALSA_CHECK(x) \
        if ( err < 0 ) { LOG->Info("ALSA9: %s: %s (%i)", x, snd_strerror(err), err); return false; }
#define ALSA_ASSERT(x) \
        if (err < 0) { LOG->Trace("RageSound_ALSA9: ASSERT %s: %s (%i)", x, snd_strerror(err), err); }

bool Alsa9Buf::SetHWParams()
{
	int err;

	if( snd_pcm_state(pcm) == SND_PCM_STATE_PREPARED )
		snd_pcm_drop( pcm );

	if( snd_pcm_state(pcm) != SND_PCM_STATE_OPEN )
	{
		/* Reset the stream to SND_PCM_STATE_OPEN. */
		err = snd_pcm_hw_free( pcm );
		ALSA_ASSERT("snd_pcm_hw_free");
	}
//	RAGE_ASSERT_M( snd_pcm_state(pcm) == SND_PCM_STATE_OPEN, ssprintf("(%s)", snd_pcm_state_name(snd_pcm_state(pcm))) );

	/* allocate the hardware parameters structure */
	snd_pcm_hw_params_t *hwparams;
	snd_pcm_hw_params_alloca( &hwparams );

	err = snd_pcm_hw_params_any(pcm, hwparams);
	ALSA_CHECK("snd_pcm_hw_params_any");

	/* set to mmap mode (with channels interleaved) */
	err = snd_pcm_hw_params_set_access(pcm, hwparams, SND_PCM_ACCESS_MMAP_INTERLEAVED);
	ALSA_CHECK("snd_pcm_hw_params_set_access");

	/* set PCM format (signed 16bit, little endian) */
	err = snd_pcm_hw_params_set_format(pcm, hwparams, SND_PCM_FORMAT_S16_LE);
	ALSA_CHECK("snd_pcm_hw_params_set_format");

	/* set number of channels */
	err = snd_pcm_hw_params_set_channels(pcm, hwparams, 2);
	ALSA_CHECK("snd_pcm_hw_params_set_channels");

	if( samplerate != Alsa9Buf::DYNAMIC_SAMPLERATE )
	{
		unsigned int rate = samplerate;
		err = snd_pcm_hw_params_set_rate_near(pcm, hwparams, &rate, 0);
		ALSA_CHECK("snd_pcm_hw_params_set_rate_near");

		if( (int) rate != samplerate )
			LOG->Warn("Alsa9Buf::SetHWParams: Couldn't get %ihz (got %ihz instead)", samplerate, rate);
	}

	/* write the hardware parameters to the device */
	err = snd_pcm_hw_params(pcm, hwparams);
	ALSA_CHECK("snd_pcm_hw_params");

	return true;
}

void Alsa9Buf::GetSoundCardDebugInfo()
{
	static bool done = false;	
	if( done )
		return;
	done = true;

	int card = -1;
	while( snd_card_next( &card ) >= 0 && card >= 0 )
	{
		const CString id = ssprintf( "hw:%d", card );
		snd_ctl_t *handle;
		int err;
		err = snd_ctl_open( &handle, id, 0 );
		ALSA_ASSERT("snd_pcm_sw_params_set_stop_threshold");
		if ( err < 0 )
		{
			LOG->Info( "Couldn't open card #%i (\"%s\"): %s", card, id.c_str(), snd_strerror(err) );
			continue;
		}

		snd_ctl_card_info_t *info;
		snd_ctl_card_info_alloca(&info);
		err = snd_ctl_card_info( handle, info );
		if ( err < 0 )
		{
			LOG->Info( "Couldn't get card info for card #%i (\"%s\"): %s", card, id.c_str(), snd_strerror(err) );
			snd_ctl_close( handle );
			continue;
		}

		int dev = -1;
		while ( snd_ctl_pcm_next_device( handle, &dev ) >= 0 && dev >= 0 )
		{
			snd_pcm_info_t *pcminfo;
			snd_pcm_info_alloca(&pcminfo);
			snd_pcm_info_set_device(pcminfo, dev);
			snd_pcm_info_set_stream(pcminfo, SND_PCM_STREAM_PLAYBACK);

			err = snd_ctl_pcm_info(handle, pcminfo);
			if ( err < 0 )
			{
				if (err != -ENOENT)
					LOG->Info("snd_ctl_pcm_info(%i) (%s) failed: %s", card, id.c_str(), snd_strerror(err));
				continue;
			}

			LOG->Info( "ALSA Driver: %i: %s [%s], device %i: %s [%s], %i/%i subdevices avail",
					card, snd_ctl_card_info_get_name(info), snd_ctl_card_info_get_id(info), dev,
					snd_pcm_info_get_id(pcminfo), snd_pcm_info_get_name(pcminfo),
					snd_pcm_info_get_subdevices_avail(pcminfo),
					snd_pcm_info_get_subdevices_count(pcminfo) );

			snd_ctl_close(handle);
		}
	}

	if( card == 0 )
		LOG->Info( "No ALSA sound cards were found.");
}

void Alsa9Buf::ErrorHandler(const char *file, int line, const char *function, int err, const char *fmt, ...)
{
/* NOP */
}

Alsa9Buf::Alsa9Buf( hw hardware, int channels_, int samplerate_ )
{
	GetSoundCardDebugInfo();
		
	snd_lib_error_set_handler( ErrorHandler );
	
	channels = channels_;
	samplerate = samplerate_;
	samplebits = 16;
	last_cursor_pos = 0;
	
	/* Open the device. */
	int err;
//	err = snd_pcm_open( &pcm, "dmix", SND_PCM_STREAM_PLAYBACK, SND_PCM_NONBLOCK );
	err = snd_pcm_open( &pcm, "default", SND_PCM_STREAM_PLAYBACK, SND_PCM_NONBLOCK );
	if (err < 0)
		RageException::ThrowNonfatal("snd_pcm_open: %s", snd_strerror(err));

	if( !SetHWParams() )
	{
		snd_pcm_close(pcm);
		RageException::ThrowNonfatal( "SetHWParams failed" );
	}

	snd_pcm_sw_params_t *swparams;
	snd_pcm_sw_params_alloca( &swparams );
	snd_pcm_sw_params_current( pcm, swparams );

	xfer_align = snd_pcm_sw_params_get_xfer_align( swparams );
	
	/* Disable SND_PCM_STATE_XRUN. */
	snd_pcm_uframes_t boundary = 0;
	err = snd_pcm_sw_params_get_boundary( swparams, &boundary );
	ALSA_ASSERT("snd_pcm_sw_params_get_boundary");
	if( err == 0 )
	{
		err = snd_pcm_sw_params_set_stop_threshold( pcm, swparams, boundary );
		ALSA_ASSERT("snd_pcm_sw_params_set_stop_threshold");
		err = snd_pcm_sw_params(pcm, swparams);
		ALSA_ASSERT("snd_pcm_sw_params");
	}

	err = snd_pcm_prepare(pcm);
	ALSA_ASSERT("snd_pcm_prepare");


	/* prepare a snd_output_t for use with LOG->Trace */
	/* snd_output_t *errout = NULL;
	snd_output_buffer_open(&errout);
	snd_pcm_dump(pcm, errout);
	snd_output_flush(errout);

	char *errstring;
	snd_output_buffer_string(errout, &errstring);
	LOG->Trace("%s", errstring);
	snd_output_close( errout );
*/
	total_frames = snd_pcm_avail_update(pcm);
}

Alsa9Buf::~Alsa9Buf()
{
	snd_pcm_close(pcm);
}


int Alsa9Buf::GetNumFramesToFill( int writeahead )
{
    snd_pcm_sframes_t avail_frames = snd_pcm_avail_update(pcm);
	
	/* If this happens, we've underrun by avail_frames-total_frames frames.  If we fill in that
	 * number of frames, then we won't actually skip any music; there'll just be a delay.  Don't
	 * do that; it means the beat of the music will shift, which is much harder to recover
	 * from while playing. */
/*	if( avail_frames > total_frames )
	{
		int size = avail_frames-total_frames;
		snd_pcm_forward( pcm, size );
		LOG->Trace("ur write %i", size);
	}
*/
	
	if( avail_frames < 0 && Recover(avail_frames) )
		avail_frames = snd_pcm_avail_update(pcm);

	if( avail_frames < 0 )
	{
		LOG->Trace( "RageSoundDriver_ALSA9::GetData: snd_pcm_avail_update: %s", snd_strerror(avail_frames) );
		return 0;
	}

	const snd_pcm_sframes_t filled_frames = max( 0l, total_frames - avail_frames );

	snd_pcm_sframes_t frames_to_fill = clamp( writeahead - filled_frames, 0l, (snd_pcm_sframes_t)writeahead );
	frames_to_fill -= frames_to_fill % xfer_align;

	return frames_to_fill;
}

void Alsa9Buf::Write( const Sint16 *buffer, int frames )
{
	/* We should be able to write it all.  If we don't, treat it as an error. */
	int wrote = snd_pcm_mmap_writei( pcm, (const char *) buffer, frames );
	if( wrote < 0 )
	{
		LOG->Trace( "RageSoundDriver_ALSA9::GetData: snd_pcm_mmap_writei: %s (%i)", snd_strerror(wrote), wrote );
		return;
	}

	last_cursor_pos += wrote;
	if( wrote < frames )
		LOG->Trace("Couldn't write whole buffer? (%i < %i)\n", wrote, frames );
}



/*
 * When the play buffer underruns, subsequent writes to the buffer
 * return -EPIPE.  When this happens, call Recover() to restart playback.
 */
bool Alsa9Buf::Recover( int r )
{
	if( r == -EPIPE )
	{
		LOG->Trace("RageSound_ALSA9::Recover (prepare)");
		int err = snd_pcm_prepare(pcm);
		ALSA_ASSERT("snd_pcm_prepare (Recover)");
		return true;
	}

	if( r == -ESTRPIPE )
	{
		LOG->Trace("RageSound_ALSA9::Recover (resume)");
		int err;
		while ((err = snd_pcm_resume(pcm)) == -EAGAIN)
			SDL_Delay(10);

		ALSA_ASSERT("snd_pcm_resume (Recover)");
		return true;
	}

	return false;
}

int Alsa9Buf::GetPosition() const
{
	if( snd_pcm_state(pcm) == SND_PCM_STATE_PREPARED )
	{
		LOG->Trace("???");
		return 0;
	}

	snd_pcm_hwsync( pcm );

	/* delay is returned in frames */
	snd_pcm_sframes_t delay;
	int err = snd_pcm_delay( pcm, &delay );

	return last_cursor_pos - delay;
}

void Alsa9Buf::Reset()
{
	/* Nothing is playing.  Reset the sample count; this is just to
	 * prevent eventual overflow. */
	last_cursor_pos = 0;
}

void Alsa9Buf::Play()
{
	/* NOP.  It'll start playing when it gets some data. */
}

void Alsa9Buf::Stop()
{
	snd_pcm_drop( pcm );
	snd_pcm_prepare( pcm );
}

void Alsa9Buf::SetSampleRate(int hz)
{
	samplerate = hz;

	SetHWParams();
	
	snd_pcm_prepare( pcm );
}

