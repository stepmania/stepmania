#include "global.h"
#include "RageFileDriverDirect.h"
#include "RageFileDriverDirectHelpers.h"
#include "RageFile.h"
#include "RageUtil.h"
#include "RageUtil_FileDB.h"
#include "RageLog.h"

#include <fcntl.h>
#include <cerrno>
#include <sys/types.h>
#include <sys/stat.h>

#if !defined(WIN32)
#include <dirent.h>
#include <fcntl.h>
#else
#if !defined(_XBOX)
#include <windows.h>
#endif
#include <io.h>
#endif

static struct FileDriverEntry_DIR: public FileDriverEntry
{
	FileDriverEntry_DIR(): FileDriverEntry( "DIR" ) { }
	RageFileDriver *Create( CString Root ) const { return new RageFileDriverDirect( Root ); }
} const g_RegisterDriver;

/* This driver handles direct file access. */

class RageFileObjDirect: public RageFileObj
{
public:
	RageFileObjDirect( const CString &sPath, int iFD, int iMode );
	virtual ~RageFileObjDirect();
	virtual int ReadInternal( void *pBuffer, size_t iBytes );
	virtual int WriteInternal( const void *pBuffer, size_t iBytes );
	virtual int FlushInternal();
	virtual int SeekInternal( int offset );
	virtual RageFileBasic *Copy() const;
	virtual CString GetDisplayPath() const { return m_sPath; }
	virtual int GetFileSize() const;

private:
	int m_iFD;
	CString m_sPath; /* for Copy */

	CString m_sWriteBuf;
	
	bool FinalFlush();

	int m_iMode;
};


RageFileDriverDirect::RageFileDriverDirect( CString root_ ):
	RageFileDriver( new DirectFilenameDB(root_) )
{
	Remount( root_ );
}


static CString MakeTempFilename( const CString &sPath )
{
	/* "Foo/bar/baz" -> "Foo/bar/new.baz.new".  Both prepend and append: we don't
	 * want a wildcard search for the filename to match (foo.txt.new matches foo.txt*),
	 * and we don't want to have the same extension (so "new.foo.sm" doesn't show up
	 * in *.sm). */
	return Dirname(sPath) + "new." + Basename(sPath) + ".new";
}

RageFileObj *MakeFileObjDirect( CString sPath, int mode, int &err )
{
	int fd;
	if( mode & RageFile::READ )
	{
		fd = DoOpen( sPath, O_BINARY|O_RDONLY, 0666 );

		/* XXX: Windows returns EACCES if we try to open a file on a CDROM that isn't
		 * ready, instead of something like ENODEV.  We want to return that case as
		 * ENOENT, but we can't distinguish it from file permission errors. */
	}
	else
	{
		CString out;
		if( mode & RageFile::STREAMED )
			out = sPath;
		else
			out = MakeTempFilename(sPath);

		/* Open a temporary file for writing. */
		fd = DoOpen( out, O_BINARY|O_WRONLY|O_CREAT|O_TRUNC, 0666 );
	}

	if( fd == -1 )
	{
		err = errno;
		return NULL;
	}

	return new RageFileObjDirect( sPath, fd, mode );
}

RageFileBasic *RageFileDriverDirect::Open( const CString &path, int mode, int &err )
{
	ASSERT( path.size() && path[0] == '/' );
	CString sPath = path;

	/* This partially resolves.  For example, if "abc/def" exists, and we're opening
	 * "ABC/DEF/GHI/jkl/mno", this will resolve it to "abc/def/GHI/jkl/mno"; we'll
	 * create the missing parts below. */
	FDB->ResolvePath( sPath );

	if( mode & RageFile::WRITE )
	{
		const CString dir = Dirname(sPath);
		if( this->GetFileType(dir) != RageFileManager::TYPE_DIR )
			CreateDirectories( root + dir );
	}

	return MakeFileObjDirect( root + sPath, mode, err );
}

bool RageFileDriverDirect::Move( const CString &sOldPath_, const CString &sNewPath_ )
{
	CString sOldPath = sOldPath_;
	CString sNewPath = sNewPath_;
	FDB->ResolvePath( sOldPath );
	FDB->ResolvePath( sNewPath );

	if( this->GetFileType(sOldPath) == RageFileManager::TYPE_NONE )
		return false;

        {
		const CString dir = Dirname(sNewPath);
		CreateDirectories( root + dir );
	}

	LOG->Trace("rename \"%s\" -> \"%s\"", (root + sOldPath).c_str(), (root + sNewPath).c_str() );
	if( DoRename(root + sOldPath, root + sNewPath) == -1 )
	{
		LOG->Warn( "rename(%s,%s) failed: %s", (root + sOldPath).c_str(), (root + sNewPath).c_str(), strerror(errno) );
		return false;
	}

	return true;
}

