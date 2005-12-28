/*
 * This provides an interface to open files in RageFileManager's namespace
 * This is just a simple RageFileBasic wrapper on top of another RageFileBasic;
 * when a file is open, is acts like the underlying RageFileBasic, except that
 * a few extra sanity checks are made to check file modes.  
 */

#include "global.h"
#include "RageFileBasic.h"
#include "RageFile.h"
#include "RageUtil.h"
#include "RageFileDriver.h"

RageFile::RageFile()
{
    m_File = NULL;
}
	
RageFile::RageFile( const RageFile &cpy ):
	RageFileBasic( cpy )
{
	/* This will copy the file driver, including its internal file pointer. */
	m_File = cpy.m_File->Copy();
	m_Path = cpy.m_Path;
	m_Mode = cpy.m_Mode;
}

RageFileBasic *RageFile::Copy() const
{
	return new RageFile( *this );
}

RString RageFile::GetPath() const
{
	if ( !IsOpen() )
		return RString();

	RString sRet = m_File->GetDisplayPath();
	if( sRet != "" )
		return sRet;

	return GetRealPath();
}

bool RageFile::Open( const RString& path, int mode )
{
	ASSERT( FILEMAN );
	Close();

	m_Path = path;
	FixSlashesInPlace(m_Path);

	m_Mode = mode;

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
	m_File = FILEMAN->Open( path, mode, error );

	if( m_File == NULL )
	{
		SetError( strerror(error) );
		return false;
	}

    return true;
}

void RageFile::Close()
{
	SAFE_DELETE( m_File );
}

#define ASSERT_OPEN ASSERT_M( IsOpen(), ssprintf("\"%s\" is not open.", m_Path.c_str()) );
#define ASSERT_READ ASSERT_OPEN; ASSERT_M( !!(m_Mode&READ), ssprintf("\"%s\" is not open for reading", m_Path.c_str()) );
#define ASSERT_WRITE ASSERT_OPEN; ASSERT_M( !!(m_Mode&WRITE), ssprintf("\"%s\" is not open for writing", m_Path.c_str()) );
int RageFile::GetLine( RString &out )
{
	ASSERT_READ;
	return m_File->GetLine( out );
}

int RageFile::PutLine( const RString &str )
{
	ASSERT_WRITE;
	return m_File->PutLine( str );
}

void RageFile::EnableCRC32( bool on )
{
	ASSERT_OPEN;
	m_File->EnableCRC32( on );
}

bool RageFile::GetCRC32( uint32_t *iRet )
{
	ASSERT_OPEN;
	return m_File->GetCRC32( iRet );
}


bool RageFile::AtEOF() const
{
	ASSERT_READ;
	return m_File->AtEOF();
}

void RageFile::ClearError()
{
	if( m_File != NULL )
		m_File->ClearError();
	m_sError = "";
}

RString RageFile::GetError() const
{
	if( m_File != NULL && m_File->GetError() != "" )
		return m_File->GetError();
	return m_sError;
}


void RageFile::SetError( const RString &err )
{
	if( m_File != NULL )
		m_File->ClearError();
	m_sError = err;
}

int RageFile::Read( void *pBuffer, size_t iBytes )
{
	ASSERT_READ;
	return m_File->Read( pBuffer, iBytes );
}

int RageFile::Seek( int offset )
{
	ASSERT_READ;
	return m_File->Seek( offset );
}

int RageFile::Tell() const
{
	ASSERT_READ;
	return m_File->Tell();
}

int RageFile::GetFileSize() const
{
	ASSERT_READ;
	return m_File->GetFileSize();
}

int RageFile::Read( RString &buffer, int bytes )
{
	ASSERT_READ;
	return m_File->Read( buffer, bytes );
}

int RageFile::Write( const void *buffer, size_t bytes )
{
	ASSERT_WRITE;
	return m_File->Write( buffer, bytes );
}


int RageFile::Write( const void *buffer, size_t bytes, int nmemb )
{
	ASSERT_WRITE;
	return m_File->Write( buffer, bytes, nmemb );
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
	ASSERT_READ;
	return m_File->Read( buffer, bytes, nmemb );
}

int RageFile::Seek( int offset, int whence )
{
	ASSERT_READ;
	return m_File->Seek( offset, whence );
}

void FileReading::ReadBytes( RageFileBasic &f, void *buf, int size, RString &sError )
{
	if( sError.size() != 0 )
		return;

	int ret = f.Read( buf, size );
	if( ret == -1 )
		sError = f.GetError();
	else if( ret < size )
		sError = "Unexpected end of file";
}

RString FileReading::ReadString( RageFileBasic &f, int size, RString &sError )
{
	if( sError.size() != 0 )
		return RString();

	RString sBuf;
	int ret = f.Read( sBuf, size );
	if( ret == -1 )
		sError = f.GetError();
	else if( ret < size )
		sError = "Unexpected end of file";
	return sBuf;
}

void FileReading::SkipBytes( RageFileBasic &f, int iBytes, RString &sError )
{
	if( sError.size() != 0 )
		return;

	iBytes += f.Tell();
	FileReading::Seek( f, iBytes, sError );
}

void FileReading::Seek( RageFileBasic &f, int iOffset, RString &sError )
{
	if( sError.size() != 0 )
		return;

	int iGot = f.Seek( iOffset );
	if( iGot == iOffset )
		return;
	if( iGot == -1 )
		sError = f.GetError();
	else if( iGot < iOffset )
		sError = "Unexpected end of file";
}

uint8_t FileReading::read_8( RageFileBasic &f, RString &sError )
{
	uint8_t val;
	ReadBytes( f, &val, sizeof(uint8_t), sError );
	if( sError.size() == 0 )
		return val;
	else
		return 0;
}

uint16_t FileReading::read_u16_le( RageFileBasic &f, RString &sError )
{
	uint16_t val;
	ReadBytes( f, &val, sizeof(uint16_t), sError );
	if( sError.size() == 0 )
		return Swap16LE( val );
	else
		return 0;
}

int16_t FileReading::read_16_le( RageFileBasic &f, RString &sError )
{
	int16_t val;
	ReadBytes( f, &val, sizeof(int16_t), sError );
	if( sError.size() == 0 )
		return Swap16LE( val );
	else
		return 0;
}

uint32_t FileReading::read_u32_le( RageFileBasic &f, RString &sError )
{
	uint32_t val;
	ReadBytes( f, &val, sizeof(uint32_t), sError );
	if( sError.size() == 0 )
		return Swap32LE( val );
	else
		return 0;
}

int32_t FileReading::read_32_le( RageFileBasic &f, RString &sError )
{
	int32_t val;
	ReadBytes( f, &val, sizeof(int32_t), sError );
	if( sError.size() == 0 )
		return Swap32LE( val );
	else
		return 0;
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
