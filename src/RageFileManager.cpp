#include "global.h"
#include "RageFileManager.h"
#include "RageFileDriver.h"
#include "RageFile.h"
#include "RageUtil.h"
#include "RageUtil_FileDB.h"
#include "RageLog.h"
#include "RageThreads.h"
#include "arch/ArchHooks/ArchHooks.h"
#include "LuaManager.h"

#include <cerrno>
#include <cstddef>
#include <cstdint>
#include <sstream>
#include <vector>

#if defined(_WIN32)
#define WIN32_LEAN_AND_MEAN
#include <windows.h>
#elif defined(UNIX) || defined(MACOSX)
#include <paths.h>
#include <sys/types.h>
#endif

#include <miniz.h>

RageFileManager *FILEMAN = nullptr;

/* Lock this before touching any of these globals (except FILEMAN itself). */
static RageEvent *g_Mutex;

RString RageFileManagerUtil::sDirOfExecutable;

struct LoadedDriver
{
	/* A loaded driver may have a base path, which modifies the path we
	 * pass to the driver.  For example, if the base is "Songs/", and we
	 * want to send the path "Songs/Foo/Bar" to it, then we actually
	 * only send "Foo/Bar".  The path "Themes/Foo" is out of the scope
	 * of the driver, and GetPath returns false. */
	RageFileDriver *m_pDriver;
	RString m_sType, m_sRoot, m_sMountPoint;

	int m_iRefs;

	LoadedDriver() { m_pDriver = nullptr; m_iRefs = 0; }
	RString GetPath( const RString &sPath ) const;
};

static std::vector<LoadedDriver *> g_pDrivers;
static std::map<const RageFileBasic *,LoadedDriver *> g_mFileDriverMap;

static void ReferenceAllDrivers( std::vector<LoadedDriver *> &apDriverList )
{
	g_Mutex->Lock();
	apDriverList = g_pDrivers;
	for( unsigned i = 0; i < apDriverList.size(); ++i )
		++apDriverList[i]->m_iRefs;
	g_Mutex->Unlock();
}

static void UnreferenceAllDrivers( std::vector<LoadedDriver *> &apDriverList )
{
	g_Mutex->Lock();
	for( unsigned i = 0; i < apDriverList.size(); ++i )
		--apDriverList[i]->m_iRefs;
	g_Mutex->Broadcast();
	g_Mutex->Unlock();

	/* Clear the temporary list, to make it clear that the drivers may no longer be accessed. */
	apDriverList.clear();
}

RageFileDriver *RageFileManager::GetFileDriver( RString sMountpoint )
{
	FixSlashesInPlace( sMountpoint );
	if( sMountpoint.size() && sMountpoint.Right(1) != "/" )
		sMountpoint += '/';

	g_Mutex->Lock();
	RageFileDriver *pRet = nullptr;
	for( unsigned i = 0; i < g_pDrivers.size(); ++i )
	{
		if( g_pDrivers[i]->m_sType == "mountpoints" )
			continue;
		if( g_pDrivers[i]->m_sMountPoint.CompareNoCase( sMountpoint ) )
			continue;

		pRet = g_pDrivers[i]->m_pDriver;
		++g_pDrivers[i]->m_iRefs;
		break;
	}
	g_Mutex->Unlock();

	return pRet;
}

void RageFileManager::ReleaseFileDriver( RageFileDriver *pDriver )
{
	ASSERT( pDriver != nullptr );

	g_Mutex->Lock();
	unsigned i;
	for( i = 0; i < g_pDrivers.size(); ++i )
	{
		if( g_pDrivers[i]->m_pDriver == pDriver )
			break;
	}
	ASSERT( i != g_pDrivers.size() );

	--g_pDrivers[i]->m_iRefs;

	g_Mutex->Broadcast();
	g_Mutex->Unlock();
}

std::size_t zipRead(void *pOpaque, mz_uint64 file_ofs, void *pBuf, std::size_t n)
{
	RageFile *f = static_cast<RageFile*>(pOpaque);

	const int pos = f->Seek(file_ofs);
	if (pos >= 0 && static_cast<std::uint64_t>(pos) != file_ofs)
	{
		return 0;
	}

	return f->Read(pBuf, n);
}

std::size_t zipWriteFile(void *pOpaque, mz_uint64 file_ofs, const void *pBuf, std::size_t n)
{
	RageFile *f = static_cast<RageFile*>(pOpaque);

	/*
	 * XXX: RageFile doesn't allow to Seek() in a file open for writing. We
	 * rely on the fact that miniz writes the file in order. Tell() is not
	 * allowed either, so we can't even check that the current offset is
	 * correct.
	 */
	/*
	int pos = f->Seek(file_ofs);
	if (pos != file_ofs)
	{
		return 0;
	}
	*/

	return f->Write(pBuf, n);
}

