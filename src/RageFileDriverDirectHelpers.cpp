#include "global.h"
#include "RageFileDriverDirectHelpers.h"
#include "RageUtil.h"
#include "RageLog.h"
#include "RageString.hpp"

#include <cerrno>
#include <sys/types.h>
#include <sys/stat.h>

#if !defined(WIN32)

#if defined(HAVE_DIRENT_H)
#include <dirent.h>
#endif

#else
#include <windows.h>
#include <io.h>
#endif

using std::vector;

std::string DoPathReplace(const std::string &sPath)
{
	std::string TempPath = sPath;
	return TempPath;
}


#if defined(WIN32)
static bool WinMoveFileInternal( const std::string &sOldPath, const std::string &sNewPath )
{
	static bool Win9x = false;

	/* Windows botches rename: it returns error if the file exists. In NT,
	 * we can use MoveFileEx( new, old, MOVEFILE_REPLACE_EXISTING ) (though I
	 * don't know if it has similar atomicity guarantees to rename). In
	 * 9x, we're screwed, so just delete any existing file (we aren't going
	 * to be robust on 9x anyway). */
	if( !Win9x )
	{
		if( MoveFileEx( sOldPath.c_str(), sNewPath.c_str(), MOVEFILE_REPLACE_EXISTING ) )
			return true;

		// On Win9x, MoveFileEx is expected to fail (returns ERROR_CALL_NOT_IMPLEMENTED).
		DWORD err = GetLastError();
		if( err == ERROR_CALL_NOT_IMPLEMENTED )
			Win9x = true;
		else
			return false;
	}

	if( MoveFile( sOldPath.c_str(), sNewPath.c_str() ) )
		return true;

	if( GetLastError() != ERROR_ALREADY_EXISTS )
		return false;

	if( !DeleteFile( sNewPath.c_str() ) )
		return false;

	return !!MoveFile( sOldPath.c_str(), sNewPath.c_str() );
}

bool WinMoveFile( std::string sOldPath, std::string sNewPath )
{
	if( WinMoveFileInternal( DoPathReplace(sOldPath), DoPathReplace(sNewPath) ) )
		return true;
	if( GetLastError() != ERROR_ACCESS_DENIED )
		return false;
	/* Try turning off the read-only bit on the file we're overwriting. */
	SetFileAttributes( DoPathReplace(sNewPath).c_str(), FILE_ATTRIBUTE_NORMAL );

	return WinMoveFileInternal( DoPathReplace(sOldPath), DoPathReplace(sNewPath) );
}
#endif

/* mkdir -p.  Doesn't fail if Path already exists and is a directory. */
bool CreateDirectories( std::string Path )
{
	// XXX: handle "//foo/bar" paths in Windows
	std::string curpath;

	// If Path is absolute, add the initial slash ("ignore empty" will remove it).
	if (Rage::starts_with(Path, "/"))
	{
		curpath = "/";
	}
	// Ignore empty, so eg. "/foo/bar//baz" doesn't try to create "/foo/bar" twice.
	auto parts = Rage::split(Path, "/", Rage::EmptyEntries::skip);

	for(unsigned i = 0; i < parts.size(); ++i)
	{
		if( i )
		{
			curpath += "/";
		}
		curpath += parts[i];

#if defined(WIN32)
		if( curpath.size() == 2 && curpath[1] == ':' )  /* C: */
		{
			/* Don't try to create the drive letter alone. */
			continue;
		}
#endif

		if( DoMkdir(curpath.c_str(), 0777) == 0 )
			continue;

#if defined(WIN32)
		/* When creating a directory that already exists over Samba, Windows is
		 * returning ENOENT instead of EEXIST. */
		/* I can't reproduce this anymore.  If we get ENOENT, log it but keep
		 * going. */
		if( errno == ENOENT )
		{
			WARN( fmt::sprintf("Couldn't create %s: %s", curpath.c_str(), strerror(errno)) );
			errno = EEXIST;
		}
#endif

		if( errno == EEXIST )
		{
			/* Make sure it's a directory. */
			struct stat st;
			if( DoStat(curpath.c_str(), &st) != -1 && !(st.st_mode & S_IFDIR) )
			{
				WARN( fmt::sprintf("Couldn't create %s: path exists and is not a directory", curpath.c_str()) );
				return false;
			}

			continue;		// we expect to see this error
		}

		WARN( fmt::sprintf("Couldn't create %s: %s", curpath.c_str(), strerror(errno)) );
		return false;
	}

	return true;
}

DirectFilenameDB::DirectFilenameDB( std::string root_ )
{
	ExpireSeconds = 30;
	SetRoot( root_ );
}