bool RageFileDriverDirect::Remove( const CString &path )
{
	CString sPath = path;
	FDB->ResolvePath( sPath );
	switch( this->GetFileType(sPath) )
	{
	case RageFileManager::TYPE_FILE:
		LOG->Trace("remove '%s'", (root + sPath).c_str());
		if( DoRemove( root + sPath ) == -1 )
		{
			LOG->Warn( "remove(%s) failed: %s", (root + sPath).c_str(), strerror(errno) );
			return false;
		}
		FDB->DelFile( sPath );
		return true;

	case RageFileManager::TYPE_DIR:
		LOG->Trace("rmdir '%s'", (root + sPath).c_str());
		if( DoRmdir( root + sPath ) == -1 )
		{
			LOG->Warn( "rmdir(%s) failed: %s", (root + sPath).c_str(), strerror(errno) );
			return false;
		}
		FDB->DelFile( sPath );
		return true;

	case RageFileManager::TYPE_NONE: return false;
	default: ASSERT(0); return false;
	}
}

RageFileBasic *RageFileObjDirect::Copy() const
{
	int iErr;
	RageFileObj *ret = MakeFileObjDirect( m_sPath, m_iMode, iErr );

	if( ret == NULL )
		RageException::Throw( "Couldn't reopen \"%s\": %s", m_sPath.c_str(), strerror(iErr) );

	ret->Seek( lseek( m_iFD, 0, SEEK_CUR ) );

	return ret;
}

bool RageFileDriverDirect::Remount( const CString &sPath )
{
	root = sPath;
	((DirectFilenameDB *) FDB)->SetRoot( sPath );

	/* If the root path doesn't exist, create it. */
	CreateDirectories( root );

	return true;
}

static const unsigned int BUFSIZE = 1024*64;
RageFileObjDirect::RageFileObjDirect( const CString &sPath, int iFD, int iMode )
{
	m_sPath = sPath;
	m_iFD = iFD;
	m_iMode = iMode;
	ASSERT( m_iFD != -1 );

	if( m_iMode & RageFile::WRITE )
		m_sWriteBuf.reserve( BUFSIZE );
}

bool RageFileObjDirect::FinalFlush()
{
	if( !(m_iMode & RageFile::WRITE) )
		return true;

	/* Flush the output buffer. */
	if( Flush() == -1 )
		return false;

	/* Only do the rest of the flushes if SLOW_FLUSH is enabled. */
	if( !(m_iMode & RageFile::SLOW_FLUSH) )
		return true;
	
	/* Force a kernel buffer flush. */
	if( fsync( m_iFD ) == -1 )
	{
		LOG->Warn( "Error synchronizing %s: %s", this->m_sPath.c_str(), strerror(errno) );
		SetError( strerror(errno) );
		return false;
	}

#if !defined(WIN32)
	/* Wait for the directory to be flushed. */
	int dirfd = open( Dirname(m_sPath), O_RDONLY );
	if( dirfd == -1 )
	{
		LOG->Warn( "Error synchronizing open(%s dir): %s", this->m_sPath.c_str(), strerror(errno) );
		SetError( strerror(errno) );
		return false;
	}

	if( fsync( dirfd ) == -1 )
	{
		LOG->Warn( "Error synchronizing fsync(%s dir): %s", this->m_sPath.c_str(), strerror(errno) );
		SetError( strerror(errno) );
		close( dirfd );
		return false;
	}

	close( dirfd );
#endif

	return true;
}

