#include "global.h"
#include "RageSoundDriver_ALSA9.h"

#include "RageLog.h"
#include "RageSound.h"
#include "RageSoundManager.h"
#include "RageUtil.h"
#include "ALSA9Dynamic.h"

const int channels = 2;
const int samplerate = 44100;

const int samples_per_frame = channels;
const int bytes_per_frame = sizeof(Sint16) * samples_per_frame;

const unsigned max_writeahead = 1024*8;
const int num_chunks = 8;
const int chunksize = max_writeahead / num_chunks;

int RageSound_ALSA9::MixerThread_start(void *p)
{
	((RageSound_ALSA9 *) p)->MixerThread();
	return 0;
}

void RageSound_ALSA9::MixerThread()
{
	/* SOUNDMAN will be set once RageSoundManager's ctor returns and
	 * assigns it; we might get here before that happens, though. */
	while(!SOUNDMAN && !shutdown) SDL_Delay(10);

	while(!shutdown)
	{
		/* Sleep for the size of one chunk. */
		const int chunksize_frames = max_writeahead / num_chunks;
		float sleep_secs = (float(chunksize_frames) / samplerate);
		SDL_Delay( 20 ); // int(1000 * sleep_secs));

		LockMut( m_Mutex );

		for( unsigned i = 0; i < stream_pool.size(); ++i )
		{
			/* We're only interested in PLAYING and FLUSHING sounds. */
			if( stream_pool[i]->state != stream::PLAYING &&
				stream_pool[i]->state != stream::FLUSHING )
				continue; /* inactive */

			bool bEOF = false;
			while( !shutdown && stream_pool[i]->GetData(false, bEOF) && !bEOF )
				;

			if( bEOF )
			{
				/* FLUSHING tells the mixer thread to release the stream once str->flush_bufs
				 * buffers have been flushed. */
				stream_pool[i]->state = stream::FLUSHING;

				/* Flush two buffers worth of data. */
				stream_pool[i]->flush_pos = stream_pool[i]->pcm->GetPlayPos();
				LOG->Trace("eof, fl to %i", (int)stream_pool[i]->flush_pos);
			}

		}

		for(unsigned i = 0; i < stream_pool.size(); ++i)
		{
			if( stream_pool[i]->state != stream_pool[i]->FLUSHING )
				continue;

			int ps = stream_pool[i]->pcm->GetPosition();
			LOG->Trace("fl #%i: pos %i to %i", 
					i, ps, (int) stream_pool[i]->flush_pos);
			if( ps < stream_pool[i]->flush_pos )
				continue; /* stopping but still flushing */

			/* The sound has stopped and flushed all of its buffers. */
			stream_pool[i]->pcm->Stop();
			stream_pool[i]->state = stream::FINISHED;
		}
	}
}

RageSound_ALSA9::stream::~stream()
{
	delete pcm;
}

