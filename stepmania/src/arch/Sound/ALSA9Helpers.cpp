#include "global.h"
#include "RageLog.h"
#include "RageUtil.h"
#include "ALSA9Helpers.h"
#include "SDL_utils.h"
#include "ALSA9Dynamic.h"

/* int err; must be defined before using this macro */
#define ALSA_CHECK(x) \
       if ( err < 0 ) { LOG->Info("ALSA: %s: %s", x, dsnd_strerror(err)); return false; }
#define ALSA_ASSERT(x) \
        if (err < 0) { LOG->Warn("ALSA: %s: %s", x, dsnd_strerror(err)); }

/* If the given sample rate can be used, return it.  Otherwise, return the
 * samplerate to use instead. */
unsigned Alsa9Buf::FindSampleRate( unsigned rate )
{
	snd_pcm_hw_params_t *testhw;
	dsnd_pcm_hw_params_alloca( &testhw );
	dsnd_pcm_hw_params_any( pcm, testhw );

	int err = dsnd_pcm_hw_params_set_rate_near(pcm, testhw, &rate, 0);
	if( err >= 0 )
		return rate;

	return 0;
}

bool Alsa9Buf::SetHWParams()
{
	int err;

	if( dsnd_pcm_state(pcm) == SND_PCM_STATE_PREPARED )
		dsnd_pcm_drop( pcm );

	if( dsnd_pcm_state(pcm) != SND_PCM_STATE_OPEN )
	{
		/* Reset the stream to SND_PCM_STATE_OPEN. */
		err = dsnd_pcm_hw_free( pcm );
		ALSA_ASSERT("dsnd_pcm_hw_free");
	}
//	RAGE_ASSERT_M( dsnd_pcm_state(pcm) == SND_PCM_STATE_OPEN, ssprintf("(%s)", dsnd_pcm_state_name(dsnd_pcm_state(pcm))) );

	/* allocate the hardware parameters structure */
	snd_pcm_hw_params_t *hwparams;
	dsnd_pcm_hw_params_alloca( &hwparams );

	err = dsnd_pcm_hw_params_any(pcm, hwparams);
	ALSA_CHECK("dsnd_pcm_hw_params_any");

	/* set to mmap mode (with channels interleaved) */
	err = dsnd_pcm_hw_params_set_access(pcm, hwparams, SND_PCM_ACCESS_MMAP_INTERLEAVED);
	ALSA_CHECK("dsnd_pcm_hw_params_set_access");

	/* set PCM format (signed 16bit, little endian) */
	err = dsnd_pcm_hw_params_set_format(pcm, hwparams, SND_PCM_FORMAT_S16_LE);
	ALSA_CHECK("dsnd_pcm_hw_params_set_format");

	/* set number of channels */
	err = dsnd_pcm_hw_params_set_channels(pcm, hwparams, 2);
	ALSA_CHECK("dsnd_pcm_hw_params_set_channels");

	/* Set the sample rate. */
	unsigned int rate = samplerate;
	err = dsnd_pcm_hw_params_set_rate_near(pcm, hwparams, &rate, 0);
	ALSA_CHECK("dsnd_pcm_hw_params_set_rate_near");

	if( samplerate_set_explicitly && (int) rate != samplerate )
		LOG->Warn("Alsa9Buf::SetHWParams: Couldn't get %ihz (got %ihz instead)", samplerate, rate);

	snd_pcm_uframes_t buffersize = 1024*32;
	err = dsnd_pcm_hw_params_set_buffer_size_near( pcm, hwparams, &buffersize );
	ALSA_CHECK("dsnd_pcm_hw_params_set_buffer_size_near");

	/* write the hardware parameters to the device */
	err = dsnd_pcm_hw_params( pcm, hwparams );
	ALSA_CHECK("dsnd_pcm_hw_params");

	return true;
}

bool Alsa9Buf::SetSWParams()
{
	snd_pcm_sw_params_t *swparams;
	dsnd_pcm_sw_params_alloca( &swparams );
	dsnd_pcm_sw_params_current( pcm, swparams );

	int err = dsnd_pcm_sw_params_get_xfer_align( swparams, &xfer_align );
	ALSA_ASSERT("dsnd_pcm_sw_params_get_xfer_align");

	/* If this fails, we might have bound dsnd_pcm_sw_params_get_xfer_align to
	 * the old SW API. */
	ASSERT( err <= 0 );
	
	/* Disable SND_PCM_STATE_XRUN. */
	snd_pcm_uframes_t boundary = 0;
	err = dsnd_pcm_sw_params_get_boundary( swparams, &boundary );
	ALSA_ASSERT("dsnd_pcm_sw_params_get_boundary");
	if( err == 0 )
	{
		err = dsnd_pcm_sw_params_set_stop_threshold( pcm, swparams, boundary );
		ALSA_ASSERT("dsnd_pcm_sw_params_set_stop_threshold");
		err = dsnd_pcm_sw_params(pcm, swparams);
		ALSA_ASSERT("dsnd_pcm_sw_params");
	}

	err = dsnd_pcm_prepare(pcm);
	ALSA_ASSERT("dsnd_pcm_prepare");

	return true;
}

