#include "global.h"
#include "RageFileManager.h"

#include <array>

#include "RageFileDriver.h"
#include "RageFile.h"
#include "RageUtil.h"
#include "RageUtil_FileDB.h"
#include "RageLog.h"
#include "RageThreads.h"
#include "RageString.hpp"
#include "arch/ArchHooks/ArchHooks.h"
#include "LuaManager.h"

#include <cerrno>

#if defined(WIN32)
#include <windows.h>
#elif defined(UNIX) || defined(MACOSX)
#include <paths.h>
#endif

using std::vector;

RageFileManager *FILEMAN = nullptr;

/* Lock this before touching any of these globals (except FILEMAN itself). */
static RageEvent *g_Mutex;

std::string RageFileManagerUtil::sInitialWorkingDirectory;
std::string RageFileManagerUtil::sDirOfExecutable;

struct LoadedDriver
{
	/* A loaded driver may have a base path, which modifies the path we
	 * pass to the driver.  For example, if the base is "Songs/", and we
	 * want to send the path "Songs/Foo/Bar" to it, then we actually
	 * only send "Foo/Bar".  The path "Themes/Foo" is out of the scope
	 * of the driver, and GetPath returns false. */
	RageFileDriver *m_pDriver;
	std::string m_sType, m_sRoot, m_sMountPoint;

	int m_iRefs;

	LoadedDriver() { m_pDriver = nullptr; m_iRefs = 0; }
	std::string GetPath( const std::string &sPath ) const;
};

static vector<LoadedDriver *> g_pDrivers;
static std::map<const RageFileBasic *,LoadedDriver *> g_mFileDriverMap;

static void ReferenceAllDrivers( vector<LoadedDriver *> &apDriverList )
{
	g_Mutex->Lock();
	apDriverList = g_pDrivers;
	for (auto *driver: apDriverList)
	{
		++driver->m_iRefs;
	}
	g_Mutex->Unlock();
}

static void UnreferenceAllDrivers( vector<LoadedDriver *> &apDriverList )
{
	g_Mutex->Lock();
	for (auto *driver: apDriverList)
	{
		--driver->m_iRefs;
	}
	g_Mutex->Broadcast();
	g_Mutex->Unlock();

	/* Clear the temporary list, to make it clear that the drivers may no longer be accessed. */
	apDriverList.clear();
}

