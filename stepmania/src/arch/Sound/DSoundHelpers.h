#ifndef DSOUND_HELPERS
#define DSOUND_HELPERS 1

#include "SDL.h"
#if defined(_WINDOWS)
#include "windows.h"
#include "wtypes.h"
#endif

struct IDirectSound;
struct IDirectSoundBuffer;

class DSound
{
	IDirectSound *ds;
	static BOOL CALLBACK EnumCallback( LPGUID lpGuid, LPCSTR lpcstrDescription, LPCSTR  lpcstrModule, LPVOID lpContext);

	void SetPrimaryBufferMode();
    
public:
	IDirectSound *GetDS() const { return ds; }
	bool IsEmulated() const;

	DSound();
	~DSound();
};

class DSoundBuf
{
	IDirectSoundBuffer *buf;

	int channels, samplerate, samplebits, writeahead;

	int buffersize;
	
	int buffersize_frames() const { return buffersize / bytes_per_frame(); }
	int bytes_per_frame() const { return channels*samplebits/8; }

	int write_cursor, buffer_bytes_filled; /* bytes */
	int64_t last_cursor_pos; /* frames */
	mutable int64_t LastPosition;

	bool buffer_locked;
	char *locked_buf;
	int locked_len;

public:
	enum hw { HW_HARDWARE, HW_SOFTWARE, HW_DONT_CARE };

	/* If samplerate is DYNAMIC_SAMPLERATE, then call SetSampleRate before
	 * you use the sample. */
	enum { DYNAMIC_SAMPLERATE = -1 };
	DSoundBuf(DSound &ds, hw hardware, 
		int channels, int samplerate, int samplebits, int writeahead);

	bool get_output_buf(char **buffer, unsigned *bufsiz, int chunksize);
	void release_output_buf(char *buffer, unsigned bufsiz);

	void Play();
	void Stop();
	void SetVolume(float vol);
	void SetSampleRate(int hz);
	int GetSampleRate() { return samplerate; }

	~DSoundBuf();
	int64_t GetPosition() const;
	int64_t GetOutputPosition() const { return last_cursor_pos; }
};

#endif

/*
 * Copyright (c) 2002 by the person(s) listed below.  All rights reserved.
 *
 * Glenn Maynard
 */
