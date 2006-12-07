#ifndef RAGE_SOUND_GENERIC_SOFTWARE
#define RAGE_SOUND_GENERIC_SOFTWARE

#include "RageSoundDriver.h"
#include "RageThreads.h"
#include "RageTimer.h"
#include "RageUtil_CircularBuffer.h"

static const int samples_per_block = 512;
class RageSound_Generic_Software: public RageSoundDriver
{
public:
	virtual void Update();

	void StartMixing( RageSoundBase *pSound );		/* used by RageSound */
	void StopMixing( RageSoundBase *pSound );		/* used by RageSound */
	bool PauseMixing( RageSoundBase *pSound, bool bStop );

	RageSound_Generic_Software();
	virtual ~RageSound_Generic_Software();

protected:
	/* Start the decoding.  This should be called once the hardware is set up and
	 * GetSampleRate will return the correct value. */
	void StartDecodeThread();

	/* Call this before calling StartDecodeThread to set the desired decoding buffer
	 * size.  This is the number of frames that Mix() will try to be able to return
	 * at once.  This should generally be slightly larger than the sound writeahead,
	 * to allow filling the buffer after an underrun.  The default is 4096 frames. */
	void SetDecodeBufferSize( int frames );
	
	/* Override this to set the priority of the decoding thread, which should be above
	 * normal priority but not realtime. */
	virtual void SetupDecodingThread() { }

	/*
	 * Read mixed data.
	 *
	 * pBuf: buffer to read into
	 * iFrames: number of frames (not samples) to read
	 * frameno: frame number at which this sound will be heard
	 * iCurrentFrame: frame number that is currently being heard
	 *
	 * iCurrentFrame is used for handling start timing.
	 *
	 * This function only mixes data; it will not lock any mutexes or do any file access, and
	 * is safe to call from a realtime thread.
	 */
	void Mix( int16_t *pBuf, int iFrame, int64_t iFrameNumber, int64_t iCurrentFrame );

	/* This mutex is used for serializing with the decoder thread.  Locking this mutex
	 * can take a while. */
	RageMutex m_Mutex;

	/* This mutex locks all sounds[] which are "available".  (Other sound may safely
	 * be accessed, and sounds may be set to available, without locking this.) */
	RageMutex m_SoundListMutex;

private:
	/*
	 * Thread safety and state transitions:
	 *
	 * AVAILABLE: The sound is available to play a new sound. The decoding and mixing threads
	 * will not touch a sound in this state.
	 *
	 * BUFFERING: The sound is stopped but StartMixing() is prebuffering. No other threads
	 * will touch a sound that is BUFFERING. This isn't necessary if only the main thread
	 * can call StartMixing().
	 *
	 * STOPPED: The sound is idle, but memory is still allocated for its buffer. Update()
	 * will deallocate memory and the sound will be changed to AVAILABLE.
	 *
	 * PLAYING: The sound is being decoded by the decoding thread, and played by the mixing
	 * thread.  If the decoding thread hits EOF, the decoding thread will change the state
	 * to STOPPING.
	 *
	 * STOPPING: The sound is being played by the mixing thread.  No new data will be decoded.
	 * Once the data buffer is empty (all sound has been played), Update() will change the
	 * sound to HALTING.
	 *
	 * HALTING: The main thread has called StopMixing or the data buffer is empty.  The mixing
	 * thread will flush any remaining buffered data without playing it, and then move the
	 * sound to STOPPED.
	 * 
	 * The mixing thread operates without any locks.  This can lead to a little overlap.  For
	 * example, if StopMixing() is called, moving the sound from PLAYING to HALTING, the mixing
	 * thread might be in the middle of mixing data.  Although HALTING means "discard buffered
	 * data", some data will still be mixed.  This is OK; the data is valid, and the flush will
	 * happen on the next iteration.
	 * 
	 * The only state change made by the decoding thread is on EOF: the state is changed
	 * from PLAYING to STOPPING.  This is done while m_Mutex is held, to prevent
	 * races with other threads.
	 *
	 * The only state change made by the mixing thread is from HALTING to STOPPED.
	 * This is done with no locks; no other thread can take a sound out of the HALTING state.
	 *
	 * Do not allocate or deallocate memory in the mixing thread since allocating memory
	 * involves taking a lock. Instead, push the deallocation to the main thread.
	 */
	struct sound_block
	{
		int16_t m_Buffer[samples_per_block];
		int16_t *m_BufferNext; // beginning of the unread data
		int m_FramesInBuffer; // total number of frames at m_BufferNext
		int64_t m_iPosition; // stream frame of m_BufferNext
		sound_block() { m_FramesInBuffer = m_iPosition = 0; m_BufferNext = m_Buffer; }
	};

	struct Sound
	{
		Sound();
		void Allocate( int iFrames );
		void Deallocate();

		RageSoundBase *m_pSound;
		int m_iSoundID;
		RageTimer m_StartTime;
		float m_fVolume;
		CircBuf<sound_block> m_Buffer;
		bool m_bPaused;

		enum
		{
			AVAILABLE,
			BUFFERING,
			STOPPED,	/* idle */

			/* This state is set by the decoder thread, indicating that the sound has just
			 * reached EOF.  Once the mixing thread finishes flushing buffer, it'll change
			 * to the STOPPING_FINISH state. */
			STOPPING,

			HALTING,	/* stop immediately */
			PLAYING
		} m_State;
	};

	/* List of currently playing sounds: XXX no vector */
	Sound m_Sounds[32];

	bool m_bShutdownDecodeThread;

	static int DecodeThread_start( void *p );
	void DecodeThread();
	RageThread m_DecodeThread;

	int GetDataForSound( Sound &s );
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