bool RageFileManager::Unzip(const std::string &zipPath, std::string targetPath, int strip)
{
	if (targetPath.empty() || targetPath.back() != '/')
		targetPath.push_back('/');

	RageFile zipFile;
	if (!zipFile.Open(zipPath, RageFile::READ))
	{
		RString error = zipFile.GetError();
		LOG->Warn("Could not unzip %s: %s", zipPath.c_str(), error.c_str());
		return false;
	}

	mz_zip_archive zip = {};
	zip.m_pRead = zipRead;
	zip.m_pIO_opaque = &zipFile;

	if (!mz_zip_reader_init(&zip, zipFile.GetFileSize(), 0))
	{
		LOG->Warn("Could not unzip %s: %s", zipPath.c_str(), mz_zip_get_error_string(zip.m_last_error));
		mz_zip_reader_end(&zip);
		return false;
	}

	bool success = true;
	mz_uint file_count = mz_zip_reader_get_num_files(&zip);

	for (mz_uint fileIndex = 0; fileIndex < file_count; fileIndex++)
	{
		mz_zip_archive_file_stat info;
		if (!mz_zip_reader_file_stat(&zip, fileIndex, &info))
		{
			LOG->Warn("Could not unzip %s: %s", zipPath.c_str(), mz_zip_get_error_string(zip.m_last_error));
			success = false;
			break;
		}

		std::string filename(info.m_filename);
		if (filename.back() == '/')
			filename.pop_back();

		for (int i = 0; i < strip; i++)
		{
			std::size_t pos = filename.find('/');
			if (pos != std::string::npos)
				pos++;
			filename.erase(0, pos);
		}
		if (filename.empty())
			continue;

		std::string filepath = targetPath + filename;

		if (FILEMAN->IsPathProtected(filepath))
		{
			LOG->Warn("Overwriting %s is not allowed", filepath.c_str());
			continue;
		}

		if (info.m_is_directory)
		{
			CreateDir(filepath);
		}
		else
		{
			RageFile f;
			if (!f.Open(filepath, RageFile::WRITE | RageFile::STREAMED))
			{
				RString error = zipFile.GetError();
				LOG->Warn("Could not write to %s: %s", filepath.c_str(), error.c_str());
				success = false;
				break;
			}

			success = mz_zip_reader_extract_to_callback(&zip, fileIndex, zipWriteFile, &f, 0);
			if (!success)
			{
				RString error = f.GetError();
				LOG->Warn("Could not write to %s: %s", filepath.c_str(), error.c_str());
				FILEMAN->Remove(filepath);
				break;
			}
		}
	}

	mz_zip_reader_end(&zip);
	return success;
}

static void NormalizePath( RString &sPath )
{
	FixSlashesInPlace( sPath );
	CollapsePath( sPath, true );
	if (sPath.size() == 0)
	{
		sPath = '/';
	}
	else if (sPath[0] != '/')
	{
		sPath = '/' + sPath;
	}
}

void RageFileManager::ProtectPath(const std::string& path)
{
	RString normalizedPath(path);
	NormalizePath(normalizedPath);
	normalizedPath.MakeLower();

	m_protectedPaths.insert(normalizedPath);
}

bool RageFileManager::IsPathProtected(const std::string& path)
{
	RString normalizedPath(path);
	NormalizePath(normalizedPath);
	normalizedPath.MakeLower();

	return m_protectedPaths.count(normalizedPath) > 0;
}

// Mountpoints as directories cause a problem.  If "Themes/default" is a mountpoint, and
// doesn't exist anywhere else, then GetDirListing("Themes/*") must return "default".  The
// driver containing "Themes/default" won't do this; its world view begins at "BGAnimations"
// (inside "Themes/default").  We need a dummy driver that handles mountpoints. */
class RageFileDriverMountpoints: public RageFileDriver
{
public:
	RageFileDriverMountpoints(): RageFileDriver( new FilenameDB ) { }
	RageFileBasic *Open( const RString &sPath, int iMode, int &iError )
	{
		iError = (iMode == RageFile::WRITE)? ERROR_WRITING_NOT_SUPPORTED:ENOENT;
		return nullptr;
	}
	/* Never flush FDB, except in LoadFromDrivers. */
	void FlushDirCache( const RString &sPath ) { }

	void LoadFromDrivers( const std::vector<LoadedDriver *> &apDrivers )
	{
		/* XXX: Even though these two operations lock on their own, lock around
		 * them, too.  That way, nothing can sneak in and get incorrect
		 * results between the flush and the re-population. */
		FDB->FlushDirCache();
		for( unsigned i = 0; i < apDrivers.size(); ++i )
			if( apDrivers[i]->m_sMountPoint != "/" )
				FDB->AddFile( apDrivers[i]->m_sMountPoint, 0, 0 );
	}
};
static RageFileDriverMountpoints *g_Mountpoints = nullptr;

static RString ExtractDirectory( RString sPath )
{
	// return the directory containing sPath
	std::size_t n = sPath.find_last_of("/");
	if( n != sPath.npos )
		sPath.erase(n);
	else
		sPath.erase();
	return sPath;
}

#if defined(UNIX) || defined(MACOSX)
static RString ReadlinkRecursive( RString sPath )
{
	// unices support symbolic links; dereference them
	RString dereferenced = sPath;
	do
	{
		sPath = dereferenced;
		char derefPath[512];
		ssize_t linkSize = readlink(sPath, derefPath, sizeof(derefPath));
		if ( linkSize != -1 && linkSize != sizeof(derefPath) )
		{
			dereferenced = RString( derefPath, linkSize );
			if (derefPath[0] != '/')
			{
				// relative link
				dereferenced = RString( ExtractDirectory(sPath) + "/" + dereferenced);
			}
		}
	} while (sPath != dereferenced);

	return sPath;
}
#endif

