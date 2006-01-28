/* RageSoundReader_Resample_Fast - interface for RageSoundResampler. */

#ifndef RAGE_SOUND_READER_RESAMPLE_FAST_H
#define RAGE_SOUND_READER_RESAMPLE_FAST_H

#include "RageSoundReader.h"
#include "RageSoundReader_Resample.h"

/* This class changes the sampling rate of a sound. */
class RageSoundReader_Resample_Fast: public RageSoundReader_Resample
{
public:
	/* We own pSource. */
	void Open( SoundReader *pSource );
	int GetLength() const;
	int GetLength_Fast() const;
	int SetPosition_Accurate( int ms );
	int SetPosition_Fast( int ms );
	int Read( char *pBuf, unsigned iSize );
	RageSoundReader_Resample_Fast();
	virtual ~RageSoundReader_Resample_Fast();
	SoundReader *Copy() const;

	/* Change the actual sample rate of a sound. */
	void SetSampleRate( int hz );

	int GetSampleRate() const { return m_iOutputRate; }
	unsigned GetNumChannels() const { return m_pSource->GetNumChannels(); }
	bool IsStreamingFromDisk() const { return m_pSource->IsStreamingFromDisk(); }

private:
	void Reset();
	bool FillBuf();

	/* Write data to be converted. */
	void WriteSamples( const void *data, int bytes );

	SoundReader *m_pSource;

	int m_iInputRate;
	int m_iOutputRate;
	int m_iChannels;

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

