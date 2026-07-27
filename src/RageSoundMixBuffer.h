/* RageSoundMixBuffer - Simple audio mixing. */

#ifndef RAGE_SOUND_MIX_BUFFER_H
#define RAGE_SOUND_MIX_BUFFER_H

#include <cstdint>

class RageSoundMixBuffer
{
public:
	RageSoundMixBuffer();
	~RageSoundMixBuffer();

	void write( const float *pBuf, unsigned iSize, int iSourceStride = 1, int iDestStride = 1 ) noexcept;
	void Extend( unsigned iSamples ) noexcept;
	void read_deinterlace( float **pBufs, int channels ) noexcept;
	float *read() { return m_pMixbuf; }
	unsigned size() const { return m_iBufUsed; }

	void SetWriteOffset(int iOffset) noexcept;
	void read(int16_t *pBuf) noexcept;
	void read(float *pBuf) noexcept;

private:
	float *m_pMixbuf;
	int_fast64_t m_iBufSize; // actual allocated samples
	int_fast64_t m_iBufUsed; // used samples
	int_fast32_t m_iOffset;
};

inline void RageSoundMixBuffer::SetWriteOffset(int iOffset) noexcept
{
	m_iOffset = iOffset;
}

inline void RageSoundMixBuffer::read(int16_t *pBuf) noexcept
{
	constexpr int16_t MAX_INT16 = 32767;
	for (unsigned iPos = 0; iPos < m_iBufUsed; ++iPos)
	{
		float iOut = m_pMixbuf[iPos];
		iOut = std::max(-1.0f, std::min(iOut, 1.0f));
		pBuf[iPos] = static_cast<int16_t>((iOut * MAX_INT16) + 0.5f);
	}
	m_iBufUsed = 0;
}

inline void RageSoundMixBuffer::read(float *pBuf) noexcept
{
	std::memcpy(pBuf, m_pMixbuf, m_iBufUsed * sizeof(float));
	m_iBufUsed = 0;
}

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