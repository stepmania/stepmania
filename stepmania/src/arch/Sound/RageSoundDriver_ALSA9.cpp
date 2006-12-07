#include "global.h"
#include "RageSoundDriver_ALSA9.h"

#include "RageLog.h"
#include "RageSound.h"
#include "RageSoundManager.h"
#include "RageSoundUtil.h"
#include "RageUtil.h"
#include "ALSA9Dynamic.h"

REGISTER_SOUND_DRIVER_CLASS( ALSA9 );

const int channels = 2;
const int samplerate = 44100;

const int samples_per_frame = channels;
const int bytes_per_frame = sizeof(int16_t) * samples_per_frame;

const unsigned max_writeahead = 1024*8;
const int num_chunks = 8;
const int chunksize = max_writeahead / num_chunks;

int RageSoundDriver_ALSA9::MixerThread_start(void *p)
{
	((RageSoundDriver_ALSA9 *) p)->MixerThread();
	return 0;
}

void RageSoundDriver_ALSA9::MixerThread()
{
	while(!shutdown)
	{
		/* Sleep for the size of one chunk. */
		const int chunksize_frames = max_writeahead / num_chunks;
		float sleep_secs = (float(chunksize_frames) / samplerate);
		usleep( 20000 ); // int(1000 * sleep_secs));

		LockMut( m_Mutex );

		for( unsigned i = 0; i < stream_pool.size(); ++i )
		{
			/* We're only interested in PLAYING and FLUSHING sounds. */
			if( stream_pool[i]->state != stream::PLAYING &&
				stream_pool[i]->state != stream::FLUSHING )
				continue; /* inactive */

			if( stream_pool[i]->bPaused )
				continue; /* paused */

			bool bEOF = false;
			while( !shutdown && stream_pool[i]->GetData(bEOF) && !bEOF )
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
			if( ps < stream_pool[i]->flush_pos )
				continue; /* stopping but still flushing */

			/* The sound has stopped and flushed all of its buffers. */
			stream_pool[i]->bPaused = false;
			stream_pool[i]->pcm->Stop();
			stream_pool[i]->state = stream::FINISHED;
		}
	}
}

RageSoundDriver_ALSA9::stream::~stream()
{
	delete pcm;
}

/* Returns the number of frames processed */
bool RageSoundDriver_ALSA9::stream::GetData( bool &bEOF )
{
	bEOF = false;

	int frames_to_fill = pcm->GetNumFramesToFill();
	if( frames_to_fill < chunksize )
		return false;

	static int16_t *buf = NULL;
	if ( !buf )
		buf = new int16_t[max_writeahead*samples_per_frame];
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

		RageSoundUtil::Attenuate( buf, got/sizeof(int16_t), snd->GetAbsoluteVolume() );

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


void RageSoundDriver_ALSA9::StartMixing(RageSoundBase *snd)
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
	stream_pool[i]->pcm->SetChunksize( chunksize );
	stream_pool[i]->pcm->SetWriteahead( max_writeahead );
	stream_pool[i]->start_time = snd->GetStartTime();

	/* Pre-buffer the stream, and start it immediately. */
	bool bEOF;
	while( stream_pool[i]->GetData(bEOF) && !bEOF )
		;

	stream_pool[i]->pcm->Play();

	/* If bEOF is true, we actually finished the whole file in the prebuffering
	 * GetData call above, and the sound should go straight to FLUSHING.  Otherwise,
	 * set PLAYING. */
	if( bEOF )
	{
		stream_pool[i]->state = stream_pool[i]->FLUSHING;
		stream_pool[i]->flush_pos = stream_pool[i]->pcm->GetPlayPos();
	}
	else
		stream_pool[i]->state = stream_pool[i]->PLAYING;
}

void RageSoundDriver_ALSA9::Update()
{
	for(unsigned i = 0; i < stream_pool.size(); ++i)
	{
		if( stream_pool[i]->state != stream_pool[i]->FINISHED )
			continue;

		/* The sound has stopped and flushed all of its buffers. */
		if( stream_pool[i]->snd )
			stream_pool[i]->snd->SoundIsFinishedPlaying();
		stream_pool[i]->snd = NULL;

		/* Once we do this, the sound is again available for use; we must lock
		 * m_InactiveSoundMutex to take it out of INACTIVE again. */
		stream_pool[i]->state = stream_pool[i]->INACTIVE;
	}
}

