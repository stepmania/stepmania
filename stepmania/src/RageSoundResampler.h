/* RageSoundResampler - Very low quality linear-interpolating audio resampler. */

#ifndef RAGE_SOUND_RESAMPLER_H
#define RAGE_SOUND_RESAMPLER_H

class RageSoundResampler
{
public:
	RageSoundResampler();

	/* Configuration: */
	void SetChannels( int c ) { ASSERT( c < MAX_CHANNELS ); m_iChannels = c; }
	void SetInputSampleRate(int hz) { m_iInputRate = hz; }
	void SetOutputSampleRate(int hz) { m_iOutputRate = hz; }

	/* Write data to be converted. */
	void write(const void *data, int bytes);

	/* Indicate that there is no more data. */
	void eof();

	void reset();

	/* Return the number of bytes currently readable. */
	unsigned avail() const { return m_OutBuf.size() / 2; }

	/* Read converted data.  Returns the number of bytes filled.
	 * If eof() has been called and the output is completely
	 * flushed, returns -1. */
	int read(void *data, unsigned bytes);

private:
	int m_iInputRate, m_iOutputRate, m_iChannels;

	enum { MAX_CHANNELS = 15 };
	int16_t m_iPrevSample[MAX_CHANNELS];

	vector<int16_t> m_OutBuf;
	bool m_bAtEof;
	int m_iPos;
};

#endif

/*
 * Copyright (c) 2003-2005 Glenn Maynard
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
