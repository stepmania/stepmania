#include "global.h"
#include "RageSoundDriver_DSound_Software.h"
#include "DSoundHelpers.h"

#include "RageTimer.h"
#include "RageLog.h"
#include "RageSound.h"
#include "RageUtil.h"
#include "RageSoundManager.h"

#include "SDL.h"

/* samples */
const int channels = 2;
const int bytes_per_frame = channels*2; /* 16-bit */
const int samplerate = 44100;
const int buffersize_frames = 1024*4;	/* in frames */
const int buffersize = buffersize_frames * bytes_per_frame; /* in bytes */

/* We'll fill the buffer in chunks this big.  This should evenly divide the
 * buffer size. */
const int num_chunks = 8;
const int chunksize_frames = buffersize_frames / num_chunks;
const int chunksize = buffersize / num_chunks;

int RageSound_DSound_Software::MixerThread_start(void *p)
{
	((RageSound_DSound_Software *) p)->MixerThread();
	return 0;
}

void RageSound_DSound_Software::MixerThread()
{
	/* SOUNDMAN will be set once RageSoundManager's ctor returns and
	 * assigns it; we might get here before that happens, though. */
	while(!SOUNDMAN && !shutdown) Sleep(10);

	if(!SetThreadPriority(GetCurrentThread(), THREAD_PRIORITY_ABOVE_NORMAL))
		LOG->Warn(werr_ssprintf(GetLastError(), "Failed to set sound thread priority"));

	/* Fill a buffer before we start playing, so we don't play whatever junk is
	 * in the buffer. */
	while(GetData())
		;

	/* Start playing. */
	pcm->Play();

	while(!shutdown) {
		Sleep(10);
		while(GetData())
			;
	}

	/* I'm not sure why, but if we don't stop the stream now, then the thread will take
	 * 90ms (our buffer size) longer to close. */
	pcm->Stop();
}

bool RageSound_DSound_Software::GetData()
{
	LockMut(SOUNDMAN->lock);

	char *locked_buf;
	unsigned len;
	const int play_pos = pcm->GetOutputPosition();
    const int cur_play_pos = pcm->GetPosition();

	if(!pcm->get_output_buf(&locked_buf, &len, chunksize))
		return false;

	/* Silence the buffer. */
	memset(locked_buf, 0, len);

	static Sint16 *buf = NULL;
	int bufsize = buffersize_frames * channels;
	if(!buf)
	{
		buf = new Sint16[bufsize];
	}
	memset(buf, 0, bufsize*sizeof(Uint16));

	static SoundMixBuffer mix;
	mix.SetVolume( SOUNDMAN->GetMixVolume() );

	for(unsigned i = 0; i < sounds.size(); ++i)
	{
		if(sounds[i]->stopping)
			continue;

        int bytes_read = 0;
        int bytes_left = len;

		if( !sounds[i]->start_time.IsZero() )
		{
			/* If the sound is supposed to start at a time past this buffer, insert silence. */
			const int iFramesUntilThisBuffer = play_pos - cur_play_pos;
			const float fSecondsBeforeStart = -sounds[i]->start_time.Ago();
			const int iFramesBeforeStart = int(fSecondsBeforeStart * samplerate);
			const int iSilentFramesInThisBuffer = iFramesBeforeStart-iFramesUntilThisBuffer;
			const int iSilentBytesInThisBuffer = clamp( iSilentFramesInThisBuffer * bytes_per_frame, 0, bytes_left );

			memset( buf+bytes_read, 0, iSilentBytesInThisBuffer );
			bytes_read += iSilentBytesInThisBuffer;
			bytes_left -= iSilentBytesInThisBuffer;

			if( !iSilentBytesInThisBuffer )
				sounds[i]->start_time.SetZero();
		}

		/* Call the callback. */
		int got = sounds[i]->snd->GetPCM( (char *) buf+bytes_read, bytes_left, play_pos+bytes_read/bytes_per_frame );
        bytes_read += got;
        bytes_left -= got;

		mix.write( (Sint16 *) buf, bytes_read / sizeof(Sint16) );

		if( bytes_left > 0 )
		{
			/* This sound is finishing. */
			sounds[i]->stopping = true;
			sounds[i]->flush_pos = pcm->GetOutputPosition();
		}
	}

	mix.read((Sint16 *) locked_buf);

	pcm->release_output_buf(locked_buf, len);

	return true;
}


void RageSound_DSound_Software::StartMixing( RageSoundBase *snd )
{
	sound *s = new sound;
	s->snd = snd;
	s->start_time = snd->GetStartTime();

	LockMut(SOUNDMAN->lock);
	sounds.push_back(s);
}

void RageSound_DSound_Software::Update(float delta)
{
	ASSERT(SOUNDMAN);
	LockMut(SOUNDMAN->lock);

	/* StopPlaying might erase sounds out from under us, so make a copy
	 * of the sound list. */
	vector<sound *> snds = sounds;
	for(unsigned i = 0; i < snds.size(); ++i)
	{
		if(!snds[i]->stopping)  continue;

		if(GetPosition(snds[i]->snd) < snds[i]->flush_pos)
			continue; /* stopping but still flushing */

		/* This sound is done. */
		snds[i]->snd->StopPlaying();
	}
}

void RageSound_DSound_Software::StopMixing( RageSoundBase *snd )
{
	LockMut(SOUNDMAN->lock);

	/* Find the sound. */
	unsigned i;
	for(i = 0; i < sounds.size(); ++i)
		if(sounds[i]->snd == snd) break;
	if(i == sounds.size())
	{
		LOG->Trace("not stopping a sound because it's not playing");
		return;
	}

	delete sounds[i];
	sounds.erase(sounds.begin()+i, sounds.begin()+i+1);

	/* If nothing is playing, reset the frame count; this is just to
     * prevent eventual overflow. */
	if(sounds.empty())
		pcm->Reset();
}

int RageSound_DSound_Software::GetPosition( const RageSoundBase *snd ) const
{
	LockMut(SOUNDMAN->lock);
	return pcm->GetPosition();
}

RageSound_DSound_Software::RageSound_DSound_Software()
{
	shutdown = false;

	/* If we're emulated, we're better off with the WaveOut driver; DS
	 * emulation tends to be desynced. */
	if(ds.IsEmulated())
		RageException::ThrowNonfatal("Driver unusable (emulated device)");

	/* Create a DirectSound stream, but don't force it into hardware. */
	pcm = new DSoundBuf(ds, 
		DSoundBuf::HW_DONT_CARE, 
		channels, samplerate, 16, buffersize);

	MixingThread.SetName("Mixer thread");
	MixingThread.Create( MixerThread_start, this );
}

RageSound_DSound_Software::~RageSound_DSound_Software()
{
	/* Signal the mixing thread to quit. */
	shutdown = true;
	LOG->Trace("Shutting down mixer thread ...");
	LOG->Flush();
	MixingThread.Wait();
	LOG->Trace("Mixer thread shut down.");
	LOG->Flush();

	delete pcm;
}

float RageSound_DSound_Software::GetPlayLatency() const
{
	return (1.0f / samplerate) * buffersize_frames;
}

int RageSound_DSound_Software::GetSampleRate( int rate ) const
{
	return samplerate;
}

/*
 * Copyright (c) 2002-2004 by the person(s) listed below.  All rights reserved.
 *
 * Glenn Maynard
 */