void RageSoundDriver_ALSA9::StopMixing(RageSoundBase *snd)
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

bool RageSoundDriver_ALSA9::PauseMixing( RageSoundBase *snd, bool bStop )
{
	unsigned i;
	for( i = 0; i < stream_pool.size(); ++i )
		if(stream_pool[i]->snd == snd)
			break;

	/* A sound can be paused in PLAYING or FLUSHING.  (FLUSHING means the sound
	 * has been decoded to the end, and we're waiting for that data to finish, so
	 * externally it looks and acts like PLAYING.) */
	if( i == stream_pool.size() ||
			(stream_pool[i]->state != stream::PLAYING && stream_pool[i]->state != stream::FLUSHING) )
	{
		LOG->Trace("not stopping a sound because it's not playing");
		return false;
	}

	stream_pool[i]->bPaused = bStop;

	return true;
}

int64_t RageSoundDriver_ALSA9::GetPosition(const RageSoundBase *snd) const
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

int RageSoundDriver_ALSA9::GetSampleRate( int rate ) const
{
	stream *str = stream_pool[0];
	return str->pcm->FindSampleRate( rate );
}
	
static RString CheckMixingBlacklist()
{
	RString sID = Alsa9Buf::GetHardwareID();
	const RString blacklist[] = {
		/* ALSA Driver: 0: Aureal Vortex au8830 [au8830], device 0: AU88x0 ADB [adb], 32/32 subdevices avail
		 * ALSA segfaults after creating about ten subdevices. */
		"au8830",
	}, *blacklist_end = blacklist+ARRAYLEN(blacklist);
	
	if( find( &blacklist[0], blacklist_end, sID ) != blacklist_end )
		return ssprintf( "ALSA driver \"%s\" not using hardware mixing", sID.c_str() );
	return "";
}

RageSoundDriver_ALSA9::RageSoundDriver_ALSA9():
	m_Mutex("ALSAMutex"),
	m_InactiveSoundMutex("InactiveSoundMutex")
{
	shutdown = false;
}

RString RageSoundDriver_ALSA9::Init()
{
	RString sError = LoadALSA();
	if( sError != "" )
		return ssprintf( "Driver unusable: %s", sError.c_str() );

	sError = CheckMixingBlacklist();
	if( sError != "" )
		return sError;
	
	/* Create a bunch of streams and put them into the stream pool. */
	for( int i = 0; i < 32; ++i )
	{
		Alsa9Buf *newbuf = new Alsa9Buf;
		sError = newbuf->Init( Alsa9Buf::HW_HARDWARE, channels );
		if( sError != "" )
		{
			delete newbuf;

			/* If we didn't get at least 8, fail. */
			if( i >= 8 )
				break; /* OK */

			if( i == 0 )
				return sError;

			/* We created at least one hardware buffer. */
			LOG->Trace( "Could only create %i buffers; need at least 8 (failed with %s).  Hardware ALSA driver can't be used.", i, sError.c_str() );
			return "Not enough substreams for hardware mixing, using software mixing";
		}

		stream *s = new stream;
		s->pcm = newbuf;
		stream_pool.push_back(s);
	}

	LOG->Info( "ALSA: Got %i hardware buffers", stream_pool.size() );

	MixingThread.SetName( "RageSoundDriver_ALSA9" );
	MixingThread.Create( MixerThread_start, this );

	return "";
}

RageSoundDriver_ALSA9::~RageSoundDriver_ALSA9()
{
	if( MixingThread.IsCreated() )
	{
		/* Signal the mixing thread to quit. */
		shutdown = true;
		LOG->Trace("Shutting down mixer thread ...");
		MixingThread.Wait();
		LOG->Trace("Mixer thread shut down.");
	}
 
	for(unsigned i = 0; i < stream_pool.size(); ++i)
		delete stream_pool[i];

	UnloadALSA();
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
