#include "global.h"
#include "RageLog.h"
#include "ALSA9Helpers.h"
#include "SDL_utils.h"

/* int err; must be defined before using this macro */
#define ALSA_ASSERT(x) \
        if (err < 0) { LOG->Trace("RageSound_ALSA9: ASSERT %s: %s", x, snd_strerror(err)); }

Alsa9Buf::Alsa9Buf( hw hardware, int channels_, int samplerate_, int writeahead_ )
{
	channels = channels_;
	samplerate = samplerate_;
	samplebits = 16;
	writeahead = writeahead_;
	// buffer_locked = false;
	last_cursor_pos = 0;
	//write_cursor = LastPosition = buffer_bytes_filled = 0;
	
	/* open the device */
	// if we instead use plughw: then our requested format WILL be provided
	int err;
	err = snd_pcm_open( &pcm, "hw:0,0", SND_PCM_STREAM_PLAYBACK, SND_PCM_NONBLOCK );
	if (err < 0)
		RageException::ThrowNonfatal("snd_pcm_open: %s", snd_strerror(err));

	/* allocate the hardware parameters structure */
	snd_pcm_hw_params_t *hwparams;
	err = snd_pcm_hw_params_malloc(&hwparams);
	ALSA_ASSERT("snd_pcm_hw_params_malloc");

	/* not exactly sure what this does */
	err = snd_pcm_hw_params_any(pcm, hwparams);
	ALSA_ASSERT("snd_pcm_hw_params_any");

	/* set to mmap mode (with channels interleaved) */
	err = snd_pcm_hw_params_set_access(pcm, hwparams, SND_PCM_ACCESS_MMAP_INTERLEAVED);
	ALSA_ASSERT("snd_pcm_hw_params_set_access");

	/* set PCM format (signed 16bit, little endian) */
	err = snd_pcm_hw_params_set_format(pcm, hwparams, SND_PCM_FORMAT_S16_LE);
	ALSA_ASSERT("snd_pcm_hw_params_set_format");

	/* set number of channels */
	err = snd_pcm_hw_params_set_channels(pcm, hwparams, 2);
	ALSA_ASSERT("snd_pcm_hw_params_set_channels");

	unsigned int rate = samplerate;
	err = snd_pcm_hw_params_set_rate_near(pcm, hwparams, &rate, 0);
	// check if we got the rate we desire

	/* write the hardware parameters to the device */
	err = snd_pcm_hw_params(pcm, hwparams);
	ALSA_ASSERT("snd_pcm_hw_params");

	snd_pcm_hw_params_free(hwparams);

	/* prepare the device to reveice data */
	err = snd_pcm_prepare(pcm);
	ALSA_ASSERT("snd_pcm_prepare");

	//XXX should RageException::ThrowNonfatal if something went wrong


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


int Alsa9Buf::GetNumFramesToFill()
{
    snd_pcm_sframes_t avail_frames = snd_pcm_avail_update(pcm);
	if( avail_frames < 0 && Recover(avail_frames) )
		avail_frames = snd_pcm_avail_update(pcm);

	if( avail_frames < 0 )
	{
		LOG->Trace( "RageSoundDriver_ALSA9::GetData: snd_pcm_avail_update: %s", snd_strerror(avail_frames) );
		return 0;
	}

	const snd_pcm_sframes_t filled_frames = total_frames - avail_frames;
	const snd_pcm_sframes_t frames_to_fill = (writeahead - filled_frames) & ~3;
	ASSERT( frames_to_fill <= (int)writeahead );

	//  LOG->Trace("%li avail, %li filled, %li to", avail_frames, filled_frames, frames_to_fill);
	return max( 0l, frames_to_fill );
}

void Alsa9Buf::Write( const Sint16 *buffer, int frames )
{
	/* We should be able to write it all.  If we don't, treat it as an error. */
	int wrote = snd_pcm_mmap_writei( pcm, (const char *) buffer, frames );
	if( wrote < 0 && Recover(wrote) )
		wrote = snd_pcm_mmap_writei( pcm, (const char *) buffer, frames );

	if( wrote < 0 )
	{
		LOG->Trace( "RageSoundDriver_ALSA9::GetData: snd_pcm_mmap_writei: %s", snd_strerror(wrote) );
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
	snd_pcm_status_t *status;
	snd_pcm_status_alloca(&status);

	int err = snd_pcm_status( pcm, status );

	ALSA_ASSERT("snd_pcm_status");

	snd_pcm_state_t state = snd_pcm_status_get_state( status );
	if ( state == SND_PCM_STATE_PREPARED )
		return 0;

	snd_pcm_hwsync( pcm );
	/* delay is returned in frames */
	snd_pcm_sframes_t delay = snd_pcm_status_get_delay(status);

	return last_cursor_pos - delay;
}

