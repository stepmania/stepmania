#include "global.h"
#include "RageFileDriverDirect.h"
#include "RageUtil.h"
#include "RageUtil_FileDB.h"
#include "RageLog.h"

#include <fcntl.h>
#include <errno.h>
#include <sys/types.h>
#include <sys/stat.h>

#if !defined(WIN32)
#include <dirent.h>
#include <fcntl.h>
#else
#include <windows.h>
#include <io.h>
#endif

#if !defined(O_BINARY)
#define O_BINARY 0
#endif

#if defined(XBOX)
/* Wrappers for low-level file functions, to work around Xbox issues: */
static int DoMkdir( const CString &sPath, int perm )
{
	CString TempPath = sPath;
	TempPath.Replace( "/", "\\" );
	return mkdir( TempPath );
}

static int DoOpen( const CString &sPath, int flags, int perm )
{
	CString TempPath = sPath;
	TempPath.Replace( "/", "\\" );
	return open( TempPath, flags, perm );
}

static int DoStat( const CString &sPath, struct stat *st )
{
	CString TempPath = sPath;
	TempPath.Replace( "/", "\\" );
	return stat( sPath, st );
}

static HANDLE DoFindFirstFile( const CString &sPath, WIN32_FIND_DATA *fd )
{
	CString TempPath = sPath;
	TempPath.Replace( "/", "\\" );
	return FindFirstFile( TempPath, fd );
}

#else
#define DoOpen open
#define DoStat stat
#define DoMkdir mkdir
#define DoFindFirstFile FindFirstFile
#endif

#if defined(WIN32)

static int WinMoveFile( const CString sOldPath, const CString sNewPath )
{
	static bool Win9x = false;

	if( !Win9x )
	{
		if( MoveFileEx( sOldPath, sNewPath, MOVEFILE_REPLACE_EXISTING ) )
			return 1;

		if( GetLastError() == ERROR_NOT_SUPPORTED || GetLastError() == ERROR_CALL_NOT_IMPLEMENTED )
			return 0;

		Win9x = true;
	}

	if( MoveFile( sOldPath, sNewPath ) )
		return 1;
	
	if( GetLastError() != ERROR_ALREADY_EXISTS )
		return 0;

	if( !DeleteFile( sNewPath ) )
		return 0;

	return MoveFile( sOldPath, sNewPath );
}
#endif

static int DoRemove( const CString &sPath )
{
#if defined(XBOX)
	CString TempPath = sPath;
	TempPath.Replace( "/", "\\" );
	return remove( sPath );
#else
	return remove( sPath );
#endif
}

static int DoRmdir( const CString &sPath )
{
#if defined(XBOX)
	CString TempPath = sPath;
	TempPath.Replace( "/", "\\" );
	return rmdir( sPath );
#else
	return rmdir( sPath );
#endif
}


/* This driver handles direct file access. */
class DirectFilenameDB: public FilenameDB
{
protected:
	virtual void PopulateFileSet( FileSet &fs, const CString &sPath );
	CString root;

public:
	DirectFilenameDB( CString root_ )
	{
		ExpireSeconds = 30;
		root = root_;
		if( root.Right(1) != "/" )
			root += '/';
		if( root == "./" )
			root = "";
	}
};


