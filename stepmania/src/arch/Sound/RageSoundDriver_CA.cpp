#include "global.h"
#include "RageUtil.h"
#include "RageSoundDriver_CA.h"
#include "CAHelpers.h"
#include "RageLog.h"
#include "RageTimer.h"

#include "CAAudioHardwareSystem.h"
#include "CAAudioHardwareDevice.h"
#include "CAAudioHardwareStream.h"
#include "CAStreamBasicDescription.h"
#include "CAException.h"
#include "archutils/Unix/CrashHandler.h"
#include <mach/thread_act.h>
#include <mach/mach_init.h>
#include <mach/mach_error.h>

static AudioConverter *gConverter = NULL;

/* temporary hack: */
static float g_fLastIOProcTime = 0;
static const int NUM_MIX_TIMES = 16;
static float g_fLastMixTimes[NUM_MIX_TIMES];
static int g_fLastMixTimePos = 0;
static int g_iNumIOProcCalls = 0;

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

static Desc FindClosestFormat(const vector<Desc>& formats,
							  RageSound_CA::format& type)
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
            (format.mFormatFlags & kFormatFlags) == kFormatFlags)
		{ // exact match
			type = RageSound_CA::EXACT;
			return format;
		}
		v.push_back(format);
	}

	const Desc CanonicalFormat(44100.0, kAudioFormatLinearPCM, 8, 1, 8, 2, 32,
							   kAudioFormatFlagsNativeFloatPacked);

	for (i = v.begin(); i != v.end(); ++i)
	{
		const Desc& format = *i;

		if (format == CanonicalFormat)
		{
			type = RageSound_CA::CANONICAL;
			return format;
		}
	}
	if (v.empty())
		RageException::ThrowNonfatal("Couldn't find a close format.");
	type = RageSound_CA::OTHER;
	return v[0]; // something is better than nothing.
}

static void GetIOProcFormats( CAAudioHardwareStream stream,
							  vector<Desc> &procFormats )
{
	UInt32 numFormats = stream.GetNumberAvailableIOProcFormats();

	for( UInt32 i=0; i<numFormats; ++i )
	{
		Desc desc;

		stream.GetAvailableIOProcFormatByIndex( i, desc );
		procFormats.push_back( desc );
	}
}

static void GetPhysicalFormats( CAAudioHardwareStream stream,
								vector<Desc> &physicalFormats )
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
		AudioDeviceID dID;
	
		dID = CAAudioHardwareSystem::GetDefaultDevice(false, false);
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
	AudioStreamID sID;

	sID = mOutputDevice->GetStreamByIndex( kAudioDeviceSectionOutput, 0 );
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
        
		LOG->Info("Format %u:  Rate: %i  ID: %s  Flags 0x%lx  bpp %lu  fpp %lu"
				  " bpf %lu  channels %lu  bits %lu", i, int(f.mSampleRate),
				  FormatToString(f.mFormatID).c_str(), f.mFormatFlags,
				  f.mBytesPerPacket, f.mFramesPerPacket, f.mBytesPerFrame,
				  f.mChannelsPerFrame, f.mBitsPerChannel);
	}
	const Desc& physicalFormat = FindClosestFormat( physicalFormats, mFormat );
	stream.SetCurrentPhysicalFormat( physicalFormat );

	vector<Desc> procFormats;
	GetIOProcFormats( sID, procFormats );
	LOG->Info("Available I/O procedure formats:");
	for (i = 0; i < procFormats.size(); ++i)
	{
		const Desc& f = procFormats[i];
        
		LOG->Info("Format %u:  Rate: %i  ID: %s  Flags 0x%lx  bpp %lu  fpp %lu"
				  " bpf %lu  channels %lu  bits %lu", i, int(f.mSampleRate),
				  FormatToString(f.mFormatID).c_str(), f.mFormatFlags,
				  f.mBytesPerPacket, f.mFramesPerPacket, f.mBytesPerFrame,
				  f.mChannelsPerFrame, f.mBitsPerChannel);
	}

	const Desc& procFormat = FindClosestFormat( procFormats, mFormat );
	stream.SetCurrentIOProcFormat( procFormat );
	//mFormat = OTHER; // XXX Temporary
	LOG->Info("Proc format is %s.",
			  mFormat == EXACT ? "exact" : (mFormat == CANONICAL ?
											"canonical" : "other"));

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

	if (mFormat == OTHER)
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
	delete mOutputDevice;
	delete gConverter;
}