/* Returns the number of frames processed */
bool RageSound_ALSA9::stream::GetData( bool init, bool &bEOF )
{
	bEOF = false;

	int frames_to_fill = pcm->GetNumFramesToFill( max_writeahead, init? max_writeahead:chunksize );
	if( frames_to_fill < chunksize )
		return false;

	static Sint16 *buf = NULL;
	if ( !buf )
		buf = new Sint16[max_writeahead*samples_per_frame];
	char *cbuf = (char*) buf;

	const int64_t play_pos = pcm->GetPlayPos();
	const int64_t cur_play_pos = pcm->GetPosition();

	int len = frames_to_fill*bytes_per_frame;
	/* It might be INACTIVE, when we're prebuffering. We just don't want to
	 * fill anything in FLUSHING; in that case, we just clear the audio buffer. */
	if( state != FLUSHING )
	{
		int bytes_read = 0;
		int bytes_left = len;

		/* Does the sound have a start time? */
		if( !start_time.IsZero() )
		{
			/* If the sound is supposed to start at a time past this buffer, insert silence. */
			const int64_t iFramesUntilThisBuffer = play_pos - cur_play_pos;
			const float fSecondsBeforeStart = -start_time.Ago();
			const int64_t iFramesBeforeStart = int64_t(fSecondsBeforeStart * pcm->GetSampleRate());
			const int64_t iSilentFramesInThisBuffer = iFramesBeforeStart-iFramesUntilThisBuffer;
			const int iSilentBytesInThisBuffer = clamp( int(iSilentFramesInThisBuffer * bytes_per_frame), 0, bytes_left );

			memset( cbuf+bytes_read, 0, iSilentBytesInThisBuffer );
			bytes_read += iSilentBytesInThisBuffer;
			bytes_left -= iSilentBytesInThisBuffer;

			if( !iSilentBytesInThisBuffer )
				start_time.SetZero();
		}

		int got = snd->GetPCM( cbuf+bytes_read, len-bytes_read, play_pos + (bytes_read/bytes_per_frame) );
		bytes_read += got;
		bytes_left -= got;

		RageSoundManager::AttenuateBuf( buf, got/sizeof(Sint16), snd->GetVolume() );

		if( bytes_left > 0 )
		{
			/* Fill the remainder of the buffer with silence. */
			memset( cbuf+bytes_read, 0, bytes_left );

			LOG->Trace("eof");
			bEOF = true;
		}
	} else {
		/* Silence the buffer. */
		memset( buf, 0, len );
	}

	pcm->Write( buf, frames_to_fill );

	return true;
}


void RageSound_ALSA9::StartMixing(RageSoundBase *snd)
{
	/* Lock INACTIVE sounds[], and reserve a slot. */
	m_InactiveSoundMutex.Lock();

	/* Find an unused buffer. */
	unsigned i;
	for( i = 0; i < stream_pool.size(); ++i )
	{
		if( stream_pool[i]->state == stream_pool[i]->INACTIVE )
			break;
	}

	if( i == stream_pool.size() )
	{
		/* We don't have a free sound buffer. */
		m_InactiveSoundMutex.Unlock();
		return;
	}

	/* Place the sound in SETUP, where nobody else will touch it, until we put it
	 * in FLUSHING or PLAYING below. */
	stream_pool[i]->state = stream::SETUP;
	m_InactiveSoundMutex.Unlock();

	/* Give the stream to the playing sound and remove it from the pool. */
	stream_pool[i]->snd = snd;
	stream_pool[i]->pcm->SetSampleRate( snd->GetSampleRate() );
	stream_pool[i]->start_time = snd->GetStartTime();

	/* Pre-buffer the stream, and start it immediately. */
	bool bEOF;
	stream_pool[i]->GetData( true, bEOF );
	stream_pool[i]->pcm->Play();

	/* If bEOF is true, we actually finished the whole file in the prebuffering
	 * GetData call above, and the sound should go straight to FLUSHING.  Otherwise,
	 * set PLAYING. */
	if( bEOF )
	{
		stream_pool[i]->state = stream_pool[i]->FLUSHING;
		stream_pool[i]->flush_pos = stream_pool[i]->pcm->GetPosition();
	}
	else
		stream_pool[i]->state = stream_pool[i]->PLAYING;
}

void RageSound_ALSA9::Update(float delta)
{
	for(unsigned i = 0; i < stream_pool.size(); ++i)
	{
		if( stream_pool[i]->state != stream_pool[i]->FINISHED )
			continue;

		/* The sound has stopped and flushed all of its buffers. */
		if( stream_pool[i]->snd )
			stream_pool[i]->snd->SoundIsFinishedPlaying();
		stream_pool[i]->snd = NULL;

		/* Once we do this, the sound is once available for use; we must lock
		 * m_InactiveSoundMutex to take it out of INACTIVE again. */
		stream_pool[i]->state = stream_pool[i]->INACTIVE;
	}
}