void DirectFilenameDB::PopulateFileSet( FileSet &fs, const CString &path )
{
	CString sPath = path;

	/* Resolve path cases (path/Path -> PATH/path). */
	ResolvePath( sPath );

	fs.age.GetDeltaTime(); /* reset */
	fs.files.clear();

#if defined(WIN32)
	WIN32_FIND_DATA fd;

	if ( sPath.size() > 0  && sPath.Right(1) == "/" )
		sPath.erase( sPath.size() - 1 );

	HANDLE hFind = DoFindFirstFile( root+sPath+"/*", &fd );

	if( hFind == INVALID_HANDLE_VALUE )
		return;

	do {
		if(!strcmp(fd.cFileName, ".") || !strcmp(fd.cFileName, ".."))
			continue;

		File f;
		f.SetName( fd.cFileName );
		f.dir = !!(fd.dwFileAttributes & FILE_ATTRIBUTE_DIRECTORY);
		f.size = fd.nFileSizeLow;
		f.hash = fd.ftLastWriteTime.dwLowDateTime;

		fs.files.insert(f);
	} while( FindNextFile( hFind, &fd ) );
	FindClose(hFind);
#else
	int OldDir = DoOpen(".", O_RDONLY);
	if( OldDir == -1 )
		RageException::Throw( "Couldn't open(.): %s", strerror(errno) );

	if( chdir(root+sPath) == -1 )
	{
		/* Only log once per dir. */
		if( LOG )
			LOG->MapLog("chdir " + sPath, "Couldn't chdir(%s): %s", sPath.c_str(), strerror(errno) );
		close( OldDir );
		return;
	}
	DIR *d = opendir(".");

	while(struct dirent *ent = readdir(d))
	{
		if(!strcmp(ent->d_name, ".")) continue;
		if(!strcmp(ent->d_name, "..")) continue;
		
		File f;
		f.SetName( ent->d_name );
		
		struct stat st;
		if( DoStat(ent->d_name, &st) == -1 )
		{
			/* If it's a broken symlink, ignore it.  Otherwise, warn. */
			if( lstat(ent->d_name, &st) == 0 )
				continue;
			
			/* Huh? */
			if(LOG)
				LOG->Warn("Got file '%s' in '%s' from list, but can't stat? (%s)",
					ent->d_name, sPath.c_str(), strerror(errno));
			continue;
		} else {
			f.dir = (st.st_mode & S_IFDIR);
			f.size = st.st_size;
			f.hash = st.st_mtime;
		}

		fs.files.insert(f);
	}
	       
	closedir(d);
	if( fchdir( OldDir ) == -1 )
		RageException::Throw( "Couldn't fchdir(): %s", strerror(errno) );
	close( OldDir );
#endif
}

class RageFileObjDirect: public RageFileObj
{
private:
	int fd;
	CString path; /* for Copy */

	CString write_buf;

public:
	RageFileObjDirect( const CString &path, int fd_, RageFile &p );
	virtual ~RageFileObjDirect();
	virtual int Read(void *buffer, size_t bytes);
	virtual int Write(const void *buffer, size_t bytes);
	virtual int Flush();
	virtual void Rewind();
	virtual int Seek( int offset );
	virtual int GetFileSize();
	virtual RageFileObj *Copy( RageFile &p ) const;
	virtual CString GetDisplayPath() const { return path; }
};


RageFileDriverDirect::RageFileDriverDirect( CString root_ ):
	RageFileDriver( new DirectFilenameDB(root_) ),
	root(root_)
{
	if( root.Right(1) != "/" )
		root += '/';
}

/* mkdir -p.  Doesn't fail if Path already exists and is a directory. */
static bool CreateDirectories( CString Path )
{
	CStringArray parts;
	CString curpath;
	split(Path, "/", parts);

	for(unsigned i = 0; i < parts.size(); ++i)
	{
		curpath += parts[i] + "/";

#if defined(WIN32)
		if( i == 0 && curpath.size() > 1 && curpath[1] == ':' )
		{
			/* Don't try to create the drive letter alone. */
			continue;
		}
#endif

		if( DoMkdir(curpath, 0755) == 0 )
			continue;

		if(errno == EEXIST)
			continue;		// we expect to see this error

		// Log the error, but continue on.
		/* When creating a directory that already exists over Samba, Windows is
		 * returning ENOENT instead of EEXIST. */
		if( LOG )
			LOG->Warn("Couldn't create %s: %s", curpath.c_str(), strerror(errno) );

		/* Make sure it's a directory. */
		struct stat st;
		DoStat( curpath, &st );
		if( !(st.st_mode & S_IFDIR) )
		{
			if( LOG )
				LOG->Warn("Couldn't create %s: path exists and is not a directory", curpath.c_str() );
			
			return false;
		}
	}
	
	return true;
}


static CString MakeTempFilename( const CString &sPath )
{
	/* "Foo/bar/baz" -> "Foo/bar/new.baz".  Prepend to the basename, so if we're
	 * writing something that might be wildcard-searched, these temp files won't
	 * match. */
	return Dirname(sPath) + "new." + Basename(sPath);
}

