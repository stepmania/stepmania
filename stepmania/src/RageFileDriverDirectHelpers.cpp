#include "global.h"
#include "RageFileDriverDirectHelpers.h"
#include "RageUtil.h"
#include "RageLog.h"
#include "Foreach.h"

#include <cerrno>
#include <sys/types.h>
#include <sys/stat.h>

#if !defined(WIN32)
#include <dirent.h>
#include <fcntl.h>
#else
#if !defined(_XBOX)
#include <windows.h>
#endif
#include <io.h>
#endif

#if defined(_XBOX)
/* Wrappers for low-level file functions, to work around Xbox issues: */
int DoMkdir( const CString &sPath, int perm )
{
	return mkdir( DoPathReplace(sPath), perm );
}

int DoOpen( const CString &sPath, int flags, int perm )
{
	return open( DoPathReplace(sPath), flags, perm );
}

int DoStat( const CString &sPath, struct stat *st )
{
	return stat( DoPathReplace(sPath), st );
}

int DoRename( const CString &sOldPath, const CString &sNewPath )
{
	CString TempPath = sPath;
	TempPath.Replace( "/", "\\" );
	return rename( sPath );
}

int DoRemove( const CString &sPath )
{
	return remove( DoPathReplace(sPath) );
}

int DoRmdir( const CString &sPath )
{
	return rmdir( DoPathReplace(sPath) );
}

HANDLE DoFindFirstFile( const CString &sPath, WIN32_FIND_DATA *fd )
{
	return FindFirstFile( DoPathReplace(sPath), fd );
}

#endif
CString DoPathReplace(const CString &sPath)
{
	CString TempPath = sPath;
#if defined(XBOX)
	TempPath.Replace( "//", "\\" );
	TempPath.Replace( "/", "\\" );
#endif
	return TempPath;
}


#if defined(WIN32)
static bool WinMoveFileInternal( const CString &sOldPath, const CString &sNewPath )
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
			return true;

		// On Win9x, MoveFileEx is expected to fail (returns ERROR_CALL_NOT_IMPLEMENTED).
		DWORD err = GetLastError();
		if( err == ERROR_CALL_NOT_IMPLEMENTED )
			Win9x = true;
		else
			return false;
	}

	if( MoveFile( sOldPath, sNewPath ) )
		return true;
	
	if( GetLastError() != ERROR_ALREADY_EXISTS )
		return false;

	if( !DeleteFile( sNewPath ) )
		return false;

	return !!MoveFile( sOldPath, sNewPath );
}

bool WinMoveFile( CString sOldPath, CString sNewPath )
{
	if( WinMoveFileInternal( DoPathReplace(sOldPath), DoPathReplace(sNewPath) ) )
		return true;
	if( GetLastError() != ERROR_ACCESS_DENIED )
		return false;
	/* Try turning off the read-only bit on the file we're overwriting. */
	SetFileAttributes( DoPathReplace(sNewPath), FILE_ATTRIBUTE_NORMAL );

	return WinMoveFileInternal( DoPathReplace(sOldPath), DoPathReplace(sNewPath) );
}
#endif

