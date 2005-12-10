#include "global.h"
#include "RageFileManager.h"
#include "RageFileDriver.h"
#include "RageFile.h"
#include "RageUtil.h"
#include "RageUtil_FileDB.h"
#include "RageLog.h"
#include "RageThreads.h"
#include "Foreach.h"
#include "arch/ArchHooks/ArchHooks.h"

#include <cerrno>
#if defined(LINUX)
#include <sys/stat.h>
#endif

#if defined(WIN32) && !defined(XBOX)
#include <windows.h>
#endif

RageFileManager *FILEMAN = NULL;

/* Lock this before touching any of these globals (except FILEMAN itself). */
static RageEvent *g_Mutex;

CString RageFileManagerUtil::sInitialWorkingDirectory;
CString RageFileManagerUtil::sDirOfExecutable;

struct LoadedDriver
{
	/* A loaded driver may have a base path, which modifies the path we
	 * pass to the driver.  For example, if the base is "Songs/", and we
	 * want to send the path "Songs/Foo/Bar" to it, then we actually
	 * only send "Foo/Bar".  The path "Themes/Foo" is out of the scope
	 * of the driver, and GetPath returns false. */
	RageFileDriver *m_pDriver;
	CString m_sType, m_sRoot, m_sMountPoint;

	int m_iRefs;

	LoadedDriver() { m_pDriver = NULL; m_iRefs = 0; }
	CString GetPath( const CString &sPath ) const;
};

static vector<LoadedDriver *> g_pDrivers;

static void ReferenceAllDrivers( vector<LoadedDriver *> &apDriverList )
{
	g_Mutex->Lock();
	apDriverList = g_pDrivers;
	for( unsigned i = 0; i < apDriverList.size(); ++i )
		++apDriverList[i]->m_iRefs;
	g_Mutex->Unlock();
}

static void UnreferenceAllDrivers( vector<LoadedDriver *> &apDriverList )
{
	g_Mutex->Lock();
	for( unsigned i = 0; i < apDriverList.size(); ++i )
		--apDriverList[i]->m_iRefs;
	g_Mutex->Broadcast();
	g_Mutex->Unlock();

	/* Clear the temporary list, to make it clear that the drivers may no longer be accessed. */
	apDriverList.clear();
}

