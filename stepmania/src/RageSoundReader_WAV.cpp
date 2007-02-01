/*
 * Straightforward WAV reading.  This only supports 8-bit and 16-bit PCM,
 * 4-bit ADPCM with one or two channels.  No other decompressors are planned:
 * this format is only useful for fast uncompressed audio, and ADPCM is only
 * supported to retain compatibility.
 *
 * http://www.saettler.com/RIFFNEW/RIFFNEW.htm
 * http://www.kk.iij4u.or.jp/~kondo/wave/wavecomp.htm
 * http://www.sonicspot.com/guide/wavefiles.html
 */

#include "global.h"
#include "RageSoundReader_WAV.h"
#include "RageUtil.h"
#include "RageLog.h"
#include "RageFile.h"

namespace
{
	/* pBuf contains iSamples 8-bit samples; convert to 16-bit.  pBuf must
	 * have enough storage to hold the resulting data. */
	void Convert8bitToFloat( void *pBuf, int iSamples )
	{
		/* Convert in reverse, so we can do it in-place. */
		const uint8_t *pIn = (uint8_t *) pBuf;
		float *pOut = (float *) pBuf;
		for( int i = iSamples-1; i >= 0; --i )
		{
			int iSample = pIn[i];
			iSample -= 128; /* 0..255 -> -128..127 */
			pOut[i] = iSample / 128.0f;
		}
	}

	/* Flip 16-bit samples if necessary.  On little-endian systems, this will
	 * optimize out. */
	void ConvertLittleEndian16BitToFloat( void *pBuf, int iSamples )
	{
		/* Convert in reverse, so we can do it in-place. */
		const int16_t *pIn = (int16_t *) pBuf;
		float *pOut = (float *) pBuf;
		for( int i = iSamples-1; i >= 0; --i )
		{
                        int16_t iSample = Swap16LE( pIn[i] );
			pOut[i] = iSample / 32768.0f;
		}
	}

	void ConvertLittleEndian24BitToFloat( void *pBuf, int iSamples )
	{
		/* Convert in reverse, so we can do it in-place. */
		const unsigned char *pIn = (unsigned char *) pBuf;
		float *pOut = (float *) pBuf;
		pIn += iSamples * 3;
		for( int i = iSamples-1; i >= 0; --i )
		{
			pIn -= 3;

			int32_t iSample =
				(int(pIn[0]) << 0) |
				(int(pIn[1]) << 8) |
				(int(pIn[2]) << 16);

			/* Sign-extend 24-bit to 32-bit: */
			if( iSample & 0x800000 )
				iSample |= 0xFF000000;

			pOut[i] = iSample / 8388608.0f;
		}
	}

	void ConvertLittleEndian32BitToFloat( void *pBuf, int iSamples )
	{
		/* Convert in reverse, so we can do it in-place. */
		const int32_t *pIn = (int32_t *) pBuf;
		float *pOut = (float *) pBuf;
		for( int i = iSamples-1; i >= 0; --i )
		{
                        int32_t iSample = Swap32LE( pIn[i] );
			pOut[i] = iSample / 2147483648.0f;
		}
	}
};

struct WavReader
{
	WavReader( RageFile &f, const RageSoundReader_WAV::WavData &data ):
		m_File(f), m_WavData(data) { }
	virtual ~WavReader() { }
	virtual int Read( float *pBuf, int iFrames ) = 0;
	virtual int GetLength() const = 0;
	virtual bool Init() = 0;
	virtual int SetPosition( int iFrame ) = 0;
	virtual int GetNextSourceFrame() const = 0;
	RString GetError() const { return m_sError; }

protected:
	RageFile &m_File;
	const RageSoundReader_WAV::WavData &m_WavData;
	RString m_sError;
};

struct WavReaderPCM: public WavReader
{
	WavReaderPCM( RageFile &f, const RageSoundReader_WAV::WavData &data ):
		WavReader(f, data) { }

	bool Init()
	{
		if( QuantizeUp(m_WavData.m_iBitsPerSample, 8) < 8 ||
		    QuantizeUp(m_WavData.m_iBitsPerSample, 8) > 32 )
		{
			m_sError = ssprintf("Unsupported sample size %i", m_WavData.m_iBitsPerSample);
			return false;
		}

		if( m_WavData.m_iFormatTag == 3 && m_WavData.m_iBitsPerSample != 32 )
		{
			m_sError = ssprintf( "Unsupported float sample size %i", m_WavData.m_iBitsPerSample );
			return false;
		}

		m_File.Seek( m_WavData.m_iDataChunkPos );
		return true;
	}

