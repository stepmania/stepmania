#include "global.h"
#include "RageFileDriverDeflate.h"
#include "RageFileDriverSlice.h"
#include "RageFile.h"
#include "RageLog.h"
#include "RageUtil.h"

#if defined(_WINDOWS) || defined(_XBOX)
        #include "zlib/zlib.h"
#if defined(_MSC_VER)
        #pragma comment(lib, "zlib/zdll.lib")
#endif
#elif defined(__MACOSX__)
    /* Since crypto++ was added to the repository, <zlib.h> includes the zlib.h
     * in there rather than the correct system one. I don't know why it would do
     * this since crypto51 is not being listed as one of the include directories.
     * I've never run into this problem before and looking at the command line
     * used to compile RageFileDriverZip.o, I have no idea how it's happening.
     * --Steve
     */
	#include "/usr/include/zlib.h"
#else
        #include <zlib.h>
#endif

RageFileObjInflate::RageFileObjInflate( RageFileBasic *pFile, int iUncompressedSize )
{
	m_bFileOwned = false;
	m_pFile = pFile;
	decomp_buf_avail = 0;
	m_pInflate = new z_stream;
	memset( m_pInflate, 0, sizeof(z_stream) );
	
	m_iUncompressedSize = iUncompressedSize;

	int err = inflateInit2( m_pInflate, -MAX_WBITS );
	if( err == Z_MEM_ERROR )
		RageException::Throw( "inflateInit2( %i ): out of memory", -MAX_WBITS );
	if( err != Z_OK )
		LOG->Trace( "Huh? inflateInit2() err = %i", err );

	decomp_buf_ptr = decomp_buf;
	m_iFilePos = 0;
}

RageFileObjInflate::RageFileObjInflate( const RageFileObjInflate &cpy ):
	RageFileObj( cpy )
{
	/* XXX completely untested */
	/* Copy the entire decode state. */
	/* inflateInit2 isn't widespread yet */
	ASSERT( 0 );
/*
	m_pFile = cpy.m_pFile->Copy();
	m_bFileOwned = true;
	m_pInflate = new z_stream;
	inflateCopy( m_pInflate, const_cast<z_stream*>(cpy.m_pInflate) );

	// memcpy decomp_buf?
	decomp_buf_ptr = decomp_buf + (cpy.decomp_buf_ptr - cpy.decomp_buf);
	decomp_buf_avail = cpy.decomp_buf_avail;
	m_iFilePos = cpy.m_iFilePos;
	*/
}

RageFileBasic *RageFileObjInflate::Copy() const
{
	RageException::Throw( "Loading ZIPs from deflated ZIPs is currently disabled; see RageFileObjInflate" );

	// return new RageFileObjInflate( *this, p );
}
	

RageFileObjInflate::~RageFileObjInflate()
{
	if( m_bFileOwned )
		delete m_pFile;

	int err = inflateEnd( m_pInflate );
	if( err != Z_OK )
		LOG->Trace( "Huh? inflateEnd() err = %i", err );

	delete m_pInflate;
}

int RageFileObjInflate::ReadInternal( void *buf, size_t bytes )
{
	/* Don't read more than m_iUncompressedSize of data.  If we don't do this, it's
	 * possible for a .gz to contain a header claiming 500k of data, but to actually
	 * contain much more deflated data. */
	ASSERT_M( m_iFilePos <= m_iUncompressedSize, ssprintf("%i, %i",m_iFilePos, m_iUncompressedSize) );
	bytes = min( bytes, size_t(m_iUncompressedSize-m_iFilePos) );

	bool done=false;
	int ret = 0;
	while( bytes && !done )
	{
		if ( !decomp_buf_avail )
		{
			decomp_buf_ptr = decomp_buf;
			decomp_buf_avail = 0;
			int got = m_pFile->Read( decomp_buf, sizeof(decomp_buf) );
			if( got == -1 )
			{
				SetError( m_pFile->GetError() );
				return -1;
			}
			if( got == 0 )
				break;

			decomp_buf_avail = got;
		}

		m_pInflate->next_in = (Bytef *) decomp_buf_ptr;
		m_pInflate->avail_in = decomp_buf_avail;
		m_pInflate->next_out = (Bytef *) buf;
		m_pInflate->avail_out = bytes;


		int err = inflate( m_pInflate, Z_PARTIAL_FLUSH );
		switch( err )
		{
		case Z_DATA_ERROR:
			SetError( "Data error" );
			return -1;
		case Z_MEM_ERROR:
			SetError( "out of memory" );
			return -1;
		case Z_STREAM_END:
			done = true;
			break;
		case Z_OK:
			break;
		default:
			LOG->Trace( "Huh? inflate err %i", err );
		}

		const int used = (char *)m_pInflate->next_in - decomp_buf_ptr;
		decomp_buf_ptr += used;
		decomp_buf_avail -= used;

		const int got = (char *)m_pInflate->next_out - (char *)buf;
		m_iFilePos += got;
		ret += got;
		buf = (char *)buf + got;
		bytes -= got;
	}

	return ret;
}

