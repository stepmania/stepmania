#include "global.h"
#include "RageSoundDriver_ALSA9_Software.h"

#include "RageLog.h"
#include "RageSound.h"
#include "RageSoundManager.h"
#include "RageUtil.h"
#include "RageTimer.h"
#include "ALSA9Dynamic.h"
#include "PrefsManager.h"

#include "archutils/Unix/GetSysInfo.h"

#include <sys/time.h>
#include <sys/resource.h>

const int channels = 2;
int samplerate = 44100;

const int samples_per_frame = channels;
const int bytes_per_frame = sizeof(Sint16) * samples_per_frame;

/* Linux 2.6 has a fine-grained scheduler; use a small buffer size.  Otherwise, use a larger one. */
static const unsigned max_writeahead_linux_26 = 512;
static const unsigned safe_writeahead = 1024*4;
static unsigned max_writeahead;
const int num_chunks = 8;

int RageSound_ALSA9_Software::MixerThread_start(void *p)
{
	((RageSound_ALSA9_Software *) p)->MixerThread();
	return 0;
}

void RageSound_ALSA9_Software::MixerThread()
{
	/* SOUNDMAN will be set once RageSoundManager's ctor returns and
	 * assigns it; we might get here before that happens, though. */
	while(!SOUNDMAN && !shutdown) SDL_Delay(10);

	setpriority( PRIO_PROCESS, getpid(), -15 );

//	RageTimer UnderrunTest;
	while(!shutdown)
	{
		while( !shutdown && GetData() )
			;
		const float delay_ms = 1000 * float(max_writeahead) / samplerate;
		SDL_Delay( int(delay_ms) / 4 );

//		if( UnderrunTest.PeekDeltaTime() > 10 )
//		{
//			UnderrunTest.GetDeltaTime();
//			SDL_Delay( 250 );
//		}
	}
}

/* Returns the number of frames processed */
bool RageSound_ALSA9_Software::GetData()
{
	const int chunksize = max_writeahead / num_chunks;
	
	const int frames_to_fill = pcm->GetNumFramesToFill( max_writeahead, chunksize );
	if( frames_to_fill <= 0 )
		return false;

	/* Sint16 represents a single sample
	 * each frame contains one sample per channel
	 */
    static Sint16 *buf = NULL;
	if (!buf)
		buf = new Sint16[max_writeahead*samples_per_frame];

    static SoundMixBuffer mix;

	const int64_t play_pos = pcm->GetPlayPos();
	const int64_t cur_play_pos = pcm->GetPosition();

	LockMutex L(SOUNDMAN->lock);
	for(unsigned i = 0; i < sounds.size(); ++i)
	{
		if(sounds[i]->stopping)
			continue;

		int bytes_read = 0;
		int bytes_left = frames_to_fill*bytes_per_frame;

		/* Does the sound have a start time? */
		if( !sounds[i]->start_time.IsZero() )
		{
			/* If the sound is supposed to start at a time past this buffer, insert silence. */
			const int64_t iFramesUntilThisBuffer = play_pos - cur_play_pos;
			const float fSecondsBeforeStart = -sounds[i]->start_time.Ago();
			const int64_t iFramesBeforeStart = int64_t( fSecondsBeforeStart * samplerate );
			const int64_t iSilentFramesInThisBuffer = iFramesBeforeStart-iFramesUntilThisBuffer;
			const int iSilentBytesInThisBuffer = clamp( int(iSilentFramesInThisBuffer * bytes_per_frame), 0, bytes_left );

			memset( buf+bytes_read, 0, iSilentBytesInThisBuffer );
			bytes_read += iSilentBytesInThisBuffer;
			bytes_left -= iSilentBytesInThisBuffer;

			if( !iSilentBytesInThisBuffer )
				sounds[i]->start_time.SetZero();
		}

		/* Call the callback.
		 * Get the units straight,
		 * <bytes> = GetPCM(<bytes*>, <bytes>, <frames>)
		 */
		int got = sounds[i]->snd->GetPCM( (char *) buf+bytes_read, bytes_left, play_pos+bytes_read/bytes_per_frame );
		bytes_read += got;
		bytes_left -= got;

		mix.write( (Sint16 *) buf, bytes_read / sizeof(Sint16), sounds[i]->snd->GetVolume() );

		if( bytes_left > 0 )
		{
			/* This sound is finishing. */
			sounds[i]->stopping = true;
			sounds[i]->flush_pos = pcm->GetPlayPos() + (bytes_read / bytes_per_frame);
		}
    }
	L.Unlock();

    memset( buf, 0, frames_to_fill * bytes_per_frame );
	mix.read( buf );

	pcm->Write( buf, frames_to_fill );
	return true;
}