RageFileObj *MakeFileObjDirect( CString sPath, int mode, RageFile &p, int &err )
{
	int fd;
	if( mode & RageFile::READ )
	{
		fd = DoOpen( sPath, O_BINARY|O_RDONLY, 0644 );
	}
	else
	{
		CString out;
		if( mode & RageFile::STREAMED )
			out = sPath;
		else
			out = MakeTempFilename(sPath);

		/* Open a temporary file for writing. */
		fd = DoOpen( out, O_BINARY|O_WRONLY|O_CREAT|O_TRUNC, 0644 );
	}

	if( fd == -1 )
	{
		err = errno;
		return NULL;
	}

	return new RageFileObjDirect( sPath, fd, p );
}

RageFileObj *RageFileDriverDirect::Open( const CString &path, int mode, RageFile &p, int &err )
{
	CString sPath = path;

	/* This partially resolves.  For example, if "abc/def" exists, and we're opening
	 * "ABC/DEF/GHI/jkl/mno", this will resolve it to "abc/def/GHI/jkl/mno"; we'll
	 * create the missing parts below. */
	/* XXX: does it? */
	FDB->ResolvePath( sPath );

	if( mode & RageFile::WRITE )
	{
		const CString dir = Dirname(sPath);
		if( this->GetFileType(dir) != RageFileManager::TYPE_DIR )
			CreateDirectories( root + dir );
	}

	return MakeFileObjDirect( root + sPath, mode, p, err );
}

bool RageFileDriverDirect::Remove( const CString &path )
{
	CString sPath = path;
	FDB->ResolvePath( sPath );
	switch( this->GetFileType(sPath) )
	{
	case TTYPE_FILE:
		LOG->Trace("remove '%s'", (root + sPath).c_str());
		if( DoRemove( root + sPath ) == -1 )
		{
			LOG->Warn("remove(%s) failed: %s",
				(root + sPath).c_str(), strerror(errno) );
			return false;
		}
		FDB->DelFile( sPath );
		return true;

	case TTYPE_DIR:
		LOG->Trace("rmdir '%s'", (root + sPath).c_str());
		if( DoRmdir( root + sPath ) == -1 )
		{
			LOG->Warn("rmdir(%s) failed: %s",
				(root + sPath).c_str(), strerror(errno) );
			return false;
		}
		FDB->DelFile( sPath );
		return true;

	case TTYPE_NONE: return false;
	default: ASSERT(0); return false;
	}
}

RageFileObj *RageFileObjDirect::Copy( RageFile &p ) const
{
	int err;
	RageFileObj *ret = MakeFileObjDirect( path, parent.GetOpenMode(), p, err );

	if( ret == NULL )
		RageException::Throw("Couldn't reopen \"%s\": %s", path.c_str(), strerror(err) );

	ret->Seek( parent.Tell() );

	return ret;
}

#ifdef _WINDOWS
#include "windows.h"
#endif
bool RageFileDriverDirect::Ready()
{
#ifdef _WINDOWS
	// Windows will throw up a message box if we try to write to a
	// removable drive with no disk inserted.  Find out whether there's a 
	// disk in the drive w/o writing a file.

	// find drive letter
	vector<CString> matches;
	static Regex parse("^([A-Za-z]+):");
	parse.Compare( root, matches );
	if( matches.size() != 1 )
		return false;

	CString sDrive = matches[0];
	TCHAR szVolumeNameBuffer[MAX_PATH];
	DWORD dwVolumeSerialNumber;
	DWORD dwMaximumComponentLength;
	DWORD lpFileSystemFlags;
	TCHAR szFileSystemNameBuffer[MAX_PATH];
	BOOL bResult = GetVolumeInformation( 
		sDrive + ":\\",
		szVolumeNameBuffer,
		sizeof(szVolumeNameBuffer),
		&dwVolumeSerialNumber,
		&dwMaximumComponentLength,
		&lpFileSystemFlags,
		szFileSystemNameBuffer,
		sizeof(szFileSystemNameBuffer) );
	return !!bResult;
#else
	// Try to create directory before writing a temp file.
	CreateDirectories( root );
	
	// Try to write a file.
	const CString sFile = root + "temp";
	int fd = DoOpen( sFile, O_WRONLY|O_CREAT|O_TRUNC );
	if( fd == -1 )
		return false;

	close( fd );
	remove( sFile );
	return true;
#endif
}

static const unsigned int BUFSIZE = 1024*64;
RageFileObjDirect::RageFileObjDirect( const CString &path_, int fd_, RageFile &p ):
	RageFileObj( p )
{
	path = path_;
	fd = fd_;
	ASSERT( fd != -1 );

	if( parent.GetOpenMode() & RageFile::WRITE )
		write_buf.reserve( BUFSIZE );
}

