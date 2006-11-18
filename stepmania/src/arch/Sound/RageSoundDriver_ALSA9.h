#ifndef RAGE_SOUND_ALSA9_H
#define RAGE_SOUND_ALSA9_H

#include "RageSound.h"
#include "RageThreads.h"
#include "RageSoundDriver.h"

#include "ALSA9Helpers.h"

class RageSound_ALSA9: public RageSoundDriver
{
public:
	/* virtuals: */
	void StartMixing(RageSoundBase *snd);
	void StopMixing(RageSoundBase *snd);
	bool PauseMixing( RageSoundBase *snd, bool bStop );
	int64_t GetPosition( const RageSoundBase *snd ) const;
	int GetSampleRate( int rate ) const;

	void Update();

	RageSound_ALSA9();
	RString Init();
	~RageSound_ALSA9();

private:
	/* This mutex serializes the decode thread and StopMixing. */
	RageMutex m_Mutex;

	/* The only place that takes sounds out of INACTIVE is StartMixing; this mutex
	 * serializes inactive sounds. */
	RageMutex m_InactiveSoundMutex;

	struct stream
	{
		/* Actual audio stream: */
		Alsa9Buf *pcm;

		/* Sound object that's playing on this stream, or NULL if this
		 * channel is available: */
		RageSoundBase *snd;
		RageTimer start_time;
		bool bPaused;

		enum {
			INACTIVE,
			SETUP,
			PLAYING,
			FLUSHING,
			FINISHED
		} state;

		int64_t flush_pos; /* state == STOPPING only */

		bool GetData( bool &bEOF );

		stream() { pcm = NULL; snd = NULL; state = INACTIVE; bPaused = false; }
		~stream();
	};
	friend struct stream;

	/* Pool of available streams. */
	vector<stream *> stream_pool;

	bool shutdown;


	static int MixerThread_start(void *p);
	void MixerThread();
	RageThread MixingThread;

	void GetData();
};

#endif

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
