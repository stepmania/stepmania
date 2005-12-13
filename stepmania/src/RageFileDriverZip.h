/* RageFileDriverZip - A read-only file driver for ZIPs. */

#ifndef RAGE_FILE_DRIVER_ZIP_H
#define RAGE_FILE_DRIVER_ZIP_H

#include "RageFileDriver.h"
#include "RageThreads.h"

class RageFileDriverZip: public RageFileDriver
{
public:
	RageFileDriverZip();
	RageFileDriverZip( RString sPath );
	RageFileDriverZip( RageFileBasic *pFile );
	bool Load( const RString &sPath );
	bool Load( RageFileBasic *pFile );

	virtual ~RageFileDriverZip();

	RageFileBasic *Open( const RString &sPath, int iMode, int &iErr );
	void FlushDirCache( const RString &sPath );

	void DeleteFileWhenFinished() { m_bFileOwned = true; }

	/* Lower-level access: */
	enum ZipCompressionMethod { STORED = 0, DEFLATED = 8 };
	struct FileInfo
	{
		RString m_sName;
		int m_iOffset;
		int m_iDataOffset;

		ZipCompressionMethod m_iCompressionMethod;
		int m_iCRC32;
		int m_iCompressedSize, m_iUncompressedSize;

		/* If 0, unknown. */
		int m_iFilePermissions;
	};
	const FileInfo *GetFileInfo( const RString &sPath ) const;

	RString GetGlobalComment() const { return m_sComment; }

private:
	bool m_bFileOwned;

	RageFileBasic *m_pZip;
	vector<FileInfo *> m_pFiles;

	RString m_sPath;
	RString m_sComment;

	/* Open() must be threadsafe.  Mutex access to "zip", since we seek
	 * around in it when reading files. */
	RageMutex m_Mutex;

	bool ParseZipfile();
	bool ReadEndCentralRecord( int &total_entries_central_dir, int &offset_start_central_directory );
	int ProcessCdirFileHdr( FileInfo &info );
	bool SeekToEndCentralRecord();
	bool ReadLocalFileHeader( FileInfo &info );
};

#endif

/*
 * Copyright (c) 2003-2005 Glenn Maynard.  All rights reserved.
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