RageFileObjDirect::~RageFileObjDirect()
{
	bool failed = false;
	if( parent.GetOpenMode() & RageFile::WRITE )
	{
		/* Flush the output buffer. */
		Flush();

 		if( !(parent.GetOpenMode() & RageFile::STREAMED) )
		{
			/* Force a kernel buffer flush. */
			if( fsync( fd ) == -1 )
			{
				LOG->Warn("Error synchronizing %s: %s", this->path.c_str(), strerror(errno) );
				SetError( strerror(errno) );
				failed = true;
			}
		}
	}

	if( fd != -1 )
	{
		if( close( fd ) == -1 )
		{
			LOG->Warn("Error closing %s: %s", this->path.c_str(), strerror(errno) );
			SetError( strerror(errno) );
			failed = true;
		}
	}

	/* If we failed to flush the file properly, something's amiss--don't touch the original file! */
	if( !failed &&
		 (parent.GetOpenMode()&RageFile::WRITE) &&
		!(parent.GetOpenMode() & RageFile::STREAMED) )
	{
		/*
		 * We now have path written to MakeTempFilename(path).  Rename the temporary
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

		CString sOldPath = MakeTempFilename(path);
		CString sNewPath = path;

#if defined(XBOX)
		sOldPath.Replace( "/", "\\" );
		sNewPath.Replace( "/", "\\" );
#endif

#if defined(WIN32)
		/* Windows botches rename: it returns error if the file exists.  In NT,
		 * we can use MoveFileEx( new, old, MOVEFILE_REPLACE_EXISTING ) (though I
		 * don't know if it has similar atomicity guarantees to rename).  In
		 * 9x, we're screwed, so just delete any existing file (we aren't going
		 * to be robust on 9x anyway). */
		if( WinMoveFile(sOldPath, sNewPath) )
			return;
		if( GetLastError() == ERROR_ACCESS_DENIED )
		{
			/* Try turning off the read-only bit on the file we're overwriting. */
			SetFileAttributes( sNewPath, FILE_ATTRIBUTE_NORMAL );

			if( WinMoveFile( sOldPath, sNewPath ) )
				return;
		}

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

int RageFileObjDirect::Read( void *buf, size_t bytes )
{
	int ret = read( fd, buf, bytes );
	if( ret == -1 )
	{
		SetError( strerror(errno) );
		return -1;
	}

	return ret;
}

int RageFileObjDirect::Flush()
{
	if( !write_buf.size() )
		return 0;

	int ret = write( fd, write_buf.data(), write_buf.size() );
	if( ret == -1 )
	{
		LOG->Warn("Error writing %s: %s", this->path.c_str(), strerror(errno) );
		SetError( strerror(errno) );
	}

	write_buf.erase();
	write_buf.reserve( BUFSIZE );
	return ret;
}

int RageFileObjDirect::Write( const void *buf, size_t bytes )
{
	if( write_buf.size()+bytes > BUFSIZE )
	{
		if( Flush() == -1 )
			return -1;
		ASSERT(	!write_buf.size() );

		/* The buffer is cleared.  If we still don't have space, it's bigger than
		 * the buffer size, so just write it directly. */
		if( bytes >= BUFSIZE )
		{
			int ret = write( fd, buf, bytes );
			if( ret == -1 )
			{
				LOG->Warn("Error writing %s: %s", this->path.c_str(), strerror(errno) );
				SetError( strerror(errno) );
				return -1;
			}
			return bytes;
		}
	}

	write_buf.append( (const char *)buf, (const char *)buf+bytes );
	return bytes;
}

void RageFileObjDirect::Rewind()
{
	lseek( fd, 0, SEEK_SET );
}

int RageFileObjDirect::Seek( int offset )
{
	return lseek( fd, offset, SEEK_SET );
}

int RageFileObjDirect::GetFileSize()
{
	const int OldPos = lseek( fd, 0, SEEK_CUR );
	const int ret = lseek( fd, 0, SEEK_END );
	lseek( fd, OldPos, SEEK_SET );
	return ret;
}

/*
 * Copyright (c) 2003 by the person(s) listed below.  All rights reserved.
 *   Glenn Maynard
 *   Chris Danford
 */

