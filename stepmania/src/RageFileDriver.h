/* RageFileDriver - File driver base classes. */

#ifndef RAGE_FILE_DRIVER_H
#define RAGE_FILE_DRIVER_H

#include "RageFileManager.h"

class RageFileBasic;
class FilenameDB;
class RageFileDriver
{
public:
	RageFileDriver( FilenameDB *pDB ) { FDB = pDB; }
	virtual ~RageFileDriver();
	virtual RageFileBasic *Open( const RString &sPath, int iMode, int &iError ) = 0;
	virtual void GetDirListing( const RString &sPath, vector<RString> &asAddTo, bool bOnlyDirs, bool bReturnPathToo );
	virtual RageFileManager::FileType GetFileType( const RString &sPath );
	virtual int GetFileSizeInBytes( const RString &sFilePath );
	virtual int GetFileHash( const RString &sPath );
	virtual int GetPathValue( const RString &sPath );
	virtual void FlushDirCache( const RString &sPath );
	virtual bool Move( const RString &sOldPath, const RString &sNewPath ) { return false; }
	virtual bool Remove( const RString &sPath ) { return false; }

	/* Optional: Move to a different place, as if reconstructed with a different path. */
	virtual bool Remount( const RString &sPath ) { return false; }

	/* Possible error returns from Open, in addition to standard errno.h values: */
	enum { ERROR_WRITING_NOT_SUPPORTED = -1 };
// protected:
	FilenameDB *FDB;
};

/* This is used to register the driver, so RageFileManager can see it. */
struct FileDriverEntry
{
	FileDriverEntry( RString sType );
	virtual ~FileDriverEntry();
	virtual RageFileDriver *Create( RString sRoot ) const = 0;

	RString m_sType;
	const FileDriverEntry *m_pLink;
};
RageFileDriver *MakeFileDriver( RString Type, RString Root );

#endif

/*
 * Copyright (c) 2003-2004 Glenn Maynard
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
