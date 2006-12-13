#ifndef RAGE_SOUND_WAVEOUT_H
#define RAGE_SOUND_WAVEOUT_H

#include "RageSoundDriver_Generic_Software.h"
#include "RageThreads.h"
#include <windows.h>

struct WinWdmStream;
struct WinWdmFilter;

class RageSoundDriver_WDMKS: public RageSound_Generic_Software
{
public:
	RageSoundDriver_WDMKS();
	~RageSoundDriver_WDMKS();
	RString Init();

	int64_t GetPosition() const;
	float GetPlayLatency() const;
	int GetSampleRate( int iRate ) const;

private:
	static int MixerThread_start( void *p );
	void MixerThread();
	bool Fill( int iPacket, RString &sError );
	void Read( void *pData, int iFrames, int iLastCursorPos, int iCurrentFrame );

	RageThread MixingThread;
	void SetupDecodingThread();

	bool m_bShutdown;
	int m_iLastCursorPos;

	HANDLE m_hSignal;
	WinWdmStream *m_pStream;
	WinWdmFilter *m_pFilter;
};

#endif

/*
 * (c) 2002-2006 Glenn Maynard
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
