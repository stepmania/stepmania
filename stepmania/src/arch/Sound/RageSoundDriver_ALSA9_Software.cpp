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

static const int channels = 2;
int samplerate = 44100;

static const int samples_per_frame = channels;
static const int bytes_per_frame = sizeof(Sint16) * samples_per_frame;

/* Linux 2.6 has a fine-grained scheduler.  We can almost always use a smaller buffer
 * size than in 2.4.  XXX: Some cards can handle smaller buffer sizes than others. */
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

	setpriority( PRIO_PROCESS, 0, -15 );

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

	static Sint16 *buf = NULL;
	if (!buf)
		buf = new Sint16[max_writeahead*samples_per_frame];

	const int64_t play_pos = pcm->GetPlayPos();
	const int64_t cur_play_pos = pcm->GetPosition();

	this->Mix( buf, frames_to_fill, play_pos, cur_play_pos );
	pcm->Write( buf, frames_to_fill );

	return true;
}


int64_t RageSound_ALSA9_Software::GetPosition(const RageSoundBase *snd) const
{
	return pcm->GetPosition();
}       

void RageSound_ALSA9_Software::SetupDecodingThread()
{
	setpriority( PRIO_PROCESS, 0, -5 );
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
