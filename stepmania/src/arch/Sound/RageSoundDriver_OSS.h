#ifndef RAGE_SOUND_WAVEOUT
#define RAGE_SOUND_WAVEOUT

#include "RageSoundDriver_Generic_Software.h"
#include "RageThreads.h"
#include "RageTimer.h"

class RageSound_OSS: public RageSound_Generic_Software
{
	int fd;

	bool shutdown;
	int last_cursor_pos;
	int samplerate;

	static int MixerThread_start(void *p);
	void MixerThread();
	RageThread MixingThread;

	static void CheckOSSVersion( int fd );
	
public:
	bool GetData();
	int GetSampleRate( int rate ) const { return samplerate; }

	/* virtuals: */
	int64_t GetPosition( const RageSoundBase *snd ) const;
	float GetPlayLatency() const;
	void SetupDecodingThread();

	RageSound_OSS();
	~RageSound_OSS();
};

#endif

/*
 * Copyright (c) 2002 by the person(s) listed below.  All rights reserved.
 *
 * Glenn Maynard
 */
