#ifndef RAGE_SOUND_CA
#define RAGE_SOUND_CA
/*
 *  RageSoundDriver_CA.h
 *  stepmania
 *
 *  Created by Steve Checkoway on Tue Aug 26 2003.
 *  Copyright (c) 2003 Steve Checkoway. All rights reserved.
 *
 */

#include "RageSoundDriver.h"

struct AudioTimeStamp;
struct AudioBufferList;
struct OpaqueAudioConverter;
typedef struct OpaqueAudioConverter *AudioConverterRef;
typedef long OSStatus;
typedef unsigned long UInt32;
typedef UInt32 AudioDeviceID;

class RageSound_CA : public RageSoundDriver
{
private:
    typedef struct sound {
        RageSound *snd;
        bool stopping;
        int flush_pos;
        sound() { snd=NULL; stopping=false; flush_pos=0; }
    } *soundPtr;

    vector<soundPtr> sounds;
    AudioConverterRef converter;
    float latency;
    UInt32 buffersize;

    static int ConvertAudioTimeStampToPosition(const AudioTimeStamp *time);
protected:
    virtual void StartMixing(RageSound *snd);
    virtual void StopMixing(RageSound *snd);
    virtual int GetPosition(const RageSound *snd) const;
    virtual void Update (float delta);
    virtual float GetPlayLatency() const { return latency; }

public:
    RageSound_CA();
    virtual ~RageSound_CA();
    static OSStatus GetData(AudioDeviceID			inDevice,
                            const AudioTimeStamp*	inNow,
                            const AudioBufferList*	inInputData,
                            const AudioTimeStamp*	inInputTime,
                            AudioBufferList*		outOutputData,
                            const AudioTimeStamp*	inOutputTime,
                            void*					inClientData);
};

#endif RAGE_SOUND_CA
