#include "global.h"
#include "RageSoundDriver_OSS.h"

#include "RageTimer.h"
#include "RageLog.h"
#include "RageSound.h"
#include "RageUtil.h"

#include "SDL.h"
#include <unistd.h>
#include <fcntl.h>
#include <errno.h>
#include <string.h>
#include <sys/ioctl.h>
#include <sys/soundcard.h>
#include <sys/select.h>

/* samples */
const int channels = 2;
const int samplesize = channels*2;		/* 16-bit */
const int buffersize_frames = 1024*8;	/* in frames */
const int buffersize = buffersize_frames * samplesize; /* in bytes */

const int num_chunks = 8;


int RageSound_OSS::MixerThread_start(void *p)
{
	((RageSound_OSS *) p)->MixerThread();
	return 0;
}

void RageSound_OSS::MixerThread()
{
	/* SOUNDMAN will be set once RageSoundManager's ctor returns and
	 * assigns it; we might get here before that happens, though. */
	while(!SOUNDMAN && !shutdown) SDL_Delay(10);

	/* We want to set a higher priority, but Unix only lets root renice
	 * < 0, which is silly.  Give it a try, anyway. */
	nice(-5);

	while(!shutdown) {
		while(GetData())
			;

		fd_set f;
		FD_ZERO(&f);
		FD_SET(fd, &f);

		SDL_Delay(10);

		struct timeval tv = { 0, 10000 };
		select(fd+1, NULL, &f, NULL, &tv);
	}
}

bool RageSound_OSS::GetData()
{
	LockMutex L(SOUNDMAN->lock);

	/* Look for a free buffer. */
	audio_buf_info ab;
	if(ioctl(fd, SNDCTL_DSP_GETOSPACE, &ab) == -1)
		RageException::Throw("ioctl(SNDCTL_DSP_GETOSPACE): %s", strerror(errno) );

	if(!ab.fragments)
		return false;
		
	const int chunksize = ab.fragsize;
	
	static Sint16 *buf = NULL;
	if(!buf)
		buf = new Sint16[chunksize / sizeof(Sint16)];
	memset(buf, 0, chunksize);

	/* Create a 32-bit buffer to mix sounds. */
	static SoundMixBuffer mix;

	for(unsigned i = 0; i < sounds.size(); ++i)
	{
		if(sounds[i]->stopping)
			continue;

		/* Call the callback. */
		unsigned got = sounds[i]->snd->GetPCM((char *) buf, chunksize, last_cursor_pos);

		mix.write((Sint16 *) buf, got/2);

		if(got < chunksize)
		{
			/* This sound is finishing. */
			sounds[i]->stopping = true;
			sounds[i]->flush_pos = last_cursor_pos + (got / samplesize);
		}
	}

	mix.read(buf);
	int wrote = write(fd, buf, chunksize);
  	if(wrote != chunksize)
		RageException::Throw("write didn't: %i (%s)", wrote, wrote == -1? strerror(errno): "" );

	/* Increment last_cursor_pos. */
	last_cursor_pos += chunksize / samplesize;

	return true;
}

void RageSound_OSS::StartMixing(RageSound *snd)
{
	sound *s = new sound;
	s->snd = snd;

	LockMutex L(SOUNDMAN->lock);
	sounds.push_back(s);
}

void RageSound_OSS::Update(float delta)
{
	LockMutex L(SOUNDMAN->lock);

	/* SoundStopped might erase sounds out from under us, so make a copy
	 * of the sound list. */
	vector<sound *> snds = sounds;
	for(unsigned i = 0; i < snds.size(); ++i)
	{
		if(!sounds[i]->stopping) continue;

		if(GetPosition(snds[i]->snd) < sounds[i]->flush_pos)
			continue; /* stopping but still flushing */

		/* This sound is done. */
		snds[i]->snd->StopPlaying();
	}
}

void RageSound_OSS::StopMixing(RageSound *snd)
{
	LockMutex L(SOUNDMAN->lock);

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
}

int RageSound_OSS::GetPosition(const RageSound *snd) const
{
	LockMutex L(SOUNDMAN->lock);

	int delay;
	if(ioctl(fd, SNDCTL_DSP_GETODELAY, &delay) == -1)
		RageException::ThrowNonfatal("RageSound_OSS: ioctl(SNDCTL_DSP_GETODELAY): %s", strerror(errno));

	return last_cursor_pos - (delay / samplesize);

/*	MMTIME tm;
	tm.wType = TIME_SAMPLES;
	MMRESULT ret = waveOutGetPosition(wo, &tm, sizeof(tm));
  	if(ret != MMSYSERR_NOERROR)
		RageException::Throw(wo_ssprintf(ret, "waveOutGetPosition failed"));

	return tm.u.sample;
	*/
}


RageSound_OSS::RageSound_OSS()
{
	shutdown = false;
	last_cursor_pos = 0;

	fd = open("/dev/dsp", O_WRONLY|O_NONBLOCK);
	if(fd == -1)
		RageException::ThrowNonfatal("RageSound_OSS: Couldn't open /dev/dsp: %s", strerror(errno));

	int i = AFMT_S16_LE;
	if(ioctl(fd, SNDCTL_DSP_SETFMT, &i) == -1)
		RageException::ThrowNonfatal("RageSound_OSS: ioctl(SNDCTL_DSP_SETFMT, %i): %s", i, strerror(errno));
	if(i != AFMT_S16_LE)
		RageException::ThrowNonfatal("RageSound_OSS: Wanted format %i, got %i instead", AFMT_S16_LE, i);

	i = channels;
	if(ioctl(fd, SNDCTL_DSP_CHANNELS, &i) == -1)
		RageException::ThrowNonfatal("RageSound_OSS: ioctl(SNDCTL_DSP_CHANNELS, %i): %s", i, strerror(errno));
	if(i != channels)
		RageException::ThrowNonfatal("RageSound_OSS: Wanted %i channels, got %i instead", channels, i);
		
	i = 48000;
	if(ioctl(fd, SOUND_PCM_WRITE_RATE, &i) == -1 )
		RageException::ThrowNonfatal("RageSound_OSS: ioctl(SOUND_PCM_WRITE_RATE, %i): %s", i, strerror(errno));
	samplerate = i;
	LOG->Trace("RageSound_OSS: sample rate %i", samplerate);

	MixerThreadPtr = SDL_CreateThread(MixerThread_start, this);
}


RageSound_OSS::~RageSound_OSS()
{
	/* Signal the mixing thread to quit. */
	shutdown = true;
	LOG->Trace("Shutting down mixer thread ...");
	SDL_WaitThread(MixerThreadPtr, NULL);
	LOG->Trace("Mixer thread shut down.");

	close(fd);
}

float RageSound_OSS::GetPlayLatency() const
{
	return 0; // (1.0f / samplerate) * (buffersize_frames - chunksize_frames);
}

/*
 * Copyright (c) 2002-2003 by the person(s) listed below.  All rights reserved.
 *
 * Glenn Maynard
 */
