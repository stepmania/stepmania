#ifndef RAGE_FILE_MANAGER_H
#define RAGE_FILE_MANAGER_H

#include <unordered_set>
#include <vector>


/** @brief Constants for working with the RageFileManager. */
namespace RageFileManagerUtil
{
	extern RString sDirOfExecutable;
}

class RageFileDriver;
class RageFileBasic;
struct lua_State;

bool ilt( const RString &a, const RString &b );
bool ieq( const RString &a, const RString &b );

/** @brief File utilities and high-level manager for RageFile objects. */
class RageFileManager
{
public:
	RageFileManager( const RString &argv0 );
	~RageFileManager();
	void MountInitialFilesystems();
	void MountUserFilesystems();

	void GetDirListing( const RString &sPath, std::vector<RString> &AddTo, bool bOnlyDirs, bool bReturnPathToo );
	void GetDirListingWithMultipleExtensions(const RString &sPath,
		std::vector<RString> const& ExtensionList, std::vector<RString> &AddTo,
		bool bOnlyDirs= false, bool bReturnPathToo= false);
	bool Move( const RString &fromPath, const RString &toPath );
	bool Copy( const std::string &fromPath, const std::string &toPath );
	bool Remove( const RString &sPath );
	bool DeleteRecursive( const RString &sPath );
	void CreateDir( const RString &sDir );

	enum FileType { TYPE_FILE, TYPE_DIR, TYPE_NONE };
	FileType GetFileType( const RString &sPath );

	bool IsAFile( const RString &sPath );
	bool IsADirectory( const RString &sPath );
	bool DoesFileExist( const RString &sPath );

	int GetFileSizeInBytes( const RString &sPath );
	int GetFileHash( const RString &sPath );

	/**
	 * @brief Get the absolte path from the VPS.
	 * @param path the VPS path.
	 * @return the absolute path. */
	RString ResolvePath(const RString &path);

	bool Mount( const RString &sType, const RString &sRealPath, const RString &sMountPoint );
	void Unmount( const RString &sType, const RString &sRoot, const RString &sMountPoint );

	/* Change the root of a filesystem.  Only a couple drivers support this; it's
	 * used to change memory card mountpoints without having to actually unmount
	 * the driver. */
	void Remount( RString sMountpoint, RString sPath );
	bool IsMounted( RString MountPoint );
	struct DriverLocation
	{
		RString Type, Root, MountPoint;
	};
	void GetLoadedDrivers( std::vector<DriverLocation> &asMounts );

	void FlushDirCache( const RString &sPath = RString() );

	/* Used only by RageFile: */
	RageFileBasic *Open( const RString &sPath, int iMode, int &iError );
	void CacheFile( const RageFileBasic *fb, const RString &sPath );

	/* Retrieve or release a reference to the low-level driver for a mountpoint. */
	RageFileDriver *GetFileDriver( RString sMountpoint );
	void ReleaseFileDriver( RageFileDriver *pDriver );

	bool Unzip(const std::string &zipPath, std::string targetPath, int strip);

	// path protection
	void ProtectPath(const std::string& path);
	bool IsPathProtected(const std::string& path);

	// Lua
	void PushSelf( lua_State *L );

private:
	RageFileBasic *OpenForReading( const RString &sPath, int iMode, int &iError );
	RageFileBasic *OpenForWriting( const RString &sPath, int iMode, int &iError );

	std::unordered_set<std::string> m_protectedPaths;
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
