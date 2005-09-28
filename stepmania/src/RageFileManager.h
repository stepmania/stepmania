/* RageFileManager - File utilities and high-level manager for RageFile objects. */

#ifndef RAGE_FILE_MANAGER_H
#define RAGE_FILE_MANAGER_H

extern CString InitialWorkingDirectory;
extern CString DirOfExecutable;
class RageFileDriver;
class RageFileBasic;

class RageFileManager
{
public:
	RageFileManager( CString argv0 );
	~RageFileManager();
	void MountInitialFilesystems();

	void GetDirListing( CString sPath, CStringArray &AddTo, bool bOnlyDirs, bool bReturnPathToo );
	bool MoveFile( CString sOldPath, CString sNewPath );
	bool Remove( CString sPath );
	void CreateDir( CString sDir );
	
	enum FileType { TYPE_FILE, TYPE_DIR, TYPE_NONE };
	FileType GetFileType( CString sPath );

	bool IsAFile( const CString &sPath );
	bool IsADirectory( const CString &sPath );
	bool DoesFileExist( const CString &sPath );

	int GetFileSizeInBytes( CString sPath );
	int GetFileHash( CString sPath );

	void Mount( CString sType, CString sRealPath, CString sMountPoint, bool bAddToEnd = true );
	void Mount( RageFileDriver *pDriver, CString sMountPoint, bool bAddToEnd = true );
	void Unmount( CString sType, CString sRoot, CString sMountPoint );

	/* Change the root of a filesystem.  Only a couple drivers support this; it's
	 * used to change memory card mountpoints without having to actually unmount
	 * the driver. */
	static void Remount( CString sMountpoint, CString sPath );
	bool IsMounted( CString MountPoint );
	struct DriverLocation
	{
		CString Type, Root, MountPoint;
	};
	void GetLoadedDrivers( vector<DriverLocation> &asMounts );

	void FlushDirCache( CString sPath );

	/* Used only by RageFile: */
	RageFileBasic *Open( CString sPath, int iMode, int &iError );

	/* Retrieve or release a reference to the low-level driver for a mountpoint. */
	RageFileDriver *GetFileDriver( CString sMountpoint );
	void ReleaseFileDriver( RageFileDriver *pDriver );

private:
	RageFileBasic *OpenForReading( CString sPath, int iMode, int &iError );
	RageFileBasic *OpenForWriting( CString sPath, int iMode, int &iError );
};

extern RageFileManager *FILEMAN;

#endif

/*
 * Copyright (c) 2001-2004 Glenn Maynard, Chris Danford
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
