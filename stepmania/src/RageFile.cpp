#include "global.h"
/*
 -----------------------------------------------------------------------------
 Class: RageFile
 
 Desc: See header.
 
 Copyright (c) 2001-2003 by the person(s) listed below.  All rights reserved.
	Chris Danford
        Steve Checkoway
 -----------------------------------------------------------------------------
 */

#include "global.h"
#include "RageFile.h"
#include "RageUtil.h"


void FixSlashesInPlace( CString &sPath )
{
    sPath.Replace( "/", SLASH );
    sPath.Replace( "\\", SLASH );
}

CString FixSlashes( CString sPath )
{
    sPath.Replace( "/", SLASH );
    sPath.Replace( "\\", SLASH );
    return sPath;
}

void CollapsePath( CString &sPath )
{
    CStringArray as;
    split( sPath, SLASH, as );
    
    for( unsigned i=0; i<as.size(); i++ )
    {
	if( as[i] == ".." )
	{
	    as.erase( as.begin()+i-1 );
	    as.erase( as.begin()+i-1 );
	    i -= 2;
	}
    }
    sPath = join( SLASH, as );
}

RageFile::RageFile( const CString& path, RageFile::OpenMode mode )
{
    mFP = NULL;
	m_BufUsed = 0;
	Open(path, mode);
}

bool RageFile::Open( const CString& path, RageFile::OpenMode mode )
{
	Close();
	mPath = path;
	FixSlashesInPlace(mPath);
	mFP = fopen( mPath, mode == READ? "r":"w" );
	ResetBuf();

    return mFP == NULL;
}

void RageFile::Close()
{
	if (IsOpen())
		fclose(mFP);
    
	mFP = NULL;
}

void RageFile::ResetBuf()
{
	m_BufUsed = 0;
	m_pBuf = m_Buffer;
}

/* Read up to the next \n, and return it in out.  Strip the \n.  If the \n is
 * preceded by a \r (DOS newline), strip that, too. */
void RageFile::GetLine( CString &out )
{
	out = "";

    if (!IsOpen())
        RageException::Throw("\"%s\" is not open.", mPath.c_str());

	while( 1 )
	{
		bool done = false;

		/* Find the end of the block we'll move to out. */
		char *p = (char *) memchr( m_pBuf, '\n', m_BufUsed );
		if( p == NULL )
			p = m_pBuf+m_BufUsed; /* everything */
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
			m_BufUsed -= p-m_pBuf;
			m_pBuf = p;
		}

		if( done )
			break;

		/* We need more data.  That implies that we moved everything. */
		ASSERT( !m_BufUsed );
		m_pBuf = m_Buffer;

		size_t size = Read( m_pBuf, sizeof(m_Buffer) );
		if( size <= 0 )
			break; // EOF or error
		m_BufUsed += size;
	}
}

int RageFile::Read( void *buffer, size_t bytes )
{
	if( !IsOpen() )
		RageException::Throw("\"%s\" is not open.", mPath.c_str());

	int ret = 0;

	int FromBuffer = min( (int) bytes, m_BufUsed );
	memcpy( buffer, m_pBuf, FromBuffer );
	ret += FromBuffer;
	buffer = (char *) buffer + FromBuffer;
	bytes -= FromBuffer;
	
	if( bytes )
	{
		int FromFile = fread( buffer, 1L, bytes, mFP );
		if( FromFile < 0 )
			return -1;
		ret += FromFile;
	}
	return ret;
}

int RageFile::Write(const void *buffer, size_t bytes)
{
	if (!IsOpen())
		RageException::Throw("\"%s\" is not open.", mPath.c_str());

	return fwrite(buffer, 1L, bytes, mFP);
}

CString RageFile::GetLine()
{
	CString ret;
	GetLine( ret );
	return ret;
}