RageFileDriver *RageFileManager::GetFileDriver( std::string sMountpoint )
{
	FixSlashesInPlace( sMountpoint );
	if( sMountpoint.size() && !Rage::ends_with( sMountpoint, "/") )
		sMountpoint += '/';

	g_Mutex->Lock();
	RageFileDriver *pRet = nullptr;
	Rage::ci_ascii_string ciMount{ sMountpoint.c_str() };
	for (auto *driver: g_pDrivers)
	{
		if (driver->m_sType == "mountpoints")
		{
			continue;
		}
		if (ciMount != driver->m_sMountPoint)
		{
			continue;
		}
		pRet = driver->m_pDriver;
		++driver->m_iRefs;
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

/* Wait for the given driver to become unreferenced, and remove it from the list
 * to get exclusive access to it.  Returns false if the driver is no longer available
 * (somebody else got it first). */
#if 0
static bool GrabDriver( RageFileDriver *pDriver )
{
	g_Mutex->Lock();

	for(;;)
	{
		unsigned i;
		for( i = 0; i < g_pDrivers.size(); ++i )
		{
			if( g_pDrivers[i]->m_pDriver == pDriver )
			{
				break;
			}
		}
		if( i == g_pDrivers.size() )
		{
			g_Mutex->Unlock();
			return false;
		}

		if( g_pDrivers[i]->m_iRefs == 0 )
		{
			g_pDrivers.erase( g_pDrivers.begin()+i );
			return true;
		}

		/* The driver is in use.  Wait for somebody to release a driver, and
		 * try again. */
		g_Mutex->Wait();
	}
}
#endif

// Mountpoints as directories cause a problem.  If "Themes/default" is a mountpoint, and
// doesn't exist anywhere else, then GetDirListing("Themes/*") must return "default".  The
// driver containing "Themes/default" won't do this; its world view begins at "BGAnimations"
// (inside "Themes/default").  We need a dummy driver that handles mountpoints. */
class RageFileDriverMountpoints: public RageFileDriver
{
public:
	RageFileDriverMountpoints(): RageFileDriver( new FilenameDB ) { }
	RageFileBasic *Open( const std::string &sPath, int iMode, int &iError )
	{
		iError = (iMode == RageFile::WRITE)? ERROR_WRITING_NOT_SUPPORTED:ENOENT;
		return nullptr;
	}
	/* Never flush FDB, except in LoadFromDrivers. */
	void FlushDirCache( const std::string &sPath ) { }

	void LoadFromDrivers( const vector<LoadedDriver *> &apDrivers )
	{
		/* XXX: Even though these two operations lock on their own, lock around
		 * them, too.  That way, nothing can sneak in and get incorrect
		 * results between the flush and the re-population. */
		FDB->FlushDirCache();
		for (auto *driver: apDrivers)
		{
			if( driver->m_sMountPoint != "/" )
			{
				FDB->AddFile( driver->m_sMountPoint, 0, 0 );
			}
		}
	}
};
static RageFileDriverMountpoints *g_Mountpoints = nullptr;

static std::string ExtractDirectory( std::string sPath )
{
	// return the directory containing sPath
	size_t n = sPath.find_last_of("/");
	if( n != sPath.npos )
		sPath.erase(n);
	else
		sPath.erase();
	return sPath;
}

static std::string ReadlinkRecursive( std::string sPath )
{
#if defined(UNIX) || defined(MACOSX)
	// unices support symbolic links; dereference them
	std::string dereferenced = sPath;
	do
	{
		sPath = dereferenced;
		char derefPath[512];
		ssize_t linkSize = readlink(sPath.c_str(), derefPath, sizeof(derefPath));
		if ( linkSize != -1 && linkSize != sizeof(derefPath) )
		{
			dereferenced = std::string( derefPath, linkSize );
			if (derefPath[0] != '/')
			{
				// relative link
				dereferenced = std::string( ExtractDirectory(sPath) + "/" + dereferenced);
			}
		}
	} while (sPath != dereferenced);
#endif

	return sPath;
}

static std::string GetDirOfExecutable( std::string argv0 )
{
	// argv[0] can be wrong in most OS's; try to avoid using it.

	std::string sPath;
#if defined(WIN32)
	char szBuf[MAX_PATH];
	GetModuleFileName( nullptr, szBuf, sizeof(szBuf) );
	sPath = szBuf;
#else
	sPath = argv0;
#endif

	Rage::replace(sPath, '\\', '/' );

	bool bIsAbsolutePath = false;
	if( sPath.size() == 0 || sPath[0] == '/' )
		bIsAbsolutePath = true;
#if defined(WIN32)
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

			auto vPath = Rage::split(path, ":");
			for (auto &i: vPath)
			{
				if( access((i + "/" + argv0).c_str(), X_OK|R_OK) )
				{
					continue;
				}
				sPath = ExtractDirectory(ReadlinkRecursive(i + "/" + argv0));
				break;
			}
			if( sPath.empty() )
			{
				sPath = GetCwd(); // What?
			}
			else if( sPath[0] != '/' ) // For example, if . is in $PATH.
			{
				sPath = GetCwd() + "/" + sPath;
			}
		}
		else
		{
			sPath = ExtractDirectory(ReadlinkRecursive(GetCwd() + "/" + argv0));
		}
#else
		sPath = GetCwd() + "/" + sPath;
		Rage::replace(sPath, '\\', '/' );
#endif
	}
	return sPath;
}

