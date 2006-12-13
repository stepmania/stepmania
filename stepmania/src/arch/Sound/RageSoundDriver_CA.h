#ifndef RAGE_SOUND_DRIVER_CA_H
#define RAGE_SOUND_DRIVER_CA_H

#include "RageSoundDriver_Generic_Software.h"
#include <vector>
#include <CoreFoundation/CoreFoundation.h>
#include <AudioToolbox/AudioToolbox.h>

class RageSoundBase;
class RageThreadRegister;

class RageSoundDriver_CA : public RageSound_Generic_Software
{
public:
	RageSoundDriver_CA();
	RString Init();
	~RageSoundDriver_CA();
	float GetPlayLatency() const { return m_fLatency; }
	int GetSampleRate( int rate ) const { return m_iSampleRate; }
	int64_t GetPosition() const;
	void SetupDecodingThread();

private:
	float m_fLatency;
	AudioDeviceID m_OutputDevice;
	AudioConverterRef m_Converter;
	bool m_bStarted;
	int m_iSampleRate;
	mutable int64_t m_iLastSampleTime;
	int64_t m_iOffset;
	UInt32 m_iBufferNumber;
	vector<pair<AudioDevicePropertyID, AudioDevicePropertyListenerProc> > m_vPropertyListeners;
	RageThreadRegister *m_pIOThread, *m_pNotificationThread;
	
	void AddListener( AudioDevicePropertyID propertyID,
			  AudioDevicePropertyListenerProc handler, const char *name );
	void RemoveListeners();
	static OSStatus GetData( AudioDeviceID inDevice,
				 const AudioTimeStamp *inNow,
				 const AudioBufferList *inInputData,
				 const AudioTimeStamp *inInputTime,
				 AudioBufferList *outOutputData,
				 const AudioTimeStamp *inOutputTime,
				 void *inClientData );
	
	static void NameHALThread( CFRunLoopObserverRef observer, CFRunLoopActivity activity, void *info );
	static OSStatus OverloadListener( AudioDeviceID inDevice, UInt32 inChannel, Boolean isInput,
					  AudioDevicePropertyID inPropertyID, void *inData );
	static OSStatus DeviceChanged( AudioDeviceID inDevice, UInt32 inChannel, Boolean isInput,
				       AudioDevicePropertyID inPropertyID, void *inData );
	static OSStatus JackChanged( AudioDeviceID inDevice, UInt32 inChannel, Boolean isInput,
				     AudioDevicePropertyID inPropertyID, void *inData );
};

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