int RageFileObjInflate::SeekInternal( int iPos )
{
	/* Optimization: if offset is the end of the file, it's a lseek(0,SEEK_END).  Don't
	 * decode anything. */
	if( iPos >= m_iUncompressedSize )
	{
		m_iFilePos = m_iUncompressedSize;
		m_pFile->Seek( m_pFile->GetFileSize() );
		decomp_buf_ptr = decomp_buf;
		decomp_buf_avail = 0;
		inflateReset( m_pInflate );
		return m_iUncompressedSize;
	}

	if( iPos < m_iFilePos )
	{
		inflateReset( m_pInflate );
		decomp_buf_ptr = decomp_buf;
		decomp_buf_avail = 0;

		m_pFile->Seek( 0 );
		m_iFilePos = 0;
	}

	int iOffset = iPos - m_iFilePos;

	/* Can this be optimized? */
	char buf[1024*4];
	while( iOffset )
	{
		int got = ReadInternal( buf, min( (int) sizeof(buf), iOffset ) );
		if( got == -1 )
			return -1;

		if( got == 0 )
			break;
		iOffset -= got;
	}

	return m_iFilePos;
}

RageFileObjDeflate::RageFileObjDeflate( RageFileBasic *pFile )
{
	m_pFile = pFile;
	m_bFileOwned = false;

	m_pDeflate = new z_stream;
	memset( m_pDeflate, 0, sizeof(z_stream) );

	int err = deflateInit2( m_pDeflate,
				3,
				Z_DEFLATED,
				-15, // windowBits
				8, // memLevel
				Z_DEFAULT_STRATEGY );

	if( err == Z_MEM_ERROR )
		RageException::Throw( "inflateInit2( %i ): out of memory", -MAX_WBITS );
	if( err != Z_OK )
		LOG->Trace( "Huh? inflateInit2() err = %i", err );

}

RageFileObjDeflate::~RageFileObjDeflate()
{
	FlushInternal();

	if( m_bFileOwned )
		delete m_pFile;

	int err = deflateEnd( m_pDeflate );
	if( err != Z_OK )
		LOG->Trace( "Huh? deflateEnd() err = %i", err );

	delete m_pDeflate;
}

int RageFileObjDeflate::WriteInternal( const void *pBuffer, size_t iBytes )
{
	if( iBytes == 0 )
		return 0;

	m_pDeflate->next_in  = (Bytef*) pBuffer;
	m_pDeflate->avail_in = iBytes;

	while( 1 )
	{
		char buf[1024*4];
		m_pDeflate->next_out = (Bytef *) buf;
		m_pDeflate->avail_out = sizeof(buf);

		int err = deflate( m_pDeflate, Z_NO_FLUSH );

		if( err != Z_OK )
			FAIL_M( ssprintf("deflate: err %i", err) );
			
		if( m_pDeflate->avail_out < sizeof(buf) )
		{
			int iBytes = sizeof(buf)-m_pDeflate->avail_out;
			int iRet = m_pFile->Write( buf, iBytes );
			if( iRet == -1 )
			{
				SetError( m_pFile->GetError() );
				return -1;
			}
			if( iRet < iBytes )
			{
				SetError( "Partial write" );
				return -1;
			}
		}

		if( m_pDeflate->avail_in == 0 && m_pDeflate->avail_out != 0 )
			break;
	}

	return iBytes;
}

