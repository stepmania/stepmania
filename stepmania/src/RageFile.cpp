#include "global.h"
#include "RageFile.h"
#include "RageUtil.h"
#include "RageFileDriver.h"

RageFile::RageFile()
{
    m_File = NULL;
	m_BufAvail = 0;
	m_EOF = false;
	m_FilePos = 0;
}
	
RageFile::RageFile( const CString& path, int mode )
{
    m_File = NULL;
	m_BufAvail = 0;
	m_EOF = false;
	m_FilePos = 0;
	Open(path, mode);
}

RageFile::RageFile( const RageFile &cpy )
{
	ResetBuf();

	/* This will copy the file driver, including its internal file pointer. */
	m_File = FILEMAN->CopyFileObj( cpy.m_File, *this );
	m_Path = cpy.m_Path;
	m_Mode = cpy.m_Mode;
	m_Error = cpy.m_Error;
	m_EOF = cpy.m_EOF;
	m_FilePos = cpy.m_FilePos;
	memcpy( this->m_Buffer, cpy.m_Buffer, sizeof(m_Buffer) );
	m_pBuf = m_Buffer + (cpy.m_pBuf-cpy.m_Buffer);

	m_BufAvail = cpy.m_BufAvail;
}

CString RageFile::GetPath() const
{
    if ( !IsOpen() )
		return "";

	return m_File->GetDisplayPath();
}

bool RageFile::Open( const CString& path, int mode )
{
	ASSERT( FILEMAN );
	Close();

	m_Path = path;
	FixSlashesInPlace(m_Path);

	m_Mode = mode;
	m_EOF = false;
	m_FilePos = 0;
	ResetBuf();

	if( (m_Mode&READ) && (m_Mode&WRITE) )
	{
		SetError( "Reading and writing are mutually exclusive" );
		return false;
	}

	if( !(m_Mode&READ) && !(m_Mode&WRITE) )
	{
		SetError( "Neither reading nor writing specified" );
		return false;
	}

	int error;
	m_File = FILEMAN->Open( path, mode, *this, error );

	if( m_File == NULL )
	{
		SetError( strerror(error) );
		return false;
	}

    return true;
}

void RageFile::Close()
{
	FILEMAN->Close( m_File );
	m_File = NULL;
}

/* Fill the internal buffer.  This never marks EOF, since this is an internal, hidden
 * read; EOF should only be set as a result of a real read.  (That is, disabling buffering
 * shouldn't cause the results of AtEOF to change.) */
int RageFile::FillBuf()
{
	/* The buffer starts at m_Buffer; any data in it starts at m_pBuf; space between
	 * the two is old data that we've read.  (Don't mangle that data; we can use it
	 * for seeking backwards.) */
	const int iBufAvail = sizeof(m_Buffer) - (m_pBuf-m_Buffer) - m_BufAvail;
	ASSERT_M( iBufAvail >= 0, ssprintf("%p, %p, %i", m_pBuf, m_Buffer, (int) sizeof(m_Buffer) ) );
	const int size = m_File->Read( m_pBuf+m_BufAvail, iBufAvail );

	if( size > 0 )
		m_BufAvail += size;

	return size;
}

void RageFile::ResetBuf()
{
	m_BufAvail = 0;
	m_pBuf = m_Buffer;
}

/* Read up to the next \n, and return it in out.  Strip the \n.  If the \n is
 * preceded by a \r (DOS newline), strip that, too. */
