#ifndef RAGE_UTIL_FILEDB
#define RAGE_UTIL_FILEDB 1

#include <set>
#include <map>
#include "RageTimer.h"
#include "RageFileManager.h"

struct FileSet;
struct File
{
	CString name;
	CString lname;

	void SetName( const CString &fn )
	{
		name = fn;
		lname = name;
		lname.MakeLower();
	}
	
	bool dir;
	int size;
	/* Modification time of the file.  The contents of this is undefined, except that
	 * when the file has been modified, this value will change. */
	int hash;

	/* Private data, for RageFileDrivers. */
	void *priv;

	/* If this is non-NULL, and dir is true, this is a pointer to the FileSet containing
	 * the directory contents.  (This is a cache; it isn't always set.) */
	const FileSet *dirp;

	File() { dir=false; dirp=NULL; size=-1; hash=-1; priv=NULL;}
	File( const CString &fn )
	{
		SetName( fn );
		dir=false; size=-1; hash=-1; priv=NULL;
	}
	
	bool operator== (const File &rhs) const { return lname==rhs.lname; }
	bool operator< (const File &rhs) const { return lname<rhs.lname; }

	bool equal(const File &rhs) const { return lname == rhs.lname; }
	bool equal(const CString &rhs) const
	{
		CString l = rhs;
		l.MakeLower();
		return lname == l;
	}
};

/* This represents a directory. */
struct FileSet
{
	set<File> files;
	RageTimer age;
	void GetFilesMatching(
		const CString &beginning, const CString &containing, const CString &ending,
		vector<CString> &out, bool bOnlyDirs) const;
	void GetFilesEqualTo(const CString &pat, vector<CString> &out, bool bOnlyDirs) const;

	RageFileManager::FileType GetFileType( const CString &path ) const;
	int GetFileSize(const CString &path) const;
	int GetFileHash(const CString &path) const;
};

class FilenameDB
{
protected:
	FileSet *GetFileSet( CString dir, bool create=true );

	/* Directories we have cached: */
	map<CString, FileSet *> dirs;

	int ExpireSeconds;

	void GetFilesEqualTo(const CString &dir, const CString &fn, vector<CString> &out, bool bOnlyDirs);
	void GetFilesMatching(const CString &dir, 
		const CString &beginning, const CString &containing, const CString &ending, 
		vector<CString> &out, bool bOnlyDirs);
	void DelFileSet( map<CString, FileSet *>::iterator dir );

	/* The given path wasn't cached.  Cache it. */
	virtual void PopulateFileSet( FileSet &fs, const CString &sPath ) { }

public:
	FilenameDB::FilenameDB():
		ExpireSeconds( -1 ) { }
	virtual FilenameDB::~FilenameDB() { FlushDirCache(); }

	void AddFile( const CString &sPath, int size, int hash, void *priv=NULL );
	void DelFile( const CString &sPath );
	const File *GetFile( const CString &path );
	const void *GetFilePriv( const CString &path );

	/* This handles at most two * wildcards.  If we need anything more complicated,
	 * we'll need to use fnmatch or regex. */
	void GetFilesSimpleMatch(const CString &dir, const CString &fn, vector<CString> &out, bool bOnlyDirs);

	/* Search for "path" case-insensitively and replace it with the correct
	 * case.  If only a portion of the path exists, resolve as much as possible.
	 * Return true if the entire path was matched. */
	bool ResolvePath( CString &path );

	RageFileManager::FileType GetFileType( const CString &path );
	int GetFileSize(const CString &path);
	int GetFileHash( const CString &sFilePath );
	void GetDirListing( CString sPath, CStringArray &AddTo, bool bOnlyDirs, bool bReturnPathToo );

	void FlushDirCache();
};

/* This FilenameDB must be populated in advance. */
class NullFilenameDB: public FilenameDB
{
public:
	NullFilenameDB() { ExpireSeconds = -1; }

protected:
	void PopulateFileSet( FileSet &fs, const CString &sPath ) { }
};

#endif

/*
 * Copyright (c) 2003-2004 Glenn Maynard
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
