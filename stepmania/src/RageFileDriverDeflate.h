/* RageFileObjInflate - decompress streams compressed with "deflate" compression. */

#ifndef RAGE_FILE_DRIVER_DEFLATE_H
#define RAGE_FILE_DRIVER_DEFLATE_H

#include "RageFileBasic.h"

typedef struct z_stream_s z_stream;

class RageFileObjInflate: public RageFileObj
{
public:
	/* By default, pFile will not be freed.  To implement GetFileSize(), the
	 * container format must store the file size. */
	RageFileObjInflate( RageFileBasic *pFile, int iUncompressedSize );
	RageFileObjInflate( const RageFileObjInflate &cpy );
	~RageFileObjInflate();
	int ReadInternal( void *pBuffer, size_t iBytes );
	int WriteInternal( const void *pBuffer, size_t iBytes ) { SetError( "Not implemented" ); return -1; }
	int SeekInternal( int iOffset );
	int GetFileSize() const { return m_iUncompressedSize; }
	RageFileBasic *Copy() const;

	void DeleteFileWhenFinished() { m_bFileOwned = true; }

private:
	int m_iUncompressedSize;
	RageFileBasic *m_pFile;
	int m_iFilePos;
	bool m_bFileOwned;

	z_stream *m_pInflate;
	enum { INBUFSIZE = 1024*4 };
	char decomp_buf[INBUFSIZE], *decomp_buf_ptr;
	int decomp_buf_avail;
};

class RageFileObjDeflate: public RageFileObj
{
public:
	/* By default, pFile will not be freed. */
	RageFileObjDeflate( RageFileBasic *pOutput );
	~RageFileObjDeflate();

	int GetFileSize() const { return m_pFile->GetFileSize(); }
	void DeleteFileWhenFinished() { m_bFileOwned = true; }

protected:
	int ReadInternal( void *pBuffer, size_t iBytes ) { SetError( "Not implemented" ); return -1; }
	int WriteInternal( const void *pBuffer, size_t iBytes );
	int FlushInternal();
	
	RageFileBasic *m_pFile;
	z_stream *m_pDeflate;
	bool m_bFileOwned;
};

class RageFileObjGzip: public RageFileObjDeflate
{
public:
	RageFileObjGzip( RageFileBasic *pFile );
	int Start();
	int Finish();

private:
	int m_iDataStartOffset;
};

RageFileBasic *GunzipFile( RageFileBasic &file, RString &sError, uint32_t *iCRC32 );

/* Quick helpers: */
void GzipString( const RString &sIn, RString &sOut );
bool GunzipString( const RString &sIn, RString &sOut ); // returns false on CRC, etc. error

#endif

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

