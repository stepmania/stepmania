#include "global.h"
#include "RageFileDriverDirectHelpers.h"
#include "RageUtil.h"
#include "RageLog.h"

#include <errno.h>
#include <sys/types.h>
#include <sys/stat.h>

#if !defined(WIN32)
#include <dirent.h>
#include <fcntl.h>
#else
#include <windows.h>
#include <io.h>
#endif

#if defined(XBOX)
/* Wrappers for low-level file functions, to work around Xbox issues: */
int DoMkdir( const CString &sPath, int perm )
{
	CString TempPath = sPath;
	TempPath.Replace( "/", "\\" );
	return mkdir( TempPath );
}

int DoOpen( const CString &sPath, int flags, int perm )
{
	CString TempPath = sPath;
	TempPath.Replace( "/", "\\" );
	return open( TempPath, flags, perm );
}

int DoStat( const CString &sPath, struct stat *st )
{
	CString TempPath = sPath;
	TempPath.Replace( "/", "\\" );
	return stat( sPath, st );
}

int DoRemove( const CString &sPath )
{
	CString TempPath = sPath;
	TempPath.Replace( "/", "\\" );
	return remove( sPath );
}

int DoRmdir( const CString &sPath )
{
	CString TempPath = sPath;
	TempPath.Replace( "/", "\\" );
	return rmdir( sPath );
}

HANDLE DoFindFirstFile( const CString &sPath, WIN32_FIND_DATA *fd )
{
	CString TempPath = sPath;
	TempPath.Replace( "/", "\\" );
	return FindFirstFile( TempPath, fd );
}

#endif

#if defined(WIN32)
int WinMoveFileInternal( const CString &sOldPath, const CString &sNewPath )
{
	static bool Win9x = false;

	/* Windows botches rename: it returns error if the file exists.  In NT,
	 * we can use MoveFileEx( new, old, MOVEFILE_REPLACE_EXISTING ) (though I
	 * don't know if it has similar atomicity guarantees to rename).  In
	 * 9x, we're screwed, so just delete any existing file (we aren't going
	 * to be robust on 9x anyway). */
	if( !Win9x )
	{
		if( MoveFileEx( sOldPath, sNewPath, MOVEFILE_REPLACE_EXISTING ) )
			return 1;

		if( GetLastError() == ERROR_NOT_SUPPORTED || GetLastError() == ERROR_CALL_NOT_IMPLEMENTED )
			return 0;

		Win9x = true;
	}

	if( MoveFile( sOldPath, sNewPath ) )
		return 1;
	
	if( GetLastError() != ERROR_ALREADY_EXISTS )
		return 0;

	if( !DeleteFile( sNewPath ) )
		return 0;

	return MoveFile( sOldPath, sNewPath );
}

int WinMoveFile( CString sOldPath, CString sNewPath )
{
#if defined(XBOX)
	sOldPath.Replace( "/", "\\" );
	sNewPath.Replace( "/", "\\" );
#endif

	if( WinMoveFileInternal(sOldPath, sNewPath) )
		return 1;
	if( GetLastError() != ERROR_ACCESS_DENIED )
		return 0;
	/* Try turning off the read-only bit on the file we're overwriting. */
	SetFileAttributes( sNewPath, FILE_ATTRIBUTE_NORMAL );

	return WinMoveFileInternal( sOldPath, sNewPath );
}
#endif

bool PathReady( CString path )
{
#ifdef _WINDOWS
	// Windows will throw up a message box if we try to write to a
	// removable drive with no disk inserted.  Find out whether there's a 
	// disk in the drive w/o writing a file.

	// find drive letter
	vector<CString> matches;
	static Regex parse("^([A-Za-z]+):");
	parse.Compare( path, matches );
	if( matches.size() != 1 )
		return false;

	CString sDrive = matches[0];
	TCHAR szVolumeNameBuffer[MAX_PATH];
	DWORD dwVolumeSerialNumber;
	DWORD dwMaximumComponentLength;
	DWORD lpFileSystemFlags;
	TCHAR szFileSystemNameBuffer[MAX_PATH];
	BOOL bResult = GetVolumeInformation( 
		sDrive + ":\\",
		szVolumeNameBuffer,
		sizeof(szVolumeNameBuffer),
		&dwVolumeSerialNumber,
		&dwMaximumComponentLength,
		&lpFileSystemFlags,
		szFileSystemNameBuffer,
		sizeof(szFileSystemNameBuffer) );
	return !!bResult;
#else
	// Try to create directory before writing a temp file.
	CreateDirectories( path );
	
	// Try to write a file.
	const CString sFile = path + "temp";
	int fd = DoOpen( sFile, O_WRONLY|O_CREAT|O_TRUNC );
	if( fd == -1 )
		return false;

	close( fd );
	remove( sFile );
	return true;
#endif
}

/* mkdir -p.  Doesn't fail if Path already exists and is a directory. */
bool CreateDirectories( CString Path )
{
	CStringArray parts;
	CString curpath;
	split(Path, "/", parts);

	for(unsigned i = 0; i < parts.size(); ++i)
	{
		curpath += parts[i] + "/";

#if defined(WIN32)
		if( i == 0 && curpath.size() > 1 && curpath[1] == ':' )
		{
			/* Don't try to create the drive letter alone. */
			continue;
		}
#endif

		if( DoMkdir(curpath, 0755) == 0 )
			continue;

		if(errno == EEXIST)
			continue;		// we expect to see this error

		// Log the error, but continue on.
		/* When creating a directory that already exists over Samba, Windows is
		 * returning ENOENT instead of EEXIST. */
		if( LOG )
			LOG->Warn("Couldn't create %s: %s", curpath.c_str(), strerror(errno) );

		/* Make sure it's a directory. */
		struct stat st;
		DoStat( curpath, &st );
		if( !(st.st_mode & S_IFDIR) )
		{
			if( LOG )
				LOG->Warn("Couldn't create %s: path exists and is not a directory", curpath.c_str() );
			
			return false;
		}
	}
	
	return true;
}

