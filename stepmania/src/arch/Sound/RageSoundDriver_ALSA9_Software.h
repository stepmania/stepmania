#ifndef RAGE_SOUND_ALSA9_SOFTWARE_H
#define RAGE_SOUND_ALSA9_SOFTWARE_H

#include "RageSound.h"
#include "RageThreads.h"
#include "RageSoundDriver_Generic_Software.h"

#include "ALSA9Helpers.h"

class RageSound_ALSA9_Software: public RageSound_Generic_Software
{
private:
	bool shutdown;

	Alsa9Buf *pcm;

	static int MixerThread_start(void *p);
	void MixerThread();
	RageThread MixingThread;

	bool GetData();

public:
	/* virtuals: */
	int64_t GetPosition( const RageSoundBase *snd ) const;
	float GetPlayLatency() const;
        int GetSampleRate( int rate ) const;

	void SetupDecodingThread();
		

	RageSound_ALSA9_Software();
	~RageSound_ALSA9_Software();
};

#endif

/*
 * Copyright (c) 2003-2004 by the person(s) listed below.  All rights reserved.
 *
 * Glenn Maynard
 * 
 * 2003-02   Modified to fake playing sound   Aaron VonderHaar
 * 
 */