	int Read( float *buf, int iFrames )
	{
		int iBytesPerSample = QuantizeUp(m_WavData.m_iBitsPerSample, 8) / 8;
		int len = iFrames * m_WavData.m_iChannels;
		len *= iBytesPerSample;

		const int iBytesLeftInDataChunk = m_WavData.m_iDataChunkSize - (m_File.Tell() - m_WavData.m_iDataChunkPos);
		if( !iBytesLeftInDataChunk )
			return RageSoundReader::END_OF_FILE;

		len = min( len, iBytesLeftInDataChunk );
		int iGot = m_File.Read( buf, len );

		int iGotSamples = iGot / iBytesPerSample;
		if( m_WavData.m_iFormatTag == 1 )
		{
			switch( iBytesPerSample )
			{
			case 1:
				Convert8bitToFloat( buf, iGotSamples );
				break;
			case 2:
				ConvertLittleEndian16BitToFloat( buf, iGotSamples );
				break;
			case 3:
				ConvertLittleEndian24BitToFloat( buf, iGotSamples );
				break;
			case 4:
				ConvertLittleEndian32BitToFloat( buf, iGotSamples );
				/* otherwise 3; already a float */
				break;
			}
		}
		return iGotSamples / m_WavData.m_iChannels;
	}

	int GetLength() const
	{
		const int iBytesPerSec = m_WavData.m_iSampleRate * m_WavData.m_iChannels * m_WavData.m_iBitsPerSample / 8;
		int64_t iMS = (int64_t(m_WavData.m_iDataChunkSize) * 1000) / iBytesPerSec;
		return (int) iMS;
	}

	int SetPosition( int iFrame )
	{
		int iByte = (int) (int64_t(iFrame) * m_WavData.m_iChannels * m_WavData.m_iBitsPerSample / 8);
		if( iByte > m_WavData.m_iDataChunkSize )
		{
			m_File.Seek( m_WavData.m_iDataChunkSize+m_WavData.m_iDataChunkPos );
			return 0;
		}

		m_File.Seek( iByte+m_WavData.m_iDataChunkPos );
		return 1;
	}

	// XXX: untested
	int GetNextSourceFrame() const
	{
		int iByte = m_File.Tell() - m_WavData.m_iDataChunkPos;
		int iFrame = iByte / (m_WavData.m_iChannels * m_WavData.m_iBitsPerSample / 8);
		return iFrame;
	}
};

struct WavReaderADPCM: public WavReader
{
public:
	vector<int16_t> m_iaCoef1, m_iaCoef2;
	int16_t m_iFramesPerBlock;
	float *m_pBuffer;
	int m_iBufferAvail, m_iBufferUsed;
	
	WavReaderADPCM( RageFile &f, const RageSoundReader_WAV::WavData &data ):
		WavReader(f, data)
	{
		m_pBuffer = NULL;
	}

	virtual ~WavReaderADPCM()
	{
		delete[] m_pBuffer;
	}

	bool Init()
	{
		if( m_WavData.m_iBitsPerSample != 4 )
		{
			m_sError = ssprintf( "Unsupported ADPCM sample size %i", m_WavData.m_iBitsPerSample );
			return false;
		}

		m_File.Seek( m_WavData.m_iExtraFmtPos );

		m_iFramesPerBlock = FileReading::read_16_le( m_File, m_sError );
		int16_t iNumCoef = FileReading::read_16_le( m_File, m_sError );
		m_iaCoef1.resize( iNumCoef );
		m_iaCoef2.resize( iNumCoef );
		for( int i = 0; i < iNumCoef; ++i )
		{
			m_iaCoef1[i] = FileReading::read_16_le( m_File, m_sError );
			m_iaCoef2[i] = FileReading::read_16_le( m_File, m_sError );
		}

		if( m_sError.size() != 0 )
			return false;

		m_pBuffer = new float[m_iFramesPerBlock*m_WavData.m_iChannels];
		m_iBufferAvail = m_iBufferUsed = 0;

		m_File.Seek( m_WavData.m_iDataChunkPos );
		return true;
	}

	void SetEOF()
	{
		m_iBufferUsed = m_iBufferAvail = 0;
		m_File.Seek( m_WavData.m_iDataChunkSize+m_WavData.m_iDataChunkPos );
	}

