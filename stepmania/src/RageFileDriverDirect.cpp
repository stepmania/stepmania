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
#include "archutils/Win32/ErrorStrings.h"
#if !defined(_XBOX)
#include <windows.h>
#endif
#include <io.h>
#endif

static struct FileDriverEntry_DIR: public FileDriverEntry
{
	FileDriverEntry_DIR(): FileDriverEntry( "DIR" ) { }
	RageFileDriver *Create( const RString &sRoot ) const { return new RageFileDriverDirect( sRoot ); }
} const g_RegisterDriver;

/* This driver handles direct file access. */

class RageFileObjDirect: public RageFileObj
{
public:
	RageFileObjDirect( const RString &sPath, int iFD, int iMode );
	virtual ~RageFileObjDirect();
	virtual int ReadInternal( void *pBuffer, size_t iBytes );
	virtual int WriteInternal( const void *pBuffer, size_t iBytes );
	virtual int FlushInternal();
	virtual int SeekInternal( int offset );
	virtual RageFileObjDirect *Copy() const;
	virtual RString GetDisplayPath() const { return m_sPath; }
	virtual int GetFileSize() const;

private:
	bool FinalFlush();

	int m_iFD;
	int m_iMode;
	RString m_sPath; /* for Copy */
	
	/*
	 * When not streaming to disk, we write to a temporary file, and rename to the
	 * real file on completion.  If any write, this is aborted.  When streaming to
	 * disk, allow recovering from errors.
	 */
	bool m_bWriteFailed;
	bool WriteFailed() const { return !(m_iMode & RageFile::STREAMED) && m_bWriteFailed; }
};


RageFileDriverDirect::RageFileDriverDirect( const RString &sRoot ):
	RageFileDriver( new DirectFilenameDB(sRoot) )
{
	Remount( sRoot );
}


static RString MakeTempFilename( const RString &sPath )
{
	/* "Foo/bar/baz" -> "Foo/bar/new.baz.new".  Both prepend and append: we don't
	 * want a wildcard search for the filename to match (foo.txt.new matches foo.txt*),
	 * and we don't want to have the same extension (so "new.foo.sm" doesn't show up
	 * in *.sm). */
	return Dirname(sPath) + "new." + Basename(sPath) + ".new";
}

RageFileObjDirect *MakeFileObjDirect( RString sPath, int iMode, int &iError )
{
	int iFD;
	if( iMode & RageFile::READ )
	{
		iFD = DoOpen( sPath, O_BINARY|O_RDONLY, 0666 );

		/* XXX: Windows returns EACCES if we try to open a file on a CDROM that isn't
		 * ready, instead of something like ENODEV.  We want to return that case as
		 * ENOENT, but we can't distinguish it from file permission errors. */
	}
	else
	{
		RString sOut;
		if( iMode & RageFile::STREAMED )
			sOut = sPath;
		else
			sOut = MakeTempFilename(sPath);

		/* Open a temporary file for writing. */
		iFD = DoOpen( sOut, O_BINARY|O_WRONLY|O_CREAT|O_TRUNC, 0666 );
	}

	if( iFD == -1 )
	{
		iError = errno;
		return NULL;
	}

	return new RageFileObjDirect( sPath, iFD, iMode );
}

RageFileBasic *RageFileDriverDirect::Open( const RString &sPath_, int iMode, int &iError )
{
	RString sPath = sPath_;
	ASSERT( sPath.size() && sPath[0] == '/' );

	/* This partially resolves.  For example, if "abc/def" exists, and we're opening
	 * "ABC/DEF/GHI/jkl/mno", this will resolve it to "abc/def/GHI/jkl/mno"; we'll
	 * create the missing parts below. */
	FDB->ResolvePath( sPath );

	if( iMode & RageFile::WRITE )
	{
		const RString dir = Dirname(sPath);
		if( this->GetFileType(dir) != RageFileManager::TYPE_DIR )
			CreateDirectories( m_sRoot + dir );
	}

	return MakeFileObjDirect( m_sRoot + sPath, iMode, iError );
}

bool RageFileDriverDirect::Move( const RString &sOldPath_, const RString &sNewPath_ )
{
	RString sOldPath = sOldPath_;
	RString sNewPath = sNewPath_;
	FDB->ResolvePath( sOldPath );
	FDB->ResolvePath( sNewPath );

	if( this->GetFileType(sOldPath) == RageFileManager::TYPE_NONE )
		return false;

	{
		const RString sDir = Dirname(sNewPath);
		CreateDirectories( m_sRoot + sDir );
	}

	LOG->Trace("rename \"%s\" -> \"%s\"", (m_sRoot + sOldPath).c_str(), (m_sRoot + sNewPath).c_str() );
	if( DoRename(m_sRoot + sOldPath, m_sRoot + sNewPath) == -1 )
	{
		LOG->Warn( "rename(%s,%s) failed: %s", (m_sRoot + sOldPath).c_str(), (m_sRoot + sNewPath).c_str(), strerror(errno) );
		return false;
	}

	return true;
}