static RString GetDirOfExecutable( RString argv0 )
{
	// argv[0] can be wrong in most OS's; try to avoid using it.

	RString sPath;
#if defined(_WIN32)
	char szBuf[MAX_PATH];
	GetModuleFileName( nullptr, szBuf, sizeof(szBuf) );
	sPath = szBuf;
#else
	sPath = argv0;
#endif

	sPath.Replace( "\\", "/" );

	bool bIsAbsolutePath = false;
	if( sPath.size() == 0 || sPath[0] == '/' )
		bIsAbsolutePath = true;
#if defined(_WIN32)
	if( sPath.size() > 2 && sPath[1] == ':' && sPath[2] == '/' )
		bIsAbsolutePath = true;
#endif

	// strip off executable name
	sPath = ExtractDirectory(sPath);

	if( !bIsAbsolutePath )
	{
#if defined(UNIX) || defined(MACOSX)
		if( sPath.empty() )
		{
			// This is in our path so look for it.
			const char *path = getenv( "PATH" );

			if( !path )
				path = _PATH_DEFPATH;

			std::vector<RString> vPath;
			split( path, ":", vPath );
			for (RString &i : vPath)
			{
				if( access(i + "/" + argv0, X_OK|R_OK) )
					continue;
				sPath = ExtractDirectory(ReadlinkRecursive(i + "/" + argv0));
				break;
			}
			if( sPath.empty() )
				sPath = GetCwd(); // What?
			else if( sPath[0] != '/' ) // For example, if . is in $PATH.
				sPath = GetCwd() + "/" + sPath;

		}
		else
		{
			sPath = ExtractDirectory(ReadlinkRecursive(GetCwd() + "/" + argv0));
		}
#else
		sPath = GetCwd() + "/" + sPath;
		sPath.Replace( "\\", "/" );
#endif
	}
	return sPath;
}

static void ChangeToDirOfExecutable( const RString &argv0 )
{
	RageFileManagerUtil::sDirOfExecutable = GetDirOfExecutable( argv0 );

	/* Set the CWD.  Any effects of this is platform-specific; most files are read and
	 * written through RageFile.  See also RageFileManager::RageFileManager. */
#if defined(_WINDOWS)
	if( _chdir( RageFileManagerUtil::sDirOfExecutable + "/.." ) )
#elif defined(UNIX)
	if( chdir( RageFileManagerUtil::sDirOfExecutable + "/" ) )
#elif defined(MACOSX)
	/* If the basename is not MacOS, then we've likely been launched via the command line
	 * through a symlink. Assume this is the case and change to the dir of the symlink. */
	if( Basename(RageFileManagerUtil::sDirOfExecutable) == "MacOS" )
		CollapsePath( RageFileManagerUtil::sDirOfExecutable += "/../../../" );
	if( chdir( RageFileManagerUtil::sDirOfExecutable ) )
#endif
	{
		LOG->Warn("Can't set current working directory to %s", RageFileManagerUtil::sDirOfExecutable.c_str());
		return;
	}
}

RageFileManager::RageFileManager( const RString &argv0 )
{
	CHECKPOINT_M( argv0 );
	ChangeToDirOfExecutable( argv0 );

	g_Mutex = new RageEvent("RageFileManager");

	g_Mountpoints = new RageFileDriverMountpoints;
	LoadedDriver *pLoadedDriver = new LoadedDriver;
	pLoadedDriver->m_pDriver = g_Mountpoints;
	pLoadedDriver->m_sMountPoint = "/";
	pLoadedDriver->m_sType = "mountpoints";
	g_pDrivers.push_back( pLoadedDriver );

	/* The mount path is unused, but must be nonempty. */
	RageFileManager::Mount( "mem", "(cache)", "/@mem" );

	// Register with Lua.
	{
		Lua *L = LUA->Get();
		lua_pushstring( L, "FILEMAN" );
		this->PushSelf( L );
		lua_settable( L, LUA_GLOBALSINDEX );
		LUA->Release( L );
	}
}

void RageFileManager::MountInitialFilesystems()
{
	HOOKS->MountInitialFilesystems( RageFileManagerUtil::sDirOfExecutable );
}

void RageFileManager::MountUserFilesystems()
{
	HOOKS->MountUserFilesystems( RageFileManagerUtil::sDirOfExecutable );
}

RageFileManager::~RageFileManager()
{
	// Unregister with Lua.
	LUA->UnsetGlobal( "FILEMAN" );

	/* Note that drivers can use previously-loaded drivers, eg. to load a ZIP
	 * from the FS.  Unload drivers in reverse order. */
	for( int i = g_pDrivers.size()-1; i >= 0; --i )
	{
		delete g_pDrivers[i]->m_pDriver;
		delete g_pDrivers[i];
	}
	g_pDrivers.clear();

//	delete g_Mountpoints; // g_Mountpoints was in g_pDrivers
	g_Mountpoints = nullptr;

	delete g_Mutex;
	g_Mutex = nullptr;
}

/* path must be normalized (FixSlashesInPlace, CollapsePath). */
RString LoadedDriver::GetPath( const RString &sPath ) const
{
	/* If the path begins with /@, only match mountpoints that begin with /@. */
	if( sPath.size() >= 2 && sPath[1] == '@' )
	{
		if( m_sMountPoint.size() < 2 || m_sMountPoint[1] != '@' )
			return RString();
	}

	if( sPath.Left(m_sMountPoint.size()).CompareNoCase(m_sMountPoint) )
		return RString(); /* no match */

	/* Add one, so we don't cut off the leading slash. */
	RString sRet = sPath.Right( sPath.size() - m_sMountPoint.size() + 1 );
	return sRet;
}

