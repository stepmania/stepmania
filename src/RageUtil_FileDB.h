#ifndef RAGE_UTIL_FILEDB
#define RAGE_UTIL_FILEDB

#include <set>
#include <map>
#include "RageTimer.h"
#include "RageThreads.h"
#include "RageFileManager.h"
#include "RageString.hpp"

struct FileSet;
struct File
{
	std::string name;
	std::string lname;

	void SetName( std::string const &fn );
	bool dir;
	int size;
	/* Modification time of the file.  The contents of this is undefined, except that
	 * when the file has been modified, this value will change. */
	int hash;

	/* Private data, for RageFileDrivers. */
	void *priv;

	/* If this is non-nullptr, and dir is true, this is a pointer to the FileSet containing
	 * the directory contents.  (This is a cache; it isn't always set.) */
	const FileSet *dirp;

	File() { dir=false; dirp=nullptr; size=-1; hash=-1; priv=nullptr;}
	File( const std::string &fn )
	{
		SetName( fn );
		dir=false; size=-1; hash=-1; priv=nullptr; dirp=nullptr;
	}

	bool operator< (const File &rhs) const { return lname<rhs.lname; }

	bool equal(const File &rhs) const { return lname == rhs.lname; }
	bool equal(const std::string &rhs) const
	{
		std::string l = Rage::make_lower(rhs);
		return lname == l;
	}
};

inline bool operator==(File const &lhs, File const &rhs)
{
	return lhs.lname == rhs.lname;
}
inline bool operator!=(File const &lhs, File const &rhs)
{
	return !operator==(lhs, rhs);
}

/** @brief This represents a directory. */
struct FileSet
{
	std::set<File> files;
	RageTimer age;

	/*
	 * If m_bFilled is false, this FileSet hasn't completed being filled in yet; it's
	 * owned by the thread filling it in.  Wait on FilenameDB::m_Mutex and retry until
	 * it becomes true.
	 */
	bool m_bFilled;

	FileSet() { m_bFilled = true; }

	void GetFilesMatching( std::string const &sBeginning, std::string const &sContaining, std::string const &sEnding,
		std::vector<std::string> &asOut, bool bOnlyDirs ) const;
	void GetFilesEqualTo( std::string const &pat, std::vector<std::string> &out, bool bOnlyDirs ) const;

	RageFileManager::FileType GetFileType( const std::string &sPath ) const;
	int GetFileSize( const std::string &sPath ) const;
	int GetFileHash( const std::string &sPath ) const;
};
/** @brief A container for a file listing. */
class FilenameDB
{
public:
	FilenameDB():
		m_Mutex("FilenameDB"), ExpireSeconds( -1 ) { }
	virtual ~FilenameDB() { FlushDirCache(); }

	void AddFile( const std::string &sPath, int iSize, int iHash, void *pPriv=nullptr );
	void DelFile( const std::string &sPath );
	void *GetFilePriv( const std::string &sPath );

	/* This handles at most two * wildcards.  If we need anything more complicated,
	 * we'll need to use fnmatch or regex. */
	void GetFilesSimpleMatch( std::string const &sDir, std::string const &sFile, std::vector<std::string> &asOut, bool bOnlyDirs );

	/* Search for "path" case-insensitively and replace it with the correct
	 * case.  If only a portion of the path exists, resolve as much as possible.
	 * Return true if the entire path was matched. */
	bool ResolvePath( std::string &sPath );

	RageFileManager::FileType GetFileType( const std::string &sPath );
	int GetFileSize( const std::string &sPath );
	int GetFileHash( const std::string &sFilePath );
	void GetDirListing( std::string const &sPath, std::vector<std::string> &asAddTo, bool bOnlyDirs, bool bReturnPathToo );

	void FlushDirCache( const std::string &sDir = std::string() );

	void GetFileSetCopy( const std::string &dir, FileSet &out );
	/* Probably slow, so override it. */
	virtual void CacheFile( const std::string &sPath );

protected:
	RageEvent m_Mutex;

	const File *GetFile( const std::string &sPath );
	FileSet *GetFileSet( const std::string &sDir, bool create=true );

	/* Directories we have cached: */
	std::map<std::string, FileSet *> dirs;

	int ExpireSeconds;

	void GetFilesEqualTo( std::string const &sDir, std::string const &sName, std::vector<std::string> &asOut, bool bOnlyDirs );
	void GetFilesMatching( std::string const &sDir,
		std::string const &sBeginning, std::string const &sContaining, std::string const &sEnding,
		std::vector<std::string> &asOut, bool bOnlyDirs );
	void DelFileSet( std::map<std::string, FileSet *>::iterator dir );

	/* The given path wasn't cached.  Cache it. */
	virtual void PopulateFileSet( FileSet & /* fs */, const std::string & /* sPath */ ) { }
};

/* This FilenameDB must be populated in advance. */
class NullFilenameDB: public FilenameDB
{
public:
	NullFilenameDB() { ExpireSeconds = -1; }
	void CacheFile( const std::string & /* sPath */ ) { }
};

#endif

/**
 * @file
 * @author Glenn Maynard (c) 2003-2004
 * @section LICENSE
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
