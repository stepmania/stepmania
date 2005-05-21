#include "global.h"
#include "RageFileManager.h"
#include "RageFileDriver.h"
#include "RageUtil.h"
#include "RageUtil_FileDB.h"
#include "RageLog.h"
#include "RageThreads.h"

#include <cerrno>
#if defined(LINUX)
#include <sys/stat.h>
#endif

#if defined(_WIN32) && !defined(XBOX)
#include <windows.h>
#endif

RageFileManager *FILEMAN = NULL;

/* Lock this before touching any of these globals (except FILEMAN itself). */
static RageEvent *g_Mutex;

CString InitialWorkingDirectory;
CString DirOfExecutable;

typedef map< const RageFileBasic *, RageFileDriver * > FileReferences;
static FileReferences g_Refs;

struct LoadedDriver
{
	/* A loaded driver may have a base path, which modifies the path we
	 * pass to the driver.  For example, if the base is "Songs/", and we
	 * want to send the path "Songs/Foo/Bar" to it, then we actually
	 * only send "Foo/Bar".  The path "Themes/Foo" is out of the scope
	 * of the driver, and GetPath returns false. */
	RageFileDriver *driver;
	CString Type, Root, MountPoint;

	int m_iRefs;

	LoadedDriver() { driver = NULL; m_iRefs = 0; }
	CString GetPath( const CString &path );
};

static vector<LoadedDriver> g_Drivers;

void ReferenceAllDrivers( vector<LoadedDriver> &aDriverList )
{
	g_Mutex->Lock();
	aDriverList = g_Drivers;
	for( unsigned i = 0; i < aDriverList.size(); ++i )
		++aDriverList[i].m_iRefs;
	g_Mutex->Unlock();
}

void UnreferenceAllDrivers( vector<LoadedDriver> &aDriverList )
{
	g_Mutex->Lock();
	for( unsigned i = 0; i < aDriverList.size(); ++i )
		--aDriverList[i].m_iRefs;
	g_Mutex->Broadcast();
	g_Mutex->Unlock();

	/* Clear the temporary list, to make it clear that the drivers may no longer be accessed. */
	aDriverList.clear();
}