	/* Return false on error, true on success (even if we hit EOF). */
	bool DecodeADPCMBlock()
	{
		ASSERT_M( m_iBufferUsed == m_iBufferAvail, ssprintf("%i", m_iBufferUsed) );

		m_iBufferUsed = m_iBufferAvail = 0;

		int8_t iPredictor[2];
		int16_t iDelta[2], iSamp1[2], iSamp2[2];
		for( int i = 0; i < m_WavData.m_iChannels; ++i )
			iPredictor[i] = FileReading::read_8( m_File, m_sError );
		for( int i = 0; i < m_WavData.m_iChannels; ++i )
			iDelta[i] = FileReading::read_16_le( m_File, m_sError );
		for( int i = 0; i < m_WavData.m_iChannels; ++i )
			iSamp1[i] = FileReading::read_16_le( m_File, m_sError );
		for( int i = 0; i < m_WavData.m_iChannels; ++i )
			iSamp2[i] = FileReading::read_16_le( m_File, m_sError );

		if( m_sError.size() != 0 )
			return false;

		if( m_File.Tell() >= m_WavData.m_iDataChunkSize+m_WavData.m_iDataChunkPos || m_File.AtEOF() )
			return true; /* past the data chunk */

		float *pBuffer = m_pBuffer;
		int iCoef1[2], iCoef2[2];
		for( int i = 0; i < m_WavData.m_iChannels; ++i )
		{
			if( iPredictor[i] >= (int) m_iaCoef1.size() )
			{
				LOG->Trace( "%s: predictor out of range", m_File.GetPath().c_str() );

				/* XXX: silence this block? */
				iPredictor[i] = 0;
			}

			iCoef1[i] = m_iaCoef1[iPredictor[i]];
			iCoef2[i] = m_iaCoef2[iPredictor[i]];
		}

		/* We've read the block header; read the rest.  Don't read past the end of the data chunk. */
		int iMaxSize = min( (int) m_WavData.m_iBlockAlign - 7 * m_WavData.m_iChannels, (m_WavData.m_iDataChunkSize+m_WavData.m_iDataChunkPos) - m_File.Tell() );

		char *pBuf = (char *) alloca( iMaxSize );
		ASSERT( pBuf != NULL );

		int iBlockSize = m_File.Read( pBuf, iMaxSize );
		if( iBlockSize == 0 )
			return true;
		if( iBlockSize == -1 )
		{
			m_sError = m_File.GetError();
			return false;
		}

		for( int i = 0; i < m_WavData.m_iChannels; ++i )
			pBuffer[m_iBufferAvail++] = iSamp2[i] / 32768.0f;
		for( int i = 0; i < m_WavData.m_iChannels; ++i )
			pBuffer[m_iBufferAvail++] = iSamp1[i] / 32768.0f;

		int8_t iBuf = 0, iBufSize = 0;

		bool bDone = false;
		for( int i = 2; !bDone && i < m_iFramesPerBlock; ++i )
		{
			for( int c = 0; !bDone && c < m_WavData.m_iChannels; ++c )
			{
				if( iBufSize == 0 )
				{
					if( !iBlockSize )
					{
						bDone = true;
						continue;
					}
					iBuf = *pBuf;
					++pBuf;
					--iBlockSize;
					iBufSize = 2;
				}

				/* Store the nibble in signed char, so we get an arithmetic shift. */
				int iErrorDelta = iBuf >> 4;
				iBuf <<= 4;
				--iBufSize;

				int32_t iPredSample = (iSamp1[c] * iCoef1[c] + iSamp2[c] * iCoef2[c]) / (1<<8);
				int32_t iNewSample = iPredSample + (iDelta[c] * iErrorDelta);
				iNewSample = clamp( iNewSample, -32768, 32767 );
				
				pBuffer[m_iBufferAvail++] = iNewSample / 32768.0f;

				static const int aAdaptionTable[] = {
					768, 614, 512, 409, 307, 230, 230, 230,
					230, 230, 230, 230, 307, 409, 512, 614
				};
				iDelta[c] = int16_t( (iDelta[c] * aAdaptionTable[iErrorDelta+8]) / (1<<8) );
				iDelta[c] = max( (int16_t) 16, iDelta[c] );
				
				iSamp2[c] = iSamp1[c];
				iSamp1[c] = (int16_t) iNewSample;
			}
		}
		
		m_iBufferAvail *= sizeof(int16_t);
		return true;
	}

