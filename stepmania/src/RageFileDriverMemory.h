/* RageFileDriverMem: Simple memory-based "filesystem". */

#ifndef RAGE_FILE_DRIVER_MEMORY_H
#define RAGE_FILE_DRIVER_MEMORY_H

#include "RageFileDriver.h"
#include "RageThreads.h"

struct RageFileObjMemFile;
class RageFileDriverMem: public RageFileDriver
{
public:
	RageFileDriverMem();
	~RageFileDriverMem();

	RageFileObj *Open( const CString &sPath, int mode, RageFile &p, int &err );
	void FlushDirCache( const CString &sPath ) { }

	bool Remove( const CString &sPath );

private:
	RageMutex m_Mutex;
    vector<RageFileObjMemFile *> m_Files;
};

#endif

/*
 * (c) 2004 Glenn Maynard
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
