#ifndef RAGE_SOUND_GENERIC_SOFTWARE
#define RAGE_SOUND_GENERIC_SOFTWARE

#include "RageSoundDriver.h"
#include "RageThreads.h"
#include "RageTimer.h"
#include "RageUtil_CircularBuffer.h"

static const int samples_per_block = 512;
class RageSound_Generic_Software: public RageSoundDriver
{
	/*
	 * Thread safety and state transitions:
	 *
	 * STOPPED: The sound is idle, and can be used to play a new sound.  The decoding and
	 * mixing threads will not touch a sound in this state.
	 *
	 * PLAYING: The sound is being decoded by the decoding thread, and played by the mixing
	 * thread.  If the decoding thread hits EOF, the decoding thread will changed the state
	 * to STOPPING.
	 *
	 * STOPPING: The sound is being played by the mixing thread.  No new data will be decoded.
	 * Once the data buffer is empty (all sound has been played), Update() will call StopMixing,
	 * and the sound will be changed to STOPPED.
	 *
	 * HALTING: The main thread has called StopMixing.  The mixing thread will flush the buffered
	 * data without playing it, and then move the sound to STOPPED.
	 * 
	 * The mixing thread operates without any locks.  This can lead to a little overlap.  For
	 * example, if StopMixing() is called, moving the sound from PLAYING to HALTING, the mixing
	 * thread might be in the middle of mixing data.  Although HALTING means "discard buffered
	 * data", some data will still be mixed.  This is OK; the data is valid, and the flush will
	 * happen on the next iteration.
	 * 
	 * The only state change made by the decoding thread is on EOF: the state is changed
	 * from PLAYING to STOPPING.  This is done while SOUNDMAN->lock is held, to prevent
	 * races with other threads.
	 *
	 * The only state change made by the mixing thread is from HALTING to STOPPED.
	 * This is done with no locks; no other thread can take a sound out of the HALTING state.
	 */
	struct sound_block
	{
		int16_t buf[samples_per_block];
		int16_t *p; // beginning of the unread data
		int frames_in_buffer; // total number of frames (not samples) at p
		int position; // position value of p
		sound_block() { frames_in_buffer = position = 0; p = buf; }
	};

	struct sound
	{
	    RageSoundBase *snd;
		int sound_id;
		RageTimer start_time;
		float volume;
		CircBuf<sound_block> buffer;
		int64_t frames_read, frames_written;

		int64_t frames_buffered() const { return frames_written - frames_read; }
		enum
		{
			STOPPED,	/* idle */

			/* This state is set by the decoder thread, indicating that the sound has just
			 * reached EOF.  Once the mixing thread finishes flushing buffer, it'll change
			 * to the STOPPING_FINISH state. */
			STOPPING,

			HALTING,	/* stop immediately */
			PLAYING
		} state;

	    sound();
	};

	/* List of currently playing sounds: XXX no vector */
	sound sounds[32];

	bool shutdown_decode_thread;

	static int DecodeThread_start(void *p);
	void DecodeThread();
	RageThread m_DecodeThread;

	bool GetDataForSound( sound &s );

protected:
	/* Override this to set the priority of the decoding thread, which should be above
	 * normal priority but not realtime. */
	virtual void SetupDecodingThread() { }

	/*
	 * Read mixed data.
	 *
	 * frames: buffer to read into
	 * nframes: number of frames (not samples) to read
	 * frameno: frame number at which this sound will be heard
	 * current_frameno: frame number that is currently being heard
	 *
	 * current_frameno is used for handling start timing.
	 *
	 * This function only mixes data; it will not lock any mutexes or do any file access, and
	 * is safe to call from a realtime thread.
	 */
	void Mix( int16_t *frames, int nframes, int64_t frameno, int64_t current_frameno );

public:
	virtual void Update(float delta);

	void StartMixing( RageSoundBase *snd );		/* used by RageSound */
	void StopMixing( RageSoundBase *snd );		/* used by RageSound */

	RageSound_Generic_Software();
	virtual ~RageSound_Generic_Software();
};

#endif

/*
 * Copyright (c) 2002-2004 by the person(s) listed below.  All rights reserved.
 *
 * Glenn Maynard
 */
