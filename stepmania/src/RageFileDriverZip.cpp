#include "global.h"
#include "RageFileDriverZip.h"
#include "RageUtil.h"
#include "RageUtil_FileDB.h"
#include <errno.h>
#include <zzip/zzip.h>

#if defined(_WINDOWS)
	#ifdef DEBUG
	#pragma comment(lib, "zzip/zziplibd.lib")
	#else
	#pragma comment(lib, "zzip/zziplib.lib")
	#endif	
#endif
#pragma comment(lib, "zzip/zlib.lib")

class RageFileObjZip: public RageFileObj
{
private:
	ZZIP_FILE *fp;
	RageFileDriverZip *dir; /* hack: for errors */

public:
	RageFileObjZip( ZZIP_FILE *fp, RageFileDriverZip *dir, RageFile &p );
	virtual ~RageFileObjZip();
	virtual int Read(void *buffer, size_t bytes);
	virtual int Write(const void *buffer, size_t bytes);
	virtual void Rewind();
	virtual int Seek( int offset );
	virtual int SeekCur( int offset );
	virtual int GetFileSize();
};

class ZipFilenameDB: public FilenameDB
{
protected:
	void PopulateFileSet( FileSet &fs, const CString &sPath ) { }
public:
	ZipFilenameDB() { ExpireSeconds = -1; }
};


RageFileDriverZip::RageFileDriverZip( CString path ):
	RageFileDriver( new ZipFilenameDB )
{
	zzip_error_t err;
	dir = zzip_dir_open( path, &err );
	if( dir == NULL )
		RageException::Throw( "Couldn't open %s: %s", path.c_str(), zzip_strerror(err) );

	while( ZZIP_DIRENT *ent = zzip_readdir(dir) )
		FDB->AddFile( ent->d_name, ent->st_size, 0 );

	zzip_rewinddir( dir );
}

RageFileDriverZip::~RageFileDriverZip()
{
	zzip_dir_close( dir );
}

RageFileObj *RageFileDriverZip::Open( const CString &path, RageFile::OpenMode mode, RageFile &p, int &err )
{
	if( mode == RageFile::WRITE )
	{
		err = EINVAL;
		return NULL;
	}

	LockMut( lock );

	ZZIP_FILE *fp = zzip_file_open( dir, path, ZZIP_CASELESS|ZZIP_ONLYZIP );

	if( !fp )
	{
		err = zzip_errno( zzip_error(dir) );
		return NULL;
	}

	return new RageFileObjZip( fp, this, p );
}

int RageFileDriverZip::GetFileModTime( const CString &sPath )
{
	/* XXX: we can use CRC instead */
	/* Don't spend time looking up the real mod time; use the mtime of the ZIP itself. */
	return FDB->GetFileModTime( sPath );
}

/* NOP for now.  This could check to see if the ZIP's mtime has changed, and reload. */
void RageFileDriverZip::FlushDirCache( const CString &sPath )
{

}

RageFileObjZip::RageFileObjZip( ZZIP_FILE *fp_, RageFileDriverZip *dir_, RageFile &p ):
	RageFileObj( p )
{
	fp=fp_;
	dir=dir_;
}

RageFileObjZip::~RageFileObjZip()
{
	LockMut( dir->lock );
	zzip_file_close( fp );
}

int RageFileObjZip::Read( void *buf, size_t bytes )
{
	LockMut( dir->lock );

	const int ret = zzip_file_read( fp, (char *) buf, bytes );
	if( ret == -1 )
	{
		SetError( zzip_strerror_of(dir->dir) );
		return -1;
	}

	return ret;
}

int RageFileObjZip::Write(const void *buffer, size_t bytes)
{
	SetError( "Not implemented" );
	return -1;
}

void RageFileObjZip::Rewind()
{
	LockMut( dir->lock );
	zzip_rewind( fp );
}

int RageFileObjZip::Seek( int offset )
{
	LockMut( dir->lock );
	return zzip_seek( fp, offset, SEEK_SET );
}

int RageFileObjZip::SeekCur( int offset )
{
	LockMut( dir->lock );
	return zzip_seek( fp, offset, SEEK_CUR );
}

int RageFileObjZip::GetFileSize()
{
	LockMut( dir->lock );

	/* XXX very slow */
	const int OldPos = zzip_tell( fp );
	int ret = zzip_seek( fp, 0, SEEK_END );
	zzip_seek( fp, OldPos, SEEK_SET );
	return ret;
}
