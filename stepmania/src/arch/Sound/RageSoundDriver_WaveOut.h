#ifndef RAGE_SOUND_WAVEOUT
#define RAGE_SOUND_WAVEOUT

#include "RageSoundDriver_Generic_Software.h"
#include "RageSoundDriver.h"
#include "RageThreads.h"
#include "windows.h"
#include "Mmsystem.h"
#include "RageTimer.h"

class RageSound_WaveOut: public RageSound_Generic_Software
{
	HWAVEOUT wo;
	HANDLE sound_event;
	WAVEHDR buffers[8];
	bool shutdown;
	int last_cursor_pos;

	static int MixerThread_start(void *p);
	void MixerThread();
	RageThread MixingThread;
	bool GetData();

protected:
	void SetupDecodingThread();

public:
	int64_t GetPosition( const RageSoundBase *snd ) const;
	float GetPlayLatency() const;

	RageSound_WaveOut();
	~RageSound_WaveOut();
};

#endif

/*
 * Copyright (c) 2002-2004 by the person(s) listed below.  All rights reserved.
 *
 * Glenn Maynard
 */