/* mkdir -p.  Doesn't fail if Path already exists and is a directory. */
bool CreateDirectories( CString Path )
{
	/* XXX: handle "//foo/bar" paths in Windows */
	CStringArray parts;
	CString curpath;

	/* If Path is absolute, add the initial slash ("ignore empty" will remove it). */
	if( Path.Left(1) == "/" )
		curpath = "/";

	/* Ignore empty, so eg. "/foo/bar//baz" doesn't try to create "/foo/bar" twice. */
	split( Path, "/", parts, true );

	for(unsigned i = 0; i < parts.size(); ++i)
	{
		if( i )
			curpath += "/";
		curpath += parts[i];

#if defined(WIN32)
		if( curpath.size() == 2 && curpath[1] == ':' )  /* C: */
		{
			/* Don't try to create the drive letter alone. */
			continue;
		}
#endif

		if( DoMkdir(curpath, 0777) == 0 )
			continue;

		/* When creating a directory that already exists over Samba, Windows is
		 * returning ENOENT instead of EEXIST. */
		/* I can't reproduce this anymore.  If we get ENOENT, log it but keep
		 * going. */
		if( errno == ENOENT && LOG )
			LOG->Warn("Couldn't create %s: %s", curpath.c_str(), strerror(errno) );
		if( errno == EEXIST || errno == ENOENT )
			continue;		// we expect to see these errors

		if( LOG )
		{
			LOG->Warn("Couldn't create %s: %s", curpath.c_str(), strerror(errno) );
			return false;
		}

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

DirectFilenameDB::DirectFilenameDB( CString root_ )
{
	ExpireSeconds = 30;
	SetRoot( root_ );
}


void DirectFilenameDB::SetRoot( CString root_ )
{
	root = root_;

	/* "\abcd\" -> "/abcd/": */
	root.Replace( "\\", "/" );

	/* "/abcd/" -> "/abcd": */
	if( root.Right(1) == "/" )
		root.erase( root.size()-1, 1 );
}


void DirectFilenameDB::PopulateFileSet( FileSet &fs, const CString &path )
{
	CString sPath = path;

#if defined(XBOX)
	/* Xbox doesn't handle path names which end with ".", which are used when using an
	 * alternative song directory */
	if( sPath.size() > 0 && sPath.Right(1) == "." )
		sPath.erase( sPath.size() - 1 );
#endif

	/* Resolve path cases (path/Path -> PATH/path). */
	ResolvePath( sPath );

	fs.age.GetDeltaTime(); /* reset */
	fs.files.clear();

#if defined(WIN32)
	WIN32_FIND_DATA fd;

	if ( sPath.size() > 0  && sPath.Right(1) == "/" )
		sPath.erase( sPath.size() - 1 );

	HANDLE hFind = DoFindFirstFile( root+sPath+"/*", &fd );
	CHECKPOINT_M( root+sPath+"/*" );

	if( hFind == INVALID_HANDLE_VALUE )
		return;

	do {
		if( !strcmp(fd.cFileName, ".") || !strcmp(fd.cFileName, "..") )
			continue;

		File f;
		f.SetName( fd.cFileName );
		f.dir = !!(fd.dwFileAttributes & FILE_ATTRIBUTE_DIRECTORY);
		f.size = fd.nFileSizeLow;
		f.hash = fd.ftLastWriteTime.dwLowDateTime;

		fs.files.insert( f );
	} while( FindNextFile( hFind, &fd ) );
	FindClose( hFind );
#else
	/* Ugly: POSIX threads are not guaranteed to have per-thread cwds, and only
	 * a few systems have openat() or equivalent; one or the other is needed
	 * to do efficient, thread-safe directory traversal.  Instead, we have to
	 * use absolute paths, which forces the system to re-parse the directory
	 * for each file.  This isn't a major issue, since most large directory
	 * scans are I/O-bound. */
	 
	DIR *pDir = opendir(root+sPath);
	if( pDir == NULL )
	{
		LOG->MapLog("opendir " + root+sPath, "Couldn't opendir(%s%s): %s", root.c_str(), sPath.c_str(), strerror(errno) );
		return;
	}

	while( struct dirent *pEnt = readdir(pDir) )
	{
		if( !strcmp(pEnt->d_name, ".") )
			continue;
		if( !strcmp(pEnt->d_name, "..") )
			continue;
		
		File f;
		f.SetName( pEnt->d_name );
		
		struct stat st;
		if( DoStat(root+sPath + "/" + pEnt->d_name, &st) == -1 )
		{
			/* If it's a broken symlink, ignore it.  Otherwise, warn. */
			if( lstat(pEnt->d_name, &st) == 0 )
				continue;
			
			/* Huh? */
			if( LOG )
				LOG->Warn( "Got file '%s' in '%s' from list, but can't stat? (%s)",
					pEnt->d_name, sPath.c_str(), strerror(errno) );
			continue;
		} else {
			f.dir = (st.st_mode & S_IFDIR);
			f.size = st.st_size;
			f.hash = st.st_mtime;
		}

		fs.files.insert(f);
	}
	       
	closedir( pDir );
#endif

	/*
	 * Check for any ".ignore" markers.  If a marker exists, hide the marker and its 
	 * corresponding file.
	 * For example, if "file.xml.ignore" exists, hide both it and "file.xml" by 
	 * removing them from the file set.
	 * Ignore markers are used for convenience during build staging and are not used in 
	 * performance-critical situations.  To avoid incurring some of the overheard 
	 * due to ignore markers, delete the file instead instead of using an ignore marker.
	 */
	static const CString IGNORE_MARKER_BEGINNING = "ignore-";

	vector<CString> vsFilesToRemove;
	for( set<File>::iterator iter = fs.files.lower_bound(IGNORE_MARKER_BEGINNING); 
		 iter != fs.files.end(); 
		 iter++ )
	{
		if( !BeginsWith( iter->lname, IGNORE_MARKER_BEGINNING ) )
			break;
		CString sFileLNameToIgnore = iter->lname.Right( iter->lname.length() - IGNORE_MARKER_BEGINNING.length() );
		vsFilesToRemove.push_back( iter->name );
		vsFilesToRemove.push_back( sFileLNameToIgnore );
	}
	
	FOREACH_CONST( CString, vsFilesToRemove, iter )
	{
		// Erase the file corresponding to the ignore marker
		File fileToDelete;
		fileToDelete.SetName( *iter );
		set<File>::iterator iter2 = fs.files.find( fileToDelete );
		if( iter2 != fs.files.end() )
			fs.files.erase( iter2 );
	}
}

/*
 * Copyright (c) 2003-2005 Glenn Maynard, Chris Danford, Renaud Lepage
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
