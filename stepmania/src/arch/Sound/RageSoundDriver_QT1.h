#ifndef RAGE_SOUND_QT1
#define RAGE_SOUND_QT1
/*
 *  RageSoundDriver_QT1.h
 *  stepmania
 *
 *  Use only a single Sound Manager channel to output sound.
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
*/
namespace QT {
#include <QuickTime/QuickTime.h>
}
#include "RageSound.h"
#include "RageSoundDriver.h"

class RageSound_QT1: public RageSoundDriver {
private:
    struct sound {
        RageSound *snd;
        bool stopping;
        int flush_pos;
        sound() { snd=NULL; stopping=false; flush_pos=0; }
    };

    vector<sound *> sounds;
    QT::ComponentInstance clock;
    QT::SndChannelPtr channel;
    int last_pos;
    float latency;

protected:
    virtual void StartMixing(RageSound *snd);
    virtual void StopMixing(RageSound *snd);
    virtual int GetPosition(const RageSound *snd) const;
    virtual void Update (float delta);
    virtual float GetPlayLatency() const;

public:
    RageSound_QT1();
    virtual ~RageSound_QT1();
    static void GetData(QT::SndChannel *chan, QT::SndCommand *cmd_passed);
};

#endif /* RAGE_SOUND_QT1 */