void DirectFilenameDB::SetRoot(std::string root_)
{
	root = root_;

	// "\abcd\" -> "/abcd/":
	Rage::replace(root, '\\', '/');

	// "/abcd/" -> "/abcd":
	if (Rage::ends_with(root, "/"))
	{
		root.erase(root.size() - 1, 1);
	}
}

void DirectFilenameDB::CacheFile( const std::string &sPath )
{
	CHECKPOINT_M( root+sPath );
	std::string sDir = Rage::dir_name( sPath );
	FileSet *pFileSet = GetFileSet( sDir, false );
	if( pFileSet == nullptr )
	{
		// This directory isn't cached so do nothing.
		m_Mutex.Unlock(); // Locked by GetFileSet()
		return;
	}
	while( !pFileSet->m_bFilled )
		m_Mutex.Wait();

#if defined(WIN32)
	// There is almost surely a better way to do this
	WIN32_FIND_DATA fd;
	HANDLE hFind = DoFindFirstFile( (root+sPath).c_str(), &fd );
	if( hFind == INVALID_HANDLE_VALUE )
	{
		m_Mutex.Unlock(); // Locked by GetFileSet()
		return;
	}
	File f( fd.cFileName );
	f.dir = !!(fd.dwFileAttributes & FILE_ATTRIBUTE_DIRECTORY);
	f.size = fd.nFileSizeLow;
	f.hash = fd.ftLastWriteTime.dwLowDateTime;

	pFileSet->files.insert( f );
	FindClose( hFind );
#else
	File f( Rage::base_name(sPath) );

	struct stat st;
	if( DoStat((root+sPath).c_str(), &st) == -1 )
	{
		int iError = errno;
		// If it's a broken symlink, ignore it.  Otherwise, warn.
		// Huh?
		WARN( fmt::sprintf("File '%s' is gone! (%s)",
				sPath.c_str(), strerror(iError)) );
	}
	else
	{
		f.dir = (st.st_mode & S_IFDIR);
		f.size = (int)st.st_size;
		f.hash = st.st_mtime;
	}

	pFileSet->files.insert(f);
#endif
	m_Mutex.Unlock(); // Locked by GetFileSet()
}

void DirectFilenameDB::PopulateFileSet( FileSet &fs, const std::string &path )
{
	std::string sPath = path;

	// Resolve path cases (path/Path -> PATH/path).
	ResolvePath( sPath );

	fs.age.GetDeltaTime(); // reset
	fs.files.clear();

#if defined(WIN32)
	WIN32_FIND_DATA fd;

	if (sPath.size() > 0 && Rage::ends_with(sPath, "/"))
	{
		sPath.erase(sPath.size() - 1);
	}
	HANDLE hFind = DoFindFirstFile( (root+sPath+"/*").c_str(), &fd );
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

	DIR *pDir = opendir((root+sPath).c_str());
	if( pDir == nullptr )
		return;

	while( struct dirent *pEnt = readdir(pDir) )
	{
		if( !strcmp(pEnt->d_name, ".") )
			continue;
		if( !strcmp(pEnt->d_name, "..") )
			continue;

		File f( pEnt->d_name );

		struct stat st;
		if( DoStat((root+sPath + "/" + pEnt->d_name).c_str(), &st) == -1 )
		{
			int iError = errno;
			/* If it's a broken symlink, ignore it.  Otherwise, warn. */
			if( lstat((root+sPath + "/" + pEnt->d_name).c_str(), &st) == 0 )
				continue;

			/* Huh? */
			WARN( fmt::sprintf("Got file '%s' in '%s' from list, but can't stat? (%s)",
					pEnt->d_name, sPath.c_str(), strerror(iError)) );
			continue;
		}
		else
		{
			f.dir = (st.st_mode & S_IFDIR);
			f.size = (int)st.st_size;
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
	static const std::string IGNORE_MARKER_BEGINNING = "ignore-";

	vector<std::string> vsFilesToRemove;
	for( auto iter = fs.files.lower_bound(IGNORE_MARKER_BEGINNING); iter != fs.files.end(); ++iter )
	{
		if( !Rage::starts_with( iter->lname, IGNORE_MARKER_BEGINNING ) )
		{
			break;
		}
		std::string sFileLNameToIgnore = Rage::tail( iter->lname, -IGNORE_MARKER_BEGINNING.size() );
		vsFilesToRemove.push_back( iter->name );
		vsFilesToRemove.push_back( sFileLNameToIgnore );
	}

	for (auto const &iter: vsFilesToRemove)
	{
		// Erase the file corresponding to the ignore marker
		File fileToDelete;
		fileToDelete.SetName( iter );
		std::set<File>::iterator iter2 = fs.files.find( fileToDelete );
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