void RageSound_ALSA9::StopMixing(RageSoundBase *snd)
{
	/* Lock, to make sure the decoder thread isn't running on this sound while we do this. */
	LockMut( m_Mutex );

	ASSERT(snd != NULL);

	unsigned i;
	for( i = 0; i < stream_pool.size(); ++i )
		if(stream_pool[i]->snd == snd)
			break;

	if( i == stream_pool.size() )
	{
		LOG->Trace("not stopping a sound because it's not playing");
		return;
	}

	/* FLUSHING tells the mixer thread to release the stream once str->flush_bufs
	 * buffers have been flushed. */
	stream_pool[i]->state = stream_pool[i]->FLUSHING;

	/* Flush two buffers worth of data. */
	stream_pool[i]->flush_pos = stream_pool[i]->pcm->GetPlayPos();

	/* This function is called externally (by RageSoundBase) to stop immediately.
	 * We need to prevent SoundStopped from being called; it should only be
	 * called when we stop implicitely at the end of a sound.  Set snd to NULL. */
	stream_pool[i]->snd = NULL;
}


int64_t RageSound_ALSA9::GetPosition(const RageSoundBase *snd) const
{
	unsigned i;
	for( i = 0; i < stream_pool.size(); ++i )
		if( stream_pool[i]->snd == snd )
			break;

	if( i == stream_pool.size() )
		RageException::Throw("GetPosition: Sound %s is not being played", snd->GetLoadedFilePath().c_str());

	/* XXX: This isn't quite threadsafe. */
	return stream_pool[i]->pcm->GetPosition();
}       

int RageSound_ALSA9::GetSampleRate( int rate ) const
{
	stream *str = stream_pool[0];
	return str->pcm->FindSampleRate( rate );
}
	
static void CheckMixingBlacklist()
{
	CString sID = Alsa9Buf::GetHardwareID();
	const CString blacklist[] = {
		/* ALSA Driver: 0: Aureal Vortex au8830 [au8830], device 0: AU88x0 ADB [adb], 32/32 subdevices avail
		 * ALSA segfaults after creating about ten subdevices. */
		"au8830",
	}, *blacklist_end = blacklist+ARRAYSIZE(blacklist);
	
	if( find( &blacklist[0], blacklist_end, sID ) != blacklist_end )
		RageException::ThrowNonfatal( "ALSA driver \"%s\" not using hardware mixing", sID.c_str() );
}

RageSound_ALSA9::RageSound_ALSA9():
	m_Mutex("ALSAMutex"),
	m_InactiveSoundMutex("InactiveSoundMutex")
{
	CString err = LoadALSA();
	if( err != "" )
		RageException::ThrowNonfatal("Driver unusable: %s", err.c_str());
try {
	CheckMixingBlacklist();
	
	shutdown = false;

	/* Create a bunch of streams and put them into the stream pool. */
	for( int i = 0; i < 32; ++i )
	{
		Alsa9Buf *newbuf;
		try {
			newbuf = new Alsa9Buf( Alsa9Buf::HW_HARDWARE, channels );
		} catch(const RageException &e) {
			/* If we didn't get at least 8, fail. */
			if(i >= 8) break; /* OK */

			/* Clean up; the dtor won't be called. */
			for(int n = 0; n < i; ++n)
				delete stream_pool[n];

			if( !i )
				RageException::ThrowNonfatal( "%s", e.what() );

			/* We created at least one hardware buffer. */
			LOG->Trace("Could only create %i buffers; need at least 8 (failed with %s).  Hardware ALSA driver can't be used.", i, e.what());
			RageException::ThrowNonfatal("Not enough substreams for hardware mixing, using software mixing");
		}

		stream *s = new stream;
		s->pcm = newbuf;
		stream_pool.push_back(s);
	}

	LOG->Info("ALSA: Got %i hardware buffers", stream_pool.size());

	MixingThread.SetName( "RageSound_ALSA9" );
	MixingThread.Create( MixerThread_start, this );
} catch(...) {
	UnloadALSA();
	throw;
}
}

RageSound_ALSA9::~RageSound_ALSA9()
{
	/* Signal the mixing thread to quit. */
	shutdown = true;
	LOG->Trace("Shutting down mixer thread ...");
	MixingThread.Wait();
	LOG->Trace("Mixer thread shut down.");
 
	for(unsigned i = 0; i < stream_pool.size(); ++i)
		delete stream_pool[i];

	UnloadALSA();
}

/*
 * Copyright (c) 2002 by the person(s) listed below.  All rights reserved.
 *
 * Glenn Maynard (RageSoundDriver_WaveOut)
 */