bool ilt( const RString &a, const RString &b ) { return a.CompareNoCase(b) < 0; }
bool ieq( const RString &a, const RString &b ) { return a.CompareNoCase(b) == 0; }
void RageFileManager::GetDirListing( const RString &sPath_, std::vector<RString> &AddTo, bool bOnlyDirs, bool bReturnPathToo )
{
	RString sPath = sPath_;
	NormalizePath( sPath );

	// NormalizePath() calls CollapsePath() which will remove "dir/.." pairs.
	// So if a "/.." is still present, they're trying to go below the root,
	// which isn't valid.
	if( sPath.find("/..") != std::string::npos )
		return;

	std::vector<LoadedDriver *> apDriverList;
	ReferenceAllDrivers( apDriverList );

	int iDriversThatReturnedFiles = 0;
	int iOldSize = AddTo.size();
	for( unsigned i = 0; i < apDriverList.size(); ++i )
	{
		LoadedDriver *pLoadedDriver = apDriverList[i];
		const RString p = pLoadedDriver->GetPath( sPath );
		if( p.size() == 0 )
			continue;

		const unsigned OldStart = AddTo.size();

		pLoadedDriver->m_pDriver->GetDirListing( p, AddTo, bOnlyDirs, bReturnPathToo );
		if( AddTo.size() != OldStart )
			++iDriversThatReturnedFiles;

		/* If returning the path, prepend the mountpoint name to the files this driver returned. */
		if( bReturnPathToo && pLoadedDriver->m_sMountPoint.size() > 0 )
		{
			RString const &mountPoint = pLoadedDriver->m_sMountPoint;
			/* Skip the trailing slash on the mountpoint; there's already a slash there. */
			RString const &trimPoint = mountPoint.substr(0, mountPoint.size() - 1);
			for( unsigned j = OldStart; j < AddTo.size(); ++j )
			{
				AddTo[j] = trimPoint + AddTo[j];
			}
		}
	}

	UnreferenceAllDrivers( apDriverList );

	// Remove files that start with ._ from the list because these are special
	// macOS files that cause interference on other platforms. -Kyz
	StripMacResourceForks(AddTo);

	if( iDriversThatReturnedFiles > 1 )
	{
		/* More than one driver returned files.  Remove duplicates (case-insensitively). */
		sort( AddTo.begin()+iOldSize, AddTo.end(), ilt );
		std::vector<RString>::iterator it = unique( AddTo.begin()+iOldSize, AddTo.end(), ieq );
		AddTo.erase( it, AddTo.end() );
	}
}

void RageFileManager::GetDirListingWithMultipleExtensions( const RString &sPath, std::vector<RString> const& ExtensionList, std::vector<RString> &AddTo, bool bOnlyDirs, bool bReturnPathToo )
{
	for(std::vector<RString>::const_iterator curr_ext= ExtensionList.begin();
		curr_ext != ExtensionList.end(); ++curr_ext)
	{
		GetDirListing(sPath + "*." + (*curr_ext), AddTo, bOnlyDirs, bReturnPathToo);
	}
}

/* Files may only be moved within the same file driver. */
bool RageFileManager::Move( const RString &fromPath_, const RString &toPath_ )
{
	RString fromPath = fromPath_;
	RString toPath = toPath_;

	std::vector<LoadedDriver *> aDriverList;
	ReferenceAllDrivers( aDriverList );

	NormalizePath( fromPath );
	NormalizePath( toPath );

	/* Multiple drivers may have the same file. */
	bool Deleted = false;
	for( unsigned i = 0; i < aDriverList.size(); ++i )
	{
		const RString sOldDriverPath = aDriverList[i]->GetPath( fromPath );
		const RString sNewDriverPath = aDriverList[i]->GetPath( toPath );
		if( sOldDriverPath.size() == 0 || sNewDriverPath.size() == 0 )
			continue;

		bool ret = aDriverList[i]->m_pDriver->Move( sOldDriverPath, sNewDriverPath );
		if( ret )
			Deleted = true;
	}

	UnreferenceAllDrivers( aDriverList );

	return Deleted;
}

bool RageFileManager::Copy(const std::string &fromPath, const std::string &toPath)
{
	RageFile fromFile, toFile;

	if (!fromFile.Open(fromPath, RageFile::READ))
		return false;

	if (!toFile.Open(toPath, RageFile::WRITE))
		return false;

	char buf[4096];
	for (;;)
	{
		int readBytes = fromFile.Read(buf, sizeof(buf));
		if (readBytes <= 0)
			break;

		int writtenBytes = toFile.Write(buf, readBytes);
		if (writtenBytes != readBytes)
			break;
	}

	if (!fromFile.GetError().empty() || !toFile.GetError().empty())
	{
		return false;
	}

	return true;
}

bool RageFileManager::Remove( const RString &sPath_ )
{
	RString sPath = sPath_;

	std::vector<LoadedDriver *> apDriverList;
	ReferenceAllDrivers( apDriverList );

	NormalizePath( sPath );

	/* Multiple drivers may have the same file. */
	bool bDeleted = false;
	for( unsigned i = 0; i < apDriverList.size(); ++i )
	{
		const RString p = apDriverList[i]->GetPath( sPath );
		if( p.size() == 0 )
			continue;

		bool ret = apDriverList[i]->m_pDriver->Remove( p );
		if( ret )
			bDeleted = true;
	}

	UnreferenceAllDrivers( apDriverList );

	return bDeleted;
}

bool RageFileManager::DeleteRecursive( const RString &sPath )
{
	// On some OS's, non-empty directories cannot be deleted.
	// This is a work-around that can delete both files and non-empty directories
	return ::DeleteRecursive(sPath);
}

void RageFileManager::CreateDir( const RString &sDir )
{
	RString sTempFile = sDir + "newdir.temp.newdir";
	RageFile f;
	f.Open( sTempFile, RageFile::WRITE );
	f.Close();

	Remove( sTempFile );
}

static void AdjustMountpoint( RString &sMountPoint )
{
	FixSlashesInPlace( sMountPoint );

	ASSERT_M( sMountPoint.Left(1) == "/", "Mountpoints must be absolute: " + sMountPoint );

	if( sMountPoint.size() && sMountPoint.Right(1) != "/" )
		sMountPoint += '/';

	if( sMountPoint.Left(1) != "/" )
		sMountPoint = "/" + sMountPoint;

}

