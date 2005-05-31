#include "global.h"
#include "RageFileDriverDirectHelpers.h"
#include "RageUtil.h"
#include "RageLog.h"

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

		// On Win9x, MoveFileEx is expected to fail (returns ERROR_CALL_NOT_IMPLEMENTED).
		DWORD err = GetLastError();
		if( err == ERROR_CALL_NOT_IMPLEMENTED )
			Win9x = true;
		else
			return 0;
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
#if defined(_XBOX)
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
	int fd = DoOpen( sFile, O_WRONLY|O_CREAT|O_TRUNC, 0644 );
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

		if( DoMkdir(curpath, 0755) == 0 )
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
		if(!strcmp(fd.cFileName, ".") || !strcmp(fd.cFileName, ".."))
			continue;

		File f;
		f.SetName( fd.cFileName );
		f.dir = !!(fd.dwFileAttributes & FILE_ATTRIBUTE_DIRECTORY);
		f.size = fd.nFileSizeLow;
		f.hash = fd.ftLastWriteTime.dwLowDateTime;

		fs.files.insert(f);
	} while( FindNextFile( hFind, &fd ) );
	FindClose(hFind);
#else
	/* Ugly: POSIX threads are not guaranteed to have per-thread cwds, and only
	 * a few systems have openat() or equivalent; one or the other is needed
	 * to do efficient, thread-safe directory traversal.  Instead, we have to
	 * use absolute paths, which forces the system to re-parse the directory
	 * for each file.  This isn't a major issue, since most large directory
	 * scans are I/O-bound. */
	 
	DIR *d = opendir(root+sPath);
	if( d == NULL )
	{
		LOG->MapLog("opendir " + root+sPath, "Couldn't opendir(%s%s): %s", root.c_str(), sPath.c_str(), strerror(errno) );
		return;
	}

	while(struct dirent *ent = readdir(d))
	{
		if(!strcmp(ent->d_name, ".")) continue;
		if(!strcmp(ent->d_name, "..")) continue;
		
		File f;
		f.SetName( ent->d_name );
		
		struct stat st;
		if( DoStat(root+sPath + "/" + ent->d_name, &st) == -1 )
		{
			/* If it's a broken symlink, ignore it.  Otherwise, warn. */
			if( lstat(ent->d_name, &st) == 0 )
				continue;
			
			/* Huh? */
			if(LOG)
				LOG->Warn("Got file '%s' in '%s' from list, but can't stat? (%s)",
					ent->d_name, sPath.c_str(), strerror(errno));
			continue;
		} else {
			f.dir = (st.st_mode & S_IFDIR);
			f.size = st.st_size;
			f.hash = st.st_mtime;
		}

		fs.files.insert(f);
	}
	       
	closedir(d);
#endif
}

/*
 * Copyright (c) 2003-2004 Glenn Maynard, Chris Danford
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
