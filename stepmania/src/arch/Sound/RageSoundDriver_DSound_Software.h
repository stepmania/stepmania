#ifndef RAGE_SOUND_DSOUND_SOFTWARE
#define RAGE_SOUND_DSOUND_SOFTWARE

#include "RageSoundDriver.h"
#include "SDL_Thread.h"

struct IDirectSound8;
struct IDirectSoundBuffer8;

class RageSound_DSound_Software: public RageSoundDriver
{
	struct sound {
	    RageSound *snd;

		bool stopping;

		int flush_bufs; /* state == STOPPING only */

	    sound() { snd = NULL; stopping=false; }
	};

	void GetPCM();

	bool shutdown;

	int last_cursor_filled, last_cursor_pos;

	void Update(float delta);

	IDirectSound8 *ds8;
	IDirectSoundBuffer8 *str_ds;

	static int MixerThread_start(void *p);
	void MixerThread();
	SDL_Thread *MixerThreadPtr;

	/* List of currently playing sounds: */
	vector<sound *> sounds;

	mutable int LastPosition;

	/* virtuals: */
	void StartMixing(RageSound *snd);	/* used by RageSound */
	void StopMixing(RageSound *snd);		/* used by RageSound */
	int GetPosition(const RageSound *snd) const;
	float GetPlayLatency() const;

public:
	RageSound_DSound_Software();
	~RageSound_DSound_Software();
};

#endif

/*
 * Copyright (c) 2002 by the person(s) listed below.  All rights reserved.
 *
 * Glenn Maynard
 */