static void AddFilesystemDriver( LoadedDriver *pLoadedDriver )
{
	g_Mutex->Lock();
	g_pDrivers.insert(g_pDrivers.begin(), pLoadedDriver);
	g_Mountpoints->LoadFromDrivers( g_pDrivers );
	g_Mutex->Unlock();
}

bool RageFileManager::Mount( const RString &sType, const RString &sRoot_, const RString &sMountPoint_ )
{
	RString sRoot = sRoot_;
	RString sMountPoint = sMountPoint_;

	FixSlashesInPlace( sRoot );
	AdjustMountpoint( sMountPoint );

	ASSERT( !sRoot.empty() );

	const RString &sPaths = ssprintf( "\"%s\", \"%s\", \"%s\"", sType.c_str(), sRoot.c_str(), sMountPoint.c_str() );
	CHECKPOINT_M( sPaths );
#if defined(DEBUG)
	puts( sPaths );
#endif

	// Unmount anything that was previously mounted here.
	Unmount( sType, sRoot, sMountPoint );

	CHECKPOINT_M( ssprintf("About to make a driver with \"%s\", \"%s\"", sType.c_str(), sRoot.c_str()));
	RageFileDriver *pDriver = MakeFileDriver( sType, sRoot );
	if( pDriver == nullptr )
	{
		const RString errorMsg = ssprintf("Can't mount unknown VFS type \"%s\", root \"%s\"", sType.c_str(), sRoot.c_str());
		CHECKPOINT_M( errorMsg );
		LOG->Warn( "%s", errorMsg.c_str() );
		return false;
	}

	CHECKPOINT_M("Driver successfully made.");

	LoadedDriver *pLoadedDriver = new LoadedDriver;
	pLoadedDriver->m_pDriver = pDriver;
	pLoadedDriver->m_sType = sType;
	pLoadedDriver->m_sRoot = sRoot;
	pLoadedDriver->m_sMountPoint = sMountPoint;

	AddFilesystemDriver( pLoadedDriver );
	return true;
}

void RageFileManager::Unmount( const RString &sType, const RString &sRoot_, const RString &sMountPoint_ )
{
	RString sRoot = sRoot_;
	RString sMountPoint = sMountPoint_;

	FixSlashesInPlace( sRoot );
	FixSlashesInPlace( sMountPoint );

	if( sMountPoint.size() && sMountPoint.Right(1) != "/" )
		sMountPoint += '/';

	/* Find all drivers we want to delete.  Remove them from g_pDrivers, and move them
	 * into aDriverListToUnmount. */
	std::vector<LoadedDriver *> apDriverListToUnmount;
	g_Mutex->Lock();
	for( unsigned i = 0; i < g_pDrivers.size(); ++i )
	{
		if( !sType.empty() && g_pDrivers[i]->m_sType.CompareNoCase( sType ) )
			continue;
		if( !sRoot.empty() && g_pDrivers[i]->m_sRoot.CompareNoCase( sRoot ) )
			continue;
		if( !sMountPoint.empty() && g_pDrivers[i]->m_sMountPoint.CompareNoCase( sMountPoint ) )
			continue;

		++g_pDrivers[i]->m_iRefs;
		apDriverListToUnmount.push_back( g_pDrivers[i] );
		g_pDrivers.erase( g_pDrivers.begin()+i );
		--i;
	}

	g_Mountpoints->LoadFromDrivers( g_pDrivers );

	g_Mutex->Unlock();

	/* Now we have a list of drivers to remove. */
	while( apDriverListToUnmount.size() )
	{
		/* If the driver has more than one reference, somebody other than us is
		 * using it; wait for that operation to complete. Note that two Unmount()
		 * calls that want to remove the same mountpoint will deadlock here. */
		g_Mutex->Lock();
		while( apDriverListToUnmount[0]->m_iRefs > 1 )
			g_Mutex->Wait();
		g_Mutex->Unlock();

		delete apDriverListToUnmount[0]->m_pDriver;
		delete apDriverListToUnmount[0];
		apDriverListToUnmount.erase( apDriverListToUnmount.begin() );
	}
}

void RageFileManager::Remount( RString sMountpoint, RString sPath )
{
	RageFileDriver *pDriver = GetFileDriver( sMountpoint );
	if( pDriver == nullptr )
	{
		if( LOG )
			LOG->Warn( "Remount(%s,%s): mountpoint not found", sMountpoint.c_str(), sPath.c_str() );
		return;
	}

	if( !pDriver->Remount(sPath) )
		LOG->Warn( "Remount(%s,%s): remount failed (does the driver support remounting?)", sMountpoint.c_str(), sPath.c_str() );
	else
		pDriver->FlushDirCache( "" );

	ReleaseFileDriver( pDriver );
}

bool RageFileManager::IsMounted( RString MountPoint )
{
	LockMut( *g_Mutex );

	for( unsigned i = 0; i < g_pDrivers.size(); ++i )
		if( !g_pDrivers[i]->m_sMountPoint.CompareNoCase( MountPoint ) )
			return true;

	return false;
}

void RageFileManager::GetLoadedDrivers( std::vector<DriverLocation> &asMounts )
{
	LockMut( *g_Mutex );

	for( unsigned i = 0; i < g_pDrivers.size(); ++i )
	{
		DriverLocation l;
		l.MountPoint = g_pDrivers[i]->m_sMountPoint;
		l.Type = g_pDrivers[i]->m_sType;
		l.Root = g_pDrivers[i]->m_sRoot;
		asMounts.push_back( l );
	}
}

