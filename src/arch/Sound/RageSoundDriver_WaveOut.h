#ifndef RAGE_SOUND_WAVEOUT_H
#define RAGE_SOUND_WAVEOUT_H

#include "RageSoundDriver.h"
#include "RageThreads.h"
#include "format.h"
#include <windows.h>
#include <mmsystem.h>

class RageSoundDriver_WaveOut: public RageSoundDriver
{
public:
	RageSoundDriver_WaveOut();
	~RageSoundDriver_WaveOut();
	std::string Init();

	int64_t GetPosition() const;
	float GetPlayLatency() const;
	int GetSampleRate() const { return m_iSampleRate; }

	template<typename... Args>
	static std::string wo_format(MMRESULT err, std::string const &msg, Args const & ...args)
	{
		std::string item = fmt::format(msg, args...);
		return wo_final(item, err);
	}
private:
	static int MixerThread_start( void *p );
	static std::string wo_final(std::string const &msg, MMRESULT err);
	void MixerThread();
	RageThread MixingThread;
	bool GetData();
	void SetupDecodingThread();

	HWAVEOUT m_hWaveOut;
	HANDLE m_hSoundEvent;
	WAVEHDR m_aBuffers[8];
	int m_iSampleRate;
	bool m_bShutdown;
	int m_iLastCursorPos;
};

#endif

/*
 * (c) 2002-2004 Glenn Maynard
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
