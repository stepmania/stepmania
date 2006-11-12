#include "RageSoundDriver_AU.h"
#include "RageUtil.h"
#include "RageLog.h"
#include "PrefsManager.h"
#include "archutils/Darwin/DarwinThreadHelpers.h"
#include "Foreach.h"
#include <CoreServices/CoreServices.h>

static const UInt32 kFramesPerPacket = 1;
static const UInt32 kChannelsPerFrame = 2;
static const UInt32 kBitsPerChannel = 16;
static const UInt32 kBytesPerPacket = kChannelsPerFrame * kBitsPerChannel / 8;
static const UInt32 kBytesPerFrame = kBytesPerPacket;
static const UInt32 kFormatFlags = kAudioFormatFlagsNativeEndian | kAudioFormatFlagIsSignedInteger;


RageSound_AU::RageSound_AU() : m_OutputUnit(NULL), m_OutputDevice(NULL), m_iSampleRate(0), m_fLatency(0.0f),
	m_iLastSampleTime(0), m_iOffset(0), m_bDone(false), m_pIOThread(NULL), m_Semaphore("Sound shutdown")
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

RString RageSound_AU::Init()
{
	ComponentDescription desc;
	
	desc.componentType = kAudioUnitType_Output;
	desc.componentSubType = kAudioUnitSubType_HALOutput;
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
	
	
	UInt32 data = 1;
	error = AudioUnitSetProperty( m_OutputUnit,
				      kAudioOutputUnitProperty_EnableIO,
				      kAudioUnitScope_Output,
				      0,
				      &data,
				      sizeof(data) );
	if( error != noErr )
		return ERROR( "Could not enable output I/O", error );
	
	UInt32 size = sizeof(AudioDeviceID);
	
	error = AudioHardwareGetProperty( kAudioHardwarePropertyDefaultOutputDevice, &size, &m_OutputDevice );
	if( error != noErr )
		return ERROR( "Could not get the default output device", error );
	
	error = AudioUnitSetProperty( m_OutputUnit,
				      kAudioOutputUnitProperty_CurrentDevice,
				      kAudioUnitScope_Global,
				      0,
				      &m_OutputDevice,
				      sizeof(AudioDeviceID) );
	if( error != noErr )
		return ERROR( "Could not set the default output device", error );
	
	// XXX set channel map
#if 1
	data = 0;
	error = AudioUnitSetProperty( m_OutputUnit,
				      2007, //kAudioOutputUnitProperty_StartTimestampsAtZero,
				      kAudioUnitScope_Global,
				      0,
				      &data,
				      sizeof(data) );
	if( error != noErr )
		return ERROR( "Could not set AudioUnit to use the device's time stamps", error );
#endif
	
	// Initialize the AU.
	error = AudioUnitInitialize( m_OutputUnit );
	if( error != noErr )
		return ERROR( "Could not initialize the AudioUnit", error );
	
	Float64 latency = 0.0;
	
	size = sizeof(latency);
	error = AudioUnitGetProperty( m_OutputUnit,
				      kAudioUnitProperty_Latency,
				      kAudioUnitScope_Global,
				      0,
				      &latency,
				      &size );
	if( error != noErr )
		LOG->Warn( WERROR("Could not get latency", error) );
	LOG->Info( "Latency: %lf", latency );
	m_fLatency = latency;
	
	// Add property listeners.
	for( AudioUnitPropertyID prop = kAudioUnitProperty_ClassInfo; prop <= kAudioUnitProperty_PresentPreset; ++prop )
		AddAUPropertyListener( prop );
	for( AudioUnitPropertyID prop = kAudioOutputUnitProperty_CurrentDevice; prop <= kAudioOutputUnitProperty_HasIO; ++prop )
		AddAUPropertyListener( prop );
#define ADD(x) AddADPropertyListener( kAudioDeviceProperty ## x )
#ifdef DEBUG
	AddADPropertyListener( kAudioPropertyWildcardPropertyID );
#else
	ADD( DeviceIsAlive );
	ADD( DeviceHasChanged );
	ADD( DeviceIsRunning );
	ADD( DeviceIsRunningSomewhere );
	AddADPropertyListener( kAudioDeviceProcessorOverload );
	ADD( Streams );
	ADD( StreamConfiguration );
	ADD( StreamFormat );
#endif
#undef ADD
	
	StartDecodeThread();
	error = AudioOutputUnitStart( m_OutputUnit );
	if( error != noErr )
		return ERROR( "Could not start the AudioUnit", error );
	return RString();
}

