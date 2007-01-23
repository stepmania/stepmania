#include "global.h"
#include "RageFileBasic.h"
#include "RageUtil.h"

RageFileObj::RageFileObj()
{
	m_pReadBuffer = NULL;
	m_pWriteBuffer = NULL;

	ResetReadBuf();

	m_iReadBufAvail = 0;
	m_iWriteBufferPos = 0;
	m_iWriteBufferUsed = 0;
	m_bEOF = false;
	m_iFilePos = 0;
	m_bCRC32Enabled = false;
	m_iCRC32 = 0;
}

RageFileObj::RageFileObj( const RageFileObj &cpy ):
	RageFileBasic(cpy)
{
	/* If the original file has a buffer, copy it. */
	if( cpy.m_pReadBuffer != NULL )
	{
		m_pReadBuffer = new char[BSIZE];
		memcpy( m_pReadBuffer, cpy.m_pReadBuffer, BSIZE );

		int iOffsetIntoBuffer = cpy.m_pReadBuf - cpy.m_pReadBuffer;
		m_pReadBuf = m_pReadBuffer + iOffsetIntoBuffer;
	}
	else
	{
		m_pReadBuffer = NULL;
	}

	if( cpy.m_pWriteBuffer != NULL )
	{
		m_pWriteBuffer = new char[cpy.m_iWriteBufferSize];
		memcpy( m_pWriteBuffer, cpy.m_pWriteBuffer, m_iWriteBufferUsed );
	}
	else
	{
		m_pWriteBuffer = NULL;
	}

	m_iReadBufAvail = cpy.m_iReadBufAvail;
	m_bEOF = cpy.m_bEOF;
	m_iFilePos = cpy.m_iFilePos;
	m_iWriteBufferPos = cpy.m_iWriteBufferPos;
	m_iWriteBufferSize = cpy.m_iWriteBufferSize;
	m_iWriteBufferUsed = cpy.m_iWriteBufferUsed;
	m_bCRC32Enabled = cpy.m_bCRC32Enabled;
	m_iCRC32 = cpy.m_iCRC32;
}

RageFileObj::~RageFileObj()
{
	delete [] m_pReadBuffer;
	delete [] m_pWriteBuffer;
}

int RageFileObj::Seek( int iOffset )
{
	/* If we're already at the requested position, short circuit and don't flush
	 * our buffer. */
	if( iOffset == m_iFilePos )
		return m_iFilePos;

	m_bEOF = false;

	/* If we're calculating a CRC32, disable it. */
	m_bCRC32Enabled = false;

	/* Note that seeks do not flush the write buffer.  Instead, we flush lazily, on the next
	 * actual Write (or Flush).  Seek is not allowed to fail, and users should not need to
	 * flush before seeking to do proper error checking. */
	ResetReadBuf();

	int iPos = SeekInternal( iOffset );
	if( iPos != -1 )
	        m_iFilePos = iPos;
	return iPos;
}

int RageFileObj::Seek( int offset, int whence )
{
	switch( whence )
	{
	case SEEK_CUR:
		return Seek( Tell() + offset );
	case SEEK_END:
		offset += GetFileSize();
	}
	return Seek( (int) offset );
}

int RageFileObj::Read( void *pBuffer, size_t iBytes )
{
	int iRet = 0;

	while( !m_bEOF && iBytes > 0 )
	{
		if( m_pReadBuffer != NULL && m_iReadBufAvail )
		{
			/* Copy data out of the buffer first. */
			int iFromBuffer = min( (int) iBytes, m_iReadBufAvail );
			memcpy( pBuffer, m_pReadBuf, iFromBuffer );
			if( m_bCRC32Enabled )
				CRC32( m_iCRC32, pBuffer, iFromBuffer );

			iRet += iFromBuffer;
			m_iFilePos += iFromBuffer;
			iBytes -= iFromBuffer;
			m_iReadBufAvail -= iFromBuffer;
			m_pReadBuf += iFromBuffer;

			pBuffer = (char *) pBuffer + iFromBuffer;
		}

		if( !iBytes )
			break;

		ASSERT( m_iReadBufAvail == 0 );

		/* If buffering is disabled, or the block is bigger than the buffer,
		 * read the remainder of the data directly into the desteination buffer. */
		if( m_pReadBuffer == NULL || iBytes >= BSIZE )
		{
			/* We have a lot more to read, so don't waste time copying it into the
			 * buffer. */
			int iFromFile = this->ReadInternal( pBuffer, iBytes );
			if( iFromFile == -1 )
				return -1;

			if( iFromFile == 0 )
				m_bEOF = true;

			if( m_bCRC32Enabled )
				CRC32( m_iCRC32, pBuffer, iFromFile );
			iRet += iFromFile;
			m_iFilePos += iFromFile;
			return iRet;
		}

		/* If buffering is enabled, and we need more data, fill the buffer. */
		m_pReadBuf = m_pReadBuffer;
		int iGot = FillReadBuf();
		if( iGot == -1 )
			return iGot;
		if( iGot == 0 )
			m_bEOF = true;
	}

	return iRet;
}