void Alsa9Buf::GetSoundCardDebugInfo()
{
	static bool done = false;	
	if( done )
		return;
	done = true;

	if( DoesFileExist("/proc/asound/version") )
	{
		const CString ver = GetRedirContents("/proc/asound/version");
		LOG->Info( "ALSA: %s", ver.c_str() );
	}
	
	int card = -1;
	while( dsnd_card_next( &card ) >= 0 && card >= 0 )
	{
		const CString id = ssprintf( "hw:%d", card );
		snd_ctl_t *handle;
		int err;
		err = dsnd_ctl_open( &handle, id, 0 );
		if ( err < 0 )
		{
			LOG->Info( "Couldn't open card #%i (\"%s\") to probe: %s", card, id.c_str(), dsnd_strerror(err) );
			continue;
		}

		snd_ctl_card_info_t *info;
		dsnd_ctl_card_info_alloca(&info);
		err = dsnd_ctl_card_info( handle, info );
		if ( err < 0 )
		{
			LOG->Info( "Couldn't get card info for card #%i (\"%s\"): %s", card, id.c_str(), dsnd_strerror(err) );
			dsnd_ctl_close( handle );
			continue;
		}

		int dev = -1;
		while ( dsnd_ctl_pcm_next_device( handle, &dev ) >= 0 && dev >= 0 )
		{
			snd_pcm_info_t *pcminfo;
			dsnd_pcm_info_alloca(&pcminfo);
			dsnd_pcm_info_set_device(pcminfo, dev);
			dsnd_pcm_info_set_stream(pcminfo, SND_PCM_STREAM_PLAYBACK);

			err = dsnd_ctl_pcm_info(handle, pcminfo);
			if ( err < 0 )
			{
				if (err != -ENOENT)
					LOG->Info("dsnd_ctl_pcm_info(%i) (%s) failed: %s", card, id.c_str(), dsnd_strerror(err));
				continue;
			}

			LOG->Info( "ALSA Driver: %i: %s [%s], device %i: %s [%s], %i/%i subdevices avail",
					card, dsnd_ctl_card_info_get_name(info), dsnd_ctl_card_info_get_id(info), dev,
					dsnd_pcm_info_get_id(pcminfo), dsnd_pcm_info_get_name(pcminfo),
					dsnd_pcm_info_get_subdevices_avail(pcminfo),
					dsnd_pcm_info_get_subdevices_count(pcminfo) );

		}
		dsnd_ctl_close(handle);
	}

	if( card == 0 )
		LOG->Info( "No ALSA sound cards were found.");
}

void Alsa9Buf::ErrorHandler(const char *file, int line, const char *function, int err, const char *fmt, ...)
{
/* NOP */
}

Alsa9Buf::Alsa9Buf( hw hardware, int channels_ )
{
	GetSoundCardDebugInfo();
		
	dsnd_lib_error_set_handler( ErrorHandler );
	
	channels = channels_;
	samplerate = 44100;
	samplebits = 16;
	last_cursor_pos = 0;
	samplerate_set_explicitly = false;

	/* Open the device. */
	int err;
//	err = dsnd_pcm_open( &pcm, "dmix", SND_PCM_STREAM_PLAYBACK, SND_PCM_NONBLOCK );
	err = dsnd_pcm_open( &pcm, "hw:0", SND_PCM_STREAM_PLAYBACK, SND_PCM_NONBLOCK );
	if (err < 0)
		RageException::ThrowNonfatal("dsnd_pcm_open: %s", dsnd_strerror(err));

	if( !SetHWParams() )
	{
		CHECKPOINT;
		dsnd_pcm_close(pcm);
		CHECKPOINT;
		RageException::ThrowNonfatal( "SetHWParams failed" );
	}

	SetSWParams();
	total_frames = dsnd_pcm_avail_update(pcm);
}

Alsa9Buf::~Alsa9Buf()
{
	dsnd_pcm_close(pcm);
}


