#include "global.h"
#include "RageFileDriver.h"
#include "RageUtil.h"
#include "RageUtil_FileDB.h"

RageFileDriver::~RageFileDriver()
{
	delete FDB;
}

int RageFileDriver::GetPathValue( const CString &path )
{
	vector<CString> parts;
	split( path, "/", parts, true );

	CString PartialPath;

	for( unsigned i = 0; i < parts.size(); ++i )
	{
		PartialPath += parts[i];
		if( i+1 < parts.size() )
			PartialPath += "/";

		const RageFileManager::FileType Type = GetFileType( PartialPath );
		switch( Type )
		{
		case RageFileManager::TYPE_NONE:
			return parts.size()-i;

		/* If this is the last part (the whole path), it needs to be a file; otherwise a directory. */
		case RageFileManager::TYPE_FILE:
			if( i != parts.size()-1 )
				return -1;
			break;
		case RageFileManager::TYPE_DIR:
			if( i == parts.size()-1 )
				return -1;
			break;
		}
	}

	return 0;
}

void RageFileDriver::GetDirListing( const CString &sPath, CStringArray &AddTo, bool bOnlyDirs, bool bReturnPathToo )
{
	FDB->GetDirListing( sPath, AddTo, bOnlyDirs, bReturnPathToo );
}

RageFileManager::FileType RageFileDriver::GetFileType( const CString &sPath )
{
	return FDB->GetFileType( sPath );
}

int RageFileDriver::GetFileSizeInBytes( const CString &sPath )
{
	return FDB->GetFileSize( sPath );
}

int RageFileDriver::GetFileHash( const CString &sPath )
{
	return FDB->GetFileHash( sPath );
}

void RageFileDriver::FlushDirCache( const CString &sPath )
{
	FDB->FlushDirCache();
}


const struct FileDriverEntry *g_FileDriverList = NULL;

FileDriverEntry::FileDriverEntry( CString Type )
{
	m_Link = g_FileDriverList;
	g_FileDriverList = this;
	m_Type = Type;
}

FileDriverEntry::~FileDriverEntry()
{
	g_FileDriverList = NULL; /* invalidate */
}

RageFileDriver *MakeFileDriver( CString Type, CString Root )
{
	for( const FileDriverEntry *p = g_FileDriverList; p; p = p->m_Link )
		if( !p->m_Type.CompareNoCase(Type) )
			return p->Create( Root );
	return NULL;
}


RageFileObj::RageFileObj()
{
	ResetBuf();

	m_iBufAvail = 0;
	m_bEOF = false;
	m_iFilePos = 0;
}

int RageFileObj::Seek( int iOffset )
{
	m_bEOF = false;

	ResetBuf();

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
        int ret = 0;

	while( !m_bEOF && iBytes > 0 )
	{
		/* Copy data out of the buffer first. */
		int FromBuffer = min( (int) iBytes, m_iBufAvail );
		memcpy( pBuffer, m_pBuf, FromBuffer );

		ret += FromBuffer;
		m_iFilePos += FromBuffer;
		iBytes -= FromBuffer;
		m_iBufAvail -= FromBuffer;
		m_pBuf += FromBuffer;

		pBuffer = (char *) pBuffer + FromBuffer;

		if( !iBytes )
			break;

		ASSERT( m_iBufAvail == 0 );

		/* We need more; either fill the buffer and keep going, or just read directly
		 * into the destination buffer. */
		if( iBytes >= sizeof(m_Buffer) )
		{
			/* We have a lot more to read, so don't waste time copying it into the
			 * buffer. */
			int iFromFile = this->ReadInternal( pBuffer, iBytes );
			if( iFromFile == -1 )
				return -1;

			if( iFromFile == 0 )
				m_bEOF = true;
			ret += iFromFile;
			m_iFilePos += iFromFile;
			return ret;
		}

		m_pBuf = m_Buffer;
		int got = FillBuf();
		if( got < 0 )
			return got;
		if( got == 0 )
			m_bEOF = true;
	}

	return ret;
}

int RageFileObj::Read( CString &sBuffer, int iBytes )
{
	sBuffer.erase( sBuffer.begin(), sBuffer.end() );
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

	return iRet;
}