void RageFileManager::FlushDirCache( const RString &sPath_ )
{
	RString sPath = sPath_;

	LockMut( *g_Mutex );

	if( sPath == "" )
	{
		for( unsigned i = 0; i < g_pDrivers.size(); ++i )
			g_pDrivers[i]->m_pDriver->FlushDirCache( "" );
		return;
	}

	/* Flush a specific path. */
	NormalizePath( sPath );
	for( unsigned i = 0; i < g_pDrivers.size(); ++i )
	{
		const RString &path = g_pDrivers[i]->GetPath( sPath );
		if( path.size() == 0 )
			continue;
		g_pDrivers[i]->m_pDriver->FlushDirCache( path );
	}
}

RageFileManager::FileType RageFileManager::GetFileType( const RString &sPath_ )
{
	RString sPath = sPath_;

	NormalizePath( sPath );

	std::vector<LoadedDriver *> apDriverList;
	ReferenceAllDrivers( apDriverList );

	RageFileManager::FileType ret = TYPE_NONE;
	for( unsigned i = 0; i < apDriverList.size(); ++i )
	{
		const RString p = apDriverList[i]->GetPath( sPath );
		if( p.size() == 0 )
			continue;
		ret = apDriverList[i]->m_pDriver->GetFileType( p );
		if( ret != TYPE_NONE )
			break;
	}

	UnreferenceAllDrivers( apDriverList );

	return ret;
}


int RageFileManager::GetFileSizeInBytes( const RString &sPath_ )
{
	RString sPath = sPath_;

	NormalizePath( sPath );

	std::vector<LoadedDriver *> apDriverList;
	ReferenceAllDrivers( apDriverList );

	int iRet = -1;
	for( unsigned i = 0; i < apDriverList.size(); ++i )
	{
		const RString p = apDriverList[i]->GetPath( sPath );
		if( p.size() == 0 )
			continue;
		iRet = apDriverList[i]->m_pDriver->GetFileSizeInBytes( p );
		if( iRet != -1 )
			break;
	}
	UnreferenceAllDrivers( apDriverList );

	return iRet;
}

int RageFileManager::GetFileHash( const RString &sPath_ )
{
	RString sPath = sPath_;

	NormalizePath( sPath );

	std::vector<LoadedDriver *> apDriverList;
	ReferenceAllDrivers( apDriverList );

	int iRet = -1;
	for( unsigned i = 0; i < apDriverList.size(); ++i )
	{
		const RString p = apDriverList[i]->GetPath( sPath );
		if( p.size() == 0 )
			continue;
		iRet = apDriverList[i]->m_pDriver->GetFileHash( p );
		if( iRet != -1 )
			break;
	}
	UnreferenceAllDrivers( apDriverList );

	return iRet;
}

RString RageFileManager::ResolvePath(const RString &path)
{
	RString tmpPath = path;
	NormalizePath(tmpPath);

	RString resolvedPath = tmpPath;

	std::vector<LoadedDriver *> apDriverList;
	ReferenceAllDrivers( apDriverList );

	for( unsigned i = 0; i < apDriverList.size(); ++i )
	{
		LoadedDriver *pDriver = apDriverList[i];
		const RString driverPath = pDriver->GetPath( tmpPath );

		if ( driverPath.empty() || pDriver->m_sRoot.empty() )
			continue;

		if ( pDriver->m_sType != "dir" && pDriver->m_sType != "dirro" )
			continue;

		int iMountPointLen = pDriver->m_sMountPoint.length();
		if( tmpPath.substr(0, iMountPointLen) != pDriver->m_sMountPoint )
			continue;

		resolvedPath = pDriver->m_sRoot + "/" + RString(tmpPath.substr(iMountPointLen));
		break;
	}

	UnreferenceAllDrivers( apDriverList );

	NormalizePath( resolvedPath );

	return resolvedPath;
}

static bool SortBySecond( const std::pair<int, int> &a, const std::pair<int, int> &b )
{
	return a.second < b.second;
}

/*
 * Return true if the given path should use slow, reliable writes.
 *
 * I haven't decided if it's better to do this here, or to specify SLOW_FLUSH
 * manually each place we want it.  This seems more reliable (we might forget
 * somewhere and not notice), and easier (don't have to pass flags down to IniFile::Write,
 * etc).
 */
static bool PathUsesSlowFlush( const RString &sPath )
{
	static const char *FlushPaths[] =
	{
		"/Save/",
		"Save/"
	};

	for( unsigned i = 0; i < ARRAYLEN(FlushPaths); ++i )
		if( !strncmp( sPath, FlushPaths[i], strlen(FlushPaths[i]) ) )
			return true;
	return false;
}

/* Used only by RageFile: */
RageFileBasic *RageFileManager::Open( const RString &sPath_, int mode, int &err )
{
	RString sPath = sPath_;

	err = ENOENT;

	if( (mode & RageFile::WRITE) && PathUsesSlowFlush(sPath) )
		mode |= RageFile::SLOW_FLUSH;

	NormalizePath( sPath );

	/* If writing, we need to do a heuristic to figure out which driver to write with--there
	 * may be several that will work. */
	if( mode & RageFile::WRITE )
		return OpenForWriting( sPath, mode, err );
	else
		return OpenForReading( sPath, mode, err );
}

void RageFileManager::CacheFile( const RageFileBasic *fb, const RString &sPath_ )
{
	std::map<const RageFileBasic*, LoadedDriver*>::iterator it = g_mFileDriverMap.find( fb );

	ASSERT_M( it != g_mFileDriverMap.end(), ssprintf("No recorded driver for file: %s", sPath_.c_str()) );

	RString sPath = sPath_;
	NormalizePath( sPath );
	sPath = it->second->GetPath( sPath );
	it->second->m_pDriver->FDB->CacheFile( sPath );
	g_mFileDriverMap.erase( it );
}