RageFileObjDirect::~RageFileObjDirect()
{
	bool failed = !FinalFlush();
	
	if( m_iFD != -1 )
	{
		if( close( m_iFD ) == -1 )
		{
			LOG->Warn( "Error closing %s: %s", this->m_sPath.c_str(), strerror(errno) );
			SetError( strerror(errno) );
			failed = true;
		}
	}

	if( !(m_iMode & RageFile::WRITE) || (m_iMode & RageFile::STREAMED) )
		return;

	/* If we failed to flush the file properly, something's amiss--don't touch the original file! */
	if( !failed )
	{
		/*
		 * We now have path written to MakeTempFilename(m_sPath).  Rename the temporary
		 * file over the real path.  This should be an atomic operation with a journalling
		 * filesystem.  That is, there should be no intermediate state a JFS might restore
		 * the file we're writing (in the case of a crash/powerdown) to an empty or partial
		 * file.
		 *
		 * If we want to keep a backup, we can move the old file to "path.old" and then
		 * the new file to "path".  However, this leaves an intermediate state between
		 * the renames where no "path" exists, so if we're restored to that, then we
		 * won't be able to find the file.  The data is still there--as path.old, provided
		 * it's not overwritten again--but the user will have to recover it on his own.
		 * A safer (but much slower) way to do this is to simply CopyFile a backup first.
		 */

		CString sOldPath = MakeTempFilename(m_sPath);
		CString sNewPath = m_sPath;

#if defined(WIN32)
		if( WinMoveFile(DoPathReplace(sOldPath), DoPathReplace(sNewPath)) )
			return;

		/* We failed. */
		int err = GetLastError();
		const CString error = werr_ssprintf( err, "Error renaming \"%s\" to \"%s\"", sOldPath.c_str(), sNewPath.c_str() );
		LOG->Warn( "%s", error.c_str() );
		SetError( error );
#else
		if( rename( sOldPath, sNewPath ) == -1 )
		{
			LOG->Warn( "Error renaming \"%s\" to \"%s\": %s", 
				sOldPath.c_str(), sNewPath.c_str(), strerror(errno) );
			SetError( strerror(errno) );
		}
#endif
	}
}

int RageFileObjDirect::ReadInternal( void *pBuf, size_t iBytes )
{
	int iRet = read( m_iFD, pBuf, iBytes );
	if( iRet == -1 )
	{
		SetError( strerror(errno) );
		return -1;
	}

	return iRet;
}

/* write(), but retry a couple times on EINTR. */
static int retried_write( int iFD, const void *pBuf, size_t iCount )
{
	int tries = 3, ret;
	do
	{
		ret = write( iFD, pBuf, iCount );
	}
	while( ret == -1 && errno == EINTR && tries-- );

	return ret;
}


int RageFileObjDirect::FlushInternal()
{
	if( !m_sWriteBuf.size() )
		return 0;

	int iRet = retried_write( m_iFD, m_sWriteBuf.data(), m_sWriteBuf.size() );
	if( iRet == -1 )
	{
		LOG->Warn("Error writing %s: %s", this->m_sPath.c_str(), strerror(errno) );
		SetError( strerror(errno) );
		return -1;
	}

	m_sWriteBuf.erase();
	m_sWriteBuf.reserve( BUFSIZE );
	return iRet;
}

int RageFileObjDirect::WriteInternal( const void *pBuf, size_t iBytes )
{
	if( m_sWriteBuf.size()+iBytes > BUFSIZE )
	{
		if( Flush() == -1 )
			return -1;
		ASSERT( !m_sWriteBuf.size() );

		/* The buffer is cleared.  If we still don't have space, it's bigger than
		 * the buffer size, so just write it directly. */
		if( iBytes >= BUFSIZE )
		{
			int iRet = retried_write( m_iFD, pBuf, iBytes );
			if( iRet == -1 )
			{
				LOG->Warn("Error writing %s: %s", this->m_sPath.c_str(), strerror(errno) );
				SetError( strerror(errno) );
				return -1;
			}
			return iBytes;
		}
	}

	m_sWriteBuf.append( (const char *) pBuf, (const char *) pBuf+iBytes );
	return iBytes;
}

int RageFileObjDirect::SeekInternal( int iOffset )
{
	return lseek( m_iFD, iOffset, SEEK_SET );
}

int RageFileObjDirect::GetFileSize() const
{
	const int iOldPos = lseek( m_iFD, 0, SEEK_CUR );
	ASSERT_M( iOldPos != -1, strerror(errno) );
	const int iRet = lseek( m_iFD, 0, SEEK_END );
	ASSERT_M( iRet != -1, strerror(errno) );
	lseek( m_iFD, iOldPos, SEEK_SET );
	return iRet;
}

/*
 * Copyright (c) 2003-2004 Glenn Maynard, Chris Danford
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