int RageFileObj::Read( void *buffer, size_t bytes, int nmemb )
{
	const int iRet = Read( buffer, bytes*nmemb );
	if( iRet == -1 )
		return -1;

	/* If we're reading 10-byte blocks, and we got 27 bytes, we have 7 extra bytes.
	 * Seek back.  XXX: seeking is very slow for eg. deflated ZIPs.  If the block is
	 * small enough, we may be able to stuff the extra data into the buffer. */
	const int iExtra = iRet % bytes;
	Seek( Tell()-iExtra );

	return iRet/bytes;
}

int RageFileObj::Write( const void *pBuffer, size_t iBytes )
{
	return WriteInternal( pBuffer, iBytes );
}

int RageFileObj::Write( const void *buffer, size_t bytes, int nmemb )
{
	/* Simple write.  We never return partial writes. */
	int ret = Write( buffer, bytes*nmemb ) / bytes;
	if( ret == -1 )
		return -1;
	return ret / bytes;
}

int RageFileObj::Flush()
{
	return FlushInternal();
}


/* Read up to the next \n, and return it in out.  Strip the \n.  If the \n is
 * preceded by a \r (DOS newline), strip that, too. */
int RageFileObj::GetLine( CString &out )
{
	out = "";

	if( m_bEOF )
		return 0;

	bool GotData = false;
	while( 1 )
	{
		bool done = false;

		/* Find the end of the block we'll move to out. */
		char *p = (char *) memchr( m_pBuf, '\n', m_iBufAvail );
		bool ReAddCR = false;
		if( p == NULL )
		{
			/* Hack: If the last character of the buffer is \r, then it's likely that an
			 * \r\n has been split across buffers.  Move everything else, then move the
			 * \r to the beginning of the buffer and handle it the next time around the loop. */
			if( m_iBufAvail && m_pBuf[m_iBufAvail-1] == '\r' )
			{
				ReAddCR = true;
				--m_iBufAvail;
			}

			p = m_pBuf+m_iBufAvail; /* everything */
		}
		else
			done = true;

		if( p >= m_pBuf )
		{
			char *RealEnd = p;
			if( done && p > m_pBuf && p[-1] == '\r' )
				--RealEnd; /* not including \r */
			out.append( m_pBuf, RealEnd );

			if( done )
				++p; /* skip \n */

			const int used = p-m_pBuf;
			if( used )
			{
				m_iBufAvail -= used;
				m_iFilePos += used;
				GotData = true;
				m_pBuf = p;
			}
		}

		if( ReAddCR )
		{
			ASSERT( m_iBufAvail == 0 );
			m_pBuf = m_Buffer;
			m_Buffer[m_iBufAvail] = '\r';
			++m_iBufAvail;
		}

		if( done )
			break;

		/* We need more data. */
		m_pBuf = m_Buffer;

		const int size = FillBuf();

		/* If we've read data already, then don't mark EOF yet.  Wait until the
		 * next time we're called. */
		if( size == 0 && !GotData )
		{
			m_bEOF = true;
			return 0;
		}
		if( size == -1 )
			return -1; // error
		if( size == 0 )
			break; // EOF or error
	}
	return GotData? 1:0;
}

// Always use "\r\n".  Even though the program may be running on Unix, the
// files written to a memory card are likely to be edited using Windows.
//#if defined(_WIN32)
#define NEWLINE "\r\n"
//#else
//#define NEWLINE "\n"
//#endif

int RageFileObj::PutLine( const CString &str )
{
	if( Write(str) == -1 )
		return -1;
	return Write( CString(NEWLINE) );
}

/* Fill the internal buffer.  This never marks EOF, since this is an internal, hidden
 * read; EOF should only be set as a result of a real read.  (That is, disabling buffering
 * shouldn't cause the results of AtEOF to change.) */
int RageFileObj::FillBuf()
{
	/* The buffer starts at m_Buffer; any data in it starts at m_pBuf; space between
	 * the two is old data that we've read.  (Don't mangle that data; we can use it
	 * for seeking backwards.) */
	const int iBufAvail = sizeof(m_Buffer) - (m_pBuf-m_Buffer) - m_iBufAvail;
	ASSERT_M( iBufAvail >= 0, ssprintf("%p, %p, %i", m_pBuf, m_Buffer, (int) sizeof(m_Buffer) ) );
	const int size = this->ReadInternal( m_pBuf+m_iBufAvail, iBufAvail );

	if( size > 0 )
		m_iBufAvail += size;

	return size;
}

void RageFileObj::ResetBuf()
{
	m_iBufAvail = 0;
	m_pBuf = m_Buffer;
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