RageFileBasic *RageFileManager::OpenForReading( const RString &sPath, int mode, int &err )
{
	std::vector<LoadedDriver*> apDriverList;
	ReferenceAllDrivers( apDriverList );

	for( unsigned i = 0; i < apDriverList.size(); ++i )
	{
		LoadedDriver &ld = *apDriverList[i];
		const RString path = ld.GetPath( sPath );
		if( path.size() == 0 )
			continue;
		int error;
		RageFileBasic *ret = ld.m_pDriver->Open( path, mode, error );
		if( ret )
		{
			UnreferenceAllDrivers( apDriverList );
			return ret;
		}

		/* ENOENT (File not found) is low-priority: if some other error
		 was reported, return that instead. */
		if( error != ENOENT )
			err = error;
	}
	UnreferenceAllDrivers( apDriverList );

	return nullptr;
}

RageFileBasic *RageFileManager::OpenForWriting( const RString &sPath, int mode, int &iError )
{
	/*
	 * The value for a driver to open a file is the number of directories and/or files
	 * that would have to be created in order to write it, or 0 if the file already exists.
	 * For example, if we're opening "foo/bar/baz.txt", and only "foo/" exists in a
	 * driver, we'd have to create the "bar" directory and the "baz.txt" file, so the
	 * value is 2.  If "foo/bar/" exists, we'd only have to create the file, so the
	 * value is 1.  Create the file with the driver that returns the lowest value;
	 * in case of a tie, earliest-loaded driver wins.
	 *
	 * The purpose of this is to create files in the expected place.  For example, if we
	 * have both C:/games/StepMania and C:/games/DWI loaded, and we're writing
	 * "Songs/Music/Waltz/waltz.ssc", and the song was loaded out of
	 * "C:/games/DWI/Songs/Music/Waltz/waltz.dwi", we want to write the new SSC into the
	 * same directory (if possible).  Don't split up files in the same directory any
	 * more than we have to.
	 *
	 * If the given path can not be created, return -1.  This happens if a path
	 * that needs to be a directory is a file, or vice versa.
	 */
	std::vector<LoadedDriver *> apDriverList;
	ReferenceAllDrivers( apDriverList );

	std::vector<std::pair<int, int>> Values;
	for( unsigned i = 0; i < apDriverList.size(); ++i )
	{
		LoadedDriver &ld = *apDriverList[i];
		const RString path = ld.GetPath( sPath );
		if( path.size() == 0 )
			continue;

		const int value = ld.m_pDriver->GetPathValue( path );
		if( value == -1 )
			continue;

		Values.push_back( std::make_pair( i, value ) );
	}

	stable_sort( Values.begin(), Values.end(), SortBySecond );

	/* Only write files if they'll be read.  If a file exists in any driver, don't
	 * create or write files in any driver mounted after it, because when we later
	 * try to read it, we'll get that file and not the one we wrote. */
	int iMaximumDriver = apDriverList.size();
	if( Values.size() > 0 && Values[0].second == 0 )
		iMaximumDriver = Values[0].first;

	iError = 0;
	for( unsigned i = 0; i < Values.size(); ++i )
	{
		const int iDriver = Values[i].first;
		if( iDriver > iMaximumDriver )
			continue;
		LoadedDriver &ld = *apDriverList[iDriver];
		const RString sDriverPath = ld.GetPath( sPath );
		ASSERT( !sDriverPath.empty() );

		int iThisError;
		RageFileBasic *pRet = ld.m_pDriver->Open( sDriverPath, mode, iThisError );
		if( pRet )
		{
			g_mFileDriverMap[pRet] = &ld;
			UnreferenceAllDrivers( apDriverList );
			return pRet;
		}

		/* The drivers are in order of priority; if they all return error, return the
		 * first.  Never return ERROR_WRITING_NOT_SUPPORTED. */
		if( !iError && iThisError != RageFileDriver::ERROR_WRITING_NOT_SUPPORTED )
			iError = iThisError;
	}

	if( !iError )
		iError = EEXIST; /* no driver could write */

	UnreferenceAllDrivers( apDriverList );

	return nullptr;
}

bool RageFileManager::IsAFile( const RString &sPath ) { return GetFileType(sPath) == TYPE_FILE; }
bool RageFileManager::IsADirectory( const RString &sPath ) { return GetFileType(sPath) == TYPE_DIR; }
bool RageFileManager::DoesFileExist( const RString &sPath ) { return GetFileType(sPath) != TYPE_NONE; }

bool DoesFileExist( const RString &sPath )
{
	return FILEMAN->DoesFileExist( sPath );
}

bool IsAFile( const RString &sPath )
{
	return FILEMAN->IsAFile( sPath );
}

bool IsADirectory( const RString &sPath )
{
	return FILEMAN->IsADirectory( sPath );
}

int GetFileSizeInBytes( const RString &sPath )
{
	return FILEMAN->GetFileSizeInBytes( sPath );
}

void GetDirListing( const RString &sPath, std::vector<RString> &AddTo, bool bOnlyDirs, bool bReturnPathToo )
{
	FILEMAN->GetDirListing( sPath, AddTo, bOnlyDirs, bReturnPathToo );
}

