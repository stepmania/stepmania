#include "global.h"
#include "RageFileDriverDirect.h"
#include "RageFileDriverDirectHelpers.h"
#include "RageFile.h"
#include "RageUtil.h"
#include "RageUtil_FileDB.h"
#include "RageLog.h"

#if defined(HAVE_FCNTL_H)
#include <fcntl.h>
#endif
#include <cerrno>
#include <sys/types.h>
#include <sys/stat.h>

#if !defined(WIN32)

#if defined(HAVE_DIRENT_H)
#include <dirent.h>
#endif

#else
#include "archutils/Win32/ErrorStrings.h"
#include <windows.h>
#include <io.h>
#endif // !defined(WIN32)

/* Direct filesystem access: */
static struct FileDriverEntry_DIR: public FileDriverEntry
{
	FileDriverEntry_DIR(): FileDriverEntry( "DIR" ) { }
	RageFileDriver *Create( const RString &sRoot ) const { return new RageFileDriverDirect( sRoot ); }
} const g_RegisterDriver;

/* Direct read-only filesystem access: */
static struct FileDriverEntry_DIRRO: public FileDriverEntry
{
	FileDriverEntry_DIRRO(): FileDriverEntry( "DIRRO" ) { }
	RageFileDriver *Create( const RString &sRoot ) const { return new RageFileDriverDirectReadOnly( sRoot ); }
} const g_RegisterDriver2;

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

static RageFileObjDirect *MakeFileObjDirect( RString sPath, int iMode, int &iError )
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
		return nullptr;
	}

#if defined(UNIX)
	struct stat st;
	if( fstat(iFD, &st) != -1 && (st.st_mode & S_IFDIR) )
	{
		iError = EISDIR;
		close( iFD );
		return nullptr;
	}
#endif

	return new RageFileObjDirect( sPath, iFD, iMode );
}

RageFileBasic *RageFileDriverDirect::Open( const RString &sPath_, int iMode, int &iError )
{
	if( m_sRoot == "(empty)" )
	{
		iError = (iMode & RageFile::WRITE) ? EROFS : ENOENT;
		return nullptr;
	}

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
	if( m_sRoot == "(empty)" )
	{
		return false;
	}

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
	int size = FDB->GetFileSize( sOldPath );
	int hash = FDB->GetFileHash( sOldPath );
	TRACE( ssprintf("rename \"%s\" -> \"%s\"", (m_sRoot + sOldPath).c_str(), (m_sRoot + sNewPath).c_str()) );
	if( DoRename(m_sRoot + sOldPath, m_sRoot + sNewPath) == -1 )
	{
		WARN( ssprintf("rename(%s,%s) failed: %s", (m_sRoot + sOldPath).c_str(), (m_sRoot + sNewPath).c_str(), strerror(errno)) );
		return false;
	}

	FDB->DelFile( sOldPath );
	FDB->AddFile( sNewPath, size, hash, nullptr );
	return true;
}

bool RageFileDriverDirect::Remove( const RString &sPath_ )
{
	if( m_sRoot == "(empty)" )
	{
		return false;
	}

	RString sPath = sPath_;
	FDB->ResolvePath( sPath );
	RageFileManager::FileType type = this->GetFileType(sPath);
	switch( type )
	{
	case RageFileManager::TYPE_FILE:
		TRACE( ssprintf("remove '%s'", (m_sRoot + sPath).c_str()) );
		if( DoRemove(m_sRoot + sPath) == -1 )
		{
			WARN( ssprintf("remove(%s) failed: %s", (m_sRoot + sPath).c_str(), strerror(errno)) );
			return false;
		}
		FDB->DelFile( sPath );
		return true;

	case RageFileManager::TYPE_DIR:
		TRACE( ssprintf("rmdir '%s'", (m_sRoot + sPath).c_str()) );
		if( DoRmdir(m_sRoot + sPath) == -1 )
		{
			WARN( ssprintf("rmdir(%s) failed: %s", (m_sRoot + sPath).c_str(), strerror(errno)) );
			return false;
		}
		FDB->DelFile( sPath );
		return true;

	case RageFileManager::TYPE_NONE:
		return false;

	default:
		FAIL_M(ssprintf("Invalid FileType: %i", type));
	}
}

RageFileObjDirect *RageFileObjDirect::Copy() const
{
	int iErr;
	RageFileObjDirect *ret = MakeFileObjDirect( m_sPath, m_iMode, iErr );

	if( ret == nullptr )
		RageException::Throw( "Couldn't reopen \"%s\": %s", m_sPath.c_str(), strerror(iErr) );

	ret->Seek( (int)lseek( m_iFD, 0, SEEK_CUR ) );

	return ret;
}

bool RageFileDriverDirect::Remount( const RString &sPath )
{
	m_sRoot = sPath;
	((DirectFilenameDB *) FDB)->SetRoot( sPath );

	/* If the root path doesn't exist, create it. */
	if( m_sRoot != "(empty)" )
	{
		CreateDirectories( m_sRoot );
	}

	return true;
}

/* The DIRRO driver is just like DIR, except writes are disallowed. */
RageFileDriverDirectReadOnly::RageFileDriverDirectReadOnly( const RString &sRoot ):
	RageFileDriverDirect( sRoot ) { }
