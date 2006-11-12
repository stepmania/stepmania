#ifndef RAGE_SOUND_DRIVER_AU_H
#define RAGE_SOUND_DRIVER_AU_H

#include "RageSoundDriver_Generic_Software.h"
#include "RageThreads.h"
#include <AudioUnit/AudioUnit.h>

class RageSound_AU : public RageSound_Generic_Software
{
public:
	RageSound_AU();
	RString Init();
	~RageSound_AU();
	float GetPlayLatency() const { return m_fLatency; }
	int GetSampleRate( int rate ) const { return m_iSampleRate; }
	int64_t GetPosition( const RageSoundBase *sound ) const;
	
protected:
	void SetupDecodingThread();
	
private:
	void ShutDown();
	void AddAUPropertyListener( AudioUnitPropertyID prop );
	void AddADPropertyListener( AudioDevicePropertyID prop );
	static OSStatus Render( void *inRefCon,
				AudioUnitRenderActionFlags *ioActionFlags,
				const AudioTimeStamp *inTimeStamp,
				UInt32 inBusNumber,
				UInt32 inNumberFrames,
				AudioBufferList *ioData );
	static void PropertyChanged( void *inRefCon,
				     AudioUnit au,
				     AudioUnitPropertyID inID,
				     AudioUnitScope inScope,
				     AudioUnitElement inElement );
	static OSStatus DevicePropertyChanged( AudioDeviceID inDevice,
					       UInt32 inChannel,
					       Boolean isInput,
					       AudioDevicePropertyID inID,
					       void *inRefCon );
	AudioUnit m_OutputUnit;
	AudioDeviceID m_OutputDevice;
	int m_iSampleRate;
	float m_fLatency;
	mutable int64_t m_iLastSampleTime;
	int64_t m_iOffset;
	vector<AudioUnitPropertyID> m_vAUProperties;
	vector<AudioDevicePropertyID> m_vADProperties;
	bool m_bDone;
	RageThreadRegister *m_pIOThread;
	RageSemaphore m_Semaphore;
};

#define USE_RAGE_SOUND_AU
#endif
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

