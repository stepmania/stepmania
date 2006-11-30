/* RageSoundReader_WAV - WAV reader. */

#ifndef RAGE_SOUND_READER_WAV_H
#define RAGE_SOUND_READER_WAV_H

#include "RageSoundReader_FileReader.h"
#include "RageFile.h"

struct WavReader;
class RageSoundReader_WAV: public RageSoundReader_FileReader
{
public:
	OpenResult Open( RString m_sFilename );
	void Close();
	int GetLength() const;
	int GetLength_Fast() const { return GetLength(); }
	int SetPosition_Accurate( int ms )  { return SetPosition(ms); }
	int SetPosition_Fast( int ms ) { return SetPosition(ms); }
	int Read( char *buf, unsigned len );
	int GetSampleRate() const { return m_WavData.m_iSampleRate; }
	unsigned GetNumChannels() const { return m_WavData.m_iChannels; }
	int GetNextSourceFrame() const;
	RageSoundReader_WAV();
	~RageSoundReader_WAV();
	RageSoundReader_WAV( const RageSoundReader_WAV & ); /* not defined; don't use */
	RageSoundReader_WAV *Copy() const;

	struct WavData
	{
		int32_t m_iDataChunkPos, m_iDataChunkSize, m_iExtraFmtPos, m_iSampleRate;
		int16_t m_iChannels, m_iBitsPerSample, m_iBlockAlign, m_iExtraFmtBytes;
	};

private:
	RageFile m_File;
	RString m_sFilename;
	WavData m_WavData;

	WavReader *m_pImpl;

	int SetPosition( int ms );
};

#endif

/*
 * (c) 2004 Glenn Maynard
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
