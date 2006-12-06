#include "global.h"
#include "RageUtil.h"
#include "RageSoundDriver_CA.h"
#include "RageLog.h"
#include "RageTimer.h"
#include "PrefsManager.h"
#include "archutils/Darwin/DarwinThreadHelpers.h"

REGISTER_SOUND_DRIVER_CLASS2( CoreAudio, CA );

static const UInt8 kAudioDeviceSectionInput    = 0x01;
static const UInt8 kAudioDeviceSectionOutput   = 0x00;
static const UInt8 kAudioDeviceSectionGlobal   = 0x00;
static const UInt8 kAudioDeviceSectionWildcard = 0xFF;

static const UInt32 kFramesPerPacket = 1;
static const UInt32 kChannelsPerFrame = 2;
static const UInt32 kBitsPerChannel = 16;
static const UInt32 kBytesPerPacket = kChannelsPerFrame * kBitsPerChannel / 8;
static const UInt32 kBytesPerFrame = kBytesPerPacket;
static const UInt32 kFormatFlags = kAudioFormatFlagsNativeEndian | kAudioFormatFlagIsSignedInteger;

struct Desc : public AudioStreamBasicDescription
{
	Desc( Float64 sampleRate, UInt32 formatID, UInt32 formatFlags, UInt32 bytesPerPacket,
	      UInt32 framesPerPacket, UInt32 bytesPerFrame, UInt32 channelsPerFrame, UInt32 bitsPerChannel );
};

Desc::Desc( Float64 sampleRate, UInt32 formatID, UInt32 formatFlags, UInt32 bytesPerPacket,
	    UInt32 framesPerPacket, UInt32 bytesPerFrame, UInt32 channelsPerFrame, UInt32 bitsPerChannel )
{
	mSampleRate = sampleRate;
	mFormatID = formatID;
	mFormatFlags = formatFlags;
	mBytesPerPacket = bytesPerPacket;
	mFramesPerPacket = framesPerPacket;
	mBytesPerFrame = bytesPerFrame;
	mChannelsPerFrame = channelsPerFrame;
	mBitsPerChannel = bitsPerChannel;
	mReserved = 0;
}

/* temporary hack: */
static float g_fLastIOProcTime = 0;
static const int NUM_MIX_TIMES = 16;
static float g_fLastMixTimes[NUM_MIX_TIMES];
static int g_iLastMixTimePos = 0;
static int g_iNumIOProcCalls = 0;

RageSound_CA::RageSound_CA() : m_fLatency(0.0f), m_Converter(NULL), m_bStarted(false), m_iSampleRate(0),
			       m_iLastSampleTime(0), m_iOffset(0), m_pIOThread(NULL), m_pNotificationThread(NULL)
{
}

static inline RString FourCCToString( uint32_t num )
{
	RString s( 4, '?' );
	char c;
	
	c = (num >> 24) & 0xFF;
	if( c >='\x20' && c <= '\x7e' )
		s[0] = c;
	c = (num >> 16) & 0xFF;
	if( c >='\x20' && c <= '\x7e' )
		s[1] = c;
	c = (num >> 8) & 0xFF;
	if( c >='\x20' && c <= '\x7e' )
		s[2] = c;
	c = num & 0xFF;
	if( c >= '\x20' && c <= '\x7e' )
		s[3] = c;
	
	return s;
}
#define WERROR(str, num, extra...) str ": '%s' (%lu).", ## extra, FourCCToString(num).c_str(), (num)
#define ERROR(str, num, extra...) (ssprintf(WERROR(str, (num), ## extra)))

