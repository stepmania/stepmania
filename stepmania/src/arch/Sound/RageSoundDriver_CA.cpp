#include "global.h"
#include "RageUtil.h"
#include "RageSoundDriver_CA.h"
#include "RageLog.h"
#include "RageTimer.h"
#include "PrefsManager.h"
#include "archutils/Darwin/DarwinThreadHelpers.h"

#include "CAAudioHardwareSystem.h"
#include "CAAudioHardwareDevice.h"
#include "CAAudioHardwareStream.h"
#include "CAStreamBasicDescription.h"
#include "CAException.h"

static const UInt32 kFramesPerPacket = 1;
static const UInt32 kChannelsPerFrame = 2;
static const UInt32 kBitsPerChannel = 16;
static const UInt32 kBytesPerPacket = kChannelsPerFrame * kBitsPerChannel / 8;
static const UInt32 kBytesPerFrame = kBytesPerPacket;
static const UInt32 kFormatFlags = kAudioFormatFlagsNativeEndian | kAudioFormatFlagIsSignedInteger;

static int64_t g_iLastSampleTime = 0;

typedef CAStreamBasicDescription Desc;

/* temporary hack: */
static float g_fLastIOProcTime = 0;
static const int NUM_MIX_TIMES = 16;
static float g_fLastMixTimes[NUM_MIX_TIMES];
static int g_iLastMixTimePos = 0;
static int g_iNumIOProcCalls = 0;

RageSound_CA::RageSound_CA()
{
	m_pOutputDevice = NULL;
	m_Converter = NULL;
	m_pIOThread = NULL;
	m_pNotificationThread = NULL;
}

RString RageSound_CA::Init()
{
	try
	{
		AudioDeviceID dID = CAAudioHardwareSystem::GetDefaultDevice( false, false );
		m_pOutputDevice = new CAAudioHardwareDevice( dID );
	}
	catch( const CAException& e )
	{
		return "Couldn't create default output device.";
	}
	// Get the HAL's runloop and attach an observer
	try
	{
		CFRunLoopRef runLoopRef;
		CFRunLoopObserverRef observerRef;
		UInt32 size = sizeof( runLoopRef );
		CFRunLoopObserverContext context = { 0, this, NULL, NULL, NULL };
		
		CAAudioHardwareSystem::GetPropertyData( kAudioHardwarePropertyRunLoop, size, &runLoopRef );
		observerRef = CFRunLoopObserverCreate( kCFAllocatorDefault, kCFRunLoopEntry,
						       false, 0, NameHALThread, &context );
		CFRunLoopAddObserver( runLoopRef, observerRef, kCFRunLoopDefaultMode );
	}
	catch( CAException& e )
	{
		LOG->Warn( "Couldn't get the HAL's run loop." );
	}
	// Log the name and manufacturer
	char str[256];
	CFStringRef ref = m_pOutputDevice->CopyName();
	
	if( !CFStringGetCString(ref, str, sizeof(str), kCFStringEncodingUTF8) )
		strcpy( str, "(unknown)" );
	LOG->Info( "Audio device: %s", str );
	CFRelease( ref );
	ref = m_pOutputDevice->CopyManufacturer();
	if( !CFStringGetCString(ref, str, sizeof(str), kCFStringEncodingUTF8) )
		strcpy( str, "(unknown)" );
	LOG->Info( "Audio device manufacturer: %s", str );
	CFRelease( ref );
	
	m_iSampleRate = PREFSMAN->m_iSoundPreferredSampleRate;
	Float64 nominalSampleRate = m_iSampleRate;
    
	try
	{
		m_pOutputDevice->SetNominalSampleRate(nominalSampleRate);
		LOG->Info( "Set the nominal sample rate to %f.", nominalSampleRate );
	}
	catch( const CAException& e )
	{
		LOG->Warn( "Couldn't set the nominal sample rate." );
		nominalSampleRate = m_pOutputDevice->GetNominalSampleRate();
		LOG->Warn( "Device's nominal sample rate is %f", nominalSampleRate );
		m_iSampleRate = int( nominalSampleRate );
	}
	AudioStreamID sID = m_pOutputDevice->GetStreamByIndex( kAudioDeviceSectionOutput, 0 );
	CAAudioHardwareStream stream( sID );
	AddListener( kAudioDeviceProcessorOverload, OverloadListener, "overload" );
	AddListener( kAudioDevicePropertyDeviceHasChanged, DeviceChanged, "device changed" );
	AddListener( kAudioDevicePropertyJackIsConnected, JackChanged, "jack changed" );

	// The canonical format
	Desc IOProcFormat( nominalSampleRate, kAudioFormatLinearPCM, 8, 1, 8, 2, 32,
			   kAudioFormatFlagsNativeFloatPacked);
	const Desc SMFormat( nominalSampleRate, kAudioFormatLinearPCM, kBytesPerPacket, kFramesPerPacket,
			     kBytesPerFrame, kChannelsPerFrame, kBitsPerChannel, kFormatFlags );
	
	try
	{
		stream.SetCurrentIOProcFormat( IOProcFormat );
	}
	catch( const CAException& e )
	{
		LOG->Warn( "Could not set the IOProc format to the canonical format." );
		stream.GetCurrentIOProcFormat( IOProcFormat );
	}
	
	if( AudioConverterNew(&SMFormat, &IOProcFormat, &m_Converter) )
		return "Couldn't create the audio converter";

	UInt32 bufferSize;
	
	try
	{
		bufferSize = m_pOutputDevice->GetIOBufferSize();
		LOG->Info("I/O Buffer size: %lu frames", bufferSize);
	}
	catch( const CAException& e )
	{
		LOG->Warn( "Could not determine buffer size." );
		bufferSize = 0;
	}    
    
	try
	{
		UInt32 frames = m_pOutputDevice->GetLatency( kAudioDeviceSectionOutput );
		if( stream.HasProperty(0, kAudioDevicePropertyLatency) )
		{
			UInt32 t, size = 4;
            
			stream.GetPropertyData( 0, kAudioDevicePropertyLatency, size, &t );
			frames += t;
			LOG->Info( "Frames of stream latency: %lu", t );
		}
		else
		{
			LOG->Warn( "Stream does not report latency." );
		}
		frames += m_pOutputDevice->GetSafetyOffset( kAudioDeviceSectionOutput );
		frames += bufferSize;
		m_fLatency = frames / nominalSampleRate;
		LOG->Info( "Frames of latency:        %lu\n"
			   "Seconds of latency:       %f", frames, m_fLatency );
	}
	catch( const CAException& e )
	{
		return "Couldn't get latency.";
	}		

	StartDecodeThread();
    
	try
	{
		m_pOutputDevice->AddIOProc( GetData, this );
		m_pOutputDevice->StartIOProc( GetData );
	}
	catch( const CAException& e )
	{
		return "Couldn't start the IOProc.";
	}
	return "";
}

