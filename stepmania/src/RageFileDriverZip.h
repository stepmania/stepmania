#ifndef RAGE_FILE_DRIVER_ZIP_H
#define RAGE_FILE_DRIVER_ZIP_H

#include "RageFileDriver.h"

struct FileInfo;
struct end_central_dir_record;
class RageFileDriverZip: public RageFileDriver
{
public:
	RageFileDriverZip( CString path );
	virtual ~RageFileDriverZip();

	RageFileObj *Open( const CString &path, int mode, RageFile &p, int &err );
	void FlushDirCache( const CString &sPath );

private:
	RageFile zip;
    vector<FileInfo *> Files;

	void ParseZipfile();
	static void ReadEndCentralRecord( RageFile &zip, end_central_dir_record &ec );
	static int ProcessCdirFileHdr( RageFile &zip, FileInfo &info );
};

#endif