RageFileDriver *RageFileManager::GetFileDriver( CString sMountpoint )
{
	FixSlashesInPlace( sMountpoint );
	if( sMountpoint.size() && sMountpoint.Right(1) != "/" )
		sMountpoint += '/';

	g_Mutex->Lock();
	RageFileDriver *pRet = NULL;
	for( unsigned i = 0; i < g_Drivers.size(); ++i )
	{
		if( g_Drivers[i].MountPoint.CompareNoCase( sMountpoint ) )
			continue;

		pRet = g_Drivers[i].driver;
		++g_Drivers[i].m_iRefs;
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
	for( i = 0; i < g_Drivers.size(); ++i )
	{
		if( g_Drivers[i].driver == pDriver )
			break;
	}
	ASSERT( i != g_Drivers.size() );

	--g_Drivers[i].m_iRefs;

	g_Mutex->Broadcast();
	g_Mutex->Unlock();
}

/* Wait for the given driver to become unreferenced, and remove it from the list
 * to get exclusive access to it.  Returns false if the driver is no longer available
 * (somebody else got it first). */
bool GrabDriver( RageFileDriver *pDriver )
{
	g_Mutex->Lock();

	while(1)
	{
		unsigned i;
		for( i = 0; i < g_Drivers.size(); ++i )
			if( g_Drivers[i].driver == pDriver )
				break;

		if( i == g_Drivers.size() )
		{
			g_Mutex->Unlock();
			return false;
		}

		if( g_Drivers[i].m_iRefs == 0 )
		{
			g_Drivers.erase( g_Drivers.begin()+i );
			return true;
		}

		/* The driver is in use.  Wait for somebody to release a driver, and
		 * try again. */
		g_Mutex->Wait();
	}
}

// Mountpoints as directories cause a problem.  If "Themes/default" is a mountpoint, and
// doesn't exist anywhere else, then GetDirListing("Themes/*") must return "default".  The
// driver containing "Themes/default" won't do this; its world view begins at "BGAnimations"
// (inside "Themes/default").  We need a dummy driver that handles mountpoints. */
class RageFileDriverMountpoints: public RageFileDriver
{
public:
	RageFileDriverMountpoints(): RageFileDriver( new FilenameDB ) { }
	RageFileBasic *Open( const CString &path, int mode, int &err )
	{
		err = (mode == RageFile::WRITE)? ERROR_WRITING_NOT_SUPPORTED:ENOENT;
		return NULL;
	}
	/* Never flush FDB, except in LoadFromDrivers. */
	void FlushDirCache( const CString &sPath ) { }

	void LoadFromDrivers( const vector<LoadedDriver> &drivers )
	{
		/* XXX: Even though these two operations lock on their own, lock around
		 * them, too.  That way, nothing can sneak in and get incorrect
		 * results between the flush and the re-population. */
		FDB->FlushDirCache();
		for( unsigned i = 0; i < drivers.size(); ++i )
			if( drivers[i].MountPoint != "/" )
				FDB->AddFile( drivers[i].MountPoint, 0, 0 );
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
#if defined(_WIN32)
	char buf[MAX_PATH];
	GetModuleFileName( NULL, buf, sizeof(buf) );
	sPath = buf;
#else
	sPath = argv0;
#endif

	sPath.Replace( "\\", "/" );

	bool IsAbsolutePath = false;
	if( sPath.size() == 0 || sPath[0] == '/' )
		IsAbsolutePath = true;
#if defined(_WIN32)
	if( sPath.size() > 2 && sPath[1] == ':' && sPath[2] == '/' )
		IsAbsolutePath = true;
#endif

	// strip off executable name
	size_t n = sPath.find_last_of("/");
	if( n != sPath.npos )
		sPath.erase(n);
	else
		sPath.erase();

	if( !IsAbsolutePath )
	{
		sPath = GetCwd() + "/" + sPath;
		sPath.Replace( "\\", "/" );
	}

	return sPath;
#endif
}

static void ChangeToDirOfExecutable( CString argv0 )
{
	InitialWorkingDirectory = GetCwd();
	DirOfExecutable = GetDirOfExecutable( argv0 );

	/* Set the CWD.  Any effects of this is platform-specific; most files are read and
	 * written through RageFile.  See also RageFileManager::RageFileManager. */
#if defined(_WINDOWS)
	chdir( DirOfExecutable + "/.." );
#elif defined(DARWIN)
	chdir(DirOfExecutable + "/../../..");
#endif
}

RageFileManager::RageFileManager( CString argv0 )
{
	CHECKPOINT_M( argv0 );
	ChangeToDirOfExecutable( argv0 );
	
	g_Mutex = new RageEvent("RageFileManager");

	g_Mountpoints = new RageFileDriverMountpoints;
	LoadedDriver ld;
	ld.driver = g_Mountpoints;
	ld.MountPoint = "/";
	g_Drivers.push_back( ld );

	/* The mount path is unused, but must be nonempty. */
	RageFileManager::Mount( "mem", "(cache)", "@mem" );
}

void RageFileManager::MountInitialFilesystems()
{
	/* Add file search paths, higher priority first. */
#if defined(XBOX)
	RageFileManager::Mount( "dir", "D:\\", "" );
#elif defined(LINUX)
	/* Mount /proc, so Alsa9Buf::GetSoundCardDebugInfo() and others can access it. */
	RageFileManager::Mount( "dir", "/proc", "/proc" );
	
	/* We can almost do this, to have machine profiles be system-global to eg. share
	 * scores.  It would need to handle permissions properly. */
/*	RageFileManager::Mount( "dir", "/var/lib/games/stepmania", "Data/Profiles" ); */
	
	// CString Home = getenv( "HOME" ) + "/" + PRODUCT_NAME;

	/*
	 * Next: path to write general mutable user data.  If the above path fails (eg.
	 * wrong permissions, doesn't exist), machine memcard data will also go in here. 
	 * XXX: It seems silly to have two ~ directories.  If we're going to create a
	 * directory on our own, it seems like it should be a dot directory, but it
	 * seems wrong to put lots of data (eg. music) in one.  Hmm. 
	 */
	/* XXX: create */
/*	RageFileManager::Mount( "dir", Home + "." PRODUCT_NAME, "Data" ); */

	/* Next, search ~/StepMania.  This is where users can put music, themes, etc. */
	/* RageFileManager::Mount( "dir", Home + PRODUCT_NAME, "" ); */

	/* Search for a directory with "Songs" in it.  Be careful: the CWD is likely to
	 * be ~, and it's possible that some users will have a ~/Songs/ directory that
	 * has nothing to do with us, so check the initial directory last. */
	CString Root = "";
	struct stat st;
	if( Root == "" && !stat( DirOfExecutable + "/Songs", &st ) && st.st_mode&S_IFDIR )
		Root = DirOfExecutable;
	if( Root == "" && !stat( InitialWorkingDirectory + "/Songs", &st ) && st.st_mode&S_IFDIR )
		Root = InitialWorkingDirectory;
	if( Root == "" )
		RageException::Throw( "Couldn't find \"Songs\"" );
			
	RageFileManager::Mount( "dir", Root, "" );
#elif defined(_WINDOWS)
	/* All Windows data goes in the directory one level above the executable. */
	CHECKPOINT_M( ssprintf( "DOE \"%s\"", DirOfExecutable.c_str()) );
	CStringArray parts;
	split( DirOfExecutable, "/", parts );
	CHECKPOINT_M( ssprintf( "... %i parts", parts.size()) );
	ASSERT_M( parts.size() > 1, ssprintf("Strange DirOfExecutable: %s", DirOfExecutable.c_str()) );
	CString Dir = join( "/", parts.begin(), parts.end()-1 );
	RageFileManager::Mount( "dir", Dir, "" );
#else
	/* Paths relative to the CWD: */
	RageFileManager::Mount( "dir", ".", "" );
#endif
}

RageFileManager::~RageFileManager()
{
	/* Note that drivers can use previously-loaded drivers, eg. to load a ZIP
	 * from the FS.  Unload drivers in reverse order. */
	for( int i = g_Drivers.size()-1; i >= 0; --i )
		delete g_Drivers[i].driver;
	g_Drivers.clear();

//	delete g_Mountpoints; // g_Mountpoints was in g_Drivers
	g_Mountpoints = NULL;

	delete g_Mutex;
	g_Mutex = NULL;
}

/* path must be normalized (FixSlashesInPlace, CollapsePath). */
CString LoadedDriver::GetPath( const CString &path )
{
	/* If the path begins with /@, only match mountpoints that begin with /@. */
	if( path.size() >= 2 && path[1] == '@' )
	{
		if( MountPoint.size() < 2 || MountPoint[1] != '@' )
			return "";
	}
	
	if( path.Left( MountPoint.size() ).CompareNoCase( MountPoint ) )
		return ""; /* no match */

	/* Add one, so we don't cut off the leading slash. */
	CString sRet = path.Right( path.size() - MountPoint.size() + 1 );
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
void RageFileManager::GetDirListing( CString sPath, CStringArray &AddTo, bool bOnlyDirs, bool bReturnPathToo )
{
	NormalizePath( sPath );

	vector<LoadedDriver> aDriverList;
	ReferenceAllDrivers( aDriverList );

	for( unsigned i = 0; i < aDriverList.size(); ++i )
	{
		LoadedDriver &ld = aDriverList[i];
		const CString p = ld.GetPath( sPath );
		if( p.size() == 0 )
			continue;

		const unsigned OldStart = AddTo.size();
		
		ld.driver->GetDirListing( p, AddTo, bOnlyDirs, bReturnPathToo );

		/* If returning the path, prepend the mountpoint name to the files this driver returned. */
		if( bReturnPathToo && ld.MountPoint.size() > 0 )
		{
			for( unsigned j = OldStart; j < AddTo.size(); ++j )
			{
				/* Skip the trailing slash on the mountpoint; there's already a slash there. */
				CString &sPath = AddTo[j];
				sPath.insert( 0, ld.MountPoint, ld.MountPoint.size()-1 );
			}
		}
	}

	UnreferenceAllDrivers( aDriverList );

	/* More than one driver might return the same file.  Remove duplicates (case-
	 * insensitively). */
	sort( AddTo.begin(), AddTo.end(), ilt );
	CStringArray::iterator it = unique( AddTo.begin(), AddTo.end(), ieq );
	AddTo.erase(it, AddTo.end());
}

bool RageFileManager::Remove( CString sPath )
{
	vector<LoadedDriver> aDriverList;
	ReferenceAllDrivers( aDriverList );

	NormalizePath( sPath );
	
	/* Multiple drivers may have the same file. */
	bool Deleted = false;
	for( unsigned i = 0; i < aDriverList.size(); ++i )
	{
		const CString p = aDriverList[i].GetPath( sPath );
		if( p.size() == 0 )
			continue;

		bool ret = aDriverList[i].driver->Remove( p );
		if( ret )
			Deleted = true;
	}

	UnreferenceAllDrivers( aDriverList );

	return Deleted;
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

void RageFileManager::Mount( CString Type, CString Root, CString MountPoint )
{
	FixSlashesInPlace( Root );
	FixSlashesInPlace( MountPoint );

	if( MountPoint.size() && MountPoint.Right(1) != "/" )
		MountPoint += '/';
	/* XXX: Backwards compatibility; */
	if( MountPoint.Left(1) != "/" )
		MountPoint = "/" + MountPoint;
	ASSERT( Root != "" );

	CHECKPOINT_M( ssprintf("\"%s\", \"%s\", \"%s\"",
		Type.c_str(), Root.c_str(), MountPoint.c_str() ) );

	// Unmount anything that was previously mounted here.
	Unmount( Type, Root, MountPoint );

	CHECKPOINT;
	RageFileDriver *driver = MakeFileDriver( Type, Root );
	if( !driver )
	{
		CHECKPOINT;

		if( LOG )
			LOG->Warn("Can't mount unknown VFS type \"%s\", root \"%s\"", Type.c_str(), Root.c_str() );
		else
			fprintf( stderr, "Can't mount unknown VFS type \"%s\", root \"%s\"\n", Type.c_str(), Root.c_str() );
		return;
	}

	CHECKPOINT;

	LoadedDriver ld;
	ld.driver = driver;
	ld.Type = Type;
	ld.Root = Root;
	ld.MountPoint = MountPoint;

	g_Mutex->Lock();
	g_Drivers.push_back( ld );
	CHECKPOINT;
	g_Mountpoints->LoadFromDrivers( g_Drivers );
	CHECKPOINT;
	g_Mutex->Unlock();
}

void RageFileManager::Unmount( CString Type, CString Root, CString MountPoint )
{
	FixSlashesInPlace( Root );
	FixSlashesInPlace( MountPoint );

	if( MountPoint.size() && MountPoint.Right(1) != "/" )
		MountPoint += '/';

	/* Find all drivers we want to delete.  Remove them from g_Drivers, and move them
	 * into aDriverListToUnmount. */
	vector<LoadedDriver> aDriverListToUnmount;
	g_Mutex->Lock();
	for( unsigned i = 0; i < g_Drivers.size(); ++i )
	{
		if( Type != "" && g_Drivers[i].Type.CompareNoCase( Type ) )
			continue;
		if( Root != "" && g_Drivers[i].Root.CompareNoCase( Root ) )
			continue;
		if( MountPoint != "" && g_Drivers[i].MountPoint.CompareNoCase( MountPoint ) )
			continue;

		++g_Drivers[i].m_iRefs;
		aDriverListToUnmount.push_back( g_Drivers[i] );
		g_Drivers.erase( g_Drivers.begin()+i );
		--i;
	}

	g_Mountpoints->LoadFromDrivers( g_Drivers );

	g_Mutex->Unlock();

	/* Now we have a list of drivers to remove. */
	while( aDriverListToUnmount.size() )
	{
		/* If the driver has more than one reference, somebody other than us is
		 * using it; wait for that operation to complete. Note that two Unmount()
		 * calls that want to remove the same mountpoint will deadlock here. */
		g_Mutex->Lock();
		while( aDriverListToUnmount[0].m_iRefs > 1 )
			g_Mutex->Wait();
		g_Mutex->Unlock();

		delete aDriverListToUnmount[0].driver;
		aDriverListToUnmount.erase( aDriverListToUnmount.begin() );
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

	for( unsigned i = 0; i < g_Drivers.size(); ++i )
		if( !g_Drivers[i].MountPoint.CompareNoCase( MountPoint ) )
			return true;

	return false;
}

/* Return true if the driver with the given root path is ready (eg. CD or memory card
 * inserted). */
bool RageFileManager::MountpointIsReady( CString MountPoint )
{
	vector<LoadedDriver> aDriverList;
	ReferenceAllDrivers( aDriverList );

	for( unsigned i = 0; i < aDriverList.size(); ++i )
	{
		if( aDriverList[i].MountPoint.CompareNoCase( MountPoint ) )
			continue;

		return aDriverList[i].driver->Ready();
	}

	UnreferenceAllDrivers( aDriverList );
	return false;
}

void RageFileManager::GetLoadedDrivers( vector<DriverLocation> &Mounts )
{
	LockMut( *g_Mutex );

	for( unsigned i = 0; i < g_Drivers.size(); ++i )
	{
		DriverLocation l;
		l.MountPoint = g_Drivers[i].MountPoint;
		l.Type = g_Drivers[i].Type;
		l.Root = g_Drivers[i].Root;
		Mounts.push_back( l );
	}
}

void RageFileManager::FlushDirCache( CString sPath )
{
	NormalizePath( sPath );

	LockMut( *g_Mutex );

	for( unsigned i = 0; i < g_Drivers.size(); ++i )
	{
		if( sPath.size() == 0 )
		{
			g_Drivers[i].driver->FlushDirCache( "" );
		}
		else
		{
			const CString path = g_Drivers[i].GetPath( sPath );
			if( path.size() == 0 )
				continue;
			g_Drivers[i].driver->FlushDirCache( path );
		}
	}
}

RageFileManager::FileType RageFileManager::GetFileType( CString sPath )
{
	NormalizePath( sPath );

	vector<LoadedDriver> aDriverList;
	ReferenceAllDrivers( aDriverList );

	for( unsigned i = 0; i < aDriverList.size(); ++i )
	{
		const CString p = aDriverList[i].GetPath( sPath );
		if( p.size() == 0 )
			continue;
		FileType ret = aDriverList[i].driver->GetFileType( p );
		if( ret != TYPE_NONE )
			return ret;
	}

	UnreferenceAllDrivers( aDriverList );

	return TYPE_NONE;
}


int RageFileManager::GetFileSizeInBytes( CString sPath )
{
	NormalizePath( sPath );

	vector<LoadedDriver> aDriverList;
	ReferenceAllDrivers( aDriverList );

	for( unsigned i = 0; i < aDriverList.size(); ++i )
	{
		const CString p = aDriverList[i].GetPath( sPath );
		if( p.size() == 0 )
			continue;
		int ret = aDriverList[i].driver->GetFileSizeInBytes( p );
		if( ret != -1 )
			return ret;
	}
	UnreferenceAllDrivers( aDriverList );

	return -1;
}

int RageFileManager::GetFileHash( CString sPath )
{
	NormalizePath( sPath );

	vector<LoadedDriver> aDriverList;
	ReferenceAllDrivers( aDriverList );

	for( unsigned i = 0; i < aDriverList.size(); ++i )
	{
		const CString p = aDriverList[i].GetPath( sPath );
		if( p.size() == 0 )
			continue;
		int ret = aDriverList[i].driver->GetFileHash( p );
		if( ret != -1 )
			return ret;
	}
	UnreferenceAllDrivers( aDriverList );

	return -1;
}

static bool SortBySecond( const pair<int,int> &a, const pair<int,int> &b )
{
	return a.second < b.second;
}

void AddReference( const RageFileBasic *obj, RageFileDriver *driver )
{
	LockMut( *g_Mutex );

	pair< const RageFileBasic *, RageFileDriver * > ref;
	ref.first = obj;
	ref.second = driver;

	/* map::insert returns an iterator (which we discard) and a bool, indicating whether
	 * this is a new entry.  This should always be new. */
	const pair< FileReferences::iterator, bool > ret = g_Refs.insert( ref );
	ASSERT_M( ret.second, ssprintf( "RemoveReference: Duplicate reference (%s)", obj->GetDisplayPath().c_str() ) );
}

void RemoveReference( const RageFileBasic *obj )
{
	LockMut( *g_Mutex );

	FileReferences::iterator it = g_Refs.find( obj );
	ASSERT_M( it != g_Refs.end(), ssprintf( "RemoveReference: Missing reference (%s)", obj->GetDisplayPath().c_str() ) );
	g_Refs.erase( it );
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
		"Data/"
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

/* Copy a RageFileBasic for a new RageFile. */
RageFileBasic *RageFileManager::CopyFileObj( const RageFileBasic *cpy )
{
	LockMut( *g_Mutex );

	FileReferences::const_iterator it = g_Refs.find( cpy );
	ASSERT_M( it != g_Refs.end(), ssprintf( "CopyFileObj: Missing reference (%s)", cpy->GetDisplayPath().c_str() ) );

	RageFileBasic *ret = cpy->Copy();

	/* It's from the same driver as the original. */
	AddReference( ret, it->second );

	return ret;	
}

RageFileBasic *RageFileManager::OpenForReading( CString sPath, int mode, int &err )
{
	vector<LoadedDriver> aDriverList;
	ReferenceAllDrivers( aDriverList );

	for( unsigned i = 0; i < aDriverList.size(); ++i )
	{
		LoadedDriver &ld = aDriverList[i];
		const CString path = ld.GetPath( sPath );
		if( path.size() == 0 )
			continue;
		int error;
		RageFileBasic *ret = ld.driver->Open( path, mode, error );
		if( ret )
		{
			AddReference( ret, ld.driver );
			UnreferenceAllDrivers( aDriverList );
			return ret;
		}

		/* ENOENT (File not found) is low-priority: if some other error
		 * was reported, return that instead. */
		if( error != ENOENT )
			err = error;
	}
	UnreferenceAllDrivers( aDriverList );

	return NULL;
}

RageFileBasic *RageFileManager::OpenForWriting( CString sPath, int mode, int &err )
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
	vector<LoadedDriver> aDriverList;
	ReferenceAllDrivers( aDriverList );

	vector< pair<int,int> > Values;
	for( unsigned i = 0; i < aDriverList.size(); ++i )
	{
		LoadedDriver &ld = aDriverList[i];
		const CString path = ld.GetPath( sPath );
		if( path.size() == 0 )
			continue;

		const int value = ld.driver->GetPathValue( path );
		if( value == -1 )
			continue;

		Values.push_back( pair<int,int>( i, value ) );
	}

	stable_sort( Values.begin(), Values.end(), SortBySecond );

	err = 0;
	for( unsigned i = 0; i < Values.size(); ++i )
	{
		const int driver = Values[i].first;
		LoadedDriver &ld = aDriverList[driver];
		const CString path = ld.GetPath( sPath );
		ASSERT( path.size() );

		int error;
		RageFileBasic *ret = ld.driver->Open( path, mode, error );
		if( ret )
		{
			AddReference( ret, ld.driver );
			return ret;
		}

		/* The drivers are in order of priority; if they all return error, return the
		 * first.  Never return ERROR_WRITING_NOT_SUPPORTED. */
		if( !err && error != RageFileDriver::ERROR_WRITING_NOT_SUPPORTED )
			err = error;
	}

	if( !err )
		err = EEXIST; /* no driver could write */

	UnreferenceAllDrivers( aDriverList );

	return NULL;
}

void RageFileManager::Close( RageFileBasic *obj )
{
	if( obj == NULL )
		return;

	RemoveReference( obj );
	delete obj;
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

void GetDirListing( const CString &sPath, CStringArray &AddTo, bool bOnlyDirs, bool bReturnPathToo )
{
	FILEMAN->GetDirListing( sPath, AddTo, bOnlyDirs, bReturnPathToo );
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

	CStringArray arrayFiles;
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
