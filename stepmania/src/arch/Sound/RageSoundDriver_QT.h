#ifndef RAGE_SOUND_QT
#define RAGE_SOUND_QT
/*
 *  RageSoundDriver_QT.h
 *  stepmania
 *
 *  Use multiple Sound Manager channels to output sound.
 *
 *  Created by Steve Checkoway on Mon Jun 23 2003.
 *  Copyright (c) 2003 Steve Checkoway. All rights reserved.
 *
 */

#include "RageSoundDriver.h"

class RageSound_QT: public RageSoundDriver {
protected:
	virtual void StartMixing(RageSoundBase *snd);
	virtual void StopMixing(RageSoundBase *snd);
	virtual int GetPosition(const RageSoundBase *snd) const;
	virtual void Update (float delta);
	virtual float GetPlayLatency() const;

public:
	RageSound_QT();
	virtual ~RageSound_QT();
};

#endif /* RAGE_SOUND_QT */