RString RageSound_CA::Init()
{
	OSStatus error;
	UInt32 size = sizeof(m_OutputDevice);
	
	if( (error = AudioHardwareGetProperty(kAudioHardwarePropertyDefaultOutputDevice, &size, &m_OutputDevice)) )
		return ERROR( "Couldn't create default output device", error );
	
	// Get the HAL's runloop and attach an observer
	{
		CFRunLoopRef runLoopRef;
		CFRunLoopObserverRef observerRef;
		CFRunLoopObserverContext context = { 0, this, NULL, NULL, NULL };
		
		size = sizeof(CFRunLoopRef);
		if( (error = AudioHardwareGetProperty(kAudioHardwarePropertyRunLoop, &size, &runLoopRef)) )
		{
			LOG->Warn( WERROR("Couldn't get the HAL's run loop", error) );
		}
		else
		{
			observerRef = CFRunLoopObserverCreate( kCFAllocatorDefault, kCFRunLoopEntry,
							       false, 0, NameHALThread, &context );
			CFRunLoopAddObserver( runLoopRef, observerRef, kCFRunLoopDefaultMode );
		}
		observerRef = CFRunLoopObserverCreate( kCFAllocatorDefault, kCFRunLoopEntry,
						       false, 0, NameHALThread, &context );
		CFRunLoopAddObserver( runLoopRef, observerRef, kCFRunLoopDefaultMode );
	}
	
	// Log the name and manufacturer
	{
		char str[256];
		CFStringRef ref;
		
		str[0] = '\0';
		size = sizeof(CFStringRef);
		if( !AudioDeviceGetProperty(m_OutputDevice, 0, kAudioDeviceSectionGlobal,
					    kAudioDevicePropertyDeviceNameCFString, &size, &ref) )
		{
			CFStringGetCString( ref, str, sizeof(str), kCFStringEncodingUTF8 );
			CFRelease( ref );
		}
		if( str[0] == '\0' )
			strcpy( str, "(unknown)" );
		
		LOG->Info( "Audio device: %s", str );
		str[0] = '\0';
		size = sizeof(CFStringRef);
		if( !AudioDeviceGetProperty(m_OutputDevice, 0, kAudioDeviceSectionGlobal,
					    kAudioDevicePropertyDeviceManufacturerCFString, &size, &ref) )
		{
			CFStringGetCString( ref, str, sizeof(str), kCFStringEncodingUTF8 );
			CFRelease( ref );
		}
		if( str[0] == '\0' )
			strcpy( str, "(unknown)" );
		LOG->Info( "Audio device manufacturer: %s", str );
	}
	
	AddListener( kAudioDeviceProcessorOverload,			OverloadListener,	"overload" );
	AddListener( kAudioDevicePropertyDeviceHasChanged,		DeviceChanged,		"device changed" );
	AddListener( kAudioDevicePropertyJackIsConnected,		JackChanged,		"jack changed" );
	AddListener( kAudioDevicePropertyStreamConfiguration,		DeviceChanged,		"configuration changed" );
	AddListener( kAudioDevicePropertyPreferredChannelsForStereo,	DeviceChanged,		"preferred channels changed" );
	AddListener( kAudioHardwarePropertyDefaultOutputDevice,		DeviceChanged,		"default output device changed" );
//	AddListener( kAudioDevicePropertyStreamFormat,			FormatChanged,		"stream format changed" );
	
	// Find the stereo channels
	UInt32 channels[2];
	
	size = sizeof( channels );
	if( (error = AudioDeviceGetProperty(m_OutputDevice, 0, kAudioDeviceSectionOutput,
					    kAudioDevicePropertyPreferredChannelsForStereo, &size, channels)) )
	{
		LOG->Warn( WERROR( "Couldn't get the preferred stereo channels", error) );
		channels[0] = channels[1] = 0; // Master channel
	}
	else
	{
		LOG->Trace( "Preferred stereo channels { %lu, %lu }.", channels[0], channels[1] );
	}
	UInt32 iFirstChannel = min( channels[0], channels[1] );
	
	m_iSampleRate = PREFSMAN->m_iSoundPreferredSampleRate;
	if( m_iSampleRate == 0 )
		m_iSampleRate = 44100;
	Float64 nominalSampleRate = m_iSampleRate;
	Float64 actualSampleRate;
	
	if( (error = AudioDeviceSetProperty(m_OutputDevice, NULL, iFirstChannel, kAudioDeviceSectionGlobal,
					    kAudioDevicePropertyNominalSampleRate, sizeof(Float64), &nominalSampleRate)) )
	{
		LOG->Warn( WERROR("Couldn't set the nominal sample rate", error) );
		nominalSampleRate = 0.0;
	}
	size = sizeof(Float64);
	if( (error = AudioDeviceGetProperty(m_OutputDevice, iFirstChannel, kAudioDeviceSectionGlobal,
					    kAudioDevicePropertyNominalSampleRate, &size, &actualSampleRate)) )
	{
		return ERROR( "Couldn't get the nominal sample rate", error );
	}
	if( actualSampleRate == nominalSampleRate )
	{
		LOG->Info( "Set the nominal sample rate to %f.", nominalSampleRate );
	}
	else
	{
		LOG->Warn( "Reported nominal sample rate of %f.", actualSampleRate );
		m_iSampleRate = int( actualSampleRate );
	}
	
	AudioStreamID *streams, stream;
	
	if( (error = AudioDeviceGetPropertyInfo(m_OutputDevice, iFirstChannel, kAudioDeviceSectionOutput,
						kAudioDevicePropertyStreams, &size, NULL)) )
	{
		return ERROR( "Couldn't get the stream size", error );
	}
	if( size == 0 )
		return "No streams.";
	streams = new AudioStreamID[size/sizeof(AudioStreamID)];
	if( (error = AudioDeviceGetProperty(m_OutputDevice, iFirstChannel, kAudioDeviceSectionOutput,
					    kAudioDevicePropertyStreams, &size, streams)) )
	{
		delete[] streams;
		return ERROR( "Couldn't get streams", error );
	}
	stream = streams[0];
	size = sizeof( UInt32 );
	for( unsigned i = 0; i < size/sizeof(AudioStreamID); ++i )
	{
		UInt32 firstChannel;
		
		// I'm not really sure why I'm doing this except it's what the c++ wrapper did, more or less.
		if( streams[i] < stream )
			stream = streams[i];
		// Look for the matching stream.
		if( (error = AudioStreamGetProperty(streams[i], 0, kAudioStreamPropertyStartingChannel,
						    &size, &firstChannel)) )
		{
			LOG->Warn( WERROR("Failed to get stream starting channel", error) );
			continue;
		}
		if( firstChannel != iFirstChannel )
		{
			stream = streams[i];
			LOG->Trace( "Found a stream with a matching channel." );
			break;
		}
	}
	delete[] streams;
	
	// The canonical format
	Desc IOProcFormat( actualSampleRate, kAudioFormatLinearPCM, kAudioFormatFlagsNativeFloatPacked, 8, 1, 8, 2, 32 );
	const Desc SMFormat( actualSampleRate, kAudioFormatLinearPCM, kFormatFlags, kBytesPerPacket,
			     kFramesPerPacket, kBytesPerFrame, kChannelsPerFrame, kBitsPerChannel );
	
	if( (error = AudioStreamSetProperty(stream, NULL, 0, kAudioDevicePropertyStreamFormat,
					    sizeof(Desc), &IOProcFormat)) )
	{
		LOG->Warn( WERROR("Couldn't set the IOProc format to the canonical format.", error) );
		size = sizeof(Desc);
		if( (error = AudioStreamGetProperty(stream, 0, kAudioDevicePropertyStreamFormat, &size, &IOProcFormat)) )
			return ERROR( "Couldn't get the IOProc format", error );
	}
	
	if( (error = AudioConverterNew(&SMFormat, &IOProcFormat, &m_Converter)) )
		return ERROR( "Couldn't create the audio converter", error );
	
	// Find which stream of the ones passed to the GetData should be used.
	AudioBufferList abl;
	
	size = sizeof(abl);
	if( (error = AudioDeviceGetProperty(m_OutputDevice, 0, kAudioDeviceSectionOutput,
					    kAudioDevicePropertyStreamConfiguration, &size, &abl)) )
	{
		return ERROR( "Couldn't get the stream configuration", error );
	}
	
	m_iBufferNumber = 0;
	for( UInt32 i = 0, channelCount = 0; i < abl.mNumberBuffers; ++i )
	{
		// 0 is the master channel so others start at 1 (I hope) so >= instead of >.
		channelCount += abl.mBuffers[i].mNumberChannels;
		if( channelCount >= iFirstChannel )
		{
			m_iBufferNumber = i;
			LOG->Trace( "Found the correct buffer number: %lu", i );
			break;
		}
	}

	UInt32 bufferSize;

	size = sizeof(UInt32);
	if( (error = AudioDeviceGetProperty(m_OutputDevice, iFirstChannel, kAudioDeviceSectionOutput,
					    kAudioDevicePropertyBufferFrameSize, &size, &bufferSize)) )
	{
		LOG->Warn( WERROR("Couldn't determine buffer size", error) );
		bufferSize = 0;
	}
	else
	{
		LOG->Trace( "I/O buffer size: %lu frames.", bufferSize );
	}
		
	UInt32 frames;
	
	size = sizeof(UInt32);
	if( (error = AudioDeviceGetProperty(m_OutputDevice, iFirstChannel, kAudioDeviceSectionOutput,
					    kAudioDevicePropertyLatency, &size, &frames)) )
	{
		return ERROR( "Couldn't get device latency", error );
	}
	bufferSize += frames;
	LOG->Trace( "Frames of device latency: %lu", frames );
	size = sizeof(UInt32);
	if( (error = AudioDeviceGetProperty(m_OutputDevice, iFirstChannel, kAudioDeviceSectionOutput,
					    kAudioDevicePropertySafetyOffset, &size, &frames)) )
	{
		return ERROR( "Couldn't get device safety offset", error );
	}
	bufferSize += frames;
	LOG->Trace( "Frames of safety offset:  %lu", frames );
	size = sizeof(UInt32);
	if( (error = AudioStreamGetProperty(stream, 0, kAudioDevicePropertyLatency, &size, &frames)) )
	{
		LOG->Warn( WERROR("Stream does not report latency", error) );
	}
	else
	{
		bufferSize += frames;
		LOG->Trace( "Frames of stream latency: %lu", frames );
	}
	m_fLatency = bufferSize / nominalSampleRate;
	LOG->Info( "Frames of latency:        %lu\n"
		   "Seconds of latency:       %f", bufferSize, m_fLatency );

	StartDecodeThread();
    
	if( (error = AudioDeviceAddIOProc(m_OutputDevice, GetData, this)) )
		return ERROR( "Couldn't add the IOProc", error );
	if( (error = AudioDeviceStart(m_OutputDevice, GetData)) )
	{
		AudioDeviceRemoveIOProc( m_OutputDevice, GetData );
		return ERROR( "Couldn't start the IOProc", error );
	}
	m_bStarted = true;
	return RString();
}

