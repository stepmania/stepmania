#ifndef RAGE_SOUND_NULL
#define RAGE_SOUND_NULL

/* RageSound_Null fakes playing sounds having GetPosition()
 * return seconds since the constructor was called.
 *
 * Only tested in linux, but intended to work across the globe.
 * ( uses time_t, sleep() and nanosleep() from <time.h> )
 *
 * The timing probably isn't accurate, but at least it is fairly
 * steady.  Someone with more knowledge of RageSound, feel free
 * to play around with this (not that it's any sort of priority).
 */

#include "RageSound.h"
#include <time.h>
#include "RageSoundDriver.h"
#include "RageThreads.h"

class RageSound_Null: public RageSoundDriver
{
private:
	struct sound {
		RageSound *snd;
        bool stopping;
		int samples_buffered; /* fake */
        sound() { snd = NULL; stopping=false; samples_buffered = 0; }
	};

    /* List of currently playing sounds: */
    vector<sound *> sounds;

	bool shutdown;
    SDL_Thread *MixerThreadPtr;

    static int MixerThread_start(void *p);
    void MixerThread();

protected:
	/* virtuals: */
	virtual void StartMixing(RageSound *snd);
	virtual void StopMixing(RageSound *snd);
	virtual int GetPosition(const RageSound *snd) const;
	virtual float GetPlayLatency() const;
	virtual void Update(float delta);
    virtual bool GetData();

public:
    RageSound_Null();
    ~RageSound_Null();
};

#endif

/*
 * Copyright (c) 2003 by the person(s) listed below.  All rights reserved.
 *
 * Glenn Maynard
 * 
 * 2003-02   Modified to fake playing sound   Aaron VonderHaar
 * 
 */
