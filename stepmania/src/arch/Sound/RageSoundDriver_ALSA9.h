#ifndef RAGE_SOUND_ALSA9_H
#define RAGE_SOUND_ALSA9_H

#include "RageSound.h"
#include "RageThreads.h"
#include "RageSoundDriver.h"

#include "ALSA9Helpers.h"

class RageSound_ALSA9: public RageSoundDriver
{
private:
	struct stream
	{
		/* Actual audio stream: */
		Alsa9Buf *pcm;

		/* Sound object that's playing on this stream, or NULL if this
		 * channel is available: */
		RageSoundBase *snd;
		RageTimer start_time;

		enum {
			INACTIVE,
			PLAYING,
			STOPPING
		} state;

		int flush_pos; /* state == STOPPING only */

		bool GetData( bool init );

		stream() { pcm = NULL; snd = NULL; state=INACTIVE; }
		~stream();
	};
	friend struct stream;

	/* Pool of available streams. */
	vector<stream *> stream_pool;

	bool shutdown;


	static int MixerThread_start(void *p);
	void MixerThread();
	RageThread MixingThread;

	void GetData();

public:
	/* virtuals: */
	void StartMixing(RageSoundBase *snd);
	void StopMixing(RageSoundBase *snd);
	int GetPosition(const RageSoundBase *snd) const;
	int GetSampleRate( int rate ) const;

	void Update(float delta);

	RageSound_ALSA9();
	~RageSound_ALSA9();
};

#endif

/*
 * Copyright (c) 2003 by the person(s) listed below.  All rights reserved.
 *
 * Glenn Maynard
 * 
 * 2003-02   Modified to fake playing sound   Aaron VonderHaar
 * 
 */
