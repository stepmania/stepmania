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
static const int bytes_per_frame = sizeof(int16_t) * samples_per_frame;

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
	setpriority( PRIO_PROCESS, 0, -15 );

	while(!shutdown)
	{
		while( !shutdown && GetData() )
			;

		pcm->WaitUntilFramesCanBeFilled( 100 );
	}
}

/* Returns the number of frames processed */
bool RageSound_ALSA9_Software::GetData()
{
	const int frames_to_fill = pcm->GetNumFramesToFill();
	if( frames_to_fill <= 0 )
		return false;

	static int16_t *buf = NULL;
	if (!buf)
		buf = new int16_t[max_writeahead*samples_per_frame];

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
	
	pcm->SetWriteahead( max_writeahead );
	pcm->SetChunksize( max_writeahead / num_chunks );
	pcm->LogParams();
	
	StartDecodeThread();
	
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
 * (c) 2002-2004 Glenn Maynard, Aaron VonderHaar
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
