/*
 *  RageSoundDriver_CA.cpp
 *  stepmania
 *
 *  Created by Steve Checkoway on Tue Aug 26 2003.
 *  Copyright (c) 2003 Steve Checkoway. All rights reserved.
 *
 */

#include "global.h"
#include "RageSoundDriver_CA.h"
#include "RageSoundManager.h"
#include "RageSound.h"
#include "RageLog.h"
#include "RageThreads.h"

#include <vector>
#include <AudioToolbox/AudioToolbox.h>
#include <CoreAudio/CoreAudio.h>

const Float64	kSampleRate = 44100.000;
const UInt32	kChannelsPerFrame = 2;
const UInt32	kBitsPerChannel = 16;
const UInt32	kSampleSize = kChannelsPerFrame * kBitsPerChannel / 8;
const UInt32	kPacketSize = kSampleSize;


static AudioDeviceID outputDevice;

#if defined(DEBUG)
static char *__errorMessage;
static char __fourcc[5];
#define TEST_ERR(result) \
CHECKPOINT; \
memcpy(__fourcc, &result, 4); \
__fourcc[4] = '\000'; \
asprintf(&__errorMessage, "err = %d, %s", result, __fourcc); \
RAGE_ASSERT_M(result == noErr, __errorMessage); \
free(__errorMessage)

#else
#define TEST_ERR(result)
#endif

RageSound_CA::RageSound_CA()
{
    UInt32 thePropertySize = sizeof(AudioDeviceID);
    OSStatus err;
    
    err = AudioHardwareGetProperty(kAudioHardwarePropertyDefaultOutputDevice, &thePropertySize,
                                   &outputDevice);
    TEST_ERR(err);

    AudioStreamBasicDescription streamFormat, idealFormat;
    
    thePropertySize = sizeof(streamFormat);

    idealFormat.mSampleRate = kSampleRate;
    idealFormat.mFormatID = kAudioFormatLinearPCM;
    idealFormat.mFormatFlags =  kLinearPCMFormatFlagIsBigEndian | kLinearPCMFormatFlagIsPacked
        | kLinearPCMFormatFlagIsSignedInteger;
    idealFormat.mBytesPerPacket = kPacketSize;
    idealFormat.mFramesPerPacket = 1;
    idealFormat.mBytesPerFrame = kPacketSize;
    idealFormat.mChannelsPerFrame = kChannelsPerFrame;
    idealFormat.mBitsPerChannel = kBitsPerChannel;
    idealFormat.mReserved = 0;

    memcpy(&streamFormat, &idealFormat, sizeof(streamFormat));


    err = AudioDeviceGetProperty(outputDevice, 0, 0, kAudioDevicePropertyStreamFormatMatch, &thePropertySize,
                                 &streamFormat);
    TEST_ERR(err);

    /* Create the converter to convert from the ideal to the actual format */
    err = AudioConverterNew(&idealFormat, &streamFormat, &converter);
    TEST_ERR(err);

    /* Calculate the input buffersize */
    thePropertySize = sizeof(UInt32);
    err = AudioDeviceGetProperty(outputDevice, 0, 0, kAudioDevicePropertyBufferSize, &thePropertySize,
                                 &buffersize);
    TEST_ERR(err);

    err = AudioConverterGetProperty(converter, kAudioConverterPropertyCalculateInputBufferSize,
                                    &thePropertySize, &buffersize);
    TEST_ERR(err);

    char fourcc[5];
    memcpy(fourcc, &streamFormat.mFormatID, 4);
    fourcc[4] = '\000';

    LOG->Info("rate:                %f\n"
              "Format ID:           %s\n"
              "Format Flags:        0x%X\n"
              "Bytes per packet:    %u\n"
              "Frames per packet:   %u\n"
              "Bytes per frame:     %u\n"
              "Channels per frame:  %u\n"
              "Bits per channel:    %u\n",
              streamFormat.mSampleRate, fourcc, streamFormat.mFormatFlags, streamFormat.mBytesPerPacket,
              streamFormat.mFramesPerPacket, streamFormat.mBytesPerFrame, streamFormat.mChannelsPerFrame,
              streamFormat.mBitsPerChannel);    

    thePropertySize = sizeof(streamFormat);
    err = AudioDeviceSetProperty(outputDevice, NULL, 0, 0, kAudioDevicePropertyStreamFormat,
                                 thePropertySize, &streamFormat);
    TEST_ERR(err);
    

    err = AudioDeviceAddIOProc(outputDevice, GetData, this);
    TEST_ERR(err);

    err = AudioDeviceStart(outputDevice, GetData);
    TEST_ERR(err);

    latency = 0; //for now
}

