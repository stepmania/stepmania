#ifndef RAGE_FILE_DRIVER_DIRECT_H
#define RAGE_FILE_DRIVER_DIRECT_H

#include "RageFileDriver.h"

class RageFileDriverDirect: public RageFileDriver
{
public:
	RageFileDriverDirect( CString root );

	RageFileObj *Open( CString path, RageFile::OpenMode mode, RageFile &p, int &err );
	bool Ready();

private:
	CString root;
};

#endif