/* Don't fill the buffer any more than than "writeahead".  Don't write any
 * more than "chunksize" at a time. */
int Alsa9Buf::GetNumFramesToFill( snd_pcm_sframes_t writeahead, snd_pcm_sframes_t chunksize )
{
    snd_pcm_sframes_t avail_frames = dsnd_pcm_avail_update(pcm);
	
	if( avail_frames > total_frames )
	{
		/* underrun */
		const int size = avail_frames-total_frames;
		LOG->Trace("underrun (%i frames)", size);
		int large_skip_threshold = 2 * samplerate;

		/* For small underruns, ignore them.  We'll return the maximum writeahead and ALSA will
		 * just discard the data.  GetPosition will return consistent values during this time,
		 * so arrows will continue to scroll smoothly until the music catches up. */
		if( size >= large_skip_threshold )
		{
			/* It's a large skip.  Catch up.  If we fall too far behind, the sound thread will
			 * be decoding as fast as it can, which will steal too many cycles from the rendering
			 * thread. */
			dsnd_pcm_forward( pcm, size );
		}
	}
	
	if( avail_frames < 0 )
		avail_frames = dsnd_pcm_avail_update(pcm);

	if( avail_frames < 0 )
	{
		LOG->Trace( "RageSoundDriver_ALSA9::GetData: dsnd_pcm_avail_update: %s", dsnd_strerror(avail_frames) );
		return 0;
	}

	const snd_pcm_sframes_t filled_frames = max( 0l, total_frames - avail_frames );

	snd_pcm_sframes_t frames_to_fill = clamp( writeahead - filled_frames, 0l, (snd_pcm_sframes_t)writeahead );
	frames_to_fill = min( frames_to_fill, chunksize );
	
	frames_to_fill -= frames_to_fill % xfer_align;

	return frames_to_fill;
}

void Alsa9Buf::Write( const Sint16 *buffer, int frames )
{
	/* We should be able to write it all.  If we don't, treat it as an error. */
	int wrote = dsnd_pcm_mmap_writei( pcm, (const char *) buffer, frames );
	if( wrote < 0 )
	{
		LOG->Trace( "RageSoundDriver_ALSA9::GetData: dsnd_pcm_mmap_writei: %s (%i)", dsnd_strerror(wrote), wrote );
		return;
	}

	last_cursor_pos += wrote;
	if( wrote < frames )
		LOG->Trace("Couldn't write whole buffer? (%i < %i)", wrote, frames );
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
		int err = dsnd_pcm_prepare(pcm);
		ALSA_ASSERT("dsnd_pcm_prepare (Recover)");
		return true;
	}

	if( r == -ESTRPIPE )
	{
		LOG->Trace("RageSound_ALSA9::Recover (resume)");
		int err;
		while ((err = dsnd_pcm_resume(pcm)) == -EAGAIN)
			SDL_Delay(10);

		ALSA_ASSERT("dsnd_pcm_resume (Recover)");
		return true;
	}

	return false;
}

int64_t Alsa9Buf::GetPosition() const
{
	if( dsnd_pcm_state(pcm) == SND_PCM_STATE_PREPARED )
		return last_cursor_pos;

	dsnd_pcm_hwsync( pcm );

	/* delay is returned in frames */
	snd_pcm_sframes_t delay;
	int err = dsnd_pcm_delay( pcm, &delay );
	ALSA_ASSERT("dsnd_pcm_delay");

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
	dsnd_pcm_drop( pcm );
	dsnd_pcm_prepare( pcm );
	last_cursor_pos = 0;
}

void Alsa9Buf::SetSampleRate(int hz)
{
	samplerate = hz;
	samplerate_set_explicitly = true;

	if( !SetHWParams() )
	{
		/*
		 * If this fails, we're no longer set up; if we call SW param calls,
		 * ALSA will assert out on us (instead of gracefully returning an error).
		 *
		 * If we fail here, it means we set up the initial stream, but can't
		 * configure it to the sample rate we want.  This happened on a CS46xx
		 * with an old ALSA version, at least: snd_pcm_hw_params failed
		 * with ENOMEM.  It set up only 10 44.1khz streams; it may have been
		 * trying to increase one to 48khz and, for some reason, that needed
		 * more card memory.  (I've tried to work around that by setting up
		 * streams as 48khz to begin with, so we set it up as the maximum
		 * to begin with.)
		 */
		FAIL_M( ssprintf("SetHWParams(%i) failed", hz) );
	}

	SetSWParams();
}

