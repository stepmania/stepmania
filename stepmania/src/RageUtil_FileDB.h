#ifndef RAGE_UTIL_FILEDB
#define RAGE_UTIL_FILEDB 1

#include <set>
#include <map>
#include "RageTimer.h"

enum FileType { TTYPE_FILE, TTYPE_DIR, TTYPE_NONE };

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
	int mtime;

	/* Private data, for RageFileDrivers. */
	void *priv;
	File() { dir=false; size=-1; mtime=-1; priv=NULL;}
	File( const CString &fn )
	{
		SetName( fn );
		dir=false; size=-1; mtime=-1; priv=NULL;
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

	FileType GetFileType( const CString &path ) const;
	int GetFileSize(const CString &path) const;
	int GetFileModTime(const CString &path) const;
};

class FilenameDB
{
protected:
	FileSet &GetFileSet( CString dir );

	/* Directories we have cached: */
	map<CString, FileSet *> dirs;

	int ExpireSeconds;

	void GetFilesEqualTo(const CString &dir, const CString &fn, vector<CString> &out, bool bOnlyDirs);
	void GetFilesMatching(const CString &dir, 
		const CString &beginning, const CString &containing, const CString &ending, 
		vector<CString> &out, bool bOnlyDirs);
	void AddFileSet( CString sPath, FileSet *fs );

	/* The given path wasn't cached.  Cache it. */
	virtual void PopulateFileSet( FileSet &fs, const CString &sPath ) { }

public:
	FilenameDB::FilenameDB():
		ExpireSeconds( -1 ) { }
	virtual FilenameDB::~FilenameDB() { FlushDirCache(); }

	void AddFile( const CString &sPath, int size, int mtime, void *priv=NULL );
	File *GetFile( const CString &path );

	/* This handles at most two * wildcards.  If we need anything more complicated,
	 * we'll need to use fnmatch or regex. */
	void GetFilesSimpleMatch(const CString &dir, const CString &fn, vector<CString> &out, bool bOnlyDirs);

	/* Search for "path" case-insensitively and replace it with the correct
	 * case.  If "path" doesn't exist at all, return false and don't change it. */
	bool ResolvePath(CString &path);

	FileType GetFileType( const CString &path );
	int GetFileSize(const CString &path);
	int GetFileModTime( const CString &sFilePath );
	void GetDirListing( CString sPath, CStringArray &AddTo, bool bOnlyDirs, bool bReturnPathToo );

	void FlushDirCache();
};

#endif
