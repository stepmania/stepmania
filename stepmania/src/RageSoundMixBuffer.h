/* RageSoundMixBuffer - Simple audio mixing. */

#ifndef RAGE_SOUND_MIX_BUFFER_H
#define RAGE_SOUND_MIX_BUFFER_H

class RageSoundMixBuffer
{
public:
	RageSoundMixBuffer();
	~RageSoundMixBuffer();

	/* Mix the given buffer of samples. */
	void write( const int16_t *buf, unsigned size );

	/* Extend the buffer as if write() was called with a buffer of silence. */
	void Extend( unsigned iSamples );

	void read( int16_t *pBuf );
	void read( float *pBuf );
	unsigned size() const { return m_iBufUsed; }
	void SetVolume( float f );
	void SetWriteOffset( int iOffset );

private:
	int32_t *m_pMixbuf;
	unsigned m_iBufSize; /* actual allocated samples */
	unsigned m_iBufUsed; /* used samples */
	int m_iVolumeFactor; /* vol * 256 */
	int m_iOffset;
};

#endif

/*
 * Copyright (c) 2002-2004 Glenn Maynard
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
