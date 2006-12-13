#include "RageSoundDriver_AU.h"
#include "RageLog.h"
#include "PrefsManager.h"
#include "archutils/Darwin/DarwinThreadHelpers.h"
#include <CoreServices/CoreServices.h>

REGISTER_SOUND_DRIVER_CLASS2( AudioUnit, AU );

static const UInt32 kFramesPerPacket = 1;
static const UInt32 kChannelsPerFrame = 2;
static const UInt32 kBitsPerChannel = 32;
static const UInt32 kBytesPerPacket = kChannelsPerFrame * kBitsPerChannel / 8;
static const UInt32 kBytesPerFrame = kBytesPerPacket;
static const UInt32 kFormatFlags = kAudioFormatFlagsNativeEndian | kAudioFormatFlagIsFloat;

#define WERROR(str, num, extra...) str ": '%s' (%lu).", ## extra, FourCCToString(num).c_str(), (num)
#define ERROR(str, num, extra...) (ssprintf(WERROR(str, (num), ## extra)))

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

RageSoundDriver_AU::RageSoundDriver_AU() : m_OutputUnit(NULL), m_iSampleRate(0), m_bDone(false), m_bStarted(false),
	m_pIOThread(NULL), m_pNotificationThread(NULL), m_Semaphore("Sound")
{
}


void RageSoundDriver_AU::NameHALThread( CFRunLoopObserverRef observer, CFRunLoopActivity activity, void *inRefCon )
{
	RageSoundDriver_AU *This = (RageSoundDriver_AU *)inRefCon;
	
	This->m_pNotificationThread = new RageThreadRegister( "HAL notification thread" );
	This->m_Semaphore.Post();
}

RString RageSoundDriver_AU::Init()
{
	ComponentDescription desc;
	
	desc.componentType = kAudioUnitType_Output;
	desc.componentSubType = kAudioUnitSubType_DefaultOutput;
	desc.componentManufacturer = kAudioUnitManufacturer_Apple;
	desc.componentFlags = 0;
	desc.componentFlagsMask = 0;
	
	Component comp = FindNextComponent( NULL, &desc );
	
	if( comp == NULL ) 
		return "Failed to find the default output unit.";
	
	OSStatus error = OpenAComponent( comp, &m_OutputUnit );
	
	if( error != noErr || m_OutputUnit == NULL )
		return ERROR( "Could not open the default output unit", error );
	
	// Set up a callback function to generate output to the output unit
	AURenderCallbackStruct input;
	input.inputProc = Render;
	input.inputProcRefCon = this;
	
	error = AudioUnitSetProperty( m_OutputUnit, 
				      kAudioUnitProperty_SetRenderCallback, 
				      kAudioUnitScope_Input,
				      0, 
				      &input, 
				      sizeof(input) );
	if( error != noErr )
		return ERROR( "Failed to set render callback", error );
	
	AudioStreamBasicDescription streamFormat;
	
	streamFormat.mSampleRate = PREFSMAN->m_iSoundPreferredSampleRate;
	if( streamFormat.mSampleRate == 0 )
		streamFormat.mSampleRate = 44100;
	streamFormat.mFormatID = kAudioFormatLinearPCM;
	streamFormat.mFormatFlags = kFormatFlags;
	streamFormat.mBytesPerPacket = kBytesPerPacket;
	streamFormat.mFramesPerPacket = kFramesPerPacket;
	streamFormat.mBytesPerFrame = kBytesPerFrame;
	streamFormat.mChannelsPerFrame = kChannelsPerFrame;
	streamFormat.mBitsPerChannel = kBitsPerChannel;
	
	if( streamFormat.mSampleRate <= 0 )
		streamFormat.mSampleRate = 44100.0;
	m_iSampleRate = int( streamFormat.mSampleRate );

	error = AudioUnitSetProperty( m_OutputUnit,
				      kAudioUnitProperty_StreamFormat,
				      kAudioUnitScope_Input,
				      0,
				      &streamFormat,
				      sizeof(AudioStreamBasicDescription) );
	if( error != noErr )
		return ERROR( "Failed to set AU stream format", error );
	UInt32 renderQuality = kRenderQuality_Max;
	
	error = AudioUnitSetProperty( m_OutputUnit,
				      kAudioUnitProperty_RenderQuality,
				      kAudioUnitScope_Global,
				      0,
				      &renderQuality,
				      sizeof(renderQuality) );
	if( error != noErr )
		LOG->Warn( WERROR("Failed to set the maximum render quality", error) );
	
	// Try to set the hardware sample rate.
	{
		AudioDeviceID OutputDevice;
		UInt32 size = sizeof( AudioDeviceID );
		
		if( (error = AudioUnitGetProperty(m_OutputUnit, kAudioOutputUnitProperty_CurrentDevice,
						  kAudioUnitScope_Global, 0, &OutputDevice, &size)) )
		{
			LOG->Warn( WERROR("No output device", error) );
		}
		else if( (error = AudioDeviceSetProperty(OutputDevice, NULL, 0, false, kAudioDevicePropertyNominalSampleRate,
							 sizeof(Float64), &streamFormat.mSampleRate)) )
		{
			LOG->Warn( WERROR("Couldn't set the device's sample rate", error) );
		}
	}

	
	// Initialize the AU.
	if( (error = AudioUnitInitialize(m_OutputUnit)) )
		return ERROR( "Could not initialize the AudioUnit", error );
	
	StartDecodeThread();
	
	// Get the HAL's runloop and attach an observer.
	{
		CFRunLoopObserverRef observerRef;
		CFRunLoopRef runLoopRef;
		CFRunLoopObserverContext context = { 0, this, NULL, NULL, NULL };
		UInt32 size = sizeof( CFRunLoopRef );

		if( (error = AudioHardwareGetProperty(kAudioHardwarePropertyRunLoop, &size, &runLoopRef)) )
			return ERROR( "Couldn't get the HAL's run loop", error);
		
		observerRef = CFRunLoopObserverCreate( kCFAllocatorDefault, kCFRunLoopAllActivities, false, 0, NameHALThread, &context );
		CFRunLoopAddObserver( runLoopRef, observerRef, kCFRunLoopDefaultMode );
		CFRunLoopWakeUp( runLoopRef );
		m_Semaphore.Wait();
		CFRunLoopObserverInvalidate( observerRef );
		CFRelease( observerRef );
	}

	if( (error = AudioOutputUnitStart(m_OutputUnit)) )
		return ERROR( "Could not start the AudioUnit", error );
	m_bStarted = true;
	return RString();
}

