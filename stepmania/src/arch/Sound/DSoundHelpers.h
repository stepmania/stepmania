#ifndef DSOUND_HELPERS
#define DSOUND_HELPERS 1

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
	int64_t GetOutputPosition() const { return write_cursor_pos; }

private:
	int buffersize_frames() const { return buffersize / bytes_per_frame(); }
	int bytes_per_frame() const { return channels*samplebits/8; }

	void CheckWriteahead( int cursorstart, int cursorend );
	void CheckUnderrun( int cursorstart, int cursorend );

	IDirectSoundBuffer *buf;

	int channels, samplerate, samplebits, writeahead;
	int volume;

	int buffersize;
	
	int write_cursor, buffer_bytes_filled; /* bytes */
	int extra_writeahead;
	int64_t write_cursor_pos; /* frames */
	mutable int64_t LastPosition;
	bool playing;

	bool buffer_locked;
	char *locked_buf1, *locked_buf2;
	int locked_size1, locked_size2;
	char *temp_buffer;
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