RageSound_AU::~RageSound_AU()
{
	ShutDown();
	delete m_pIOThread;
}

int64_t RageSound_AU::GetPosition( const RageSoundBase *sound ) const
{
	AudioTimeStamp time;
	OSStatus error;
	
	if( m_OutputDevice == NULL )
		return m_iLastSampleTime;
	if( (error = AudioDeviceGetCurrentTime(m_OutputDevice, &time)) )
	{
		if( error != kAudioHardwareNotRunningError )
			FAIL_M( ERROR("GetCurrentTime() failed", error) );
		return m_iLastSampleTime;
	}
	m_iLastSampleTime = int64_t( time.mSampleTime ) + m_iOffset;
	return m_iLastSampleTime;
}


void RageSound_AU::SetupDecodingThread()
{
	/* Increase the scheduling precedence of the decoder thread. */
	SetThreadPrecedence( 0.75f );
}

void RageSound_AU::ShutDown()
{
	if( !m_OutputUnit )
		return;
	FOREACH_CONST( AudioUnitPropertyID, m_vAUProperties, prop )
		AudioUnitRemovePropertyListener( m_OutputUnit, *prop, PropertyChanged );
	FOREACH_CONST( AudioDevicePropertyID, m_vADProperties, prop )
		AudioDeviceRemovePropertyListener( m_OutputDevice, 0, false, *prop, DevicePropertyChanged );
	m_bDone = true;
	m_Semaphore.Wait();
	m_vADProperties.clear();
	m_vAUProperties.clear();
	AudioUnitUninitialize( m_OutputUnit );
	CloseComponent( m_OutputUnit );
	m_OutputUnit = NULL;
	m_OutputDevice = NULL;
	m_iSampleRate = 0;
}

void RageSound_AU::AddAUPropertyListener( AudioUnitPropertyID prop )
{
	ComponentResult result = AudioUnitAddPropertyListener( m_OutputUnit, prop, PropertyChanged, this );

	if( result )
		LOG->Warn( "Failed to add property listener for property %d.", int(prop) );
	else
		m_vAUProperties.push_back( prop );
}

void RageSound_AU::AddADPropertyListener( AudioDevicePropertyID prop )
{
	OSStatus error = AudioDeviceAddPropertyListener( m_OutputDevice, kAudioPropertyWildcardChannel, false,
							 prop, DevicePropertyChanged, this );
	
	if( error )
		LOG->Warn( WERROR("Failed to add device property listener for property %s",
				  error, FourCCToString(prop).c_str()) );
	else
		m_vADProperties.push_back( prop );
}

OSStatus RageSound_AU::Render( void *inRefCon,
			       AudioUnitRenderActionFlags *ioActionFlags,
			       const AudioTimeStamp *inTimeStamp,
			       UInt32 inBusNumber,
			       UInt32 inNumberFrames,
			       AudioBufferList *ioData )
{
	RageSound_AU *This = (RageSound_AU *)inRefCon;

	if( unlikely(This->m_pIOThread == NULL) )
		This->m_pIOThread = new RageThreadRegister( "HAL I/O thread" );
	AudioBuffer &buf = ioData->mBuffers[0];
	int64_t decodePos = int64_t( inTimeStamp->mSampleTime ) + This->m_iOffset;
	
	This->Mix( (int16_t *)buf.mData, inNumberFrames, decodePos, This->GetPosition(NULL) );
	if( unlikely(This->m_bDone) )
	{
		AudioOutputUnitStop( This->m_OutputUnit );
		This->m_Semaphore.Post();
	}	
	return noErr;
}