static void ChangeToDirOfExecutable( const std::string &argv0 )
{
	RageFileManagerUtil::sInitialWorkingDirectory = GetCwd();
	RageFileManagerUtil::sDirOfExecutable = GetDirOfExecutable( argv0 );

	/* Set the CWD.  Any effects of this is platform-specific; most files are read and
	 * written through RageFile.  See also RageFileManager::RageFileManager. */
#if defined(_WINDOWS)
	if( _chdir( (RageFileManagerUtil::sDirOfExecutable + "/..").c_str() ) )
#elif defined(UNIX)
	if( chdir( (RageFileManagerUtil::sDirOfExecutable + "/").c_str() ) )
#elif defined(MACOSX)
	/* If the basename is not MacOS, then we've likely been launched via the command line
	 * through a symlink. Assume this is the case and change to the dir of the symlink. */
	if( Rage::base_name(RageFileManagerUtil::sDirOfExecutable) == "MacOS" )
		CollapsePath( RageFileManagerUtil::sDirOfExecutable += "/../../../" );
	if( chdir( RageFileManagerUtil::sDirOfExecutable.c_str() ) )
#endif
	{
		LOG->Warn("Can't set current working directory to %s", RageFileManagerUtil::sDirOfExecutable.c_str());
		return;
	}
}

RageFileManager::RageFileManager( const std::string &argv0 )
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

