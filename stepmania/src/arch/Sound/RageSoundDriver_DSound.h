#ifndef RAGE_SOUND_DSOUND
#define RAGE_SOUND_DSOUND

#include "RageSoundDriver.h"
#include "DSoundHelpers.h"
#include "RageThreads.h"
#include "RageTimer.h"

struct IDirectSound;
struct IDirectSoundBuffer;

class RageSound_DSound: public RageSoundDriver
{
public:
	RageSound_DSound();
	~RageSound_DSound();
	RString Init();

private:
	/* The only place that takes sounds out of INACTIVE is StartMixing; this mutex
	 * serializes inactive sounds. */
	RageMutex m_InactiveSoundMutex;

	struct stream {
	    /* Actual audio stream: */
		DSoundBuf *pcm;

	    /* Sound object that's playing on this stream, or NULL if this
	     * channel is available: */
	    RageSoundBase *snd;

		enum {
			INACTIVE,
			SETUP,
			PLAYING,
			FLUSHING,
			FINISHED
		} state;

		int64_t flush_pos; /* state == FLUSHING only */

		bool bPaused;

		RageTimer start_time;
		bool GetData( bool init, bool &bEOF );

	    stream() { pcm = NULL; snd = NULL; state = INACTIVE; bPaused = false; }
		~stream();
	};

	/* Pool of available streams. */
	vector<stream *> stream_pool;

	DSound ds;

	RageMutex m_Mutex;

	bool shutdown; /* tells the MixerThread to shut down */
	static int MixerThread_start(void *p);
	void MixerThread();
	RageThread MixingThread;

	/* virtuals: */
	void StartMixing( RageSoundBase *snd );	/* used by RageSound */
	void StopMixing( RageSoundBase *snd );		/* used by RageSound */
	bool PauseMixing( RageSoundBase *snd, bool bStop );
	int64_t GetPosition( const RageSoundBase *snd ) const;
	void Update();

	int GetSampleRate( int rate ) const { return rate; }
};

#endif

/*
 * (c) 2002-2004 Glenn Maynard
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