RageSoundDriver_AU::~RageSoundDriver_AU()
{
	if( !m_OutputUnit )
		return;
	if( m_bStarted )
	{
		m_bDone = true;
		m_Semaphore.Wait();
	}
	AudioUnitUninitialize( m_OutputUnit );
	CloseComponent( m_OutputUnit );
	delete m_pIOThread;
	delete m_pNotificationThread;
}

int64_t RageSoundDriver_AU::GetPosition() const
{
	double scale = m_iSampleRate / AudioGetHostClockFrequency();
	return int64_t( scale * AudioGetCurrentHostTime() );
}


void RageSoundDriver_AU::SetupDecodingThread()
{
	/* Increase the scheduling precedence of the decoder thread. */
	SetThreadPrecedence( 0.75f );
}

float RageSoundDriver_AU::GetPlayLatency() const
{
	OSStatus error;
	UInt32 bufferSize;
	AudioDeviceID OutputDevice;
	UInt32 size = sizeof( AudioDeviceID );
	Float64 sampleRate;
	
	if( (error = AudioUnitGetProperty(m_OutputUnit, kAudioOutputUnitProperty_CurrentDevice,
					  kAudioUnitScope_Global, 0, &OutputDevice, &size)) )
	{
		LOG->Warn( WERROR("No output device", error) );
		return 0.0f;
	}
	
	size = sizeof( Float64 );
	if( (error = AudioDeviceGetProperty(OutputDevice, 0, false, kAudioDevicePropertyNominalSampleRate, &size, &sampleRate)) )
	{
		LOG->Warn( WERROR("Couldn't get the device sample rate", error) );
		return 0.0f;
	}	

	size = sizeof( UInt32 );
	if( (error = AudioDeviceGetProperty(OutputDevice, 0, false, kAudioDevicePropertyBufferFrameSize, &size, &bufferSize)) )
	{
		LOG->Warn( WERROR("Couldn't determine buffer size", error) );
		bufferSize = 0;
	}
	
	UInt32 frames;
	
	size = sizeof( UInt32 );
	if( (error = AudioDeviceGetProperty(OutputDevice, 0, false, kAudioDevicePropertyLatency, &size, &frames)) )
	{
		LOG->Warn( WERROR( "Couldn't get device latency", error) );
		frames = 0;
	}

	bufferSize += frames;
	size = sizeof( UInt32 );
	if( (error = AudioDeviceGetProperty(OutputDevice, 0, false, kAudioDevicePropertySafetyOffset, &size, &frames)) )
	{
		LOG->Warn( WERROR("Couldn't get device safety offset", error) );
		frames = 0;
	}
	bufferSize += frames;
	size = sizeof( UInt32 );

	do {
		if( (error = AudioDeviceGetPropertyInfo(OutputDevice, 0, false, kAudioDevicePropertyStreams, &size, NULL)) )
		{
			LOG->Warn( WERROR("Device has no streams", error) );
			break;
		}
		int num = size / sizeof( AudioStreamID );
		if( num == 0 )
		{
			LOG->Warn( "Device has no streams." );
			break;
		}
		AudioStreamID *streams = new AudioStreamID[num];

		if( (error = AudioDeviceGetProperty(OutputDevice, 0, false, kAudioDevicePropertyStreams, &size, streams)) )
		{
			LOG->Warn( WERROR("Cannot get device's streams", error) );
			delete[] streams;
			break;
		}
		if( (error = AudioStreamGetProperty(streams[0], 0, kAudioDevicePropertyLatency, &size, &frames)) )
		{
			LOG->Warn( WERROR("Stream does not report latency", error) );
			frames = 0;
		}
		delete[] streams;
		bufferSize += frames;
	} while( false );
	
	return bufferSize / sampleRate;
}
	

OSStatus RageSoundDriver_AU::Render( void *inRefCon,
			       AudioUnitRenderActionFlags *ioActionFlags,
			       const AudioTimeStamp *inTimeStamp,
			       UInt32 inBusNumber,
			       UInt32 inNumberFrames,
			       AudioBufferList *ioData )
{
	RageSoundDriver_AU *This = (RageSoundDriver_AU *)inRefCon;

	if( unlikely(This->m_pIOThread == NULL) )
		This->m_pIOThread = new RageThreadRegister( "HAL I/O thread" );

	AudioBuffer &buf = ioData->mBuffers[0];
	double scale = This->m_iSampleRate / AudioGetHostClockFrequency();
	int64_t now = int64_t( scale * AudioGetCurrentHostTime() );
	int64_t next = int64_t( scale * inTimeStamp->mHostTime );
	
	This->Mix( (float *)buf.mData, inNumberFrames, next, now );
	if( unlikely(This->m_bDone) )
	{
		AudioOutputUnitStop( This->m_OutputUnit );
		This->m_Semaphore.Post();
	}	
	return noErr;
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
