#include "global.h"
#include "RageFileDriverDirect.h"
#include "RageFileDriverDirectHelpers.h"
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
	virtual RageFileObj *Copy( RageFile &p ) const;
	virtual CString GetDisplayPath() const { return path; }
	virtual int GetFileSize();
};


RageFileDriverDirect::RageFileDriverDirect( CString root_ ):
	RageFileDriver( new DirectFilenameDB(root_) ),
	root(root_)
{
	if( root.Right(1) != "/" )
		root += '/';
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
	case RageFileManager::TYPE_FILE:
		LOG->Trace("remove '%s'", (root + sPath).c_str());
		if( DoRemove( root + sPath ) == -1 )
		{
			LOG->Warn("remove(%s) failed: %s",
				(root + sPath).c_str(), strerror(errno) );
			return false;
		}
		FDB->DelFile( sPath );
		return true;

	case RageFileManager::TYPE_DIR:
		LOG->Trace("rmdir '%s'", (root + sPath).c_str());
		if( DoRmdir( root + sPath ) == -1 )
		{
			LOG->Warn("rmdir(%s) failed: %s",
				(root + sPath).c_str(), strerror(errno) );
			return false;
		}
		FDB->DelFile( sPath );
		return true;

	case RageFileManager::TYPE_NONE: return false;
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

bool RageFileDriverDirect::Ready()
{
	return PathReady( root );
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

#if defined(WIN32)
		if( WinMoveFile(sOldPath, sNewPath) )
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