RageFileDriver *RageFileManager::GetFileDriver( CString sMountpoint )
{
	FixSlashesInPlace( sMountpoint );
	if( sMountpoint.size() && sMountpoint.Right(1) != "/" )
		sMountpoint += '/';

	g_Mutex->Lock();
	RageFileDriver *pRet = NULL;
	for( unsigned i = 0; i < g_pDrivers.size(); ++i )
	{
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
	ASSERT( pDriver != NULL );

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

	while(1)
	{
		unsigned i;
		for( i = 0; i < g_pDrivers.size(); ++i )
			if( g_pDrivers[i]->m_pDriver == pDriver )
				break;

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
	RageFileBasic *Open( const CString &sPath, int iMode, int &iError )
	{
		iError = (iMode == RageFile::WRITE)? ERROR_WRITING_NOT_SUPPORTED:ENOENT;
		return NULL;
	}
	/* Never flush FDB, except in LoadFromDrivers. */
	void FlushDirCache( const CString &sPath ) { }

	void LoadFromDrivers( const vector<LoadedDriver *> &apDrivers )
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
static RageFileDriverMountpoints *g_Mountpoints = NULL;

static CString GetDirOfExecutable( CString argv0 )
{
#ifdef _XBOX
	return "D:\\";
#else
	/* argv[0] can be wrong in most OS's; try to avoid using it. */

	CString sPath;
#if defined(WIN32)
	char szBuf[MAX_PATH];
	GetModuleFileName( NULL, szBuf, sizeof(szBuf) );
	sPath = szBuf;
#else
	sPath = argv0;
#endif

	sPath.Replace( "\\", "/" );

	bool bIsAbsolutePath = false;
	if( sPath.size() == 0 || sPath[0] == '/' )
		bIsAbsolutePath = true;
#if defined(WIN32)
	if( sPath.size() > 2 && sPath[1] == ':' && sPath[2] == '/' )
		bIsAbsolutePath = true;
#endif

	// strip off executable name
	size_t n = sPath.find_last_of("/");
	if( n != sPath.npos )
		sPath.erase(n);
	else
		sPath.erase();

	if( !bIsAbsolutePath )
	{
		sPath = GetCwd() + "/" + sPath;
		sPath.Replace( "\\", "/" );
	}

	return sPath;
#endif
}

static void ChangeToDirOfExecutable( CString argv0 )
{
	RageFileManagerUtil::sInitialWorkingDirectory = GetCwd();
	RageFileManagerUtil::sDirOfExecutable = GetDirOfExecutable( argv0 );

	/* Set the CWD.  Any effects of this is platform-specific; most files are read and
	 * written through RageFile.  See also RageFileManager::RageFileManager. */
#if defined(_WINDOWS)
	chdir( RageFileManagerUtil::sDirOfExecutable + "/.." );
#elif defined(MACOSX)
	chdir( RageFileManagerUtil::sDirOfExecutable + "/../../.." );
#endif
}

RageFileManager::RageFileManager( CString argv0 )
{
	CHECKPOINT_M( argv0 );
	ChangeToDirOfExecutable( argv0 );
	
	g_Mutex = new RageEvent("RageFileManager");

	g_Mountpoints = new RageFileDriverMountpoints;
	LoadedDriver *pLoadedDriver = new LoadedDriver;
	pLoadedDriver->m_pDriver = g_Mountpoints;
	pLoadedDriver->m_sMountPoint = "/";
	g_pDrivers.push_back( pLoadedDriver );

	/* The mount path is unused, but must be nonempty. */
	RageFileManager::Mount( "mem", "(cache)", "/@mem" );
}

void RageFileManager::MountInitialFilesystems()
{
	HOOKS->MountInitialFilesystems( RageFileManagerUtil::sDirOfExecutable );
}

RageFileManager::~RageFileManager()
{
	/* Note that drivers can use previously-loaded drivers, eg. to load a ZIP
	 * from the FS.  Unload drivers in reverse order. */
	for( int i = g_pDrivers.size()-1; i >= 0; --i )
	{
		delete g_pDrivers[i]->m_pDriver;
		delete g_pDrivers[i];
	}
	g_pDrivers.clear();

//	delete g_Mountpoints; // g_Mountpoints was in g_pDrivers
	g_Mountpoints = NULL;

	delete g_Mutex;
	g_Mutex = NULL;
}

/* path must be normalized (FixSlashesInPlace, CollapsePath). */
CString LoadedDriver::GetPath( const CString &sPath ) const
{
	/* If the path begins with /@, only match mountpoints that begin with /@. */
	if( sPath.size() >= 2 && sPath[1] == '@' )
	{
		if( m_sMountPoint.size() < 2 || m_sMountPoint[1] != '@' )
			return CString();
	}
	
	if( sPath.Left(m_sMountPoint.size()).CompareNoCase(m_sMountPoint) )
		return CString(); /* no match */

	/* Add one, so we don't cut off the leading slash. */
	CString sRet = sPath.Right( sPath.size() - m_sMountPoint.size() + 1 );
	return sRet;
}

static void NormalizePath( CString &sPath )
{
	FixSlashesInPlace( sPath );
	CollapsePath( sPath, true );
	if( sPath.size() == 0 || sPath[0] != '/' )
		sPath.insert( sPath.begin(), '/' );
}

bool ilt( const CString &a, const CString &b ) { return a.CompareNoCase(b) < 0; }
bool ieq( const CString &a, const CString &b ) { return a.CompareNoCase(b) == 0; }
void RageFileManager::GetDirListing( CString sPath, vector<CString> &AddTo, bool bOnlyDirs, bool bReturnPathToo )
{
	NormalizePath( sPath );

	vector<LoadedDriver *> apDriverList;
	ReferenceAllDrivers( apDriverList );

	int iDriversThatReturnedFiles = 0;
	int iOldSize = AddTo.size();
	for( unsigned i = 0; i < apDriverList.size(); ++i )
	{
		LoadedDriver *pLoadedDriver = apDriverList[i];
		const CString p = pLoadedDriver->GetPath( sPath );
		if( p.size() == 0 )
			continue;

		const unsigned OldStart = AddTo.size();
		
		pLoadedDriver->m_pDriver->GetDirListing( p, AddTo, bOnlyDirs, bReturnPathToo );
		if( AddTo.size() != OldStart )
			++iDriversThatReturnedFiles;

		/* If returning the path, prepend the mountpoint name to the files this driver returned. */
		if( bReturnPathToo && pLoadedDriver->m_sMountPoint.size() > 0 )
		{
			for( unsigned j = OldStart; j < AddTo.size(); ++j )
			{
				/* Skip the trailing slash on the mountpoint; there's already a slash there. */
				CString &sPath = AddTo[j];
				sPath.insert( 0, pLoadedDriver->m_sMountPoint, pLoadedDriver->m_sMountPoint.size()-1 );
			}
		}
	}

	UnreferenceAllDrivers( apDriverList );

	if( iDriversThatReturnedFiles > 1 )
	{
		/* More than one driver returned files.  Remove duplicates (case-insensitively). */
		sort( AddTo.begin()+iOldSize, AddTo.end(), ilt );
		vector<CString>::iterator it = unique( AddTo.begin()+iOldSize, AddTo.end(), ieq );
		AddTo.erase( it, AddTo.end() );
	}
}

/* Files may only be moved within the same file driver. */
bool RageFileManager::Move( CString sOldPath, CString sNewPath )
{
	vector<LoadedDriver *> aDriverList;
	ReferenceAllDrivers( aDriverList );

	NormalizePath( sOldPath );
	NormalizePath( sNewPath );
	
	/* Multiple drivers may have the same file. */
	bool Deleted = false;
	for( unsigned i = 0; i < aDriverList.size(); ++i )
	{
		const CString sOldDriverPath = aDriverList[i]->GetPath( sOldPath );
		const CString sNewDriverPath = aDriverList[i]->GetPath( sNewPath );
		if( sOldDriverPath.size() == 0 || sNewDriverPath.size() == 0 )
			continue;

		bool ret = aDriverList[i]->m_pDriver->Move( sOldDriverPath, sNewDriverPath );
		if( ret )
			Deleted = true;
	}

	UnreferenceAllDrivers( aDriverList );

	return Deleted;
}

bool RageFileManager::Remove( CString sPath )
{
	vector<LoadedDriver *> apDriverList;
	ReferenceAllDrivers( apDriverList );

	NormalizePath( sPath );
	
	/* Multiple drivers may have the same file. */
	bool bDeleted = false;
	for( unsigned i = 0; i < apDriverList.size(); ++i )
	{
		const CString p = apDriverList[i]->GetPath( sPath );
		if( p.size() == 0 )
			continue;

		bool ret = apDriverList[i]->m_pDriver->Remove( p );
		if( ret )
			bDeleted = true;
	}

	UnreferenceAllDrivers( apDriverList );

	return bDeleted;
}

void RageFileManager::CreateDir( CString sDir )
{
	CString sTempFile = sDir + "temp";
	RageFile f;
	f.Open( sTempFile, RageFile::WRITE );
	f.Close();

	// YUCK: The dir cache doesn't have this new file we just created,
	// so the delete will fail unless we flush.
	FILEMAN->FlushDirCache( sDir );

	FILEMAN->Remove( sTempFile );
}

static void AdjustMountpoint( CString &sMountPoint )
{
	FixSlashesInPlace( sMountPoint );
	
	ASSERT_M( sMountPoint.Left(1) == "/", "Mountpoints must be absolute: " + sMountPoint );

	if( sMountPoint.size() && sMountPoint.Right(1) != "/" )
		sMountPoint += '/';

	if( sMountPoint.Left(1) != "/" )
		sMountPoint = "/" + sMountPoint;

}

static void AddFilesystemDriver( LoadedDriver *pLoadedDriver, bool bAddToEnd )
{
	g_Mutex->Lock();
	g_pDrivers.insert( bAddToEnd? g_pDrivers.end():g_pDrivers.begin(), pLoadedDriver );
	g_Mountpoints->LoadFromDrivers( g_pDrivers );
	g_Mutex->Unlock();
}

void RageFileManager::Mount( CString sType, CString sRoot, CString sMountPoint, bool bAddToEnd )
{
	FixSlashesInPlace( sRoot );
	AdjustMountpoint( sMountPoint );

	ASSERT( !sRoot.empty() );

	CHECKPOINT_M( ssprintf("\"%s\", \"%s\", \"%s\"", sType.c_str(), sRoot.c_str(), sMountPoint.c_str() ) );

	// Unmount anything that was previously mounted here.
	Unmount( sType, sRoot, sMountPoint );

	CHECKPOINT;
	RageFileDriver *pDriver = MakeFileDriver( sType, sRoot );
	if( pDriver == NULL )
	{
		CHECKPOINT;

		if( LOG )
			LOG->Warn("Can't mount unknown VFS type \"%s\", root \"%s\"", sType.c_str(), sRoot.c_str() );
		else
			fprintf( stderr, "Can't mount unknown VFS type \"%s\", root \"%s\"\n", sType.c_str(), sRoot.c_str() );
		return;
	}

	CHECKPOINT;

	LoadedDriver *pLoadedDriver = new LoadedDriver;
	pLoadedDriver->m_pDriver = pDriver;
	pLoadedDriver->m_sType = sType;
	pLoadedDriver->m_sRoot = sRoot;
	pLoadedDriver->m_sMountPoint = sMountPoint;

	AddFilesystemDriver( pLoadedDriver, bAddToEnd );
}

/* Mount a custom filesystem. */
void RageFileManager::Mount( RageFileDriver *pDriver, CString sMountPoint, bool bAddToEnd )
{
	AdjustMountpoint( sMountPoint );

	LoadedDriver *pLoadedDriver = new LoadedDriver;
	pLoadedDriver->m_pDriver = pDriver;
	pLoadedDriver->m_sType = "";
	pLoadedDriver->m_sRoot = "";
	pLoadedDriver->m_sMountPoint = sMountPoint;

	AddFilesystemDriver( pLoadedDriver, bAddToEnd );
}

void RageFileManager::Unmount( CString sType, CString sRoot, CString sMountPoint )
{
	FixSlashesInPlace( sRoot );
	FixSlashesInPlace( sMountPoint );

	if( sMountPoint.size() && sMountPoint.Right(1) != "/" )
		sMountPoint += '/';

	/* Find all drivers we want to delete.  Remove them from g_pDrivers, and move them
	 * into aDriverListToUnmount. */
	vector<LoadedDriver *> apDriverListToUnmount;
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

void RageFileManager::Remount( CString sMountpoint, CString sPath )
{
	RageFileDriver *pDriver = FILEMAN->GetFileDriver( sMountpoint );
	if( pDriver == NULL )
	{
		if( LOG )
			LOG->Warn( "Remount(%s,%s): mountpoint not found", sMountpoint.c_str(), sPath.c_str() );
		return;
	}

	if( !pDriver->Remount(sPath) )
		LOG->Warn( "Remount(%s,%s): remount failed (does the driver support remounting?)", sMountpoint.c_str(), sPath.c_str() );
	else
		pDriver->FlushDirCache( "" );

	FILEMAN->ReleaseFileDriver( pDriver );
}

bool RageFileManager::IsMounted( CString MountPoint )
{
	LockMut( *g_Mutex );

	for( unsigned i = 0; i < g_pDrivers.size(); ++i )
		if( !g_pDrivers[i]->m_sMountPoint.CompareNoCase( MountPoint ) )
			return true;

	return false;
}

void RageFileManager::GetLoadedDrivers( vector<DriverLocation> &asMounts )
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

void RageFileManager::FlushDirCache( CString sPath )
{
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
		const CString &path = g_pDrivers[i]->GetPath( sPath );
		if( path.size() == 0 )
			continue;
		g_pDrivers[i]->m_pDriver->FlushDirCache( path );
	}
}

RageFileManager::FileType RageFileManager::GetFileType( CString sPath )
{
	NormalizePath( sPath );

	vector<LoadedDriver *> apDriverList;
	ReferenceAllDrivers( apDriverList );

	RageFileManager::FileType ret = TYPE_NONE;
	for( unsigned i = 0; i < apDriverList.size(); ++i )
	{
		const CString p = apDriverList[i]->GetPath( sPath );
		if( p.size() == 0 )
			continue;
		ret = apDriverList[i]->m_pDriver->GetFileType( p );
		if( ret != TYPE_NONE )
			break;
	}

	UnreferenceAllDrivers( apDriverList );

	return ret;
}


int RageFileManager::GetFileSizeInBytes( CString sPath )
{
	NormalizePath( sPath );

	vector<LoadedDriver *> apDriverList;
	ReferenceAllDrivers( apDriverList );

	int iRet = -1;
	for( unsigned i = 0; i < apDriverList.size(); ++i )
	{
		const CString p = apDriverList[i]->GetPath( sPath );
		if( p.size() == 0 )
			continue;
		iRet = apDriverList[i]->m_pDriver->GetFileSizeInBytes( p );
		if( iRet != -1 )
			break;
	}
	UnreferenceAllDrivers( apDriverList );

	return iRet;
}

int RageFileManager::GetFileHash( CString sPath )
{
	NormalizePath( sPath );

	vector<LoadedDriver *> apDriverList;
	ReferenceAllDrivers( apDriverList );

	int iRet = -1;
	for( unsigned i = 0; i < apDriverList.size(); ++i )
	{
		const CString p = apDriverList[i]->GetPath( sPath );
		if( p.size() == 0 )
			continue;
		iRet = apDriverList[i]->m_pDriver->GetFileHash( p );
		if( iRet != -1 )
			break;
	}
	UnreferenceAllDrivers( apDriverList );

	return iRet;
}

static bool SortBySecond( const pair<int,int> &a, const pair<int,int> &b )
{
	return a.second < b.second;
}

/*
 * Return true if the given path should use slow, reliable writes.
 *
 * I havn't decided if it's better to do this here, or to specify SLOW_FLUSH
 * manually each place we want it.  This seems more reliable (we might forget
 * somewhere and not notice), and easier (don't have to pass flags down to IniFile::Write,
 * etc).
 */
static bool PathUsesSlowFlush( const CString &sPath )
{
	static const char *FlushPaths[] =
	{
		"Save/"
	};

	for( unsigned i = 0; i < ARRAYSIZE(FlushPaths); ++i )
		if( !strncmp( sPath, FlushPaths[i], strlen(FlushPaths[i]) ) )
			return true;
	return false;
}

/* Used only by RageFile: */
RageFileBasic *RageFileManager::Open( CString sPath, int mode, int &err )
{
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

RageFileBasic *RageFileManager::OpenForReading( CString sPath, int mode, int &err )
{
	vector<LoadedDriver *> apDriverList;
	ReferenceAllDrivers( apDriverList );

	for( unsigned i = 0; i < apDriverList.size(); ++i )
	{
		LoadedDriver &ld = *apDriverList[i];
		const CString path = ld.GetPath( sPath );
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
		 * was reported, return that instead. */
		if( error != ENOENT )
			err = error;
	}
	UnreferenceAllDrivers( apDriverList );

	return NULL;
}

RageFileBasic *RageFileManager::OpenForWriting( CString sPath, int mode, int &iError )
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
	 * "Songs/Music/Waltz/waltz.sm", and the song was loaded out of
	 * "C:/games/DWI/Songs/Music/Waltz/waltz.dwi", we want to write the new SM into the
	 * same directory (if possible).  Don't split up files in the same directory any
	 * more than we have to.
	 *
	 * If the given path can not be created, return -1.  This happens if a path
	 * that needs to be a directory is a file, or vice versa.
	 */
	vector<LoadedDriver *> apDriverList;
	ReferenceAllDrivers( apDriverList );

	vector< pair<int,int> > Values;
	for( unsigned i = 0; i < apDriverList.size(); ++i )
	{
		LoadedDriver &ld = *apDriverList[i];
		const CString path = ld.GetPath( sPath );
		if( path.size() == 0 )
			continue;

		const int value = ld.m_pDriver->GetPathValue( path );
		if( value == -1 )
			continue;

		Values.push_back( make_pair( i, value ) );
	}

	stable_sort( Values.begin(), Values.end(), SortBySecond );

	iError = 0;
	for( unsigned i = 0; i < Values.size(); ++i )
	{
		const int iDriver = Values[i].first;
		LoadedDriver &ld = *apDriverList[iDriver];
		const CString sDriverPath = ld.GetPath( sPath );
		ASSERT( !sDriverPath.empty() );

		int iThisError;
		RageFileBasic *pRet = ld.m_pDriver->Open( sDriverPath, mode, iThisError );
		if( pRet )
		{
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

	return NULL;
}

bool RageFileManager::IsAFile( const CString &sPath ) { return GetFileType(sPath) == TYPE_FILE; }
bool RageFileManager::IsADirectory( const CString &sPath ) { return GetFileType(sPath) == TYPE_DIR; }
bool RageFileManager::DoesFileExist( const CString &sPath ) { return GetFileType(sPath) != TYPE_NONE; }

bool DoesFileExist( const CString &sPath )
{
	return FILEMAN->DoesFileExist( sPath );
}

bool IsAFile( const CString &sPath )
{
	return FILEMAN->IsAFile( sPath );
}

bool IsADirectory( const CString &sPath )
{
	return FILEMAN->IsADirectory( sPath );
}

unsigned GetFileSizeInBytes( const CString &sPath )
{
	return FILEMAN->GetFileSizeInBytes( sPath );
}

void GetDirListing( const CString &sPath, vector<CString> &AddTo, bool bOnlyDirs, bool bReturnPathToo )
{
	FILEMAN->GetDirListing( sPath, AddTo, bOnlyDirs, bReturnPathToo );
}

void GetDirListingRecursive( const CString &sDir, const CString &sMatch, vector<CString> &AddTo )
{
	ASSERT( sDir.Right(1) == "/" );
	GetDirListing( sDir+sMatch, AddTo, false, true );
	GetDirListing( sDir+"*",	AddTo, true,  true );
	for( unsigned i=0; i<AddTo.size(); i++ )
	{
		if( IsADirectory( AddTo[i] ) )
		{
			GetDirListing( AddTo[i]+"/"+sMatch, AddTo, false, true );
			GetDirListing( AddTo[i]+"/*",		AddTo, true,  true );
			AddTo.erase( AddTo.begin()+i );
			i--;
		}
	}
}

bool DeleteRecursive( const CString &sDir )
{
	ASSERT( sDir.Right(1) == "/" );

	vector<CString> vsFiles;
	GetDirListing( sDir+"*", vsFiles, false, true );
	FOREACH_CONST( CString, vsFiles, s )
	{
		if( IsADirectory(*s) )
			DeleteRecursive( *s+"/" );
		else
			FILEMAN->Remove( *s );
	}

	return FILEMAN->Remove( sDir );
}

void FlushDirCache()
{
	FILEMAN->FlushDirCache( "" );
}


unsigned int GetHashForFile( const CString &sPath )
{
	return FILEMAN->GetFileHash( sPath );
}

unsigned int GetHashForDirectory( const CString &sDir )
{
	unsigned int hash = 0;

	hash += GetHashForString( sDir );

	vector<CString> arrayFiles;
	GetDirListing( sDir+"*", arrayFiles, false );
	for( unsigned i=0; i<arrayFiles.size(); i++ )
	{
		const CString sFilePath = sDir + arrayFiles[i];
		hash += GetHashForFile( sFilePath );
	}

	return hash; 
}

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
