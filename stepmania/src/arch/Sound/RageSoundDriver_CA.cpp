#include "global.h"
#include "RageSoundDriver_CA.h"
#include "CAHelpers.h"

#include "CAAudioHardwareSystem.h"
#include "CAAudioHardwareDevice.h"
#include "CAAudioHardwareStream.h"
#include "CAStreamBasicDescription.h"
#include "CAException.h"
#include "archutils/Unix/CrashHandler.h"
#include <mach/thread_act.h>
#include <mach/mach_init.h>

static AudioConverter *gConverter;

static Desc FindClosestFormat(const vector<Desc>& formats)
{
	vector<Desc> v;

	vector<Desc>::const_iterator i;
	for (i = formats.begin(); i != formats.end(); ++i)
	{
		const Desc& format = *i;

		if (!format.IsPCM() || format.mSampleRate != 44100.0)
			continue;

		if (format.SampleWordSize() == 2 &&
				(format.mFormatFlags & kAudioFormatFlagIsSignedInteger) ==
				kAudioFormatFlagIsSignedInteger)
		{ // exact match
			return format;
		}
		v.push_back(format);
	}

	for (i = v.begin(); i != v.end(); ++i)
	{
		const Desc& format = *i;
		if (format.SampleWordSize() == 2)
		{
			return format; // close
		}
	}
	if (v.empty())
		RageException::ThrowNonfatal("Couldn't find a close format.");
	return v[0]; // something is better than nothing.
}

static void GetIOProcFormats( CAAudioHardwareStream stream, vector<Desc> &procFormats )
{
	UInt32 numFormats = stream.GetNumberAvailableIOProcFormats();

	for( UInt32 i=0; i<numFormats; ++i )
	{
		Desc desc;

		stream.GetAvailableIOProcFormatByIndex( i, desc );
		procFormats.push_back( desc );
	}
}

static void GetPhysicalFormats( CAAudioHardwareStream stream, vector<Desc> &physicalFormats )
{
	UInt32 numFormats = stream.GetNumberAvailablePhysicalFormats();
	for( UInt32 i=0; i<numFormats; ++i )
	{
		Desc desc;

		stream.GetAvailablePhysicalFormatByIndex( i, desc );
		physicalFormats.push_back( desc );
	}
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
        mLatency = frames / 44100.0;
    }
    catch (const CAException& e)
    {
        delete mOutputDevice;
        RageException::ThrowNonfatal("Couldn't get Latency.");
    }
    
    AudioStreamID sID = mOutputDevice->GetStreamByIndex( kAudioDeviceSectionOutput, 0 );
    CAAudioHardwareStream stream( sID );

    vector<Desc> physicalFormats;
    GetPhysicalFormats( sID, physicalFormats );
    const Desc& physicalFormat = FindClosestFormat( physicalFormats );
    stream.SetCurrentPhysicalFormat( physicalFormat );

    vector<Desc> procFormats;
    GetIOProcFormats( sID, procFormats );
    const Desc& procFormat = FindClosestFormat( procFormats );
    stream.SetCurrentIOProcFormat( procFormat );

	StartDecodeThread();

    gConverter = new AudioConverter( this, procFormat );
    
    try
    {
        mOutputDevice->AddIOProc(GetData, this);
        mOutputDevice->StartIOProc(GetData);
    }
    catch(const CAException& e)
    {
        delete gConverter;
        delete mOutputDevice;
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

void RageSound_CA::SetupDecodingThread()
{
	/* Increase the scheduling precedence of the decoder thread. */
	thread_precedence_policy po;
	po.importance = 5;
	thread_policy_set( mach_thread_self(), THREAD_PRECEDENCE_POLICY,
                       (int *)&po, THREAD_PRECEDENCE_POLICY_COUNT );
}