RageSound_CA::~RageSound_CA()
{
    OSStatus err;

    err = AudioDeviceStop(outputDevice, GetData);
    TEST_ERR(err);

    err = AudioDeviceRemoveIOProc(outputDevice, GetData);
    TEST_ERR(err);

    err = AudioConverterDispose(converter);
    TEST_ERR(err);
}

void RageSound_CA::StartMixing(RageSound *snd)
{
    LockMutex L(SOUNDMAN->lock);
    soundPtr s = new sound;
    s->snd = snd;
    sounds.push_back(s);
}

void RageSound_CA::StopMixing(RageSound *snd)
{
    LockMutex L(SOUNDMAN->lock);

    /* Find the sound. */
    unsigned i;
    for(i = 0; i < sounds.size(); ++i)
        if(sounds[i]->snd == snd) break;
    if(i == sounds.size())
    {
        LOG->Trace("not stopping a sound because it's not playing");
        return;
    }

    delete sounds[i];
    sounds.erase(sounds.begin()+i);
}

int RageSound_CA::GetPosition(const RageSound *snd) const
{
    AudioTimeStamp time;
    OSStatus err = AudioDeviceGetCurrentTime(outputDevice, &time);
    TEST_ERR(err);

    return ConvertAudioTimeStampToPosition(&time);
}

void RageSound_CA::Update(float delta)
{
#pragma unused(delta)
    LockMutex L(SOUNDMAN->lock);

    vector<sound *>snds = sounds;
    for (unsigned i = 0; i < snds.size(); ++i) {
        if (!sounds[i]->stopping)
            continue;
        if (GetPosition(snds[i]->snd) < sounds[i]->flush_pos)
            continue;
        snds[i]->snd->StopPlaying();
    }
}

OSStatus RageSound_CA::GetData(AudioDeviceID			inDevice,
                 const AudioTimeStamp*	inNow,
                 const AudioBufferList*	inInputData,
                 const AudioTimeStamp*	inInputTime,
                 AudioBufferList*		outOutputData,
                 const AudioTimeStamp*	inOutputTime,
                 void*					inClientData)
{
    unsigned outputSize = outOutputData->mBuffers[0].mDataByteSize;
    if (!SOUNDMAN)
    {
        bzero(outOutputData->mBuffers[0].mData, outputSize);
        return noErr;
    }
    RageSound_CA *THIS = (RageSound_CA *)inClientData;
    char buffer[THIS->buffersize];

    bzero(buffer, THIS->buffersize);
    LockMutex L(SOUNDMAN->lock);
    static SoundMixBuffer mix;
    int position = ConvertAudioTimeStampToPosition(inNow);
    bool moreThanOneSound = THIS->sounds.size() > 1;

    for (unsigned i=0; i<THIS->sounds.size(); ++i)
    {
        if (THIS->sounds[i]->stopping)
            continue;

        unsigned got = THIS->sounds[i]->snd->GetPCM(buffer, THIS->buffersize, position);
        if (moreThanOneSound)
            mix.write((SInt16 *)buffer, got / 2);
        if (got < THIS->buffersize)
        {
            THIS->sounds[i]->stopping = true;
            THIS->sounds[i]->flush_pos = position + got / kSampleSize;
        }
    }

    if (moreThanOneSound)
        mix.read((SInt16 *)buffer);

    
    UInt32 size;

    OSStatus err = AudioConverterConvertBuffer(THIS->converter, THIS->buffersize, buffer, &size,
                                               outOutputData->mBuffers[0].mData);
    
    return err;
}

inline int RageSound_CA::ConvertAudioTimeStampToPosition(const AudioTimeStamp *time)
{
    return (int)(time->mSampleTime / time->mRateScalar);
}    
