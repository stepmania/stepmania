/*
 * RageFileDriver, RageFileObj: Low-level file access driver classes.
 */

#ifndef RAGE_FILE_DRIVER_H
#define RAGE_FILE_DRIVER_H

#include "RageFile.h"
#include "RageFileManager.h"

class RageFileObj;
class FilenameDB;
class RageFileDriver
{
public:
	RageFileDriver( FilenameDB *db ) { FDB = db; }
	virtual ~RageFileDriver();
	virtual RageFileObj *Open( const CString &path, int mode, int &err ) = 0;
	virtual void GetDirListing( const CString &sPath, CStringArray &AddTo, bool bOnlyDirs, bool bReturnPathToo );
	virtual RageFileManager::FileType GetFileType( const CString &sPath );
	virtual int GetFileSizeInBytes( const CString &sFilePath );
	virtual int GetFileHash( const CString &sPath );
	virtual int GetPathValue( const CString &path );
	virtual bool Ready() { return true; } /* see RageFileManager::MountpointIsReady */
	virtual void FlushDirCache( const CString &sPath );
	virtual bool Remove( const CString &sPath ) { return false; }

	/* Possible error returns from Open, in addition to standard errno.h values: */
	enum { ERROR_WRITING_NOT_SUPPORTED = -1 };
protected:
	FilenameDB *FDB;
};

class RageFileObj
{
public:
	virtual ~RageFileObj() { }

	virtual CString GetError() const { return m_sError; }
	
	/* Seek to the given absolute offset.  Return to the position actually
	 * seeked to; if the position given was beyond the end of the file, the
	 * return value will be the size of the file. */
	int Seek( int iOffset );

	/* Read at most iSize bytes into pBuf.  Return the number of bytes read,
	 * 0 on end of stream, or -1 on error.  Note that reading less than iSize
	 * does not necessarily mean that the end of the stream has been reached;
	 * keep reading until 0 is returned. */
	int Read( void *pBuffer, size_t iBytes );

	/* Write iSize bytes of data from pBuf.  Return 0 on success, -1 on error. */
	int Write( const void *pBuffer, size_t iBytes );

	/* Due to buffering, writing may not happen by the end of a Write() call, so not
	 * all errors may be returned by it.  Data will be flushed when the stream (or its
	 * underlying object) is destroyed, but errors can no longer be returned.  Call
	 * Flush() to flush pending data, in order to check for errors. */
	int Flush();

	virtual int GetFileSize() = 0;
	virtual CString GetDisplayPath() const { return ""; }
	virtual RageFileObj *Copy() const { FAIL_M( "Copying unimplemented" ); }

protected:
	virtual int SeekInternal( int iOffset ) { FAIL_M( "Seeking unimplemented" ); }
	virtual int ReadInternal( void *pBuffer, size_t iBytes ) = 0;
	virtual int WriteInternal( const void *pBuffer, size_t iBytes ) = 0;
	virtual int FlushInternal() { return 0; }

	virtual void SetError( const CString &sError ) { m_sError = sError; }
	CString m_sError;
};

/* This is used to register the driver, so RageFileManager can see it. */
struct FileDriverEntry
{
	FileDriverEntry( CString Type );
	virtual ~FileDriverEntry();
	virtual RageFileDriver *Create( CString Root ) const = 0;

	CString m_Type;
	const FileDriverEntry *m_Link;
};
RageFileDriver *MakeFileDriver( CString Type, CString Root );

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
