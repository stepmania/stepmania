#ifndef RAGE_SOUND_CA
#define RAGE_SOUND_CA
/*
 *  RageSoundDriver_CA.h
 *  stepmania
 *
 *  Sound stream pool code based on RageSoundDriver_ALSA9.h/.cpp by Glenn Maynard.
 *
 * Copyright (c) 2003 by the person(s) listed below.  All rights reserved.
 *	Steve Checkoway
 *	Charlie Reading
 *
 */

#include "RageSoundDriver.h"
#include "RageThreads.h"

#define FEEDER_DIRECT			0
#define FEEDER_THREAD			1
#define FEEDER					FEEDER_DIRECT

#define RESYNC_NEVER			0
#define RESYNC_INSTANTANEOUS	1
#define RESYNC_ACCUMULATE		2
#define RESYNC					RESYNC_ACCUMULATE


struct AudioTimeStamp;
struct AudioBufferList;
typedef unsigned char Boolean;
typedef long OSStatus;
typedef unsigned long UInt32;
typedef UInt32 AudioDeviceID;
typedef UInt32	AudioDevicePropertyID;
typedef UInt32 AudioUnitRenderActionFlags;
typedef double Float64;
struct OpaqueAudioConverter;
typedef struct OpaqueAudioConverter *AudioConverterRef;
struct AudioStreamBasicDescription;
#if (FEEDER == FEEDER_THREAD)
class VirtualRingBuffer;
#endif

class RageSound_CA: public RageSoundDriver
{
private:
	/* Sound stream */
    struct stream
    {
		enum { INACTIVE, PLAYING, STOPPING } state;

        RageSoundBase *snd;
        int flush_pos;

		void clear() { snd=NULL; state=INACTIVE; flush_pos=0; }
        stream() { clear(); }

    };
	friend struct stream;

	/* Pool of available streams. */
	vector<stream *> stream_pool;
	int streamsInUse;
	
    AudioDeviceID outputDevice;
	AudioStreamBasicDescription *idealFormat;
	AudioStreamBasicDescription *actualFormat;
    UInt32 buffersize;
	int samplesPerBuffer;

    AudioConverterRef converter;
	
	bool shutdown;
    float latency;

	Float64 startSampleTime;
	int expected;
#if (RESYNC != RESYNC_NEVER)
	int packetsOff;
	int packetsOffSamples;
#endif
	
#if (FEEDER == FEEDER_THREAD)
	size_t vrbChunkSize;
	VirtualRingBuffer *vrb;
	RageThread feederThread;
	int nextSampleToWrite;

	static int FeederThread_start(void *p);
	void FeederThread();
#endif

    int64_t ConvertAudioTimeStampToPosition(const AudioTimeStamp *time) const;

protected:
    void StartMixing(RageSoundBase *snd);
    void Update (float delta);
    void StopMixing(RageSoundBase *snd);
    int64_t GetPosition(const RageSoundBase *snd) const;
    float GetPlayLatency() const;
	int GetSampleRate() const;

	size_t smSamplesToBytes(int samples);
	int smBytesToSamples(size_t bytes);
	size_t caSamplesToBytes(int samples);
	int caBytesToSamples(size_t bytes);

public:
    RageSound_CA();
    virtual ~RageSound_CA();
	
	virtual size_t FillSoundBuffer(char* buffer, size_t maxBufferSize, int position);
	
    static OSStatus GetData(AudioDeviceID inDevice, const AudioTimeStamp* inNow, const AudioBufferList*	inInputData, const AudioTimeStamp* inInputTime, AudioBufferList* outOutputData, const AudioTimeStamp* inOutputTime, void* inClientData);
	static OSStatus OverloadListener(AudioDeviceID inDevice, UInt32 inChannel, Boolean isInput, AudioDevicePropertyID inPropertyID, void* inClientData);
};

#endif RAGE_SOUND_CA
