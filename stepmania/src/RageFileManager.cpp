#include "global.h"
#include "RageFileManager.h"
#include "RageFileDriver.h"
#include "RageUtil.h"
#include "RageUtil_FileDB.h"
#include "RageLog.h"
#include <errno.h>

RageFileManager *FILEMAN = NULL;

/* Mountpoints as directories cause a problem.  If "Themes/default" is a mountpoint, and
 * doesn't exist anywhere else, then GetDirListing("Themes/*") must return "default".  The
 * driver containing "Themes/default" won't do this; its world view begins at "BGAnimations"
 * (inside "Themes/default").  We need a dummy driver that handles mountpoints. */
class RageFileDriverMountpoints: public RageFileDriver
{
public:
	RageFileDriverMountpoints(): RageFileDriver( new FilenameDB ) { }
	RageFileObj *Open( const CString &path, RageFile::OpenMode mode, RageFile &p, int &err )
	{
		err = EINVAL;
		return NULL;
	}
	void FlushDirCache( const CString &sPath ) { }
	void Add( const CString &MountPoint )
	{
		FDB->AddFile( MountPoint, 0, 0 );
	}
};
static RageFileDriverMountpoints *g_Mountpoints = NULL;

struct LoadedDriver
{
	/* A loaded driver may have a base path, which modifies the path we
	 * pass to the driver.  For example, if the base is "Songs/", and we
	 * want to send the path "Songs/Foo/Bar" to it, then we actually
	 * only send "Foo/Bar".  The path "Themes/Foo" is out of the scope
	 * of the driver, and GetPath returns false. */
	RageFileDriver *driver;
	CString MountPoint;

	CString GetPath( CString path );
};

static vector<LoadedDriver> g_Drivers;


RageFileManager::RageFileManager()
{
	g_Mountpoints = new RageFileDriverMountpoints;
	LoadedDriver ld;
	ld.driver = g_Mountpoints;
	ld.MountPoint = "";
	g_Drivers.push_back( ld );

	/* Add file search paths, higher priority first. */
#if defined(XBOX)
	RageFileManager::Mount( "dir", SYS_BASE_PATH, "" );
#elif defined(LINUX)
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

	/* Paths relative to the CWD: */
	RageFileManager::Mount( "dir", ".", "" );

	/* Absolute paths.  This is rarely used, eg. by Alsa9Buf::GetSoundCardDebugInfo(). */
	RageFileManager::Mount( "dir", "/", "/" );
#elif defined(WINDOWS)
	/* All Windows data goes in the directory with the executable. */
	/* XXX: Test to see if we can write to the program directory tree.  If we can't,
	 * add a search path under "Documents and Settings" for writing scores, etc. Otherwise,
	 * don't.  We don't want to write to D&S if we don't have to (it'll confuse people). */
/*	RageFileManager::Mount( "dir", "******", "" ); */

	/* Paths relative to the CWD: */
	RageFileManager::Mount( "dir", ".", "" );
#elif defined(DARWIN)
    /* TopLevelDir/StepMania.app/Contents/MacOS/ is CWD */
    RageFileManager::Mount( "dir", "../../..", "");
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

//	delete g_Mountpoints; // g_Mountpoints was g_Drivers
	g_Mountpoints = NULL;
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
void RageFileManager::GetDirListing( CString sPath, CStringArray &AddTo, bool bOnlyDirs, bool bReturnPathToo )
{
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

#include "RageFileDriverDirect.h"
void RageFileManager::Mount( CString Type, CString Root, CString MountPoint )
{
	if( MountPoint.size() && MountPoint.Right(1) != "/" )
		MountPoint += '/';
	ASSERT( Root != "" );

	RageFileDriver *driver = NULL;
	if( !Type.CompareNoCase("DIR") )
	{
		driver = new RageFileDriverDirect( Root );
	}

	if( !driver )
	{
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
	for( unsigned i = 0; i < g_Drivers.size(); ++i )
		if( !g_Drivers[i].MountPoint.CompareNoCase( MountPoint ) )
			return true;

	return false;
}

/* Return true if the driver with the given root path is ready (eg. CD or memory card
 * inserted). */
bool RageFileManager::MountpointIsReady( CString MountPoint )
{
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

int RageFileManager::GetFileModTime( const CString &sPath )
{
	for( unsigned i = 0; i < g_Drivers.size(); ++i )
	{
		const CString p = g_Drivers[i].GetPath( sPath );
		if( p.size() == 0 )
			continue;
		int ret = g_Drivers[i].driver->GetFileModTime( p );
		if( ret != -1 )
			return ret;
	}

	return -1;

}

static bool SortBySecond( const pair<int,int> &a, const pair<int,int> &b )
{
	return a.second < b.second;
}

/* Used only by RageFile: */
RageFileObj *RageFileManager::Open( const CString &sPath, RageFile::OpenMode mode, RageFile &p, int &err )
{
	err = ENOENT;

	/* If writing, we need to do a heuristic to figure out which driver to write with--there
	 * may be several that will work. */
	if( mode == RageFile::WRITE )
		return OpenForWriting( sPath, mode, p, err );

	/* XXX: WRITE logic */
	for( unsigned i = 0; i < g_Drivers.size(); ++i )
	{
		const CString path = g_Drivers[i].GetPath( sPath );
		if( path.size() == 0 )
			continue;
		int error;
		RageFileObj *ret = g_Drivers[i].driver->Open( path, mode, p, error );
		if( ret )
			return ret;

		/* ENOENT (File not found) is low-priority: if some other error
		 * was reported, return that instead. */
		if( error != ENOENT )
			err = error;
	}

	return NULL;
}

RageFileObj *RageFileManager::OpenForWriting( const CString &sPath, RageFile::OpenMode mode, RageFile &p, int &err )
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
			return ret;
	}
	err = error;
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

int GetFileModTime( const CString &sPath )
{
	return FILEMAN->GetFileModTime( sPath );
}

void GetDirListing( const CString &sPath, CStringArray &AddTo, bool bOnlyDirs, bool bReturnPathToo )
{
	FILEMAN->GetDirListing( sPath, AddTo, bOnlyDirs, bReturnPathToo );
}

void FlushDirCache()
{
	FILEMAN->FlushDirCache( "" );
}