/* Note that flushing clears compression state, so (unlike most Flush() calls)
 * calling this *does* change the result of the output if you continue writing
 * data. */
int RageFileObjDeflate::FlushInternal()
{
	m_pDeflate->avail_in = 0;

	while( 1 )
	{
		char buf[1024*4];
		m_pDeflate->next_out = (Bytef *) buf;
		m_pDeflate->avail_out = sizeof(buf);

		int err = deflate( m_pDeflate, Z_FINISH );
		if( err != Z_OK && err != Z_STREAM_END )
			FAIL_M( ssprintf("deflate: err %i", err) );

		if( m_pDeflate->avail_out < sizeof(buf) )
		{
			int iBytes = sizeof(buf)-m_pDeflate->avail_out;
			int iRet = m_pFile->Write( buf, iBytes );
			if( iRet == -1 )
			{
				SetError( m_pFile->GetError() );
				return -1;
			}
			LOG->Trace("FlushInternal: wrote %i/%i", iRet, iBytes);
			if( iRet < iBytes )
			{
				SetError( "Partial write" );
				return -1;
			}
		}

		if( err == Z_STREAM_END && m_pDeflate->avail_out != 0 )
			return m_pFile->Flush();
	}
}

/*
 * Parse a .gz file, check the header CRC16 if present, and return the data
 * CRC32 and a decompressor.
 */
RageFileBasic *GunzipFile( RageFileBasic &file, CString &sError, uint32_t *iCRC32 )
{
	sError = "";

	file.Seek(0);
	file.EnableCRC32( true );

	{
		char magic[2];
		FileReading::ReadBytes( file, magic, 2, sError );
		if( sError != "" )
			return NULL;

		if( magic[0] != '\x1f' || magic[1] != '\x8b' )
		{
			sError = "Not a gzip file";
			return NULL;
		}
	}

	uint8_t iCompressionMethod = FileReading::read_8( file, sError );
	uint8_t iFlags = FileReading::read_8( file, sError );
	FileReading::read_32_le( file, sError ); /* time */
	FileReading::read_8( file, sError ); /* xfl */
	FileReading::read_8( file, sError ); /* os */
	if( sError != "" )
		return NULL;

#define FTEXT    1<<0
#define FHCRC    1<<1
#define FEXTRA   1<<2
#define FNAME    1<<3
#define FCOMMENT 1<<4
#define UNSUPPORTED_MASK ~((1<<5)-1)
	if( iCompressionMethod != 8 )
	{
		sError = ssprintf( "Unsupported compression: %i", iCompressionMethod );
		return NULL;
	}

	/* Warning: flags other than FNAME are untested, since gzip doesn't
	 * actually output them. */
	if( iFlags & UNSUPPORTED_MASK )
	{
		sError = ssprintf( "Unsupported flags: %x", iFlags );
		return NULL;
	}

	if( iFlags & FEXTRA )
	{
		int16_t iSize = FileReading::read_16_le( file, sError );
		FileReading::SkipBytes( file, iSize, sError );
	}

	if( iFlags & FNAME )
		while( sError == "" && FileReading::read_8( file, sError ) != 0 )
			;
	if( iFlags & FCOMMENT )
		while( sError == "" && FileReading::read_8( file, sError ) != 0 )
			;
	
	if( iFlags & FHCRC )
	{
		/* Get the CRC of the data read so far.  Be sure to do this before
		 * reading iExpectedCRC16. */
		uint32_t iActualCRC32;
		bool bOK = file.GetCRC32( &iActualCRC32 );
		ASSERT( bOK );
	
		uint16_t iExpectedCRC16 = FileReading::read_u16_le( file, sError );
		uint16_t iActualCRC16 = int16_t( iActualCRC32 & 0xFFFF );
		if( sError != "" )
			return NULL;

		if( iActualCRC16 != iExpectedCRC16 )
		{
			sError = "Header CRC error";
			return NULL;
		}
	}

	/* We only need CRC checking on the raw data for the header, so disable
	 * it. */
	file.EnableCRC32( false );

	if( sError != "" )
		return NULL;

	int iDataPos = file.Tell();

	/* Seek to the end, and grab the uncompressed flie size and CRC. */
	int iFooterPos = file.GetFileSize() - 8;

	FileReading::Seek( file, iFooterPos, sError );
	
	uint32_t iExpectedCRC32 = FileReading::read_u32_le( file, sError );
	uint32_t iUncompressedSize = FileReading::read_u32_le( file, sError );
	if( iCRC32 != NULL )
		*iCRC32 = iExpectedCRC32;
	
	FileReading::Seek( file, iDataPos, sError );
	
	if( sError != "" )
		return NULL;
	
	RageFileDriverSlice *pSliceFile = new RageFileDriverSlice( &file, iDataPos, iFooterPos-iDataPos );
	RageFileObjInflate *pInflateFile = new RageFileObjInflate( pSliceFile, iUncompressedSize );
	pInflateFile->DeleteFileWhenFinished();

	/* Enable CRC calculation only if the caller is interested. */
	if( iCRC32 != NULL )
		pInflateFile->EnableCRC32();

	return pInflateFile;
}