void RageSound_AU::PropertyChanged( void *inRefCon,
				    AudioUnit au,
				    AudioUnitPropertyID inID,
				    AudioUnitScope inScope,
				    AudioUnitElement inElement )
{
	RageSound_AU *This = (RageSound_AU *)inRefCon;
	OSStatus error;
	UInt32 running;
	UInt32 size = sizeof( running );
	AudioTimeStamp time;

	ASSERT( au == This->m_OutputUnit );
	
	switch( inID )
	{
	case kAudioOutputUnitProperty_CurrentDevice:
	{
		LOG->Info( "Audio device changed!" );
		This->ShutDown();
		RString result = This->Init();
		ASSERT_M( result.empty(), result );
		return;
	}
	case kAudioOutputUnitProperty_IsRunning:
		if( (error = AudioUnitGetProperty(au, inID, inScope, inElement, &running, &size)) )
			FAIL_M( ERROR("Couldn't get running property", error) );
		LOG->Trace( "Audio device %s running.", running ? "started" : "stopped" );
		if( !running )
			return;
		// Fall through.
	case kAudioUnitProperty_StreamFormat:
		if( inScope != kAudioUnitScope_Output )
			return;
		if( (error = AudioDeviceGetCurrentTime(This->m_OutputDevice, &time)) )
		{
			if( error != kAudioHardwareNotRunningError )
				FAIL_M( ERROR("Couldn't get the current time", error) );
			LOG->Trace( "Old offset %lld, last sample time %lld.",
				    This->m_iOffset, This->m_iLastSampleTime );
			This->m_iOffset = This->m_iLastSampleTime;
		}
		else
		{
			LOG->Trace( "Old offset %lld, last sample time %lld, current sample time %lld, new offset %lld.",
				    This->m_iOffset, This->m_iLastSampleTime, int64_t(time.mSampleTime), This->m_iLastSampleTime - int64_t(time.mSampleTime) );
			This->m_iOffset = This->m_iLastSampleTime - int64_t( time.mSampleTime );
		}
		//return;
	}
	
	const char *scope;
	switch( inScope )
	{
	case kAudioUnitScope_Global:	scope = "global";	break;
	case kAudioUnitScope_Input:	scope = "input";	break;
	case kAudioUnitScope_Output:	scope = "output";	break;
	case kAudioUnitScope_Group:	scope = "group";	break;
	case kAudioUnitScope_Part:	scope = "part";		break;
	DEFAULT_FAIL( int(inScope) );
	}	
	
	LOG->Trace( "PropertyChanged: id = %d, scope = %s, element %d.", int(inID), scope, int(inElement) );
}

OSStatus RageSound_AU::DevicePropertyChanged( AudioDeviceID inDevice,
					      UInt32 inChannel,
					      Boolean isInput,
					      AudioDevicePropertyID inID,
					      void *inRefCon )
{
	RageSound_AU *This = (RageSound_AU *)inRefCon;
	UInt32 value;
	UInt32 size = sizeof(value);
	bool somewhere = true;
	OSStatus error;
	
	switch( inID )
	{
	case kAudioDevicePropertyDeviceIsAlive:
		LOG->Warn( "Audio device died." );
		This->m_OutputDevice = NULL; // The AudioDeviceID is now invalid.
		This->m_vADProperties.clear();
		break;
	case kAudioDevicePropertyDeviceHasChanged:
		// Hopefully this will be handled sanely by the AudioUnit.
		LOG->Trace( "Audio device has changed in some way." );
		break;
	case kAudioDevicePropertyDeviceIsRunning:
		somewhere = false;
		// fall through.
	case kAudioDevicePropertyDeviceIsRunningSomewhere:
		if( (error = AudioDeviceGetProperty(inDevice, inChannel, isInput, inID, &size, &value)) )
			FAIL_M( ERROR("AudioDeviceGetProperty('%s') failed", error, FourCCToString(inID).c_str()) );
		LOG->Trace( "Audio device has %s running%s.", value ? "started" : "stopped", somewhere ? " somewhere" : "" );
		break;
	case kAudioDeviceProcessorOverload:
		LOG->Trace( "Audio overload." ); // This is being called on the real time thread. hmm.
		return 0; // Do nothing else.
	case kAudioDevicePropertyStreams:
		// Hopefully this will be handled sanely by the AudioUnit.
		LOG->Trace( "Audio device's streams have changed." );
		break;
	case kAudioDevicePropertyStreamConfiguration:
		// Hopefully this will be handled sanely by the AudioUnit.
		LOG->Trace( "Audio device's stream configuration has changed." );
		break;
	case kAudioDevicePropertyStreamFormat:
		// Hopefully this will be handled sanely by the AudioUnit.
		LOG->Trace( "Audio device's stream format has changed." );
		break;
	}
	LOG->Trace( "Audio device property '%s' changed.", FourCCToString(inID).c_str() );
	return 0;
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