	int Read( float *buf, int iFrames )
	{
		int iSamplesPerFrame = m_WavData.m_iChannels;
		int iBytesPerFrame = iSamplesPerFrame * sizeof(float);
		int iGotFrames = 0;
		while( iGotFrames < (int) iFrames )
		{
			if( m_iBufferUsed == m_iBufferAvail )
			{
				if( !DecodeADPCMBlock() )
					return RageSoundReader::ERROR;
			}
			if( m_iBufferAvail == 0 )
			{
				if( !iGotFrames )
					return RageSoundReader::END_OF_FILE;
				else
					return iGotFrames;
			}

			int iFramesToCopy = (m_iBufferAvail-m_iBufferUsed) / iBytesPerFrame;
			iFramesToCopy = min( iFramesToCopy, (int) (iFrames-iGotFrames) );
			int iSamplesToCopy = iFramesToCopy * iSamplesPerFrame;
			int iBytesToCopy = iSamplesToCopy * sizeof(int16_t);
			memcpy( buf, m_pBuffer+m_iBufferUsed, iBytesToCopy );
			m_iBufferUsed += iBytesToCopy;
			iGotFrames += iFramesToCopy;
			buf += iSamplesToCopy;
		}
		
		return iGotFrames;
	}

	int GetLength() const
	{
		const int iNumWholeBlocks = m_WavData.m_iDataChunkSize / m_WavData.m_iBlockAlign;
		const int iExtraBytes = m_WavData.m_iDataChunkSize - (iNumWholeBlocks*m_WavData.m_iBlockAlign);
		
		int iFrames = iNumWholeBlocks * m_iFramesPerBlock;

		const int iBlockHeaderSize = 7 * m_WavData.m_iChannels;
		if( iExtraBytes > iBlockHeaderSize )
		{
			const int iExtraADPCMNibbles = max( 0, iExtraBytes-iBlockHeaderSize )*2;
			const int iExtraADPCMFrames = iExtraADPCMNibbles/m_WavData.m_iChannels;
			
			iFrames += 2+iExtraADPCMFrames;
		}

		int iMS = int((int64_t(iFrames)*1000)/m_WavData.m_iSampleRate);
		return iMS;
	}

	int SetPosition( int iFrame )
	{
		const int iBlock = iFrame / m_iFramesPerBlock;

		m_iBufferUsed = m_iBufferAvail = 0;

		{
			const int iByte = iBlock*m_WavData.m_iBlockAlign;
			if( iByte > m_WavData.m_iDataChunkSize )
			{
				/* Past EOF. */
				SetEOF();
				return 0;
			}
			m_File.Seek( iByte+m_WavData.m_iDataChunkPos );
		}

		if( !DecodeADPCMBlock() )
			return -1;

		const int iRemainingFrames = iFrame - iBlock*m_iFramesPerBlock;
		m_iBufferUsed = iRemainingFrames * m_WavData.m_iChannels * sizeof(int16_t);
		if( m_iBufferUsed > m_iBufferAvail )
		{
			SetEOF();
			return 0;
		}

		return 1;
	}

	// XXX: untested
	int GetNextSourceFrame() const
	{
		int iByte = m_File.Tell() - m_WavData.m_iDataChunkPos;
		int iBlock = iByte / m_WavData.m_iBlockAlign;
		int iFrame = iBlock * m_iFramesPerBlock;

		int iBufferRemainingBytes = m_iBufferAvail - m_iBufferUsed;
		int iBufferRemainingFrames = iBufferRemainingBytes / (m_WavData.m_iChannels * sizeof(int16_t));
		iFrame -= iBufferRemainingFrames;

		return iFrame;
	}
};

RString ReadString( RageFile &f, int iSize, RString &sError )
{
	if( sError.size() != 0 )
		return RString();

	RString sBuf;
	char *pBuf = sBuf.GetBuffer( iSize );
	FileReading::ReadBytes( f, pBuf, iSize, sError );
	sBuf.ReleaseBuffer( iSize );
	return sBuf;
}

#define FATAL_ERROR(s) \
{ \
	if( sError.size() == 0 ) sError = (s); \
	SetError( sError ); \
	return OPEN_FATAL_ERROR; \
}

