#ifndef RAGE_FILE_DRIVER_DIRECT_H
#define RAGE_FILE_DRIVER_DIRECT_H

#include "RageFileDriver.h"

class RageFileDriverDirect: public RageFileDriver
{
public:
	RageFileDriverDirect( CString root );
	virtual ~RageFileDriverDirect() { }

	RageFileObj *Open( CString path, RageFile::OpenMode mode, RageFile &p, int &err );
	RageFileManager::FileType GetFileType( CString sPath );
	int GetFileSizeInBytes( CString sFilePath );
	int GetFileModTime( CString sPath );

private:
	CString root;
};

#endif
