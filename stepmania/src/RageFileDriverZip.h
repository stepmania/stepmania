#ifndef RAGE_FILE_DRIVER_ZIP_H
#define RAGE_FILE_DRIVER_ZIP_H

#include "RageFileDriver.h"
#include "RageThreads.h"

typedef struct zzip_dir		ZZIP_DIR;

class RageFileObjZip;

class RageFileDriverZip: public RageFileDriver
{
	friend class RageFileObjZip;
public:
	RageFileDriverZip( CString path );
	virtual ~RageFileDriverZip();

	RageFileObj *Open( const CString &path, RageFile::OpenMode mode, RageFile &p, int &err );
	int GetFileModTime( const CString &sPath );
	void FlushDirCache( const CString &sPath );

private:
	ZZIP_DIR *dir;
	RageMutex lock;
};

#endif