void RageSound_ALSA9_Software::StartMixing(RageSoundBase *snd)
{
	sound *s = new sound;
	s->snd = snd;
	s->start_time = snd->GetStartTime();

	LockMutex L(SOUNDMAN->lock);
	sounds.push_back(s);
}

void RageSound_ALSA9_Software::Update(float delta)
{
	LockMutex L(SOUNDMAN->lock);

	/* SoundStopped might erase sounds out from under us, so make a copy
	 * of the sound list. */
	vector<sound *> snds = sounds;
	for(unsigned i = 0; i < snds.size(); ++i)
	{
		if(!snds[i]->stopping) continue;

		if(GetPosition(snds[i]->snd) < snds[i]->flush_pos)
			continue; /* stopping but still flushing */

		/* This sound is done. */
		snds[i]->snd->StopPlaying();
	}
}

void RageSound_ALSA9_Software::StopMixing(RageSoundBase *snd)
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

	/* If nothing is playing, reset the sample count; this is just to
	 * prevent eventual overflow. */
	if( sounds.empty() )
		pcm->Reset();
}


int64_t RageSound_ALSA9_Software::GetPosition(const RageSoundBase *snd) const
{
	return pcm->GetPosition();
}       


RageSound_ALSA9_Software::RageSound_ALSA9_Software()
{
	CString err = LoadALSA();
	if( err != "" )
		RageException::ThrowNonfatal("Driver unusable: %s", err.c_str());
try {
	shutdown = false;

	max_writeahead = safe_writeahead;
	CString sys;
	int vers;
	GetKernel( sys, vers );
	LOG->Trace( "OS: %s ver %06i", sys.c_str(), vers );
	if( sys == "Linux" && vers >= 20600 )
		max_writeahead = max_writeahead_linux_26;

	if( PREFSMAN->m_iSoundWriteAhead )
		max_writeahead = PREFSMAN->m_iSoundWriteAhead;

	pcm = new Alsa9Buf( Alsa9Buf::HW_DONT_CARE, channels );

	samplerate = pcm->FindSampleRate( samplerate );
	pcm->SetSampleRate( samplerate );
	LOG->Info( "ALSA: Software mixing at %ihz", samplerate );
	
	MixingThread.SetName( "RageSound_ALSA9_Software" );
	MixingThread.Create( MixerThread_start, this );
} catch(...) {
	UnloadALSA();
	throw;
}

}

RageSound_ALSA9_Software::~RageSound_ALSA9_Software()
{
	/* Signal the mixing thread to quit. */
	shutdown = true;
	LOG->Trace("Shutting down mixer thread ...");
	MixingThread.Wait();
	LOG->Trace("Mixer thread shut down.");
 
	delete pcm;

	UnloadALSA();
}

float RageSound_ALSA9_Software::GetPlayLatency() const
{
	return float(max_writeahead)/samplerate;
}

int RageSound_ALSA9_Software::GetSampleRate( int rate ) const
{
	return samplerate;
}
		
/*
 * Copyright (c) 2002-2004 by the person(s) listed below.  All rights reserved.
 *
 * Glenn Maynard
 */
