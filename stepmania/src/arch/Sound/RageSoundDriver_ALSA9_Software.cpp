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
	
	const int frames_to_fill = min( chunksize, pcm->GetNumFramesToFill( max_writeahead ) );
	if( frames_to_fill <= 0 )
		return false;

	/* Sint16 represents a single sample
	 * each frame contains one sample per channel
	 */
    static Sint16 *buf = NULL;
	if (!buf)
		buf = new Sint16[max_writeahead*samples_per_frame*4];

    static SoundMixBuffer mix;
	mix.SetVolume( SOUNDMAN->GetMixVolume() );

	LockMutex L(SOUNDMAN->lock);
	for(unsigned i = 0; i < sounds.size(); ++i)
	{
		if(sounds[i]->stopping)
			continue;

		/* Call the callback.
		 * Get the units straight,
		 * <bytes> = GetPCM(<bytes*>, <bytes>, <frames>)
		 */
		unsigned got = sounds[i]->snd->GetPCM( (char *) buf, frames_to_fill*bytes_per_frame, pcm->GetPlayPos() ) / sizeof(Sint16);
		mix.write((Sint16 *) buf, got);

		if( int(got) < frames_to_fill )
		{
			/* This sound is finishing. */
			sounds[i]->stopping = true;
		}
    }
	L.Unlock();

    memset( buf, 0, sizeof(Sint16) * frames_to_fill * samples_per_frame );
	mix.read( buf );

	pcm->Write( buf, frames_to_fill );
	return true;
}


void RageSound_ALSA9_Software::StartMixing(RageSound *snd)
{
	sound *s = new sound;
	s->snd = snd;

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
		if(!sounds[i]->stopping) continue;

		if(GetPosition(snds[i]->snd) < sounds[i]->flush_pos)
			continue; /* stopping but still flushing */

		/* This sound is done. */
		snds[i]->snd->StopPlaying();
	}
}

void RageSound_ALSA9_Software::StopMixing(RageSound *snd)
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


int RageSound_ALSA9_Software::GetPosition(const RageSound *snd) const
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
	LOG->Trace( "OS: %s ver %06x", sys.c_str(), vers );
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
 * Copyright (c) 2002 by the person(s) listed below.  All rights reserved.
 *
 * Glenn Maynard (RageSoundDriver_WaveOut)
 */