int RageFileObj::Read( RString &sBuffer, int iBytes )
{
	sBuffer.reserve( iBytes != -1? iBytes: this->GetFileSize() );

	int iRet = 0;
	char buf[4096];
	while( iBytes == -1 || iRet < iBytes )
	{
		int ToRead = sizeof(buf);
		if( iBytes != -1 )
			ToRead  = min( ToRead, iBytes-iRet );

		const int iGot = Read( buf, ToRead );
		if( iGot == 0 )
			break;
		if( iGot == -1 )
			return -1;

		sBuffer.append( buf, iGot );
		iRet += iGot;
	}

	sBuffer.erase( sBuffer.begin()+iRet, sBuffer.end() );

	return iRet;
}

int RageFileObj::Read( void *pBuffer, size_t iBytes, int iNmemb )
{
	const int iRet = Read( pBuffer, iBytes*iNmemb );
	if( iRet == -1 )
		return -1;

	/* If we're reading 10-byte blocks, and we got 27 bytes, we have 7 extra bytes.
	 * Seek back.  XXX: seeking is very slow for eg. deflated ZIPs.  If the block is
	 * small enough, we may be able to stuff the extra data into the buffer. */
	const int iExtra = iRet % iBytes;
	Seek( Tell()-iExtra );

	return iRet/iBytes;
}

/* Empty the write buffer to disk.  Return -1 on error, 0 on success. */
int RageFileObj::EmptyWriteBuf()
{
	if( m_pWriteBuffer == NULL )
		return 0;

	if( m_iWriteBufferUsed )
	{
		/* The write buffer may not align with the actual file, if we've seeked.  Only
		 * seek if needed. */
		bool bSeeked = (m_iWriteBufferPos+m_iWriteBufferUsed != m_iFilePos);
		if( bSeeked )
			SeekInternal( m_iWriteBufferPos );

		int iRet = WriteInternal( m_pWriteBuffer, m_iWriteBufferUsed );

		if( bSeeked )
			SeekInternal( m_iFilePos );
		if( iRet == -1 )
			return iRet;
	}

	m_iWriteBufferPos = m_iFilePos;
	m_iWriteBufferUsed = 0;
	return 0;
}

int RageFileObj::Write( const void *pBuffer, size_t iBytes )
{
	if( m_pWriteBuffer != NULL )
	{
		/* If the file position has moved away from the write buffer, or the
		 * incoming data won't fit in the buffer, flush. */
		if( m_iWriteBufferPos+m_iWriteBufferUsed != m_iFilePos || m_iWriteBufferUsed + (int)iBytes > m_iWriteBufferSize )
		{
			int iRet = EmptyWriteBuf();
			if( iRet == -1 )
				return iRet;
		}

		if( m_iWriteBufferUsed + (int)iBytes <= m_iWriteBufferSize )
		{
			memcpy( m_pWriteBuffer+m_iWriteBufferUsed, pBuffer, iBytes );
			m_iWriteBufferUsed += iBytes;
			m_iFilePos += iBytes;
			if( m_bCRC32Enabled )
				CRC32( m_iCRC32, pBuffer, iBytes );
			return iBytes;
		}

		/* We're writing a lot of data, and it won't fit in the buffer.  We already
		 * flushed above, so m_iWriteBufferUsed; fall through and write the block normally. */
		ASSERT_M( m_iWriteBufferUsed == 0, ssprintf("%i", m_iWriteBufferUsed) );
	}

	int iRet = WriteInternal( pBuffer, iBytes );
	if( iRet != -1 )
	{
		m_iFilePos += iRet;
		if( m_bCRC32Enabled )
			CRC32( m_iCRC32, pBuffer, iBytes );
	}
	return iRet;
}

int RageFileObj::Write( const void *pBuffer, size_t iBytes, int iNmemb )
{
	/* Simple write.  We never return partial writes. */
	int iRet = Write( pBuffer, iBytes*iNmemb ) / iBytes;
	if( iRet == -1 )
		return -1;
	return iRet / iBytes;
}

int RageFileObj::Flush()
{
	int iRet = EmptyWriteBuf();
	if( iRet == -1 )
		return iRet;
	return FlushInternal();
}

void RageFileObj::EnableReadBuffering()
{
	if( m_pReadBuffer == NULL )
		m_pReadBuffer = new char[BSIZE];
}