RageSound_CA::~RageSound_CA()
{
	if( m_bStarted )
	{
		AudioDeviceStop( m_OutputDevice, GetData );
		AudioDeviceRemoveIOProc( m_OutputDevice, GetData );
		RemoveListeners();
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
	OSStatus error;
	
	if( (error = AudioDeviceAddPropertyListener(m_OutputDevice, kAudioPropertyWildcardChannel,
						    kAudioDeviceSectionOutput, propertyID, handler, this)) )
	{
	     LOG->Warn( WERROR("Couldn't install %s listener", error, name) );
	}
	else
	{
		m_vPropertyListeners.push_back( pair<AudioDevicePropertyID,
						AudioDevicePropertyListenerProc>(propertyID, handler) );
	}
}

void RageSound_CA::RemoveListeners()
{
	while( m_vPropertyListeners.size() )
	{
		pair<AudioHardwarePropertyID, AudioDevicePropertyListenerProc>& p = m_vPropertyListeners.back();
		
		AudioDeviceRemovePropertyListener( m_OutputDevice, kAudioPropertyWildcardChannel,
						   kAudioDeviceSectionOutput, p.first, p.second );
		
		m_vPropertyListeners.pop_back();
	}
}	

int64_t RageSound_CA::GetPosition( const RageSoundBase *sound ) const
{
	AudioTimeStamp time;
	OSStatus error;
	
	if( (error = AudioDeviceGetCurrentTime(m_OutputDevice, &time)) )
	{
		if( error != kAudioHardwareNotRunningError )
			FAIL_M( ERROR("GetCurrentTime() failed", error) );
		return m_iLastSampleTime;
	}
	m_iLastSampleTime = int64_t( time.mSampleTime ) + m_iOffset;
	return m_iLastSampleTime;
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
	
	AudioBuffer& buf = outOutputData->mBuffers[This->m_iBufferNumber];
	UInt32 dataPackets = buf.mDataByteSize >> 3; // 8 byes per packet
	int64_t decodePos = int64_t( inOutputTime->mSampleTime ) + This->m_iOffset;
	int64_t now = int64_t( inNow->mSampleTime ) + This->m_iOffset;
	
	This->m_iLastSampleTime = now;
	RageTimer tm2;
	int16_t buffer[dataPackets * (kBytesPerPacket >> 1)];
	
	This->Mix( buffer, dataPackets, decodePos, now) ;
	g_fLastMixTimes[g_iLastMixTimePos] = tm2.GetDeltaTime();
	++g_iLastMixTimePos;
	wrap( g_iLastMixTimePos, NUM_MIX_TIMES );
	
	AudioConverterConvertBuffer( This->m_Converter, dataPackets * kBytesPerPacket,
				     buffer, &buf.mDataByteSize, buf.mData );
	
	// Zero other buffers. Not sure if this is needed, but I suspect it is.
	for( unsigned i = 0; i < outOutputData->mNumberBuffers; ++i )
	{
		if( i == This->m_iBufferNumber )
			continue;
		memset( outOutputData->mBuffers[i].mData, 0, outOutputData->mBuffers[i].mDataByteSize );
	}
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
	LOG->Trace( "Device configuration changed." );
	return noErr;
}

OSStatus RageSound_CA::JackChanged( AudioDeviceID inDevice, UInt32 inChannel, Boolean isInput,
				    AudioDevicePropertyID inPropertyID, void *inData )
{
	if( isInput )
		return noErr;
	RageSound_CA *This = (RageSound_CA *)inData;
	UInt32 result;
	UInt32 size = sizeof( result );
	AudioTimeStamp time;
	OSStatus error;

	AudioDeviceGetProperty( inDevice, inChannel, 0, inPropertyID, &size, &result );
	CHECKPOINT_M( ssprintf("Channel %lu's has %s plugged into its jack.", inChannel,
			       result ? "something" : "nothing") );

	if( (error = AudioDeviceGetCurrentTime(inDevice, &time)) )
	{
		if( error != kAudioHardwareNotRunningError )
			FAIL_M( ERROR("Couldn't get current time when jack changed", error) );
		This->m_iOffset = This->m_iLastSampleTime;
	}
	else
	{
		This->m_iOffset = This->m_iLastSampleTime - int64_t( time.mSampleTime );
	}
	return noErr;
}


void RageSound_CA::SetupDecodingThread()
{
	/* Increase the scheduling precedence of the decoder thread. */
	SetThreadPrecedence( 0.75f );
}

/*
 * (c) 2004-2006 Steve Checkoway
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