void RageFileManager::send_init_mount_errors_to_log()
{
	ASSERT_M(LOG != nullptr, "Cannot call RageFileManager::send_init_mount_errors_to_log before LOG is created.");
	for(auto&& message : m_init_mount_errors)
	{
		LOG->Warn(message);
	}
	m_init_mount_errors.clear();
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
std::string LoadedDriver::GetPath( const std::string &sPath ) const
{
	/* If the path begins with /@, only match mountpoints that begin with /@. */
	if( sPath.size() >= 2 && sPath[1] == '@' )
	{
		if( m_sMountPoint.size() < 2 || m_sMountPoint[1] != '@' )
			return std::string();
	}

	Rage::ci_ascii_string localPath{ Rage::head(sPath, m_sMountPoint.size()).c_str() };
	if (localPath != m_sMountPoint)
	{
		return ""; /* no match */
	}
	/* Add one, so we don't cut off the leading slash. */
	return Rage::tail(sPath, sPath.size() - m_sMountPoint.size() + 1);
}

static void NormalizePath( std::string &sPath )
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

bool ilt( const std::string &a, const std::string &b )
{
	return Rage::ci_ascii_string{ a.c_str() } < Rage::ci_ascii_string{ b.c_str() };
}
bool ieq( const std::string &a, const std::string &b ) 
{
	return Rage::ci_ascii_string{ a.c_str() } == Rage::ci_ascii_string{ b.c_str() };
}
void RageFileManager::GetDirListing( std::string const &sPath_, vector<std::string> &AddTo, bool bOnlyDirs, bool bReturnPathToo )
{
	std::string sPath = sPath_;
	NormalizePath( sPath );

	// NormalizePath() calls CollapsePath() which will remove "dir/.." pairs.
	// So if a "/.." is still present, they're trying to go below the root,
	// which isn't valid.
	if( sPath.find("/..") != std::string::npos )
	{
		return;
	}
	vector<LoadedDriver *> apDriverList;
	ReferenceAllDrivers( apDriverList );

	int iDriversThatReturnedFiles = 0;
	int iOldSize = AddTo.size();
	for (auto *pLoadedDriver: apDriverList)
	{
		const std::string p = pLoadedDriver->GetPath( sPath );
		if( p.size() == 0 )
		{
			continue;
		}
		const unsigned OldStart = AddTo.size();

		pLoadedDriver->m_pDriver->GetDirListing( p, AddTo, bOnlyDirs, bReturnPathToo );
		if( AddTo.size() != OldStart )
		{
			++iDriversThatReturnedFiles;
		}
		/* If returning the path, prepend the mountpoint name to the files this driver returned. */
		if( bReturnPathToo && pLoadedDriver->m_sMountPoint.size() > 0 )
		{
			auto const &mountPoint = pLoadedDriver->m_sMountPoint;
			/* Skip the trailing slash on the mountpoint; there's already a slash there. */
			auto const &trimPoint = mountPoint.substr(0, mountPoint.size() - 1);
			std::for_each(AddTo.begin() + OldStart, AddTo.end(), [&trimPoint](std::string &path) {
				path = trimPoint + path;
			});
		}
	}

	UnreferenceAllDrivers( apDriverList );

	// Remove files that start with ._ from the list because these are special
	// OS X files that cause interference on other platforms. -Kyz
	StripMacResourceForks(AddTo);

	if( iDriversThatReturnedFiles > 1 )
	{
		/* More than one driver returned files.  Remove duplicates (case-insensitively). */
		sort( AddTo.begin()+iOldSize, AddTo.end(), ilt );
		auto it = unique( AddTo.begin()+iOldSize, AddTo.end(), ieq );
		AddTo.erase( it, AddTo.end() );
	}
}

void RageFileManager::GetDirListingWithMultipleExtensions(
	std::string const &path, vector<std::string> const& ext_list,
	vector<std::string> &add_to, bool only_dirs, bool return_path_too)
{
	vector<std::string> ret;
	GetDirListing(path + "*", ret, only_dirs, return_path_too);
	for(auto&& item : ret)
	{
		std::string item_ext= GetExtension(item);
		for(auto&& check_ext : ext_list)
		{
			if(item_ext == check_ext)
			{
				add_to.push_back(item);
			}
		}
	}
}

/* Files may only be moved within the same file driver. */
bool RageFileManager::Move( const std::string &sOldPath_, const std::string &sNewPath_ )
{
	std::string sOldPath = sOldPath_;
	std::string sNewPath = sNewPath_;

	vector<LoadedDriver *> aDriverList;
	ReferenceAllDrivers( aDriverList );

	NormalizePath( sOldPath );
	NormalizePath( sNewPath );

	/* Multiple drivers may have the same file. */
	bool Deleted = false;
	for (auto *driver: aDriverList)
	{
		const std::string sOldDriverPath = driver->GetPath( sOldPath );
		const std::string sNewDriverPath = driver->GetPath( sNewPath );
		if( sOldDriverPath.size() == 0 || sNewDriverPath.size() == 0 )
			continue;

		bool ret = driver->m_pDriver->Move( sOldDriverPath, sNewDriverPath );
		if( ret )
			Deleted = true;
	}

	UnreferenceAllDrivers( aDriverList );

	return Deleted;
}

bool RageFileManager::Remove( const std::string &sPath_ )
{
	std::string sPath = sPath_;

	vector<LoadedDriver *> apDriverList;
	ReferenceAllDrivers( apDriverList );

	NormalizePath( sPath );

	/* Multiple drivers may have the same file. */
	bool bDeleted = false;
	for (auto *driver: apDriverList)
	{
		const std::string p = driver->GetPath( sPath );
		if( p.size() == 0 )
			continue;

		bool ret = driver->m_pDriver->Remove( p );
		if( ret )
			bDeleted = true;
	}

	UnreferenceAllDrivers( apDriverList );

	return bDeleted;
}

bool RageFileManager::DeleteRecursive( const std::string &sPath )
{
	// On some OS's, non-empty directories cannot be deleted.
	// This is a work-around that can delete both files and non-empty directories
	return ::DeleteRecursive(sPath);
}

void RageFileManager::CreateDir( const std::string &sDir )
{
	std::string sTempFile = sDir + "newdir.temp.newdir";
	RageFile f;
	f.Open( sTempFile, RageFile::WRITE );
	f.Close();

	Remove( sTempFile );
}

static void AdjustMountpoint( std::string &sMountPoint )
{
	FixSlashesInPlace( sMountPoint );

	ASSERT_M( Rage::starts_with( sMountPoint, "/" ), "Mountpoints must be absolute: " + sMountPoint );

	if (sMountPoint.size() && !Rage::ends_with(sMountPoint, "/"))
	{
		sMountPoint += '/';
	}
	if (!Rage::starts_with(sMountPoint, "/"))
	{
		sMountPoint = "/" + sMountPoint;
	}
}

static void AddFilesystemDriver( LoadedDriver *pLoadedDriver, bool bAddToEnd )
{
	g_Mutex->Lock();
	g_pDrivers.insert( bAddToEnd? g_pDrivers.end():g_pDrivers.begin(), pLoadedDriver );
	g_Mountpoints->LoadFromDrivers( g_pDrivers );
	g_Mutex->Unlock();
}

bool RageFileManager::Mount( const std::string &sType, const std::string &sRoot_, const std::string &sMountPoint_, bool bAddToEnd )
{
	std::string sRoot = sRoot_;
	std::string sMountPoint = sMountPoint_;

	FixSlashesInPlace( sRoot );
	AdjustMountpoint( sMountPoint );

	ASSERT( !sRoot.empty() );

	const std::string &sPaths = fmt::sprintf( "\"%s\", \"%s\", \"%s\"", sType.c_str(), sRoot.c_str(), sMountPoint.c_str() );
	CHECKPOINT_M( sPaths );
#if defined(DEBUG)
	puts( sPaths.c_str() );
#endif

	// Unmount anything that was previously mounted here.
	Unmount( sType, sRoot, sMountPoint );

	CHECKPOINT_M( fmt::sprintf("About to make a driver with \"%s\", \"%s\"", sType.c_str(), sRoot.c_str()));
	RageFileDriver *pDriver = MakeFileDriver( sType, sRoot );
	if( pDriver == nullptr )
	{
		std::string message= fmt::sprintf("Can't mount unknown VFS type \"%s\", root \"%s\"", sType.c_str(), sRoot.c_str());
		CHECKPOINT_M(message);

		if(LOG)
		{
			LOG->Warn(message);
		}
		else
		{
			m_init_mount_errors.push_back(message);
		}
		return false;
	}

	CHECKPOINT_M("Driver %s successfully made.");

	LoadedDriver *pLoadedDriver = new LoadedDriver;
	pLoadedDriver->m_pDriver = pDriver;
	pLoadedDriver->m_sType = sType;
	pLoadedDriver->m_sRoot = sRoot;
	pLoadedDriver->m_sMountPoint = sMountPoint;

	AddFilesystemDriver( pLoadedDriver, bAddToEnd );
	return true;
}

/* Mount a custom filesystem. */
void RageFileManager::Mount( RageFileDriver *pDriver, const std::string &sMountPoint_, bool bAddToEnd )
{
	std::string sMountPoint = sMountPoint_;

	AdjustMountpoint( sMountPoint );

	LoadedDriver *pLoadedDriver = new LoadedDriver;
	pLoadedDriver->m_pDriver = pDriver;
	pLoadedDriver->m_sType = "";
	pLoadedDriver->m_sRoot = "";
	pLoadedDriver->m_sMountPoint = sMountPoint;

	AddFilesystemDriver( pLoadedDriver, bAddToEnd );
}

void RageFileManager::Unmount( const std::string &sType, const std::string &sRoot_, const std::string &sMountPoint_ )
{
	std::string sRoot = sRoot_;
	std::string sMountPoint = sMountPoint_;

	FixSlashesInPlace( sRoot );
	FixSlashesInPlace( sMountPoint );

	if (sMountPoint.size() && !Rage::ends_with(sMountPoint, "/"))
	{
		sMountPoint += '/';
	}
	/* Find all drivers we want to delete.  Remove them from g_pDrivers, and move them
	 * into aDriverListToUnmount. */
	vector<LoadedDriver *> apDriverListToUnmount;
	g_Mutex->Lock();
	for( unsigned i = 0; i < g_pDrivers.size(); ++i )
	{
		if (!sType.empty() && Rage::ci_ascii_string{ sType.c_str() } != g_pDrivers[i]->m_sType)
			continue;
		if (!sRoot.empty() && Rage::ci_ascii_string{ sRoot.c_str() } != g_pDrivers[i]->m_sRoot)
			continue;
		if (!sMountPoint.empty() && Rage::ci_ascii_string{ sMountPoint.c_str() } != g_pDrivers[i]->m_sMountPoint)
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

void RageFileManager::Remount( std::string sMountpoint, std::string sPath )
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

bool RageFileManager::IsMounted( std::string MountPoint )
{
	LockMut( *g_Mutex );
	Rage::ci_ascii_string ciMount{ MountPoint.c_str() };
	for (unsigned i = 0; i < g_pDrivers.size(); ++i)
	{
		if (ciMount == g_pDrivers[i]->m_sMountPoint)
		{
			return true;
		}
	}
	return false;
}

void RageFileManager::GetLoadedDrivers( vector<DriverLocation> &asMounts )
{
	LockMut( *g_Mutex );

	for (auto *driver: g_pDrivers)
	{
		DriverLocation l;
		l.MountPoint = driver->m_sMountPoint;
		l.Type = driver->m_sType;
		l.Root = driver->m_sRoot;
		asMounts.push_back( l );
	}
}

void RageFileManager::FlushDirCache( const std::string &sPath_ )
{
	std::string sPath = sPath_;

	LockMut( *g_Mutex );

	if( sPath == "" )
	{
		for (auto *driver: g_pDrivers)
		{
			driver->m_pDriver->FlushDirCache( "" );
		}
		return;
	}

	/* Flush a specific path. */
	NormalizePath( sPath );
	for (auto *driver: g_pDrivers)
	{
		const std::string &path = driver->GetPath( sPath );
		if( path.size() == 0 )
			continue;
		driver->m_pDriver->FlushDirCache( path );
	}
}

RageFileManager::FileType RageFileManager::GetFileType( const std::string &sPath_ )
{
	std::string sPath = sPath_;

	NormalizePath( sPath );

	vector<LoadedDriver *> apDriverList;
	ReferenceAllDrivers( apDriverList );

	RageFileManager::FileType ret = TYPE_NONE;
	for (auto *driver: apDriverList)
	{
		const std::string p = driver->GetPath( sPath );
		if( p.size() == 0 )
			continue;
		ret = driver->m_pDriver->GetFileType( p );
		if( ret != TYPE_NONE )
			break;
	}

	UnreferenceAllDrivers( apDriverList );

	return ret;
}


int RageFileManager::GetFileSizeInBytes( const std::string &sPath_ )
{
	std::string sPath = sPath_;

	NormalizePath( sPath );

	vector<LoadedDriver *> apDriverList;
	ReferenceAllDrivers( apDriverList );

	int iRet = -1;
	for (auto *driver: apDriverList)
	{
		const std::string p = driver->GetPath( sPath );
		if( p.size() == 0 )
			continue;
		iRet = driver->m_pDriver->GetFileSizeInBytes( p );
		if( iRet != -1 )
			break;
	}
	UnreferenceAllDrivers( apDriverList );

	return iRet;
}

int RageFileManager::GetFileHash( const std::string &sPath_ )
{
	std::string sPath = sPath_;

	NormalizePath( sPath );

	vector<LoadedDriver *> apDriverList;
	ReferenceAllDrivers( apDriverList );

	int iRet = -1;
	for (auto *driver: apDriverList)
	{
		const std::string p = driver->GetPath( sPath );
		if( p.size() == 0 )
			continue;
		iRet = driver->m_pDriver->GetFileHash( p );
		if( iRet != -1 )
			break;
	}
	UnreferenceAllDrivers( apDriverList );

	return iRet;
}

std::string RageFileManager::ResolvePath(const std::string &path)
{
	std::string tmpPath = path;
	NormalizePath(tmpPath);

	std::string resolvedPath = tmpPath;

	vector<LoadedDriver *> apDriverList;
	ReferenceAllDrivers( apDriverList );

	for (auto *pDriver: apDriverList)
	{
		const std::string driverPath = pDriver->GetPath( tmpPath );

		if ( driverPath.empty() || pDriver->m_sRoot.empty() )
			continue;

		if ( pDriver->m_sType != "dir" && pDriver->m_sType != "dirro" )
			continue;

		int iMountPointLen = pDriver->m_sMountPoint.length();
		if( tmpPath.substr(0, iMountPointLen) != pDriver->m_sMountPoint )
			continue;

		resolvedPath = pDriver->m_sRoot + "/" + std::string(tmpPath.substr(iMountPointLen));
		break;
	}

	UnreferenceAllDrivers( apDriverList );

	NormalizePath( resolvedPath );

	return resolvedPath;
}

static bool SortBySecond( const std::pair<int,int> &a, const std::pair<int,int> &b )
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
static bool PathUsesSlowFlush( const std::string &sPath )
{
	static std::array<std::string, 2> const FlushPaths =
	{
		{
			"/Save/",
			"Save/"
		}
	};

	auto doesPathMatch = [&sPath](std::string const &curPath) {
		return !strncmp(sPath.c_str(), curPath.c_str(), strlen(curPath.c_str()));
	};

	return std::any_of(FlushPaths.begin(), FlushPaths.end(), doesPathMatch);
}

/* Used only by RageFile: */
RageFileBasic *RageFileManager::Open( const std::string &sPath_, int mode, int &err )
{
	std::string sPath = sPath_;

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

void RageFileManager::CacheFile( const RageFileBasic *fb, const std::string &sPath_ )
{
	auto it = g_mFileDriverMap.find( fb );

	ASSERT_M( it != g_mFileDriverMap.end(), fmt::sprintf("No recorded driver for file: %s", sPath_.c_str()) );

	std::string sPath = sPath_;
	NormalizePath( sPath );
	sPath = it->second->GetPath( sPath );
	it->second->m_pDriver->FDB->CacheFile( sPath );
	g_mFileDriverMap.erase( it );
}

RageFileBasic *RageFileManager::OpenForReading( const std::string &sPath, int mode, int &err )
{
	vector<LoadedDriver *> apDriverList;
	ReferenceAllDrivers( apDriverList );

	for (auto *ld: apDriverList)
	{
		const std::string path = ld->GetPath( sPath );
		if( path.size() == 0 )
			continue;
		int error;
		RageFileBasic *ret = ld->m_pDriver->Open( path, mode, error );
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

RageFileBasic *RageFileManager::OpenForWriting( const std::string &sPath, int mode, int &iError )
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
	vector<LoadedDriver *> apDriverList;
	ReferenceAllDrivers( apDriverList );

	vector< std::pair<int,int> > Values;
	for( unsigned i = 0; i < apDriverList.size(); ++i )
	{
		LoadedDriver &ld = *apDriverList[i];
		const std::string path = ld.GetPath( sPath );
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
	for (auto &value: Values)
	{
		const int iDriver = value.first;
		if( iDriver > iMaximumDriver )
			continue;
		LoadedDriver &ld = *apDriverList[iDriver];
		const std::string sDriverPath = ld.GetPath( sPath );
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

bool RageFileManager::IsAFile( const std::string &sPath ) { return GetFileType(sPath) == TYPE_FILE; }
bool RageFileManager::IsADirectory( const std::string &sPath ) { return GetFileType(sPath) == TYPE_DIR; }
bool RageFileManager::DoesFileExist( const std::string &sPath ) { return GetFileType(sPath) != TYPE_NONE; }

bool DoesFileExist( const std::string &sPath )
{
	return FILEMAN->DoesFileExist( sPath );
}

bool IsAFile( const std::string &sPath )
{
	return FILEMAN->IsAFile( sPath );
}

bool IsADirectory( const std::string &sPath )
{
	return FILEMAN->IsADirectory( sPath );
}

int GetFileSizeInBytes( const std::string &sPath )
{
	return FILEMAN->GetFileSizeInBytes( sPath );
}

void GetDirListing( std::string const &sPath, vector<std::string> &AddTo, bool bOnlyDirs, bool bReturnPathToo )
{
	FILEMAN->GetDirListing( sPath, AddTo, bOnlyDirs, bReturnPathToo );
}

void GetDirListingRecursive( std::string const &sDir, std::string const &sMatch, vector<std::string> &AddTo )
{
	ASSERT(Rage::ends_with(sDir, "/"));
	vector<std::string> vsFiles;
	GetDirListing( sDir+sMatch, vsFiles, false, true );
	vector<std::string> vsDirs;
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
		{
			AddTo.push_back( vsFiles[i] );
		}
	}
}

void GetDirListingRecursive( RageFileDriver *prfd, std::string const &sDir, std::string const &sMatch, vector<std::string> &AddTo )
{
	ASSERT(Rage::ends_with(sDir, "/"));
	vector<std::string> vsFiles;
	prfd->GetDirListing( sDir+sMatch, vsFiles, false, true );
	vector<std::string> vsDirs;
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

bool DeleteRecursive( RageFileDriver *prfd, const std::string &sDir )
{
	ASSERT(Rage::ends_with(sDir, "/"));

	vector<std::string> vsFiles;
	prfd->GetDirListing( sDir+"*", vsFiles, false, true );
	for (auto const &s: vsFiles)
	{
		if( IsADirectory(s) )
		{
			DeleteRecursive( s+"/" );
		}
		else
		{
			FILEMAN->Remove( s );
		}
	}

	return FILEMAN->Remove( sDir );
}

bool DeleteRecursive( const std::string &sDir )
{
	ASSERT(Rage::ends_with(sDir, "/"));

	vector<std::string> vsFiles;
	GetDirListing( sDir+"*", vsFiles, false, true );
	for (auto const &s: vsFiles)
	{
		if( IsADirectory(s) )
		{
			DeleteRecursive( s+"/" );
		}
		else
		{
			FILEMAN->Remove( s );
		}
	}

	return FILEMAN->Remove( sDir );
}

unsigned int GetHashForFile( const std::string &sPath )
{
	return FILEMAN->GetFileHash( sPath );
}

unsigned int GetHashForDirectory( const std::string &sDir )
{
	unsigned int hash = 0;

	hash += GetHashForString( sDir );

	vector<std::string> arrayFiles;
	GetDirListing( sDir+"*", arrayFiles, false );
	for (auto &file: arrayFiles)
	{
		const std::string sFilePath = sDir + file;
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
	static int DoesFileExist( T* p, lua_State *L ){ lua_pushboolean( L, p->DoesFileExist(SArg(1)) ); return 1; }
	static int GetFileSizeBytes( T* p, lua_State *L ){ lua_pushnumber( L, p->GetFileSizeInBytes(SArg(1)) ); return 1; }
	static int GetHashForFile( T* p, lua_State *L ){ lua_pushnumber( L, p->GetFileHash(SArg(1)) ); return 1; }
	static int GetDirListing( T* p, lua_State *L )
	{
		vector<std::string> vDirs;
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
	/*
	static int GetDirListingRecursive( T* p, lua_State *L )
	{
		vector<std::string> vDirs;
		// (directory, match, addto)
		GetDirListingRecursive( SArg(1), SArg(2), vDirs );
		LuaHelpers::CreateTableFromArray(vDirs, L);
		return 1;
	}
	*/

	LunaRageFileManager()
	{
		ADD_METHOD( DoesFileExist );
		ADD_METHOD( GetFileSizeBytes );
		ADD_METHOD( GetHashForFile );
		ADD_METHOD( GetDirListing );
		//ADD_METHOD( GetDirListingRecursive );
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
