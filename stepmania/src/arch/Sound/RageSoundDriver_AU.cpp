#include "RageSoundDriver_AU.h"
#include "RageUtil.h"
#include "RageLog.h"
#include "PrefsManager.h"
#include "archutils/Darwin/DarwinThreadHelpers.h"
#include <CoreServices/CoreServices.h>

static const UInt32 kFramesPerPacket = 1;
static const UInt32 kChannelsPerFrame = 2;
static const UInt32 kBitsPerChannel = 16;
static const UInt32 kBytesPerPacket = kChannelsPerFrame * kBitsPerChannel / 8;
static const UInt32 kBytesPerFrame = kBytesPerPacket;
static const UInt32 kFormatFlags = kAudioFormatFlagsNativeEndian | kAudioFormatFlagIsSignedInteger;


RageSound_AU::RageSound_AU() : m_OutputUnit(NULL), m_OutputDevice(NULL), m_iSampleRate(0),
	m_fLatency(0.0), m_iLastSampleTime(0), m_iOffset(0), m_pIOThread(NULL)
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
				      kAudioUnitScope_Output,
				      0,
				      &latency,
				      &size );
	if( error != noErr )
		LOG->Warn( WERROR("Could not get latency", error) );
	LOG->Info( "Latency: %lf", latency );
	m_fLatency = latency;
	
	StartDecodeThread();
	error = AudioOutputUnitStart( m_OutputUnit );
	if( error != noErr )
		return ERROR( "Could not start the AudioUnit", error );
	return RString();
}

RageSound_AU::~RageSound_AU()
{
	AudioOutputUnitStop( m_OutputUnit );
	AudioUnitUninitialize( m_OutputUnit );
	CloseComponent( m_OutputUnit );
}

int64_t RageSound_AU::GetPosition( const RageSoundBase *sound ) const
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


void RageSound_AU::SetupDecodingThread()
{
	/* Increase the scheduling precedence of the decoder thread. */
	SetThreadPrecedence( 0.75f );
}

OSStatus RageSound_AU::Render( void *inRefCon,
			       AudioUnitRenderActionFlags *ioActionFlags,
			       const AudioTimeStamp *inTimeStamp,
			       UInt32 inBusNumber,
			       UInt32 inNumberFrames,
			       AudioBufferList *ioData )
{
	RageSound_AU *This = (RageSound_AU *)inRefCon;
	AudioTimeStamp time;
	
	time.mFlags = inTimeStamp->mFlags & ~kAudioTimeStampHostTimeValid;
	
	AudioDeviceTranslateTime( This->m_OutputDevice, inTimeStamp, &time );
	//ASSERT( time.mFlags & kAudioTimeStampHostTimeValid );
	
	if( unlikely(This->m_pIOThread == NULL) )
		This->m_pIOThread = new RageThreadRegister( "HAL I/O thread" );
	AudioBuffer &buf = ioData->mBuffers[0];
	//int64_t decodePos = int64_t( time.mSampleTime ) + This->m_iOffset;
	int64_t decodePos = int64_t( inTimeStamp->mSampleTime ) + This->m_iOffset;
	
	This->Mix( (int16_t *)buf.mData, inNumberFrames, decodePos, This->GetPosition(NULL) );
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
