#ifndef ALSA9_HELPERS_H
#define ALSA9_HELPERS_H

#define ALSA_PCM_NEW_HW_PARAMS_API
#define ALSA_PCM_NEW_SW_PARAMS_API
#include <alsa/asoundlib.h>

class Alsa9Buf
{
private:
	int channels, samplebits;
	unsigned samplerate;
	int buffersize;
	int64_t last_cursor_pos;

	snd_pcm_uframes_t preferred_writeahead, preferred_chunksize;
	snd_pcm_uframes_t writeahead, chunksize;

	snd_pcm_t *pcm;

	bool Recover( int r );
	bool SetHWParams();
	bool SetSWParams();

	static void ErrorHandler(const char *file, int line, const char *function, int err, const char *fmt, ...);
		
public:
	static void InitializeErrorHandler();
	static void GetSoundCardDebugInfo();
	static RString GetHardwareID( RString name="" );
		
	Alsa9Buf();
	RString Init( int channels,
			int iWriteahead,
			int iChunkSize,
			int iSampleRate );
	~Alsa9Buf();
	
	int GetNumFramesToFill();
	bool WaitUntilFramesCanBeFilled( int timeout_ms );
	void Write( const int16_t *buffer, int frames );
	
	void Play();
	void Stop();
	void SetVolume(float vol);
	int GetSampleRate() const { return samplerate; }

	int64_t GetPosition() const;
	int64_t GetPlayPos() const { return last_cursor_pos; }
};
#endif

/*
 * (c) 2002-2004 Glenn Maynard, Aaron VonderHaar
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
