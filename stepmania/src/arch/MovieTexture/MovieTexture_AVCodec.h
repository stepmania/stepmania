#ifndef RAGEMOVIETEXTURE_AVCODEC_H
#define RAGEMOVIETEXTURE_AVCODEC_H

#include "MovieTexture.h"

#include "RageDisplay.h"
#include "RageTexture.h"
#include "RageThreads.h"
#include "RageTimer.h"

#include "SDL_mutex.h"

namespace avcodec
{
#include "ffmpeg/common.h"
#include "ffmpeg/avformat.h"
};


class MovieTexture_AVCodec : public RageMovieTexture
{
public:
	MovieTexture_AVCodec( RageTextureID ID );
	virtual ~MovieTexture_AVCodec();
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

private:
	avcodec::AVFormatContext *m_fctx;
	avcodec::AVStream *m_stream;
	avcodec::AVCodec *m_codec;

	RageTimer m_RunningTimer;
	/* The time the movie *should* be at right now: */
	float m_Timer;

	/* The time the movie is actually at: */
	float m_Position;

	float m_ActualTimer;
	float m_Rate;

	bool m_ImageWaiting;
	bool m_shutdown;
	bool m_bLoop;

	/* If m_bPausing is true, then the decoder thread will stop on its
	 * next iteration, set m_bPlaying false and post the "paused" semaphore. */

	/*
	 * Only the main thread can change m_State, except to go from the
	 * PAUSING_DECODER to PAUSING_DECODER_OK state.
	 *
	 * PLAYING->PAUSE_DECODER (main thread only): ask the decoder thread to pause
	 * PAUSE_DECODER->PLAYING (main thread only): ask the decoder thread to resume
	 * *->QUITTING (main thread only): 
	 * 
	 * If in PLAYING, pause by setting m_State to PAUSE_DECODER.  
	 * The decoding thread will notice this, change to PAUSED and post
	 * m_paused.  Then, on the next command, CompletePause will
	 * wait for the semaphore.  CompletePause must be called before
	 * changing to another state (to clear the semaphore).
	 * 
	 * Transitions:
	 * Default: PAUSED
	 * CompletePause: If PAUSE_OK, WAITING_FOR_PAUSE, block on m_paused, then set
	 * to PAUSED.
	 * Play(): If PAUSED, set to PLAYING.  If WAITING_FOR_PAUSE, wait
	 * for the "paused" semaphore to 
	 */
	/* DECODER_QUIT: The decoder thread is not running.  We should only
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
	int m_AVTexfmt; /* of m_img */

	SDL_sem *m_BufferFinished, *m_OneFrameDecoded;

	static int DecoderThread_start(void *p) { ((MovieTexture_AVCodec *)(p))->DecoderThread(); return 0; }
	void DecoderThread();
	RageThread m_DecoderThread;

	void Create();
	void Destroy();
	void CreateTexture();
	void CheckFrame();
	void StartThread();
	void StopThread();


	unsigned GetTexHandle() const { return m_uTexHandle; }
};

/*
 Copyright (c) 2003 by the person(s) listed below.  All rights reserved.
	Glenn Maynard
*/
#endif