void GetDirListingRecursive( const RString &sDir, const RString &sMatch, std::vector<RString> &AddTo )
{
	ASSERT( sDir.Right(1) == "/" );
	std::vector<RString> vsFiles;
	GetDirListing( sDir+sMatch, vsFiles, false, true );
	std::vector<RString> vsDirs;
	GetDirListing( sDir+"*", vsDirs, true, true );
	for( int i=0; i<(int)vsDirs.size(); i++ )
	{
		GetDirListing( vsDirs[i]+"/"+sMatch, vsFiles, false, true );
		GetDirListing( vsDirs[i]+"/*", vsDirs, true, true );
		vsDirs.erase( vsDirs.begin()+i );
		i--;
	}
	for( int i=vsFiles.size()-1; i>=0; i-- )
	{
		if( !IsADirectory(vsFiles[i]) )
			AddTo.push_back( vsFiles[i] );
	}
}

void GetDirListingRecursive( RageFileDriver *prfd, const RString &sDir, const RString &sMatch, std::vector<RString> &AddTo )
{
	ASSERT( sDir.Right(1) == "/" );
	std::vector<RString> vsFiles;
	prfd->GetDirListing( sDir+sMatch, vsFiles, false, true );
	std::vector<RString> vsDirs;
	prfd->GetDirListing( sDir+"*", vsDirs, true, true );
	for( int i=0; i<(int)vsDirs.size(); i++ )
	{
		prfd->GetDirListing( vsDirs[i]+"/"+sMatch, vsFiles, false, true );
		prfd->GetDirListing( vsDirs[i]+"/*", vsDirs, true, true );
		vsDirs.erase( vsDirs.begin()+i );
		i--;
	}
	for( int i=vsFiles.size()-1; i>=0; i-- )
	{
		if( prfd->GetFileType(vsFiles[i]) != RageFileManager::TYPE_DIR )
			AddTo.push_back( vsFiles[i] );
	}
}

bool DeleteRecursive( RageFileDriver *prfd, const RString &sDir )
{
	ASSERT( sDir.Right(1) == "/" );

	std::vector<RString> vsFiles;
	prfd->GetDirListing( sDir+"*", vsFiles, false, true );
	for (RString const &s : vsFiles)
	{
		if( IsADirectory(s) )
			DeleteRecursive( s+"/" );
		else
			FILEMAN->Remove( s );
	}

	return FILEMAN->Remove( sDir );
}

bool DeleteRecursive( const RString &sDir )
{
	ASSERT( sDir.Right(1) == "/" );

	std::vector<RString> vsFiles;
	GetDirListing( sDir+"*", vsFiles, false, true );
	for (RString const &s : vsFiles)
	{
		if( IsADirectory(s) )
			DeleteRecursive( s+"/" );
		else
			FILEMAN->Remove( s );
	}

	return FILEMAN->Remove( sDir );
}

unsigned int GetHashForFile( const RString &sPath )
{
	return FILEMAN->GetFileHash( sPath );
}

unsigned int GetHashForDirectory( const RString &sDir )
{
	unsigned int hash = 0;

	hash += GetHashForString( sDir );

	std::vector<RString> arrayFiles;
	GetDirListing( sDir+"*", arrayFiles, false );
	for( unsigned i=0; i<arrayFiles.size(); i++ )
	{
		const RString sFilePath = sDir + arrayFiles[i];
		hash += GetHashForFile( sFilePath );
	}

	return hash;
}

// lua start
#include "LuaBinding.h"

/** @brief Allow Lua to have access to the RageFileManager. */
class LunaRageFileManager: public Luna<RageFileManager>
{
public:
	static int Copy(T* p, lua_State *L){
		const std::string fromPath = SArg(1);
		const std::string toPath = SArg(2);

		if (p->IsPathProtected(toPath))
		{
			LOG->Warn("Overwriting %s is not allowed", toPath.c_str());
			lua_pushboolean(L, false);
			return 1;
		}

		lua_pushboolean(L, p->Copy(fromPath, toPath));
		return 1;
	}
	static int DoesFileExist( T* p, lua_State *L ){ lua_pushboolean( L, p->DoesFileExist(SArg(1)) ); return 1; }
	static int GetFileSizeBytes( T* p, lua_State *L ){ lua_pushnumber( L, p->GetFileSizeInBytes(SArg(1)) ); return 1; }
	static int GetHashForFile( T* p, lua_State *L ){ lua_pushnumber( L, p->GetFileHash(SArg(1)) ); return 1; }
	static int GetDirListing( T* p, lua_State *L )
	{
		std::vector<RString> vDirs;
		bool bOnlyDirs = false;
		bool bReturnPathToo = false;

		// the last two arguments of GetDirListing are optional;
		// let's reflect that in the Lua too. -aj
		if( lua_gettop(L) >= 2 && !lua_isnil(L,2) )
		{
			bOnlyDirs = BArg(2);
			if( !lua_isnil(L,3) )
			{
				bReturnPathToo = BArg(3);
			}
		}
		//( Path, addTo, OnlyDirs=false, ReturnPathToo=false );
		p->GetDirListing( SArg(1), vDirs, bOnlyDirs, bReturnPathToo );
		LuaHelpers::CreateTableFromArray(vDirs, L);
		return 1;
	}
	static int Unzip(T* p, lua_State *L)
	{
		std::string zipPath = SArg(1);
		std::string targetPath = SArg(2);

		int strip = 0;
		if (lua_gettop(L) >= 3)
		{
			strip = IArg(3);
		}

		bool success = p->Unzip(zipPath, targetPath, strip);

		lua_pushboolean(L, success);
		return 1;

	}

	LunaRageFileManager()
	{
		ADD_METHOD( Copy );
		ADD_METHOD( DoesFileExist );
		ADD_METHOD( GetFileSizeBytes );
		ADD_METHOD( GetHashForFile );
		ADD_METHOD( GetDirListing );
		ADD_METHOD( Unzip );
	}
};

LUA_REGISTER_CLASS( RageFileManager )
// lua end

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