RageFileBasic *RageFileDriverDirectReadOnly::Open( const RString &sPath, int iMode, int &iError )
{
	if( iMode & RageFile::WRITE )
	{
		iError = EROFS;
		return nullptr;
	}

	return RageFileDriverDirect::Open( sPath, iMode, iError );
}
bool RageFileDriverDirectReadOnly::Move( const RString & /* sOldPath */, const RString & /* sNewPath */ ) { return false; }
bool RageFileDriverDirectReadOnly::Remove( const RString & /* sPath */ ) { return false; }

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

namespace
{
#if !defined(WIN32)
	bool FlushDir( RString sPath, RString &sError )
	{
		/* Wait for the directory to be flushed. */
		int dirfd = open( sPath, O_RDONLY );
		if( dirfd == -1 )
		{
			sError = strerror(errno);
			return false;
		}

		if( fsync( dirfd ) == -1 )
		{
			sError = strerror(errno);
			close( dirfd );
			return false;
		}

		close( dirfd );
		return true;
	}
#else
	bool FlushDir( RString /* sPath */, RString & /* sError */ )
	{
		return true;
	}
#endif
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
		WARN( ssprintf("Error synchronizing %s: %s", this->m_sPath.c_str(), strerror(errno)) );
		SetError( strerror(errno) );
		return false;
	}

	RString sError;
	if( !FlushDir(Dirname(m_sPath), sError) )
	{
		WARN( ssprintf("Error synchronizing fsync(%s dir): %s", this->m_sPath.c_str(), sError.c_str()) );
		SetError( sError );
		return false;
	}

	return true;
}

RageFileObjDirect::~RageFileObjDirect()
{
	bool bFailed = !FinalFlush();
	
	if( m_iFD != -1 )
	{
		if( close( m_iFD ) == -1 )
		{
			WARN( ssprintf("Error closing %s: %s", this->m_sPath.c_str(), strerror(errno)) );
			SetError( strerror(errno) );
			bFailed = true;
		}
	}

	if( !(m_iMode & RageFile::WRITE) || (m_iMode & RageFile::STREAMED) )
		return;

	/* We now have path written to MakeTempFilename(m_sPath).
	 * Rename the temporary file over the real path. */

	do
	{
		if( bFailed || WriteFailed() )
			break;

		/* We now have path written to MakeTempFilename(m_sPath). Rename the
		 * temporary file over the real path. This should be an atomic operation
		 * with a journalling filesystem. That is, there should be no
		 * intermediate state a JFS might restore the file we're writing (in the
		 * case of a crash/powerdown) to an empty or partial file. */

		RString sOldPath = MakeTempFilename(m_sPath);
		RString sNewPath = m_sPath;

#if defined(WIN32)
		if( WinMoveFile(DoPathReplace(sOldPath), DoPathReplace(sNewPath)) )
			return;

		/* We failed. */
		int err = GetLastError();
		const RString error = werr_ssprintf( err, "Error renaming \"%s\" to \"%s\"", sOldPath.c_str(), sNewPath.c_str() );
		WARN( ssprintf("%s", error.c_str()) );
		SetError( error );
		break;
#else
		if( rename( sOldPath, sNewPath ) == -1 )
		{
			WARN( ssprintf("Error renaming \"%s\" to \"%s\": %s", 
					sOldPath.c_str(), sNewPath.c_str(), strerror(errno)) );
			SetError( strerror(errno) );
			break;
		}


		if( m_iMode & RageFile::SLOW_FLUSH )
		{
			RString sError;
			if( !FlushDir(Dirname(m_sPath), sError) )
			{
				WARN( ssprintf("Error synchronizing fsync(%s dir): %s", this->m_sPath.c_str(), sError.c_str()) );
				SetError( sError );
			}
		}

		// Success.
		return;
#endif
	} while(0);

	// The write or the rename failed. Delete the incomplete temporary file.
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

// write(), but retry a couple times on EINTR.
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

	/* The buffer is cleared. If we still don't have space, it's bigger than
	 * the buffer size, so just write it directly. */
	int iRet = RetriedWrite( m_iFD, pBuf, iBytes );
	if( iRet == -1 )
	{
		SetError( strerror(errno) );
		m_bWriteFailed = true;
		return -1;
	}
	return iBytes;
}

int RageFileObjDirect::SeekInternal( int iOffset )
{
	return (int)lseek( m_iFD, iOffset, SEEK_SET );
}

int RageFileObjDirect::GetFileSize() const
{
	const int iOldPos = (int)lseek( m_iFD, 0, SEEK_CUR );
	ASSERT_M( iOldPos != -1, ssprintf("\"%s\": %s", m_sPath.c_str(), strerror(errno)) );
	const int iRet = (int)lseek( m_iFD, 0, SEEK_END );
	ASSERT_M( iRet != -1, ssprintf("\"%s\": %s", m_sPath.c_str(), strerror(errno)) );
	lseek( m_iFD, iOldPos, SEEK_SET );
	return iRet;
}

int RageFileObjDirect::GetFD()
{
	return m_iFD;
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

