#ifndef RAGE_FILE_DRIVER_H
#define RAGE_FILE_DRIVER_H

#include "RageFile.h"
#include "RageFileManager.h"

class RageFileObj;
class RageFileDriver
{
public:
	virtual RageFileObj *Open( CString path, RageFile::OpenMode mode, RageFile &p, int &err ) = 0;
	virtual RageFileManager::FileType GetFileType( CString sPath ) = 0;
	virtual int GetFileSizeInBytes( CString sFilePath ) = 0;
	virtual int GetFileModTime( CString sPath ) = 0;
	virtual bool Ready() { return true; } /* see RageFileManager::MountpointIsReady */
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
};

#endif

