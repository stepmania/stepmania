#ifndef RAGE_SOUND_ALSA9_SOFTWARE_H
#define RAGE_SOUND_ALSA9_SOFTWARE_H

#include "RageSound.h"
#include "RageThreads.h"
#include "RageSoundDriver.h"

#include "ALSA9Helpers.h"

class RageSound_ALSA9_Software: public RageSoundDriver
{
private:
	struct sound
	{
		RageSoundBase *snd;
		RageTimer start_time;
		bool stopping;
		int64_t flush_pos; /* state == STOPPING only */
		sound() { snd = NULL; stopping=false; }
	};

	/* List of currently playing sounds: */
	vector<sound *> sounds;

	bool shutdown;

	Alsa9Buf *pcm;

	static int MixerThread_start(void *p);
	void MixerThread();
	RageThread MixingThread;

	bool GetData();

public:
	/* virtuals: */
	void StartMixing(RageSoundBase *snd);
	void StopMixing(RageSoundBase *snd);
	int64_t GetPosition( const RageSoundBase *snd ) const;
	float GetPlayLatency() const;
        int GetSampleRate( int rate ) const;
	
	void Update(float delta);

	RageSound_ALSA9_Software();
	~RageSound_ALSA9_Software();
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
