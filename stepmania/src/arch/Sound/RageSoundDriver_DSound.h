#ifndef RAGE_SOUND_DSOUND
#define RAGE_SOUND_DSOUND

#include "RageSoundDriver.h"
#include "SDL_Thread.h"

struct IDirectSound8;
struct IDirectSoundBuffer8;

class RageSound_DSound: public RageSoundDriver
{
	struct stream {
	    /* Actual audio stream: */
		IDirectSoundBuffer8 *str_ds;

	    /* Sound object that's playing on this stream, or NULL if this
	     * channel is available: */
	    RageSound *snd;

		enum {
			INACTIVE,
			PLAYING,
			STOPPING
		} state;

		int flush_bufs; /* state == STOPPING only */
		/* Position in the DS buffer that we filled last; always
		 * either 0 or halfway through the buffer: */
		int last_cursor_filled;
		/* Position, in samples, of the last buffer filled: */
		int last_cursor_pos;

		/* Last time returned for this stream; used to make sure
		 * we never go backwards. */
		mutable int LastPosition;

		void GetPCM(bool init);

	    stream() { str_ds = NULL; snd = NULL; state=INACTIVE; LastPosition = -1; }
		~stream();
		void CallAudioCallback(char *buf, unsigned long frames, int outTime, stream *str );
	};
	friend struct stream;

	/* Pool of available streams. */
	vector<stream *> stream_pool;

	IDirectSound8 *ds8;

	bool shutdown; /* tells the MixerThread to shut down */
	static int MixerThread_start(void *p);
	void MixerThread();
	SDL_Thread *MixerThreadPtr;

	/* virtuals: */
	void StartMixing(RageSound *snd);	/* used by RageSound */
	void StopMixing(RageSound *snd);		/* used by RageSound */
	int GetPosition(const RageSound *snd) const;
	void Update(float delta);

public:
	RageSound_DSound();
	~RageSound_DSound();
};

#endif

/*
 * Copyright (c) 2002 by the person(s) listed below.  All rights reserved.
 *
 * Glenn Maynard
 */
