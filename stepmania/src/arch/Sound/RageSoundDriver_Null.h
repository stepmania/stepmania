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

class RageSound_Null: public RageSoundDriver
{
public:
	struct sound {
		RageSound *snd;
                bool stopping;
                int flush_pos; /* state == STOPPING only */
            sound() { snd = NULL; stopping=false; }
        };

        /* List of currently playing sounds: */
        vector<sound *> sounds;


	bool shutdown;
        int last_cursor_pos;
        time_t startup_time;

        static int MixerThread_start(void *p);
        void MixerThread();
        SDL_Thread *MixerThreadPtr;

        bool GetData();

	/* virtuals: */
	void StartMixing(RageSound *snd);
	void StopMixing(RageSound *snd);
	int GetPosition(const RageSound *snd) const;
	float GetPlayLatency() const;

	void RageSound_Null::Update(float delta);

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
