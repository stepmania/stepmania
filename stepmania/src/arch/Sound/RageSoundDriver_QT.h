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

/* ugh, what a hack! QT includes Carbon/Carbon.h which
* has defs for a few things defined in RageUtil.h which
* is included in arch.cpp along w/ (eventually) this.
* How do I do this without using phony namespaces?
* --Steve
*
namespace QT {
#include <QuickTime/QuickTime.h>
}*/
#include "RageSoundDriver.h"

class RageSound_QT: public RageSoundDriver {
protected:
	virtual void StartMixing(RageSound *snd);
	virtual void StopMixing(RageSound *snd);
	virtual int GetPosition(const RageSound *snd) const;
	virtual void Update (float delta);
	virtual float GetPlayLatency() const { return 0; }

public:
	RageSound_QT();
	virtual ~RageSound_QT();
};

#endif /* RAGE_SOUND_QT */
