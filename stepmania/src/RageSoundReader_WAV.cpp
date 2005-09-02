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
	void Convert8bitTo16bit( void *pBuf, int iSamples )
	{
		/* Convert in reverse, so we can do it in-place. */
		const uint8_t *pIn = (uint8_t *) pBuf;
		int16_t *pOut = (int16_t *) pBuf;
		for( int i = iSamples-1; i >= 0; --i )
			pOut[i] = SCALE( pIn[i], 0, 255, -32768, 32767 );
	}

	/* Flip 16-bit samples if necessary.  On little-endian systems, this will
	 * optimize out. */
	void Convert16BitFromLittleEndian( int16_t *pBuf, int iSamples )
	{
		for( int i = 0; i < iSamples; ++i )
			pBuf[i] = Swap16LE( pBuf[i] );
	}
};

struct WavReader
{
	WavReader( RageFile &f, const RageSoundReader_WAV::WavData &data ):
		m_File(f), m_WavData(data) { }
	virtual ~WavReader() { }
	virtual int Read( char *buf, unsigned len ) = 0;
	virtual int GetLength() const = 0;
	virtual bool Init() = 0;
	virtual int SetPosition( int iMS ) = 0;
	CString GetError() const { return m_sError; }

protected:
	RageFile &m_File;
	const RageSoundReader_WAV::WavData &m_WavData;
	CString m_sError;
};

struct WavReaderPCM: public WavReader
{
	WavReaderPCM( RageFile &f, const RageSoundReader_WAV::WavData &data ):
		WavReader(f, data) { }

	bool Init()
	{
		if( m_WavData.m_iBitsPerSample != 8 && m_WavData.m_iBitsPerSample != 16 )
		{
			m_sError = ssprintf("Unsupported sample size %i", m_WavData.m_iBitsPerSample);
			return false;
		}

		m_File.Seek( m_WavData.m_iDataChunkPos );
		return true;
	}

	int Read( char *buf, unsigned len )
	{
		if( m_WavData.m_iBitsPerSample == 8 )
			len /= 2;

		const unsigned iBytesLeftInDataChunk = m_WavData.m_iDataChunkSize - (m_File.Tell() - m_WavData.m_iDataChunkPos);
		len = min( len, iBytesLeftInDataChunk );
		int iGot = m_File.Read( buf, len );

		switch( m_WavData.m_iBitsPerSample )
		{
		case 8:
			Convert8bitTo16bit( buf, iGot );
			iGot *= 2;
			break;
		case 16:
			Convert16BitFromLittleEndian( (int16_t *) buf, iGot/2 );
			iGot &= ~1;
			break;
		}
		return iGot;
	}

	int GetLength() const
	{
		const int iBytesPerSec = m_WavData.m_iSampleRate * m_WavData.m_iChannels * m_WavData.m_iBitsPerSample / 8;
		int64_t iMS = (int64_t(m_WavData.m_iDataChunkSize) * 1000) / iBytesPerSec;
		return (int) iMS;
	}

	int SetPosition( int iMS )
	{
		const int iBytesPerSec = m_WavData.m_iSampleRate * m_WavData.m_iChannels * m_WavData.m_iBitsPerSample / 8;
		int iByte = (int) ((int64_t(iMS) * iBytesPerSec) / 1000);
		iByte = Quantize( iByte, m_WavData.m_iChannels * m_WavData.m_iBitsPerSample / 8 );
		if( iByte > m_WavData.m_iDataChunkSize )
		{
			m_File.Seek( m_WavData.m_iDataChunkSize+m_WavData.m_iDataChunkPos );
			return 0;
		}

		m_File.Seek( iByte+m_WavData.m_iDataChunkPos );
		return int((int64_t(iByte) * 1000) / iBytesPerSec);
	}
};

struct WavReaderADPCM: public WavReader
{
public:
	vector<int16_t> m_iaCoef1, m_iaCoef2;
	int16_t m_iFramesPerBlock;
	int8_t *m_pBuffer;
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

		m_pBuffer = new int8_t[m_iFramesPerBlock*m_WavData.m_iChannels*sizeof(int16_t)];
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

		int16_t *pBuffer = (int16_t *) m_pBuffer;
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
			pBuffer[m_iBufferAvail++] = iSamp2[i];
		for( int i = 0; i < m_WavData.m_iChannels; ++i )
			pBuffer[m_iBufferAvail++] = iSamp1[i];

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
				
				pBuffer[m_iBufferAvail++] = (int16_t) iNewSample;

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

	int Read( char *buf, unsigned len )
	{
		unsigned got = 0;
		while( got < len )
		{
			if( m_iBufferUsed == m_iBufferAvail )
			{
				if( !DecodeADPCMBlock() )
					return -1;
			}
			if( m_iBufferAvail == 0 )
				break; /* EOF */

			int iBytesToCopy = min( m_iBufferAvail-m_iBufferUsed, (int) (len-got) );
			memcpy( buf+got, m_pBuffer+m_iBufferUsed, iBytesToCopy );
			m_iBufferUsed += iBytesToCopy;
			got += iBytesToCopy;
		}
		
		return got;
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

	int SetPosition( int iMS )
	{
		const int iFrame = int((int64_t(iMS) * m_WavData.m_iSampleRate) / 1000);
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

		return iMS;
	}
};

CString ReadString( RageFile &f, int iSize, CString &sError )
{
	if( sError.size() != 0 )
		return NULL;

	CString sBuf;
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

SoundReader_FileReader::OpenResult RageSoundReader_WAV::Open( CString filename_ )
{
	m_sFilename = filename_;

	CString sError;

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

	int16_t iFormatTag = 0;

	bool bGotFormatChunk = false, bGotDataChunk = false;
	while( !bGotFormatChunk || !bGotDataChunk )
	{
		CString ChunkID = ReadString( m_File, 4, sError );
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

			iFormatTag = FileReading::read_16_le( m_File, sError );
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

	switch( iFormatTag )
	{
	case 1: // PCM
		m_pImpl = new WavReaderPCM( m_File, m_WavData );
		break;
	case 2: // ADPCM
		m_pImpl = new WavReaderADPCM( m_File, m_WavData );
		break;
	case 85: // MP3
		/* Return unknown, so other decoders will be tried.  MAD can read MP3s embedded in WAVs. */
		return OPEN_UNKNOWN_FILE_FORMAT;
	default:
		FATAL_ERROR( ssprintf( "Unsupported data format %i", iFormatTag) );
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

int RageSoundReader_WAV::SetPosition( int ms )
{
	ASSERT( m_pImpl != NULL );
	return m_pImpl->SetPosition( ms );
}

int RageSoundReader_WAV::Read( char *buf, unsigned len )
{
	ASSERT( m_pImpl != NULL );
	return m_pImpl->Read( buf, len );
}

RageSoundReader_WAV::RageSoundReader_WAV()
{
	m_pImpl = NULL;
}

RageSoundReader_WAV::~RageSoundReader_WAV()
{
	delete m_pImpl;
}

SoundReader *RageSoundReader_WAV::Copy() const
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
