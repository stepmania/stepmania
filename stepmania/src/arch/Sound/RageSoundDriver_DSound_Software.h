#ifndef RAGE_SOUND_DSOUND_SOFTWARE
#define RAGE_SOUND_DSOUND_SOFTWARE

#include "RageSoundDriver.h"
#include "DSoundHelpers.h"
#include "RageThreads.h"
#include "RageTimer.h"

struct IDirectSound;
struct IDirectSoundBuffer;

class RageSound_DSound_Software: public RageSoundDriver
{
	struct sound {
	    RageSoundBase *snd;
		RageTimer start_time;
		bool stopping;
		int64_t flush_pos; /* when stopping only */

	    sound() { snd = NULL; stopping=false; }
	};

	/* List of currently playing sounds: */
	vector<sound *> sounds;

	bool shutdown;

	DSound ds;
	DSoundBuf *pcm;

	bool GetData();
	void Update(float delta);

	static int MixerThread_start(void *p);
	void MixerThread();
	RageThread MixingThread;

	/* virtuals: */
	void StartMixing( RageSoundBase *snd );	/* used by RageSound */
	void StopMixing( RageSoundBase *snd );		/* used by RageSound */
	int64_t GetPosition( const RageSoundBase *snd ) const;
	float GetPlayLatency() const;
	int GetSampleRate( int rate ) const;

public:
	RageSound_DSound_Software();
	~RageSound_DSound_Software();
};

#endif

/*
 * Copyright (c) 2002-2004 by the person(s) listed below.  All rights reserved.
 *
 * Glenn Maynard
 */
