#include "global.h"
#include "RageSoundDriver_CA.h"
#include "CAHelpers.h"

#include "CAAudioHardwareSystem.h"
#include "CAAudioHardwareDevice.h"
#include "CAStreamBasicDescription.h"
#include "CAException.h"
#include "archutils/Unix/CrashHandler.h"


namespace
{
    AudioConverter *gConverter;
}

RageSound_CA::RageSound_CA()
{
    try
    {
        AudioDeviceID dID = CAAudioHardwareSystem::GetDefaultDevice(false, false);
    
        mOutputDevice = new CAAudioHardwareDevice(dID);
    }
    catch (const CAException& e)
    {
        RageException::ThrowNonfatal("Couldn't create default output device.");
    }
    try
    {
        UInt32 frames = mOutputDevice->GetLatency(kAudioDeviceSectionOutput);
        mLatency = frames / 44100.0; // maybe? I'm tired, I don't know.
    }
    catch (const CAException& e)
    {
        RageException::ThrowNonfatal("Couldn't get Latency.");
    }
    
    gConverter = new AudioConverter(mOutputDevice, this);
    
    try
    {
        mOutputDevice->AddIOProc(GetData, this);
        mOutputDevice->StartIOProc(GetData);
    }
    catch(const CAException& e)
    {
        RageException::Throw("Couldn't start the IOProc.");
    }
}

RageSound_CA::~RageSound_CA()
{
    mOutputDevice->StopIOProc(GetData);
    delete gConverter;
    delete mOutputDevice;
}

int64_t RageSound_CA::GetPosition(const RageSoundBase *sound) const
{
    AudioTimeStamp time;
    
    mOutputDevice->GetCurrentTime(time);
    return int64_t(time.mSampleTime);
}

void RageSound_CA::FillConverter(void *data, UInt32 dataByteSize)
{
    int frames = dataByteSize / gConverter->GetInputFormat().mBytesPerPacket;
    this->Mix((int16_t *)data, frames, mDecodePos, GetPosition(NULL));
}

OSStatus RageSound_CA::GetData(AudioDeviceID inDevice,
                               const AudioTimeStamp *inNow,
                               const AudioBufferList *inInputData,
                               const AudioTimeStamp *inInputTime,
                               AudioBufferList *outOutputData,
                               const AudioTimeStamp *inOutputTime,
                               void *inClientData)
{
    RageSound_CA *This = (RageSound_CA *)inClientData;
    UInt32 dataPackets = outOutputData->mBuffers[0].mDataByteSize;
    
    dataPackets /= gConverter->GetOutputFormat().mBytesPerPacket;
    
    This->mDecodePos = int64_t(inOutputTime->mSampleTime);
    gConverter->FillComplexBuffer(dataPackets, *outOutputData, NULL);
    return noErr;
}
