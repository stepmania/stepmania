#include "global.h"
#include "StepMania.h"
#include "RageFileManager.h"
#include "RageFileDriver.h"
#include "RageUtil.h"
#include "RageUtil_FileDB.h"
#include "RageLog.h"
#include "RageThreads.h"

#include <errno.h>
#if defined(LINUX)
#include <sys/stat.h>
#endif

RageFileManager *FILEMAN = NULL;

/* Lock this before touching any of these globals (except FILEMAN itself). */
static RageMutex *g_Mutex;

// Mountpoints as directories cause a problem.  If "Themes/default" is a mountpoint, and
// doesn't exist anywhere else, then GetDirListing("Themes/*") must return "default".  The
// driver containing "Themes/default" won't do this; its world view begins at "BGAnimations"
// (inside "Themes/default").  We need a dummy driver that handles mountpoints. */
class RageFileDriverMountpoints: public RageFileDriver
{
public:
	RageFileDriverMountpoints(): RageFileDriver( new FilenameDB ) { }
	RageFileObj *Open( const CString &path, int mode, RageFile &p, int &err )
	{
		err = (mode == RageFile::WRITE)? EINVAL:ENOENT;
		return NULL;
	}
	void FlushDirCache( const CString &sPath ) { }
	void Add( const CString &MountPoint )
	{
		FDB->AddFile( MountPoint, 0, 0 );
	}
};
static RageFileDriverMountpoints *g_Mountpoints = NULL;

typedef map< const RageFileObj *, RageFileDriver * > FileReferences;
static FileReferences g_Refs;

struct LoadedDriver
{
	/* A loaded driver may have a base path, which modifies the path we
	 * pass to the driver.  For example, if the base is "Songs/", and we
	 * want to send the path "Songs/Foo/Bar" to it, then we actually
	 * only send "Foo/Bar".  The path "Themes/Foo" is out of the scope
	 * of the driver, and GetPath returns false. */
	RageFileDriver *driver;
	CString MountPoint;

	LoadedDriver() { driver = NULL; }
	CString GetPath( CString path );
};

static vector<LoadedDriver> g_Drivers;


RageFileManager::RageFileManager()
{
	g_Mutex = new RageMutex;

	g_Mountpoints = new RageFileDriverMountpoints;
	LoadedDriver ld;
	ld.driver = g_Mountpoints;
	ld.MountPoint = "";
	g_Drivers.push_back( ld );

	MountInitialFilesystems();
}

#ifndef HAVE_EXTRA // set for custom initial mount rules
void RageFileManager::MountInitialFilesystems()
{
	/* Add file search paths, higher priority first. */
#if defined(XBOX)
	RageFileManager::Mount( "dir", SYS_BASE_PATH, "" );
#elif defined(LINUX)
	/* Absolute paths.  This is rarely used, eg. by Alsa9Buf::GetSoundCardDebugInfo(). 
	 * All paths that start with a slash (eg. "/proc") should use this, so put it
	 * first. */
	RageFileManager::Mount( "dir", "/", "/" );
	
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
	/* XXX: Test to see if we can write to the program directory tree.  If we can't,
	 * add a search path under "Documents and Settings" for writing scores, etc. Otherwise,
	 * don't.  We don't want to write to D&S if we don't have to (it'll confuse people). */
/*	RageFileManager::Mount( "dir", "******", "" ); */

	/* All Windows data goes in the directory one level above the executable. */
	CStringArray parts;
	split( DirOfExecutable, "/", parts );
	RAGE_ASSERT_M( parts.size() > 1, ssprintf("Strange DirOfExecutable: %s", DirOfExecutable.c_str()) );
	CString Dir = join( "/", parts.begin(), parts.end()-1 );
	RageFileManager::Mount( "dir", Dir, "" );
#else
	/* Paths relative to the CWD: */
	RageFileManager::Mount( "dir", ".", "" );
#endif
}
#endif

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

CString LoadedDriver::GetPath( CString path )
{
	path.Replace( "\\", "/" );

	/* If the path begins with @, default mount points don't count. */
	const bool Dedicated = ( path.size() && path[0] == '@' );
	if( MountPoint.size() == 0 )
		return Dedicated? "":path;
	
	/* Map all slashes to forward slash. */
	path.Replace( "\\", "/" );

	if( path.Left( MountPoint.size() ).CompareNoCase( MountPoint ) )
		return ""; /* no match */

	return path.Right( path.size() - MountPoint.size() );
}

