#ifndef RAGE_SOUND_NULL
#define RAGE_SOUND_NULL

#include "RageSound.h"

class RageSound_Null: public RageSoundDriver
{
public:
	/* virtuals: */
	void StartMixing(RageSound *snd) { }
	void StopMixing(RageSound *snd) { }
	int GetPosition(const RageSound *snd) const { return 0; }
	float GetPlayLatency() const { return 0.0f; } /* silence is fast! */
};

#endif

/*
 * Copyright (c) 2003 by the person(s) listed below.  All rights reserved.
 *
 * Glenn Maynard
 */
