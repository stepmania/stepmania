#ifndef RAGE_SOUND_DSOUND
#define RAGE_SOUND_DSOUND

#include "RageSoundDriver.h"
#include "SDL_Thread.h"
#include "DSoundHelpers.h"

struct IDirectSound;
struct IDirectSoundBuffer;

class RageSound_DSound: public RageSoundDriver
{
	struct stream {
	    /* Actual audio stream: */
		DSoundBuf *str_ds;

	    /* Sound object that's playing on this stream, or NULL if this
	     * channel is available: */
	    RageSound *snd;

		enum {
			INACTIVE,
			PLAYING,
			STOPPING
		} state;

		int flush_pos; /* state == STOPPING only */

		bool GetData(bool init);

	    stream() { str_ds = NULL; snd = NULL; state=INACTIVE; }
		~stream();
	};
	friend struct stream;

	/* Pool of available streams. */
	vector<stream *> stream_pool;

	DSound ds;

	bool shutdown; /* tells the MixerThread to shut down */
	static int MixerThread_start(void *p);
	void MixerThread();
	SDL_Thread *MixerThreadPtr;

	/* virtuals: */
	void StartMixing(RageSound *snd);	/* used by RageSound */
	void StopMixing(RageSound *snd);		/* used by RageSound */
	int GetPosition(const RageSound *snd) const;
	void Update(float delta);
	void VolumeChanged();

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