RageSound_CA::~RageSound_CA()
{
	if( m_pOutputDevice != NULL )
	{
		m_pOutputDevice->StopIOProc( GetData );
		m_pOutputDevice->RemoveIOProc( GetData );
		
		while( m_vPropertyListeners.size() )
		{
			pair<AudioHardwarePropertyID, AudioDevicePropertyListenerProc>& p = m_vPropertyListeners.back();
			
			CATry;
			m_pOutputDevice->RemovePropertyListener( kAudioPropertyWildcardChannel, kAudioDeviceSectionOutput,
								 p.first, p.second );
			CACatch;
			m_vPropertyListeners.pop_back();
		}
		delete m_pOutputDevice;
		delete m_pIOThread;
		delete m_pNotificationThread;
	}

	if( m_Converter != NULL )
		AudioConverterDispose( m_Converter );
	AudioHardwareUnload();
}

void RageSound_CA::AddListener( AudioDevicePropertyID propertyID, AudioDevicePropertyListenerProc handler,
								const char *name )
{
	try
	{
		m_pOutputDevice->AddPropertyListener( kAudioPropertyWildcardChannel, kAudioDeviceSectionOutput,
						      propertyID, handler, this );
		m_vPropertyListeners.push_back( pair<AudioDevicePropertyID,
						AudioDevicePropertyListenerProc>(propertyID, handler) );
	}
	catch( const CAException& e )
	{
		LOG->Warn( "Could not install %s listener.", name );
	}
}

int64_t RageSound_CA::GetPosition( const RageSoundBase *sound ) const
{
	AudioTimeStamp time;
	static bool bStopped = false;
	
	try
	{
		m_pOutputDevice->GetCurrentTime( time );
		if( bStopped )
			FAIL_M( ssprintf("old time %lld, new time %lld",
					 g_iLastSampleTime, int64_t(time.mSampleTime)) );
		g_iLastSampleTime = int64_t( time.mSampleTime );
		return g_iLastSampleTime;
	}
	catch( const CAException& e )
	{
		if( e.GetError() == 'stop' )
		{
			bStopped = true;
			return g_iLastSampleTime;
		}
		
		char error[5];
		
		*(int32_t*)error = e.GetError();
		error[4] = '\0';
		FAIL_M( ssprintf("GetCurrentTime() returned error '%s'.", error) );
	}
}

