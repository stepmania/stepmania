#include "global.h"
#include "RageUtil.h"
#include "RageSoundDriver_CA.h"
#include "CAHelpers.h"
#include "RageLog.h"

#include "CAAudioHardwareSystem.h"
#include "CAAudioHardwareDevice.h"
#include "CAAudioHardwareStream.h"
#include "CAStreamBasicDescription.h"
#include "CAException.h"
#include "archutils/Unix/CrashHandler.h"
#include <mach/thread_act.h>
#include <mach/mach_init.h>
#include <mach/mach_error.h>

static AudioConverter *gConverter;

static CString FormatToString( int fmt )
{
	char c[4];
	c[0] = (fmt >> 24) & 0xFF;
	c[1] = (fmt >> 16) & 0xFF;
	c[2] = (fmt >>  8) & 0xFF;
	c[3] = (fmt)       & 0xFF;
    
	/* Sanitize: */
	for( int i = 0; i < 4; ++i )
		if( c[i] < 32 || c[i] >= 127 )
			return ssprintf( "0x%X", fmt );
    
	return ssprintf( "%c%c%c%c", c[0], c[1], c[2], c[3] );
}

static Desc FindClosestFormat(const vector<Desc>& formats)
{
	vector<Desc> v;

	vector<Desc>::const_iterator i;
	for (i = formats.begin(); i != formats.end(); ++i)
	{
		const Desc& format = *i;

		if( !format.IsPCM() )
			continue;

		if( format.mSampleRate != 0. && format.mSampleRate != 44100.0 )
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
        mOutputDevice->SetNominalSampleRate(44100.0);
    }
    catch (const CAException& e)
    {
        RageException::ThrowNonfatal("Couldn't set the nominal sample rate.");
    }
    AudioStreamID sID = mOutputDevice->GetStreamByIndex( kAudioDeviceSectionOutput, 0 );
    CAAudioHardwareStream stream( sID );

    try
    {
        mOutputDevice->AddPropertyListener(kAudioPropertyWildcardChannel,
                                           kAudioPropertyWildcardSection,
                                           kAudioDeviceProcessorOverload,
                                           OverloadListener, this);
    }
    catch (const CAException& e)
    {
        LOG->Warn("Could not install the overload listener.");
    }
    

    vector<Desc> physicalFormats;
    GetPhysicalFormats( sID, physicalFormats );
    unsigned i;
    LOG->Info("Available physical formats:");
    for (i = 0; i < physicalFormats.size(); ++i)
    {
        const Desc& f = physicalFormats[i];
        
        LOG->Info("Format %u:  Rate: %i  ID: %s  Flags 0x%lx  bpp %lu  fpp %lu  bpf %lu  channels %lu  bits %lu",
                  i, (int) f.mSampleRate, FormatToString(f.mFormatID).c_str(), f.mFormatFlags,
                  f.mBytesPerPacket, f.mFramesPerPacket, f.mBytesPerFrame,
                  f.mChannelsPerFrame, f.mBitsPerChannel);
    }
    const Desc& physicalFormat = FindClosestFormat( physicalFormats );
    stream.SetCurrentPhysicalFormat( physicalFormat );

    vector<Desc> procFormats;
    GetIOProcFormats( sID, procFormats );
    LOG->Info("Available I/O procedure formats:");
    for (i = 0; i < procFormats.size(); ++i)
    {
        const Desc& f = procFormats[i];
        
        LOG->Info("Format %u:  Rate: %i  ID: %s  Flags 0x%lx  bpp %lu  fpp %lu  bpf %lu  channels %lu  bits %lu",
                  i, (int) f.mSampleRate, FormatToString(f.mFormatID).c_str(), f.mFormatFlags,
                  f.mBytesPerPacket, f.mFramesPerPacket, f.mBytesPerFrame,
                  f.mChannelsPerFrame, f.mBitsPerChannel);
    }
    const Desc& procFormat = FindClosestFormat( procFormats );
    stream.SetCurrentIOProcFormat( procFormat );
    
    
    try
    {
        UInt32 bufferSize = mOutputDevice->GetIOBufferSize();
        LOG->Info("I/O Buffer size: %lu", bufferSize);
    }
    catch (const CAException& e)
    {
        LOG->Warn("Could not determine buffer size.");
    }    
    
    try
    {
        UInt32 frames = mOutputDevice->GetLatency(kAudioDeviceSectionOutput);
        if (stream.HasProperty(0, kAudioDevicePropertyLatency))
        {
            UInt32 t, size = 4;
            
            stream.GetPropertyData(0, kAudioDevicePropertyLatency, size, &t);
            frames += t;
            LOG->Info("Frames of stream latency: %lu", t);
        }
        else
            LOG->Warn("Stream reports no latency.");
        mLatency = frames / 44100.0;
        LOG->Info("Frames of latency:        %lu\n"
                  "Seconds of latency:       %f", frames, mLatency);
    }
    catch (const CAException& e)
    {
        delete mOutputDevice;
        RageException::ThrowNonfatal("Couldn't get Latency.");
    }
    

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

OSStatus RageSound_CA::OverloadListener(AudioDeviceID inDevice,
                                        UInt32 inChannel,
                                        Boolean isInput,
                                        AudioDevicePropertyID inPropertyID,
                                        void *inData)
{
    LOG->Warn("Audio device overload.");
    return noErr;
}

void RageSound_CA::SetupDecodingThread()
{
	/* Increase the scheduling precedence of the decoder thread. */
	thread_precedence_policy po;
	po.importance = 32;
	kern_return_t ret = thread_policy_set( mach_thread_self(), THREAD_PRECEDENCE_POLICY,
                       (int *)&po, THREAD_PRECEDENCE_POLICY_COUNT );
	if( ret != KERN_SUCCESS )
		LOG->Warn( "thread_policy_set(THREAD_PRECEDENCE_POLICY) failed: %s", mach_error_string(ret) );
}

/*
 * (c) 2004 Steve Checkoway
 * All rights reserved.
 * 
 * Permission is hereby granted, free of charge, to any person obtaining a
 * copy of this software and associated documentation files (the
 * "Software"), to deal in the Software without restriction, including
 * without limitation the rights to use, copy, modify, merge, publish,
 * distribute, and/or sell copies of the Software, and to permit persons to
 * whom the Software is furnished to do so, provided that the above
 * copyright notice(s) and this permission notice appear in all copies of
 * the Software and that both the above copyright notice(s) and this
 * permission notice appear in supporting documentation.
 * 
 * THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS
 * OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF
 * MERCHANTABILITY, FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT OF
 * THIRD PARTY RIGHTS. IN NO EVENT SHALL THE COPYRIGHT HOLDER OR HOLDERS
 * INCLUDED IN THIS NOTICE BE LIABLE FOR ANY CLAIM, OR ANY SPECIAL INDIRECT
 * OR CONSEQUENTIAL DAMAGES, OR ANY DAMAGES WHATSOEVER RESULTING FROM LOSS
 * OF USE, DATA OR PROFITS, WHETHER IN AN ACTION OF CONTRACT, NEGLIGENCE OR
 * OTHER TORTIOUS ACTION, ARISING OUT OF OR IN CONNECTION WITH THE USE OR
 * PERFORMANCE OF THIS SOFTWARE.
 */
