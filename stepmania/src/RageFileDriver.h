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
	virtual RageFileObj *Open( const CString &path, int mode, RageFile &p, int &err ) = 0;
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
protected:
	RageFile &parent;
	void SetError( const CString &err );

public:
	RageFileObj( RageFile &p ): parent(p) { }
	virtual ~RageFileObj() { }

	void ClearError();

//	virtual CString RealPath() const { return parent->GetPath(); }
	
	virtual int Seek( int offset );
	virtual int SeekCur( int offset );
	virtual int GetFileSize();
	virtual CString GetDisplayPath() const { return parent.GetRealPath(); }

	/* Raw I/O: */
	virtual int Read(void *buffer, size_t bytes) = 0;
	virtual int Write(const void *buffer, size_t bytes) = 0;
	virtual int Flush() { return 0; }
	virtual void Rewind() = 0;
	virtual RageFileObj *Copy( RageFile &p ) const = 0;
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