RageSoundReader_FileReader::OpenResult RageSoundReader_WAV::Open( RString filename_ )
{
	m_sFilename = filename_;

	RString sError;

	if( !m_File.Open( m_sFilename ) )
		FATAL_ERROR( ssprintf("wav: opening \"%s\" failed: %s", m_sFilename.c_str(), m_File.GetError().c_str()) );

	/* RIFF header: */
	if( ReadString( m_File, 4, sError ) != "RIFF" )
	{
		SetError( "Not a WAV file" );
		return OPEN_UNKNOWN_FILE_FORMAT;
	}

	FileReading::read_32_le( m_File, sError ); /* file size */
	if( ReadString( m_File, 4, sError ) != "WAVE" )
	{
		SetError( "Not a WAV file" );
		return OPEN_UNKNOWN_FILE_FORMAT;
	}

	bool bGotFormatChunk = false, bGotDataChunk = false;
	while( !bGotFormatChunk || !bGotDataChunk )
	{
		RString ChunkID = ReadString( m_File, 4, sError );
		int32_t iChunkSize = FileReading::read_32_le( m_File, sError );

		if( sError.size() != 0 )
		{
			SetError( sError );
			return OPEN_FATAL_ERROR;
		}

		int iNextChunk = m_File.Tell() + iChunkSize;
		/* Chunks are always word-aligned: */
		iNextChunk = (iNextChunk+1)&~1;

		if( ChunkID == "fmt " )
		{
			if( bGotFormatChunk )
				LOG->Warn( "File %s has more than one fmt chunk", m_File.GetPath().c_str() );

			m_WavData.m_iFormatTag = FileReading::read_16_le( m_File, sError );
			m_WavData.m_iChannels = FileReading::read_16_le( m_File, sError );
			m_WavData.m_iSampleRate = FileReading::read_32_le( m_File, sError );
			FileReading::read_32_le( m_File, sError ); /* BytesPerSec */
			m_WavData.m_iBlockAlign = FileReading::read_16_le( m_File, sError );
			m_WavData.m_iBitsPerSample = FileReading::read_16_le( m_File, sError );
			m_WavData.m_iExtraFmtBytes = FileReading::read_16_le( m_File, sError );

			if( m_WavData.m_iChannels < 1 || m_WavData.m_iChannels > 2 )
				FATAL_ERROR( ssprintf( "Unsupported channel count: %i", m_WavData.m_iChannels) );

			if( m_WavData.m_iSampleRate < 4000 || m_WavData.m_iSampleRate > 100000 ) /* unlikely */
				FATAL_ERROR( ssprintf( "Invalid sample rate: %i", m_WavData.m_iSampleRate) );

			m_WavData.m_iExtraFmtPos = m_File.Tell();

			bGotFormatChunk = true;
		}

		if( ChunkID == "data" )
		{
			m_WavData.m_iDataChunkPos = m_File.Tell();
			m_WavData.m_iDataChunkSize = iChunkSize;

			int iFileSize = m_File.GetFileSize();
			int iMaxSize = iFileSize-m_WavData.m_iDataChunkPos;
			if( iMaxSize < m_WavData.m_iDataChunkSize )
			{
				LOG->Warn( "File %s truncated (%i < data chunk size %i)", m_File.GetPath().c_str(),
					iMaxSize, m_WavData.m_iDataChunkSize );

				m_WavData.m_iDataChunkSize = iMaxSize;
			}

			bGotDataChunk = true;
		}
		m_File.Seek( iNextChunk );
	}

	if( sError.size() != 0 )
	{
		SetError( sError );
		return OPEN_FATAL_ERROR;
	}

	switch( m_WavData.m_iFormatTag )
	{
	case 1: // PCM
	case 3: // FLOAT
		m_pImpl = new WavReaderPCM( m_File, m_WavData );
		break;
	case 2: // ADPCM
		m_pImpl = new WavReaderADPCM( m_File, m_WavData );
		break;
	case 85: // MP3
		/* Return unknown, so other decoders will be tried.  MAD can read MP3s embedded in WAVs. */
		return OPEN_UNKNOWN_FILE_FORMAT;
	default:
		FATAL_ERROR( ssprintf( "Unsupported data format %i", m_WavData.m_iFormatTag) );
	}

	if( !m_pImpl->Init() )
	{
		SetError( m_pImpl->GetError() );
		return OPEN_FATAL_ERROR;
	}

	return OPEN_OK;
}

int RageSoundReader_WAV::GetLength() const
{
	ASSERT( m_pImpl != NULL );
	return m_pImpl->GetLength();
}

int RageSoundReader_WAV::SetPosition( int iFrame )
{
	ASSERT( m_pImpl != NULL );
	return m_pImpl->SetPosition( iFrame );
}

int RageSoundReader_WAV::GetNextSourceFrame() const
{
	ASSERT( m_pImpl != NULL );
	return m_pImpl->GetNextSourceFrame();
}

int RageSoundReader_WAV::Read( float *pBuf, int iFrames )
{
	ASSERT( m_pImpl != NULL );
	return m_pImpl->Read( pBuf, iFrames );
}

RageSoundReader_WAV::RageSoundReader_WAV()
{
	m_pImpl = NULL;
}

RageSoundReader_WAV::~RageSoundReader_WAV()
{
	delete m_pImpl;
}

RageSoundReader_WAV *RageSoundReader_WAV::Copy() const
{
	RageSoundReader_WAV *ret = new RageSoundReader_WAV;
	ret->Open( m_sFilename );
	return ret;
}

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