void RageFileObj::EnableWriteBuffering( int iBytes )
{
	if( m_pWriteBuffer == NULL )
	{
		m_pWriteBuffer = new char[iBytes];
		m_iWriteBufferPos = m_iFilePos;
		m_iWriteBufferSize = iBytes;
	}
}

void RageFileObj::EnableCRC32( bool bOn )
{
	if( !bOn )
	{
		m_bCRC32Enabled = false;
		return;
	}

	m_bCRC32Enabled = true;
	m_iCRC32 = 0;
}

bool RageFileObj::GetCRC32( uint32_t *iRet )
{
	if( !m_bCRC32Enabled )
		return false;

	*iRet = m_iCRC32;
	return true;
}

/* Read up to the next \n, and return it in out.  Strip the \n.  If the \n is
 * preceded by a \r (DOS newline), strip that, too. */
int RageFileObj::GetLine( RString &sOut )
{
	sOut = "";

	if( m_bEOF )
		return 0;

	EnableReadBuffering();

	bool bGotData = false;
	while( 1 )
	{
		bool bDone = false;

		/* Find the end of the block we'll move to out. */
		char *p = (char *) memchr( m_pReadBuf, '\n', m_iReadBufAvail );
		bool bReAddCR = false;
		if( p == NULL )
		{
			/* Hack: If the last character of the buffer is \r, then it's likely that an
			 * \r\n has been split across buffers.  Move everything else, then move the
			 * \r to the beginning of the buffer and handle it the next time around the loop. */
			if( m_iReadBufAvail && m_pReadBuf[m_iReadBufAvail-1] == '\r' )
			{
				bReAddCR = true;
				--m_iReadBufAvail;
			}

			p = m_pReadBuf+m_iReadBufAvail; /* everything */
		}
		else
		{
			bDone = true;
		}

		if( p >= m_pReadBuf )
		{
			char *RealEnd = p;
			if( bDone && p > m_pReadBuf && p[-1] == '\r' )
				--RealEnd; /* not including \r */
			sOut.append( m_pReadBuf, RealEnd );

			if( bDone )
				++p; /* skip \n */

			const int iUsed = p-m_pReadBuf;
			if( iUsed )
			{
				m_iReadBufAvail -= iUsed;
				m_iFilePos += iUsed;
				bGotData = true;
				m_pReadBuf = p;
			}
		}

		if( bReAddCR )
		{
			ASSERT( m_iReadBufAvail == 0 );
			m_pReadBuf = m_pReadBuffer;
			m_pReadBuffer[m_iReadBufAvail] = '\r';
			++m_iReadBufAvail;
		}

		if( bDone )
			break;

		/* We need more data. */
		m_pReadBuf = m_pReadBuffer;

		const int iSize = FillReadBuf();

		/* If we've read data already, then don't mark EOF yet.  Wait until the
		 * next time we're called. */
		if( iSize == 0 && !bGotData )
		{
			m_bEOF = true;
			return 0;
		}
		if( iSize == -1 )
			return -1; // error
		if( iSize == 0 )
			break; // EOF or error
	}
	return bGotData? 1:0;
}

// Always use "\r\n".  Even though the program may be running on Unix, the
// files written to a memory card are likely to be edited using Windows.
//#if defined(WIN32)
#define NEWLINE "\r\n"
//#else
//#define NEWLINE "\n"
//#endif

int RageFileObj::PutLine( const RString &sStr )
{
	if( Write(sStr) == -1 )
		return -1;
	return Write( RString(NEWLINE) );
}

/* Fill the internal buffer.  This never marks EOF, since this is an internal, hidden
 * read; EOF should only be set as a result of a real read.  (That is, disabling buffering
 * shouldn't cause the results of AtEOF to change.) */
int RageFileObj::FillReadBuf()
{
	/* Don't call this unless buffering is enabled. */
	ASSERT( m_pReadBuffer != NULL );

	/* The buffer starts at m_Buffer; any data in it starts at m_pReadBuf; space between
	 * the two is old data that we've read.  (Don't mangle that data; we can use it
	 * for seeking backwards.) */
	const int iBufAvail = BSIZE - (m_pReadBuf-m_pReadBuffer) - m_iReadBufAvail;
	ASSERT_M( iBufAvail >= 0, ssprintf("%p, %p, %i", m_pReadBuf, m_pReadBuffer, (int) BSIZE ) );
	const int iSize = this->ReadInternal( m_pReadBuf+m_iReadBufAvail, iBufAvail );

	if( iSize > 0 )
		m_iReadBufAvail += iSize;

	return iSize;
}

void RageFileObj::ResetReadBuf()
{
	m_iReadBufAvail = 0;
	m_pReadBuf = m_pReadBuffer;
}

/*
 * Copyright (c) 2003-2004 Glenn Maynard
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