int RageFile::GetLine( CString &out )
{
	out = "";

    if ( !IsOpen() )
        RageException::Throw("\"%s\" is not open.", m_Path.c_str());

	if( !(m_Mode&READ) )
		RageException::Throw("\"%s\" is not open for reading", GetPath().c_str());

	if( m_EOF )
		return 0;

	bool GotData = false;
	while( 1 )
	{
		bool done = false;

		/* Find the end of the block we'll move to out. */
		char *p = (char *) memchr( m_pBuf, '\n', m_BufAvail );
		bool ReAddCR = false;
		if( p == NULL )
		{
			/* Hack: If the last character of the buffer is \r, then it's likely that an
			 * \r\n has been split across buffers.  Move everything else, then move the
			 * \r to the beginning of the buffer and handle it the next time around the loop. */
			if( m_BufAvail && m_pBuf[m_BufAvail-1] == '\r' )
			{
				ReAddCR = true;
				--m_BufAvail;
			}

			p = m_pBuf+m_BufAvail; /* everything */
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
				m_BufAvail -= used;
				m_FilePos += used;
				GotData = true;
				m_pBuf = p;
			}
		}

		if( ReAddCR )
		{
			ASSERT( m_BufAvail == 0 );
			m_pBuf = m_Buffer;
			m_Buffer[m_BufAvail] = '\r';
			++m_BufAvail;
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
			m_EOF = true;
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

int RageFile::PutLine( const CString &str )
{
	if( Write(str) == -1 )
		return -1;
	return Write( CString(NEWLINE) );
}


int RageFile::Read( void *buffer, size_t bytes )
{
	if( !IsOpen() )
		RageException::Throw("\"%s\" is not open.", GetPath().c_str());

	if( !(m_Mode&READ) )
		RageException::Throw("\"%s\" is not open for reading", GetPath().c_str());

	int ret = 0;

	while( !m_EOF && bytes > 0 )
	{
		/* Copy data out of the buffer first. */
		int FromBuffer = min( (int) bytes, m_BufAvail );
		memcpy( buffer, m_pBuf, FromBuffer );

		ret += FromBuffer;
		m_FilePos += FromBuffer;
		bytes -= FromBuffer;
		m_BufAvail -= FromBuffer;
		m_pBuf += FromBuffer;
		
		buffer = (char *) buffer + FromBuffer;
	
		if( !bytes )
			break;

		ASSERT( !m_BufAvail );

		/* We need more; either fill the buffer and keep going, or just read directly
		 * into the destination buffer. */
		if( bytes >= sizeof(m_Buffer) )
		{
			/* We have a lot more to read, so don't waste time copying it into the
			 * buffer. */
			int FromFile = m_File->Read( buffer, bytes );
			if( FromFile < 0 )
				return FromFile;
			if( FromFile == 0 )
				m_EOF = true;
			ret += FromFile;
			m_FilePos += FromFile;
			return ret;
		}

		m_pBuf = m_Buffer;
		int got = FillBuf();
		if( got < 0 )
			return got;
		if( got == 0 )
			m_EOF = true;
	}

	return ret;
}

int RageFile::Seek( int offset )
{
	if( !IsOpen() )
		RageException::Throw("\"%s\" is not open.", GetPath().c_str());

	if( !(m_Mode&READ) )
		RageException::Throw("\"%s\" is not open for reading; can't seek", GetPath().c_str());

	m_EOF = false;

	ResetBuf();
	
	if( offset == 0 )
	{
		m_File->Rewind();
		m_FilePos = 0;
		return 0;
	}

	int pos = m_File->Seek( offset );
	if( pos == -1 )
		return -1;

	m_FilePos = pos;
	return pos;
}

int RageFile::SeekCur( int offset )
{
	ASSERT( offset >= 0 );

	if( !IsOpen() )
		RageException::Throw("\"%s\" is not open.", GetPath().c_str());

	if( !(m_Mode&READ) )
		RageException::Throw("\"%s\" is not open for reading; can't seek", GetPath().c_str());

	if( !offset || m_EOF )
		return m_FilePos;

	int FromBuffer = min( offset, m_BufAvail );
	m_FilePos += FromBuffer;
	offset -= FromBuffer;
	m_BufAvail -= FromBuffer;
	m_pBuf += FromBuffer;
	
	if( offset )
	{
		int pos = m_File->SeekCur( offset );
		if( pos == -1 )
			return -1;

		m_FilePos = pos;
	}
	return m_FilePos;
}

int RageFile::GetFileSize() const
{
	if( !IsOpen() )
		RageException::Throw("\"%s\" is not open.", GetPath().c_str());

	if( !(m_Mode&READ) )
		RageException::Throw("\"%s\" is not open for reading; can't GetFileSize", GetPath().c_str());

	/* GetFileSize() may need to do non-const-like things--the default implementation reads
	 * until the end of file to emulate it.  However, it should always restore the state to
	 * the way it was, so pretend it's const. */
	int iRet = const_cast<RageFileObj*>(m_File)->GetFileSize();
	ASSERT_M( iRet >= 0, ssprintf("%i", iRet) );
	return iRet;
}

void RageFile::Rewind()
{
	if( !IsOpen() )
		RageException::Throw("\"%s\" is not open.", GetPath().c_str());

	if( !(m_Mode&READ) )
		RageException::Throw("\"%s\" is not open for reading; can't Rewind", GetPath().c_str());

	m_EOF = false;

	m_FilePos = 0;
	m_File->Rewind();
}

int RageFile::Read( CString &buffer, int bytes )
{
	buffer.erase( buffer.begin(), buffer.end() );
	buffer.reserve( bytes != -1? bytes: this->GetFileSize() );

	int ret = 0;
	char buf[4096];
	while( bytes == -1 || ret < bytes )
	{
		int ToRead = sizeof(buf);
		if( bytes != -1 )
			ToRead  = min( ToRead, bytes-ret );

		const int got = Read( buf, ToRead );
		if( got == 0 )
			break;
		if( got == -1 )
			return -1;

		buffer.append( buf, got );
		ret += got;
	}

	return ret;
}

int RageFile::Write( const void *buffer, size_t bytes )
{
	if( !IsOpen() )
		RageException::Throw("\"%s\" is not open.", GetPath().c_str());

	if( !(m_Mode&WRITE) )
		RageException::Throw("\"%s\" is not open for writing", GetPath().c_str());

	return m_File->Write( buffer, bytes );
}


int RageFile::Write( const void *buffer, size_t bytes, int nmemb )
{
	/* Simple write.  We never return partial writes. */
	int ret = Write( buffer, bytes*nmemb ) / bytes;
	if( ret == -1 )
		return -1;
	return ret / bytes;
}

int RageFile::Flush()
{
	if( !m_File )
	{
		SetError( "Not open" );
		return -1;
	}

	return m_File->Flush();
}

int RageFile::Read( void *buffer, size_t bytes, int nmemb )
{
	const int ret = Read( buffer, bytes*nmemb );
	if( ret == -1 )
		return -1;

	/* If we're reading 10-byte blocks, and we got 27 bytes, we have 7 extra bytes.
	 * Seek back. */
	const int extra = ret % bytes;
	Seek( Tell()-extra );

	return ret/bytes;
}

int RageFile::Seek( int offset, int whence )
{
	switch( whence )
	{
	case SEEK_CUR:
		return SeekCur( (int) offset );
	case SEEK_END:
		offset += GetFileSize();
	}
	return Seek( (int) offset );
}

/*
 * Copyright (c) 2003-2004 Glenn Maynard, Chris Danford, Steve Checkoway
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
