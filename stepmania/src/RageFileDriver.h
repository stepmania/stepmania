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
	virtual RageFileObj *Open( const CString &path, RageFile::OpenMode mode, RageFile &p, int &err ) = 0;
	virtual void GetDirListing( const CString &sPath, CStringArray &AddTo, bool bOnlyDirs, bool bReturnPathToo );
	virtual RageFileManager::FileType GetFileType( const CString &sPath );
	virtual int GetFileSizeInBytes( const CString &sFilePath );
	virtual int GetFileHash( const CString &sPath );
	virtual int GetPathValue( const CString &path );
	virtual bool Ready() { return true; } /* see RageFileManager::MountpointIsReady */
	virtual void FlushDirCache( const CString &sPath );

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
	virtual void Rewind() = 0;
	virtual RageFileObj *Copy( RageFile &p ) const = 0;
};

#endif

