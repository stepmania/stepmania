#include "global.h"
#include "RageFileDriverDeflate.h"
#include "RageLog.h"
#include "RageUtil.h"

#if defined(_WINDOWS) || defined(_XBOX)
        #include "zlib/zlib.h"
        #pragma comment(lib, "zlib/zdll.lib")
#elif defined(DARWIN)
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