/*
 * Usage:
 *
 * RageFile output;
 * output.Open( "hello.gz", RageFile::WRITE );
 *
 * RageFileObjGzip gzip( &output );
 * gzip.Start();
 * gzip.Write( "data" );
 * gzip.Finish();
 */ 
RageFileObjGzip::RageFileObjGzip( RageFileBasic *pFile ):
	RageFileObjDeflate( pFile )
{
	m_iDataStartOffset = -1;
}

/* Write the gzip header. */
int RageFileObjGzip::Start()
{
	/* We should be at the start of the file. */
	ASSERT( this->Tell() == 0 );

	static const char header[] =
	{
		'\x1f', '\x8b', // magic
		8,              // method: deflate
		0,              // no flags
		0, 0, 0, 0,     // no time
		0,              // no extra flags
		'\xFF'          // unknown os
	};

	if( m_pFile->Write( header, sizeof(header) ) == -1 )
		return -1;
	
	m_iDataStartOffset = Tell();

	/* Enable and reset the CRC32 for the uncompressed data about to be
	 * written to this file. */
	this->EnableCRC32( true );

	return 0;
}

/* Write the gzip footer. */
int RageFileObjGzip::Finish()
{
	/* We're about to write to the underlying file (so the footer isn't
	 * compressed).  Flush the compressed data first. */
	if( this->Flush() == -1 )
		return -1;

	/* Read the CRC of the data that's been written. */
	uint32_t iCRC;
	bool bOK = this->GetCRC32( &iCRC );
	ASSERT( bOK );

	/* Figure out the size of the data. */
	uint32_t iSize = Tell() - m_iDataStartOffset;

	/* Write the CRC and size directly to the file, so they don't get compressed. */
	iCRC = Swap32LE( iCRC );
	if( m_pFile->Write( &iCRC, sizeof(iCRC) ) == -1 )
	{
		SetError( m_pFile->GetError() );
		return -1;
	}

	/* Write the size. */
	iSize = Swap32LE( iSize );
	if( m_pFile->Write( &iSize, sizeof(iSize) ) == -1 )
	{
		SetError( m_pFile->GetError() );
		return -1;
	}
	
	/* Flush the CRC and wize that we just wrote directly to the file. */
	return m_pFile->Flush();
}

#include "RageFileDriverMemory.h"

void GzipString( const CString &sIn, CString &sOut )
{
	/* Gzip it. */
	RageFileObjMem mem;
	RageFileObjGzip gzip( &mem );
	gzip.Start();
	gzip.Write( sIn );
	gzip.Finish();

	sOut = mem.GetString();
}

bool GunzipString( const CString &sIn, CString &sOut )
{
	RageFileObjMem mem;
	mem.PutString( sIn );

	CString sError;
	uint32_t iCRC32;
	RageFileBasic *pFile = GunzipFile( mem, sError, &iCRC32 );
	if( pFile == NULL )
		return false;

	pFile->Read( sOut );

	/* Check the CRC. */
	unsigned iRet;
	ASSERT( pFile->GetCRC32( &iRet ) );
	SAFE_DELETE( pFile );

	if( iRet != iCRC32 )
		return false;

	return true;
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