bool ilt( const CString &a, const CString &b ) { return a.CompareNoCase(b) < 0; }
bool ieq( const CString &a, const CString &b ) { return a.CompareNoCase(b) == 0; }
void RageFileManager::GetDirListing( const CString &sPath, CStringArray &AddTo, bool bOnlyDirs, bool bReturnPathToo )
{
	LockMut( *g_Mutex );
	
	for( unsigned i = 0; i < g_Drivers.size(); ++i )
	{
		LoadedDriver &ld = g_Drivers[i];
		const CString p = ld.GetPath( sPath );
		if( p.size() == 0 )
			continue;

		const unsigned OldStart = AddTo.size();
		
		g_Drivers[i].driver->GetDirListing( p, AddTo, bOnlyDirs, bReturnPathToo );

		/* If returning the path, prepend the mountpoint name to the files this driver returned. */
		if( bReturnPathToo )
			for( unsigned j = OldStart; j < AddTo.size(); ++j )
				AddTo[j] = ld.MountPoint + AddTo[j];
	}

	/* More than one driver might return the same file.  Remove duplicates (case-
	 * insensitively). */
	sort( AddTo.begin(), AddTo.end(), ilt );
	CStringArray::iterator it = unique( AddTo.begin(), AddTo.end(), ieq );
	AddTo.erase(it, AddTo.end());
}

bool RageFileManager::Remove( const CString &sPath )
{
	LockMut( *g_Mutex );

	/* Multiple drivers may have the same file. */
	bool Deleted = false;
	for( unsigned i = 0; i < g_Drivers.size(); ++i )
	{
		const CString p = g_Drivers[i].GetPath( sPath );
		if( p.size() == 0 )
			continue;

		bool ret = g_Drivers[i].driver->Remove( p );
		if( !ret )
			Deleted = true;
	}

	return Deleted;
}

void RageFileManager::Mount( CString Type, CString Root, CString MountPoint )
{
	LockMut( *g_Mutex );

	if( MountPoint.size() && MountPoint.Right(1) != "/" )
		MountPoint += '/';
	ASSERT( Root != "" );

	RageFileDriver *driver = MakeFileDriver( Type, Root );
	if( !driver )
	{
		if( LOG )
			LOG->Warn("Can't mount unknown VFS type \"%s\", root \"%s\"", Type.c_str(), Root.c_str() );
		return;
	}

	LoadedDriver ld;
	ld.driver = driver;
	ld.MountPoint = MountPoint;
	g_Drivers.push_back( ld );

	g_Mountpoints->Add( MountPoint );
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
	LockMut( *g_Mutex );

	for( unsigned i = 0; i < g_Drivers.size(); ++i )
	{
		if( g_Drivers[i].MountPoint.CompareNoCase( MountPoint ) )
			continue;

		return g_Drivers[i].driver->Ready();
	}

	return false;
}

