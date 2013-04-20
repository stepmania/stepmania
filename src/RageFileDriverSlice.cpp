#include "global.h"
#include "RageFileDriverSlice.h"

RageFileDriverSlice::RageFileDriverSlice( RageFileBasic *pFile, int iOffset, int iFileSize )
{
	m_pFile = pFile;
	m_iOffset = iOffset;
	m_iFileSize = iFileSize;
	m_iFilePos = 0;
	m_bFileOwned = false;
}

RageFileDriverSlice::RageFileDriverSlice( const RageFileDriverSlice &cpy ):
	RageFileObj(cpy)
{
	m_pFile = cpy.m_pFile->Copy();
	m_iOffset = cpy.m_iOffset;
	m_iFileSize = cpy.m_iFileSize;
	m_iFilePos = cpy.m_iFilePos;
	m_bFileOwned = true;
}

RageFileDriverSlice::~RageFileDriverSlice()
{
	if( m_bFileOwned )
		delete m_pFile;
}

RageFileDriverSlice *RageFileDriverSlice::Copy() const
{
	RageFileDriverSlice *pRet = new RageFileDriverSlice( *this );
	return pRet;
}

int RageFileDriverSlice::ReadInternal( void *buf, size_t bytes )
{
	/* Make sure we're reading from the right place.  We might have been constructed
	 * with a file not pointing to iOffset. */
	m_pFile->Seek( m_iFilePos+m_iOffset );

	const int bytes_left = m_iFileSize-this->m_iFilePos;
	const int got = m_pFile->Read( buf, min( (int) bytes, bytes_left ) );
	if( got == -1 )
	{
		SetError( m_pFile->GetError() );
		return -1;
	}

	m_iFilePos += got;

	return got;
}


int RageFileDriverSlice::SeekInternal( int offset )
{
	ASSERT( offset >= 0 );
	offset = min( offset, m_iFileSize );

	int ret = m_pFile->Seek( m_iOffset + offset );
	if( ret == -1 )
	{
		SetError( m_pFile->GetError() );
		return -1;
	}
	ret -= m_iOffset;
	ASSERT( ret >= 0 );
	m_iFilePos = ret;

	return ret;
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

