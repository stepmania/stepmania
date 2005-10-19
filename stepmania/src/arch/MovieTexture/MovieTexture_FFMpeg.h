#ifndef RAGEMOVIETEXTURE_FFMPEG_H
#define RAGEMOVIETEXTURE_FFMPEG_H

#include "MovieTexture.h"

#include "RageDisplay.h"
#include "RageTexture.h"
#include "RageThreads.h"
#include "RageTimer.h"

/* Fix a compile problem in gcc 3.2: */
#if defined(HAVE_INTTYPES_H)
#include <inttypes.h>
#endif

class FFMpeg_Helper;

class MovieTexture_FFMpeg: public RageMovieTexture
{
public:
	MovieTexture_FFMpeg( RageTextureID ID );
	virtual ~MovieTexture_FFMpeg();
	CString Init();

	/* only called by RageTextureManager::InvalidateTextures */
	void Invalidate() { m_uTexHandle = 0; }
	void Update( float fDeltaTime );

	virtual void Reload();

	virtual void SetPosition( float fSeconds );
	virtual void DecodeSeconds( float fSeconds );
	virtual void SetPlaybackRate( float fRate ) { m_Rate=fRate; }
	void SetLooping( bool bLooping=true ) { m_bLoop = bLooping; }
	unsigned GetTexHandle() const { return m_uTexHandle; }

	static void RegisterProtocols();

private:
	FFMpeg_Helper *decoder;

	/* The time the movie is actually at: */
	float m_Rate;
	enum {
		FRAME_NONE, /* no frame available; call GetFrame to get one */
		FRAME_DECODED, /* frame decoded; call ConvertFrame */
		FRAME_WAITING /* frame converted and waiting to be uploaded */
	} m_ImageWaiting;
	bool m_bLoop;
	bool m_bWantRewind;
	bool m_bThreaded;

	/*
	 * Only the main thread can change m_State.
	 *
	 * DECODER_QUIT: The decoder thread is not running.  We should only
	 * be in this state internally; when we return after a call, we should
	 * never be in this state.  Start the thread before returning.
	 *
	 * PAUSE_DECODER: The decoder thread is idle.
	 *
	 * PLAYING: The decoder thread is running.
	 */
	enum State { DECODER_QUIT, DECODER_RUNNING } m_State;

	unsigned m_uTexHandle;

	RageSurface *m_img;
	int m_AVTexfmt; /* AVPixelFormat_t of m_img */

	RageSemaphore m_BufferFinished;

	RageTimer m_Timer;
	float m_Clock;
	bool m_FrameSkipMode;

	static int DecoderThread_start(void *p) { ((MovieTexture_FFMpeg *)(p))->DecoderThread(); return 0; }
	void DecoderThread();
	RageThread m_DecoderThread;

	void ConvertFrame();
	void UpdateFrame();

	CString CreateDecoder();
	void CreateTexture();
	void DestroyDecoder();
	void DestroyTexture();
	void StartThread();
	void StopThread();

	bool DecodeFrame();
	float CheckFrameTime();
	void DiscardFrame();
};
#define USE_MOVIE_TEXTURE_FFMPEG

#endif

/*
 * (c) 2003-2004 Glenn Maynard
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