void RageFileManager::FlushDirCache( const CString &sPath )
{
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

RageFileManager::FileType RageFileManager::GetFileType( const CString &sPath )
{
	LockMut( *g_Mutex );

	for( unsigned i = 0; i < g_Drivers.size(); ++i )
	{
		const CString p = g_Drivers[i].GetPath( sPath );
		if( p.size() == 0 )
			continue;
		FileType ret = g_Drivers[i].driver->GetFileType( p );
		if( ret != TYPE_NONE )
			return ret;
	}

	return TYPE_NONE;
}


int RageFileManager::GetFileSizeInBytes( const CString &sPath )
{
	LockMut( *g_Mutex );

	for( unsigned i = 0; i < g_Drivers.size(); ++i )
	{
		const CString p = g_Drivers[i].GetPath( sPath );
		if( p.size() == 0 )
			continue;
		int ret = g_Drivers[i].driver->GetFileSizeInBytes( p );
		if( ret != -1 )
			return ret;
	}

	return -1;
}

int RageFileManager::GetFileHash( const CString &sPath )
{
	LockMut( *g_Mutex );

	for( unsigned i = 0; i < g_Drivers.size(); ++i )
	{
		const CString p = g_Drivers[i].GetPath( sPath );
		if( p.size() == 0 )
			continue;
		int ret = g_Drivers[i].driver->GetFileHash( p );
		if( ret != -1 )
			return ret;
	}

	return -1;

}

static bool SortBySecond( const pair<int,int> &a, const pair<int,int> &b )
{
	return a.second < b.second;
}

void AddReference( const RageFileObj *obj, RageFileDriver *driver )
{
	LockMut( *g_Mutex );

	pair< const RageFileObj *, RageFileDriver * > ref;
	ref.first = obj;
	ref.second = driver;

	/* map::insert returns an iterator (which we discard) and a bool, indicating whether
	 * this is a new entry.  This should always be new. */
	const pair< FileReferences::iterator, bool > ret = g_Refs.insert( ref );
	RAGE_ASSERT_M( ret.second, ssprintf( "RemoveReference: Duplicate reference (%s)", obj->GetDisplayPath().c_str() ) );
}

void RemoveReference( const RageFileObj *obj )
{
	LockMut( *g_Mutex );

	FileReferences::iterator it = g_Refs.find( obj );
	RAGE_ASSERT_M( it != g_Refs.end(), ssprintf( "RemoveReference: Missing reference (%s)", obj->GetDisplayPath().c_str() ) );
	g_Refs.erase( it );
}

/* Used only by RageFile: */
RageFileObj *RageFileManager::Open( const CString &sPath, int mode, RageFile &p, int &err )
{
	LockMut( *g_Mutex );

	err = ENOENT;

	/* If writing, we need to do a heuristic to figure out which driver to write with--there
	 * may be several that will work. */
	if( mode == RageFile::WRITE )
		return OpenForWriting( sPath, mode, p, err );

	for( unsigned i = 0; i < g_Drivers.size(); ++i )
	{
		LoadedDriver &ld = g_Drivers[i];
		const CString path = ld.GetPath( sPath );
		if( path.size() == 0 )
			continue;
		int error;
		RageFileObj *ret = ld.driver->Open( path, mode, p, error );
		if( ret )
		{
			AddReference( ret, ld.driver );
			return ret;
		}

		/* ENOENT (File not found) is low-priority: if some other error
		 * was reported, return that instead. */
		if( error != ENOENT )
			err = error;
	}

	return NULL;
}

/* Copy a RageFileObj for a new RageFile. */
RageFileObj *RageFileManager::CopyFileObj( const RageFileObj *cpy, RageFile &p )
{
	LockMut( *g_Mutex );

	FileReferences::const_iterator it = g_Refs.find( cpy );
	RAGE_ASSERT_M( it != g_Refs.end(), ssprintf( "RemoveReference: Missing reference (%s)", cpy->GetDisplayPath().c_str() ) );

	RageFileObj *ret = cpy->Copy( p );

	/* It's from the same driver as the original. */
	AddReference( ret, it->second );

	return ret;	
}

RageFileObj *RageFileManager::OpenForWriting( const CString &sPath, int mode, RageFile &p, int &err )
{
	LockMut( *g_Mutex );

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
	vector< pair<int,int> > Values;
	unsigned i;
	for( i = 0; i < g_Drivers.size(); ++i )
	{
		LoadedDriver &ld = g_Drivers[i];
		const CString path = ld.GetPath( sPath );
		if( path.size() == 0 )
			continue;

		const int value = ld.driver->GetPathValue( path );
		if( value == -1 )
			continue;

		Values.push_back( pair<int,int>( i, value ) );
	}

	if( !Values.size() )
	{
		err = EEXIST;
		return NULL;
	}

	stable_sort( Values.begin(), Values.end(), SortBySecond );

	int error = 0;
	for( i = 0; i < Values.size(); ++i )
	{
		const int driver = Values[i].first;
		LoadedDriver &ld = g_Drivers[driver];
		const CString path = ld.GetPath( sPath );
		ASSERT( path.size() );

		RageFileObj *ret = ld.driver->Open( path, mode, p, error );
		if( ret )
		{
			AddReference( ret, ld.driver );
			return ret;
		}
	}
	err = error;
	return NULL;
}

void RageFileManager::Close( RageFileObj *obj )
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
	return GetHashForString( sPath ) + FILEMAN->GetFileHash( sPath );
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
