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

namespace QT
{
#include <QuickTime/QuickTime.h>
}
#include "RageSound.h"
#include "RageSoundDriver_Generic_Software.h"

class RageSound_QT1: public RageSound_Generic_Software
{
private:
/*    struct sound
    {
        RageSoundBase *snd;
        bool stopping;
        int flush_pos;
        sound() { snd=NULL; stopping=false; flush_pos=0; }
    };*/

//    vector<sound *> sounds;
    QT::ComponentInstance clock;
    QT::SndChannelPtr channel;
    int last_pos;
    float latency;

protected:
//    virtual void StartMixing(RageSoundBase *snd);
//    virtual void StopMixing(RageSoundBase *snd);
    int64_t GetPosition(const RageSoundBase *snd) const;
//    void Update (float delta);
    float GetPlayLatency() const;

public:
    RageSound_QT1();
    virtual ~RageSound_QT1();
    static void GetData(QT::SndChannel *chan, QT::SndCommand *cmd_passed);
};

#endif /* RAGE_SOUND_QT1 */
