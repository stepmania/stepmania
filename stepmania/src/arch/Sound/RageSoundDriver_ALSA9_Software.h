#ifndef RAGE_SOUND_ALSA9_SOFTWARE_H
#define RAGE_SOUND_ALSA9_SOFTWARE_H

#include "RageSound.h"
#include "RageThreads.h"
#include "RageSoundDriver_Generic_Software.h"

#include "ALSA9Helpers.h"

class RageSound_ALSA9_Software: public RageSound_Generic_Software
{
public:
	/* virtuals: */
	int64_t GetPosition( const RageSoundBase *pSound ) const;
	float GetPlayLatency() const;
        int GetSampleRate( int iRate ) const;

	void SetupDecodingThread();
		
	RageSound_ALSA9_Software();
	~RageSound_ALSA9_Software();
	CString Init();

private:
	static int MixerThread_start( void *p );
	void MixerThread();
	bool GetData();

	bool m_bShutdown;
	Alsa9Buf *m_pPCM;
	RageThread m_MixingThread;
};
#define USE_RAGE_SOUND_ALSA9_SOFTWARE

#endif

/*
 * (c) 2002-2004 Glenn Maynard, Aaron VonderHaar
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
