#ifndef RAGEMOVIETEXTURE_FFMPEG_H
#define RAGEMOVIETEXTURE_FFMPEG_H

#include "MovieTexture.h"

#include "RageDisplay.h"
#include "RageTexture.h"
#include "RageThreads.h"

#include "SDL_mutex.h"

#define SUPPORT_MOVIETEXTURE_FFMPEG

namespace avcodec
{
#if defined(_WIN32)
#include "ffmpeg/include/ffmpeg/avformat.h"
#else
#include <ffmpeg/avformat.h>
#endif
};


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
	avcodec::AVFormatContext *m_fctx;
	avcodec::AVStream *m_stream;
	avcodec::AVCodec *m_codec;

	/* The time the movie is actually at: */
	float m_Position;
	float m_Rate;
	bool m_ImageWaiting;
	bool m_shutdown;
	bool m_bLoop;

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
	 *
	 * PLAYING_ONE: The decoder thread will decode one frame, post the
	 * m_FrameDecoded semaphore and change to the PAUSE_DECODER state.
	 * This is only used by Create().
	 */
	enum State { DECODER_QUIT, PAUSE_DECODER, PLAYING, PLAYING_ONE } m_State;

	unsigned m_uTexHandle;

	SDL_Surface *m_img;
	int m_AVTexfmt; /* AVPixelFormat_t of m_img */

	SDL_sem *m_BufferFinished, *m_OneFrameDecoded;

	static int DecoderThread_start(void *p) { ((MovieTexture_FFMpeg *)(p))->DecoderThread(); return 0; }
	void DecoderThread();
	RageThread m_DecoderThread;

	void CreateDecoder();
	void CreateTexture();
	void DestroyDecoder();
	void DestroyTexture();
	void CheckFrame();
	void StartThread();
	void StopThread();
};

/*
 Copyright (c) 2003 by the person(s) listed below.  All rights reserved.
	Glenn Maynard
*/
#endif
