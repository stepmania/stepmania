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
#if CURRENT_DRIVER == BROKEN_DRIVER_2
#include <AudioUnit/AudioUnit.h>
#endif


const Float64	kSampleRate = 44100.000;
const UInt32	kChannelsPerFrame = 2;
const UInt32	kBitsPerChannel = 16;
const UInt32	kSampleSize = kChannelsPerFrame * kBitsPerChannel / 8;
const UInt32	kPacketSize = kSampleSize;


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
#if CURRENT_DRIVER != BROKEN_DRIVER_1 && CURRENT_DRIVER != BROKEN_DRIVER_2
    RageException::ThrowNonfatal("Class not finished");
#endif
#if CURRENT_DRIVER == BROKEN_DRIVER_1
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

    /*thePropertySize = sizeof(UInt32);
    UInt32 frames = 1024;
    err = AudioStreamSetProperty(outputDevice, NULL, kAudioPropertyWildcardChannel,
                                 kAudioDevicePropertyBufferFrameSize, thePropertySize, &frames);
    TEST_ERR(err);*/

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
#elif CURRENT_DRIVER == BROKEN_DRIVER_2
    /* Open the default output unit */
    OSStatus err;
    ComponentDescription desc;
    
    desc.componentType = kAudioUnitType_Output;
    desc.componentSubType = kAudioUnitSubType_DefaultOutput;
    desc.componentManufacturer = kAudioUnitManufacturer_Apple;
    desc.componentFlags = 0;
    desc.componentFlagsMask = 0;

    Component comp = FindNextComponent(NULL, &desc);
    RAGE_ASSERT_M(comp != NULL, "Couldn't find the component");

    err = OpenAComponent(comp, &outputUnit);
    TEST_ERR(err);

    /* Set the callback */
    AURenderCallbackStruct callback;
    callback.inputProc = GetData;
    callback.inputProcRefCon = this;

    err = AudioUnitSetProperty(outputUnit,
                               kAudioUnitProperty_SetRenderCallback,
                               kAudioUnitScope_Input,
                               0,
                               &callback,
                               sizeof(callback));
    TEST_ERR(err);

    /* Set the input format */
    AudioStreamBasicDescription idealFormat;

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

    err = AudioUnitSetProperty(outputUnit,
                               kAudioUnitProperty_StreamFormat,
                               kAudioUnitScope_Input,
                               0,
                               &idealFormat,
                               sizeof(idealFormat));
    TEST_ERR(err);    

    /* Calculate the latency */
    latency = 0;

    /* Initialize the unit */
    err = AudioUnitInitialize(outputUnit);
    TEST_ERR(err);

    /* Get the underlying device */
    UInt32 propertySize = sizeof(outputDevice);
    err = AudioUnitGetProperty(outputUnit,
                               kAudioOutputUnitProperty_CurrentDevice,
                               kAudioUnitScope_Global,
                               0,
                               &outputDevice,
                               &propertySize);
    TEST_ERR(err);
    RAGE_ASSERT(outputDevice);

    /* Start the outputUnit */
    AudioOutputUnitStart(outputUnit);
    
#endif
}

RageSound_CA::~RageSound_CA()
{
#if CURRENT_DRIVER == BROKEN_DRIVER_1
    OSStatus err;

    err = AudioDeviceStop(outputDevice, GetData);
    TEST_ERR(err);

    err = AudioDeviceRemoveIOProc(outputDevice, GetData);
    TEST_ERR(err);

    err = AudioConverterDispose(converter);
    TEST_ERR(err);
#elif CURRENT_DRIVER == BROKEN_DRIVER_2
    AudioOutputUnitStop(outputUnit);
    CloseComponent(outputUnit);
#endif
}

void RageSound_CA::StartMixing(RageSoundBase *snd)
{
    LockMutex L(SOUNDMAN->lock);
    soundPtr s = new sound;
    s->snd = snd;
    sounds.push_back(s);
}

void RageSound_CA::StopMixing(RageSoundBase *snd)
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

int RageSound_CA::GetPosition(const RageSoundBase *snd) const
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

#if CURRENT_DRIVER == BROKEN_DRIVER_1
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

    return AudioConverterConvertBuffer(THIS->converter, THIS->buffersize, buffer, &size,
                                       outOutputData->mBuffers[0].mData);
}

#elif CURRENT_DRIVER == BROKEN_DRIVER_2

OSStatus RageSound_CA::GetData(void							*inRefCon,
                               AudioUnitRenderActionFlags	*inActionFlags,
                               const AudioTimeStamp			*inTimeStamp,
                               UInt32						inBusNumber,
                               UInt32						inNumFrames,
                               AudioBufferList				*ioData)
{
    unsigned outputSize = ioData->mBuffers[0].mDataByteSize;
    if (!SOUNDMAN)
    {
        bzero(ioData->mBuffers[0].mData, outputSize);
        *inActionFlags = kAudioUnitRenderAction_OutputIsSilence;
        return noErr;
    }
    RageSound_CA *THIS = (RageSound_CA *)inRefCon;
    char *buffer = (char *)ioData->mBuffers[0].mData;

    bzero(buffer, THIS->buffersize);
    LockMutex L(SOUNDMAN->lock);
    static SoundMixBuffer mix;
    int position = int(inTimeStamp->mSampleTime); //This should be in frames
    bool moreThanOneSound = THIS->sounds.size() > 1;
    
    for (unsigned i=0; i<THIS->sounds.size(); ++i)
    {
        if (THIS->sounds[i]->stopping)
            continue;

        unsigned got = THIS->sounds[i]->snd->GetPCM(buffer, outputSize, position);
        if (moreThanOneSound)
            mix.write((SInt16 *)buffer, got / 2);
        if (got < outputSize)
        {
            THIS->sounds[i]->stopping = true;
            THIS->sounds[i]->flush_pos = position + got / kSampleSize;
        }
    }

    if (moreThanOneSound)
        mix.read((SInt16 *)buffer);
    *inActionFlags = kAudioUnitRenderAction_PostRender;

    return noErr;
}

#endif

inline int RageSound_CA::ConvertAudioTimeStampToPosition(const AudioTimeStamp *time)
{
    return (int)(time->mSampleTime / time->mRateScalar);
    //return int(time->mSampleTime);
}    
