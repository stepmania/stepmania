#ifndef RAGE_SOUND_GENERIC_TEST
#define RAGE_SOUND_GENERIC_TEST

#include "DSoundHelpers.h"
#include "RageThreads.h"
#include "RageSoundDriver_Generic_Software.h"

class RageSound_DSound_Software: public RageSound_Generic_Software
{
	DSound ds;
	DSoundBuf *pcm;

	bool shutdown_mixer_thread;

	static int MixerThread_start(void *p);
	void MixerThread();
	RageThread MixingThread;

protected:
	void SetupDecodingThread();

public:
	int64_t GetPosition( const RageSoundBase *snd ) const;
	float GetPlayLatency() const;
	int GetSampleRate( int rate ) const;
	
	RageSound_DSound_Software();
	virtual ~RageSound_DSound_Software();
};

#endif

/*
 * Copyright (c) 2002-2004 by the person(s) listed below.  All rights reserved.
 *
 * Glenn Maynard
 */
