#ifndef DSOUND_HELPERS
#define DSOUND_HELPERS 1

#include "SDL.h"

struct IDirectSound8;
struct IDirectSoundBuffer8;

class DSound
{
	IDirectSound8 *ds8;

public:
	IDirectSound8 *GetDS8() const { return ds8; }
	bool IsEmulated() const;

	DSound();
	~DSound();
};

class DSoundBuf
{
	IDirectSoundBuffer8 *buf;

	int channels, samplerate, samplebits, buffersize;

	int buffersize_frames() const { return buffersize / samplesize(); }
	int samplesize() const { return channels*samplebits/8; }

	int write_cursor, last_cursor_pos; /* frames */
	mutable int LastPosition;

	bool buffer_locked;
	char *locked_buf;
	int locked_len;

//	int GetPos();
public:
	enum hw { HW_HARDWARE, HW_SOFTWARE, HW_DONT_CARE };
	DSoundBuf(DSound &ds, hw hardware, 
		int channels, int samplerate, int samplebits, int buffersize);

	bool get_output_buf(char **buffer, unsigned *bufsiz, int *play_pos, int chunksize);
	void release_output_buf(char *buffer, unsigned bufsiz);

	void Reset();
	void Play();
	void Stop();

	~DSoundBuf();
	int GetPosition() const;
	int GetMaxPosition() const { return last_cursor_pos; }
};

#endif

/*
 * Copyright (c) 2002 by the person(s) listed below.  All rights reserved.
 *
 * Glenn Maynard
 */