int64_t RageSound_CA::GetPosition(const RageSoundBase *sound) const
{
	AudioTimeStamp time;
    
	mOutputDevice->GetCurrentTime(time);
	return int64_t(time.mSampleTime);
}

void RageSound_CA::FillConverter( void *data, UInt32 frames )
{
	RageTimer tm;
	
	Mix( (int16_t *)data, frames, mDecodePos, mNow );

	g_fLastMixTimes[g_fLastMixTimePos] = tm.GetDeltaTime();
	++g_fLastMixTimePos;
	wrap( g_fLastMixTimePos, NUM_MIX_TIMES );
}

OSStatus RageSound_CA::GetData(AudioDeviceID inDevice,
							   const AudioTimeStamp *inNow,
							   const AudioBufferList *inInputData,
							   const AudioTimeStamp *inInputTime,
							   AudioBufferList *outOutputData,
							   const AudioTimeStamp *inOutputTime,
							   void *inClientData)
{
	RageTimer tm;
	RageSound_CA *This = (RageSound_CA *)inClientData;
	AudioBuffer& buf = outOutputData->mBuffers[0];
	UInt32 dataPackets = buf.mDataByteSize;
	int64_t decodePos = int64_t(inOutputTime->mSampleTime);
	int64_t now = int64_t(inNow->mSampleTime);
	
	if (This->mFormat == OTHER)
	{
		dataPackets /= gConverter->GetOutputFormat().mBytesPerPacket;
		This->mDecodePos = decodePos;
		This->mNow = now;
		gConverter->FillComplexBuffer(dataPackets, *outOutputData, NULL);
	}
	else
	{
		RageTimer mixTM;
		
		if (This->mFormat == EXACT)
		{
			dataPackets /= kBytesPerPacket;
			This->Mix((int16_t *)buf.mData, dataPackets, decodePos, now);
		}
		else // This->mFormat == CANONICAL
		{
			dataPackets /= 8; // 8 bytes per packet (1 frame per packet)
			
			int16_t buffer[dataPackets * kBytesPerPacket];
			int16_t *ip = buffer;
			float *fp = (float *)buf.mData;
			
			This->Mix(buffer, dataPackets, decodePos, now);
			
			// Convert from signed 16 bit int to signed 32 bit float
			for (unsigned i = 0; i < buf.mDataByteSize; i += 4)
			{
				int16_t val = *(++ip);
				
				*(++fp) = val / (val < 0 ? 32768.0f : 32767.0f);
			}
		}
		
		g_fLastMixTimes[g_fLastMixTimePos] = tm.GetDeltaTime();
		++g_fLastMixTimePos;
		wrap(g_fLastMixTimePos, NUM_MIX_TIMES);
	}
	
	g_fLastIOProcTime = tm.GetDeltaTime();
	++g_iNumIOProcCalls;
	
	return noErr;
}
		

OSStatus RageSound_CA::OverloadListener(AudioDeviceID inDevice,
										UInt32 inChannel,
										Boolean isInput,
										AudioDevicePropertyID inPropertyID,
										void *inData)
{
	CString Output;
	for( int i = NUM_MIX_TIMES-1; i >= 0; --i )
	{
		int pos = (g_fLastMixTimePos+i) % NUM_MIX_TIMES;
		Output += ssprintf( "%.3f ", g_fLastMixTimes[pos] );
	}

	LOG->Warn( "Audio overload.  Last IOProc time: %f IOProc calls: %i (%s)",
			   g_fLastIOProcTime, g_iNumIOProcCalls, Output.c_str() );
	g_iNumIOProcCalls = 0;
	return noErr;
}

void RageSound_CA::SetupDecodingThread()
{
	/* Increase the scheduling precedence of the decoder thread. */
	thread_precedence_policy po;
	po.importance = 32;
	kern_return_t ret;

	ret = thread_policy_set( mach_thread_self(),
							 THREAD_PRECEDENCE_POLICY, (int *)&po,
							 THREAD_PRECEDENCE_POLICY_COUNT );
	if( ret != KERN_SUCCESS )
		LOG->Warn("thread_policy_set(THREAD_PRECEDENCE_POLICY) failed: %s",
				  mach_error_string(ret));
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