bool RageFileDriverDirect::Remove( const RString &sPath_ )
{
	RString sPath = sPath_;
	FDB->ResolvePath( sPath );
	switch( this->GetFileType(sPath) )
	{
	case RageFileManager::TYPE_FILE:
		LOG->Trace("remove '%s'", (m_sRoot + sPath).c_str());
		if( DoRemove(m_sRoot + sPath) == -1 )
		{
			LOG->Warn( "remove(%s) failed: %s", (m_sRoot + sPath).c_str(), strerror(errno) );
			return false;
		}
		FDB->DelFile( sPath );
		return true;

	case RageFileManager::TYPE_DIR:
		LOG->Trace("rmdir '%s'", (m_sRoot + sPath).c_str());
		if( DoRmdir(m_sRoot + sPath) == -1 )
		{
			LOG->Warn( "rmdir(%s) failed: %s", (m_sRoot + sPath).c_str(), strerror(errno) );
			return false;
		}
		FDB->DelFile( sPath );
		return true;

	case RageFileManager::TYPE_NONE: return false;
	default: ASSERT(0); return false;
	}
}

RageFileObjDirect *RageFileObjDirect::Copy() const
{
	int iErr;
	RageFileObjDirect *ret = MakeFileObjDirect( m_sPath, m_iMode, iErr );

	if( ret == NULL )
		RageException::Throw( "Couldn't reopen \"%s\": %s", m_sPath.c_str(), strerror(iErr) );

	ret->Seek( lseek( m_iFD, 0, SEEK_CUR ) );

	return ret;
}

bool RageFileDriverDirect::Remount( const RString &sPath )
{
	m_sRoot = sPath;
	((DirectFilenameDB *) FDB)->SetRoot( sPath );

	/* If the root path doesn't exist, create it. */
	CreateDirectories( m_sRoot );

	return true;
}

static const unsigned int BUFSIZE = 1024*64;
RageFileObjDirect::RageFileObjDirect( const RString &sPath, int iFD, int iMode )
{
	m_sPath = sPath;
	m_iFD = iFD;
	m_bWriteFailed = false;
	m_iMode = iMode;
	ASSERT( m_iFD != -1 );

	if( m_iMode & RageFile::WRITE )
		this->EnableWriteBuffering( BUFSIZE );
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
	bool bFailed = !FinalFlush();
	
	if( m_iFD != -1 )
	{
		if( close( m_iFD ) == -1 )
		{
			LOG->Warn( "Error closing %s: %s", this->m_sPath.c_str(), strerror(errno) );
			SetError( strerror(errno) );
			bFailed = true;
		}
	}

	if( !(m_iMode & RageFile::WRITE) || (m_iMode & RageFile::STREAMED) )
		return;

	/* We now have path written to MakeTempFilename(m_sPath).  Rename the temporary
	 * file over the real path. */

	do
	{
		if( bFailed || WriteFailed() )
			break;

		/*
		 * We now have path written to MakeTempFilename(m_sPath).  Rename the temporary
		 * file over the real path.  This should be an atomic operation with a journalling
		 * filesystem.  That is, there should be no intermediate state a JFS might restore
		 * the file we're writing (in the case of a crash/powerdown) to an empty or partial
		 * file.
		 */

		RString sOldPath = MakeTempFilename(m_sPath);
		RString sNewPath = m_sPath;

#if defined(WIN32)
		if( WinMoveFile(DoPathReplace(sOldPath), DoPathReplace(sNewPath)) )
			return;

		/* We failed. */
		int err = GetLastError();
		const RString error = werr_ssprintf( err, "Error renaming \"%s\" to \"%s\"", sOldPath.c_str(), sNewPath.c_str() );
		LOG->Warn( "%s", error.c_str() );
		SetError( error );
		break;
#else
		if( rename( sOldPath, sNewPath ) == -1 )
		{
			LOG->Warn( "Error renaming \"%s\" to \"%s\": %s", 
				sOldPath.c_str(), sNewPath.c_str(), strerror(errno) );
			SetError( strerror(errno) );
			break;
		}
#endif

		/* Success. */
		return;
	} while(0);

	/* The write or the rename failed.  Delete the incomplete temporary file. */
	DoRemove( MakeTempFilename(m_sPath) );
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
static int RetriedWrite( int iFD, const void *pBuf, size_t iCount )
{
	int iTries = 3, iRet;
	do
	{
		iRet = write( iFD, pBuf, iCount );
	}
	while( iRet == -1 && errno == EINTR && iTries-- );

	return iRet;
}


int RageFileObjDirect::FlushInternal()
{
	if( WriteFailed() )
	{
		SetError( "previous write failed" );
		return -1;
	}

	return 0;
}

int RageFileObjDirect::WriteInternal( const void *pBuf, size_t iBytes )
{
	if( WriteFailed() )
	{
		SetError( "previous write failed" );
		return -1;
	}

	/* The buffer is cleared.  If we still don't have space, it's bigger than
	 * the buffer size, so just write it directly. */
	int iRet = RetriedWrite( m_iFD, pBuf, iBytes );
	if( iRet == -1 )
	{
		LOG->Warn("Error writing %s: %s", this->m_sPath.c_str(), strerror(errno) );
		SetError( strerror(errno) );
		m_bWriteFailed = true;
		return -1;
	}
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