void RageSound_CA::NameHALThread( CFRunLoopObserverRef observer, CFRunLoopActivity activity, void *info )
{
	RageSound_CA *This = (RageSound_CA *)info;
	
	This->m_pNotificationThread = new RageThreadRegister( "HAL notification thread" );
	
	// Remove and release the observer.
	CFRunLoopObserverInvalidate( observer );
	CFRelease( observer );
}

OSStatus RageSound_CA::GetData( AudioDeviceID inDevice,
				const AudioTimeStamp *inNow,
				const AudioBufferList *inInputData,
				const AudioTimeStamp *inInputTime,
				AudioBufferList *outOutputData,
				const AudioTimeStamp *inOutputTime,
				void *inClientData )
{
	RageTimer tm;
	RageSound_CA *This = (RageSound_CA *)inClientData;
	
	if( unlikely(This->m_pIOThread == NULL) )
		This->m_pIOThread = new RageThreadRegister( "HAL I/O thread" );
	
	AudioBuffer& buf = outOutputData->mBuffers[0];
	UInt32 dataPackets = buf.mDataByteSize >> 3; // 8 byes per packet
	int64_t decodePos = int64_t( inOutputTime->mSampleTime );
	int64_t now = int64_t( inNow->mSampleTime );
	
	g_iLastSampleTime = now;
	RageTimer tm2;
	int16_t buffer[dataPackets * (kBytesPerPacket >> 1)];
	
	This->Mix( buffer, dataPackets, decodePos, now) ;
	g_fLastMixTimes[g_iLastMixTimePos] = tm2.GetDeltaTime();
	++g_iLastMixTimePos;
	wrap( g_iLastMixTimePos, NUM_MIX_TIMES );
	
	AudioConverterConvertBuffer( This->m_Converter, dataPackets * kBytesPerPacket,
				     buffer, &buf.mDataByteSize, buf.mData );
		
	g_fLastIOProcTime = tm.GetDeltaTime();
	++g_iNumIOProcCalls;
	
	return noErr;
}
		

OSStatus RageSound_CA::OverloadListener( AudioDeviceID inDevice,
					 UInt32 inChannel,
					 Boolean isInput,
					 AudioDevicePropertyID inPropertyID,
					 void *inData )
{
	if( isInput )
		return noErr;
	RString Output;
	for( int i = NUM_MIX_TIMES-1; i >= 0; --i )
	{
		int pos = (g_iLastMixTimePos+i) % NUM_MIX_TIMES;
		Output += ssprintf( "%.3f ", g_fLastMixTimes[pos] );
	}

	LOG->Warn( "Audio overload.  Last IOProc time: %f IOProc calls: %i (%s)",
			   g_fLastIOProcTime, g_iNumIOProcCalls, Output.c_str() );
	g_iNumIOProcCalls = 0;
	return noErr;
}

OSStatus RageSound_CA::DeviceChanged( AudioDeviceID inDevice, UInt32 inChannel, Boolean isInput,
				      AudioDevicePropertyID inPropertyID, void *inData )
{
	if( isInput )
		return noErr;
	FAIL_M( "Device configuration changed. XXX" );
}

OSStatus RageSound_CA::JackChanged( AudioDeviceID inDevice, UInt32 inChannel, Boolean isInput,
				    AudioDevicePropertyID inPropertyID, void *inData )
{
	if( isInput )
		return noErr;
	RageSound_CA *This = (RageSound_CA *)inData;
	UInt32 result;
	UInt32 size = sizeof( result );
	This->m_pOutputDevice->GetPropertyData( inChannel, 0, inPropertyID, size, &result );
	LOG->Trace( "Channel %u's has %s plugged into its jack.", unsigned(inChannel),
		    result ? "something" : "nothing" );
	return noErr;
}
							   

void RageSound_CA::SetupDecodingThread()
{
	/* Increase the scheduling precedence of the decoder thread. */
	SetThreadPrecedence( 0.75f );
}

/*
 * (c) 2004, 2006 Steve Checkoway
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
