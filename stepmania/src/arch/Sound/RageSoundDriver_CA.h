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
#define BROKEN_DRIVER_1 1
#define BROKEN_DRIVER_2 2

#define CURRENT_DRIVER BROKEN_DRIVER_2

struct AudioTimeStamp;
struct AudioBufferList;
#if CURRENT_DRIVER == BROKEN_DRIVER_1
struct OpaqueAudioConverter;
typedef struct OpaqueAudioConverter *AudioConverterRef;
#endif
typedef long OSStatus;
typedef unsigned long UInt32;
typedef UInt32 AudioDeviceID;
typedef UInt32 AudioUnitRenderActionFlags;
#if CURRENT_DRIVER == BROKEN_DRIVER_2
struct ComponentInstanceRecord;
typedef struct ComponentInstanceRecord *AudioUnit;
#endif

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
#if CURRENT_DRIVER == BROKEN_DRIVER_1
    AudioConverterRef converter;
#elif CURRENT_DRIVER == BROKEN_DRIVER_2
    AudioUnit outputUnit;
#endif
    AudioDeviceID outputDevice;
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
#if CURRENT_DRIVER == BROKEN_DRIVER_1
    static OSStatus GetData(AudioDeviceID			inDevice,
                            const AudioTimeStamp*	inNow,
                            const AudioBufferList*	inInputData,
                            const AudioTimeStamp*	inInputTime,
                            AudioBufferList*		outOutputData,
                            const AudioTimeStamp*	inOutputTime,
                            void*					inClientData);
#elif CURRENT_DRIVER == BROKEN_DRIVER_2
    static  OSStatus GetData(void						*inRecCon,
                             AudioUnitRenderActionFlags *inActionFlags,
                             const AudioTimeStamp		*inTimeStamp,
                             UInt32						inBusNumber,
                             UInt32						inNumFrames,
                             AudioBufferList			*ioData);
#endif
};

#endif RAGE_SOUND_CA
