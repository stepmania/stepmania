#ifndef RAGEMOVIETEXTURE_FFMPEG_H
#define RAGEMOVIETEXTURE_FFMPEG_H

#include "MovieTexture.h"

#include "RageDisplay.h"
#include "RageTexture.h"
#include "RageThreads.h"
#include "RageTimer.h"

#include "SDL_mutex.h"

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
	/* only called by RageTextureManager::InvalidateTextures */
	void Invalidate() { m_uTexHandle = 0; }
	void Update(float fDeltaTime);

	virtual void Reload();

	virtual void Play();
	virtual void Pause();
	virtual void SetPosition( float fSeconds );
	virtual void SetPlaybackRate( float fRate ) { m_Rate=fRate; }
	virtual bool IsPlaying() const { return m_State == PLAYING; }
	void SetLooping(bool looping=true) { m_bLoop = looping; }
	unsigned GetTexHandle() const { return m_uTexHandle; }

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
	enum State { DECODER_QUIT, PAUSE_DECODER, PLAYING } m_State;

	unsigned m_uTexHandle;

	SDL_Surface *m_img;
	int m_AVTexfmt; /* AVPixelFormat_t of m_img */

	SDL_sem *m_BufferFinished;

	RageTimer m_Timer;
	float m_Clock;
	bool m_FrameSkipMode;

	static int DecoderThread_start(void *p) { ((MovieTexture_FFMpeg *)(p))->DecoderThread(); return 0; }
	void DecoderThread();
	RageThread m_DecoderThread;

	void ConvertFrame();
	void UpdateFrame();

	void CreateDecoder();
	void CreateTexture();
	void DestroyDecoder();
	void DestroyTexture();
	void StartThread();
	void StopThread();

	void UpdateTimer();
	bool DecodeFrame();
	float CheckFrameTime();
	void DiscardFrame();
};

/*
 Copyright (c) 2003 by the person(s) listed below.  All rights reserved.
	Glenn Maynard
*/
#endif
