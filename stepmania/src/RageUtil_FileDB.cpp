#include "global.h"

#include "RageUtil_FileDB.h"
#include "RageUtil.h"
#include "RageTimer.h"
#include "RageLog.h"
#include <map>
#include <set>
#include <sys/stat.h>
#include <sys/types.h>
#include <errno.h>
#include "RageFile.h"
#include "arch/arch.h"

#if !defined(WIN32)
#include <dirent.h>
#include <fcntl.h>
#else
#include "windows.h"
#endif


struct File {
	istring name;
	bool dir;
	int size;
	/* Modification time of the file.  The contents of this is undefined, except that
	 * when the file has been modified, this value will change. */
	int mtime;

	File() { dir=false; size=-1; mtime=-1; }
	File( istring fn ): name(fn) { dir=false; size=-1; mtime=-1; }
	
	bool operator== (const File &rhs) const { return name==rhs.name; }
	bool operator< (const File &rhs) const { return name<rhs.name; }

	bool equal(const File &rhs) const { return name == rhs.name; }
	bool equal(const CString &rhs) const {
		return !stricmp(name.c_str(), rhs.c_str());
	}
};

/* This represents a directory. */
struct FileSet
{
	set<File> files;
	RageTimer age;
	void LoadFromDir(const CString &dir);
	void GetFilesMatching(
		const CString &beginning, const CString &containing, const CString &ending,
		vector<CString> &out, bool bOnlyDirs) const;
	void GetFilesEqualTo(const CString &pat, vector<CString> &out, bool bOnlyDirs) const;
	bool DoesFileExist(const CString &path) const;
	bool IsADirectory(const CString &path) const;
	bool IsAFile(const CString &path) const;
	int GetFileSize(const CString &path) const;
	int GetFileModTime(const CString &path) const;
};

void FileSet::LoadFromDir(const CString &dir)
{
	age.GetDeltaTime(); /* reset */
	files.clear();

#if defined(WIN32)
	WIN32_FIND_DATA fd;
	CString dirHolder = dir ;

	if ( dirHolder.size() > 0  && dirHolder.Right(1) == SLASH )
	{
		dirHolder.erase( dirHolder.size() - 1 ) ;
	}

	HANDLE hFind = FindFirstFile( dirHolder+SLASH "*", &fd );

	if( hFind == INVALID_HANDLE_VALUE )
		return;

	do {
		if(!strcmp(fd.cFileName, ".") || !strcmp(fd.cFileName, ".."))
			continue;

		File f;
		f.name=fd.cFileName;
		f.dir = !!(fd.dwFileAttributes & FILE_ATTRIBUTE_DIRECTORY);
		f.size = fd.nFileSizeLow;
		f.mtime = fd.ftLastWriteTime.dwLowDateTime;

		files.insert(f);
	} while( FindNextFile( hFind, &fd ) );
	FindClose(hFind);
#else
	int OldDir = open(".", O_RDONLY);
	if( OldDir == -1 )
		RageException::Throw( "Couldn't open(.): %s", strerror(errno) );

	if( chdir(dir.c_str()) == -1 )
	{
		/* Only log once per dir. */
		if( LOG )
			LOG->MapLog("chdir " + dir, "Couldn't chdir(%s): %s", dir.c_str(), strerror(errno) );
		close( OldDir );
		return;
	}
	DIR *d = opendir(".");

	while(struct dirent *ent = readdir(d))
	{
		if(!strcmp(ent->d_name, ".")) continue;
		if(!strcmp(ent->d_name, "..")) continue;
		
		File f;
		f.name=ent->d_name;
		
		struct stat st;
		if( stat(ent->d_name, &st) == -1 )
		{
			/* If it's a broken symlink, ignore it.  Otherwise, warn. */
			if( lstat(ent->d_name, &st) == 0 )
				continue;
			
			/* Huh? */
			if(LOG)
				LOG->Warn("Got file '%s' in '%s' from list, but can't stat? (%s)",
					ent->d_name, dir.c_str(), strerror(errno));
			continue;
		} else {
			f.dir = (st.st_mode & S_IFDIR);
			f.size = st.st_size;
			f.mtime = st.st_mtime;
		}

		files.insert(f);
	}
	       
	closedir(d);
	if( fchdir( OldDir ) == -1 )
		RageException::Throw( "Couldn't fchdir(): %s", strerror(errno) );
	close( OldDir );
#endif
}

/* Search for "beginning*containing*ending". */
void FileSet::GetFilesMatching(const CString &beginning, const CString &containing, const CString &ending, vector<CString> &out, bool bOnlyDirs) const
{
	/* "files" is a case-insensitive mapping, by filename.  Use lower_bound to figure
	 * out where to start. */
	set<File>::const_iterator i = files.lower_bound(File(beginning.c_str()));
	for( ; i != files.end(); ++i)
	{
		if(bOnlyDirs && !i->dir) continue;

		/* Check beginning. Once we hit a filename that no longer matches beginning,
		 * we're past all possible matches in the sort, so stop. */
		if(beginning.size() > i->name.size()) break; /* can't start with it */
		if(strnicmp(i->name.c_str(), beginning.c_str(), beginning.size())) break; /* doesn't start with it */

		/* Position the end starts on: */
		int end_pos = int(i->name.size())-int(ending.size());

		/* Check end. */
		if(end_pos < 0) continue; /* can't end with it */
		if(stricmp(i->name.c_str()+end_pos, ending.c_str())) continue; /* doesn't end with it */

		/* Check containing.  Do this last, since it's the slowest (substring
		 * search instead of string match). */
		if(containing.size())
		{
			unsigned pos = i->name.find(containing, beginning.size());
			if(pos == i->name.npos) continue; /* doesn't contain it */
			if(pos + containing.size() > unsigned(end_pos)) continue; /* found it but it overlaps with the end */
		}

		out.push_back(i->name.c_str());
	}
}

void FileSet::GetFilesEqualTo(const CString &str, vector<CString> &out, bool bOnlyDirs) const
{
	set<File>::const_iterator i = files.find(File(str.c_str()));
	if(i == files.end())
		return;

	if(bOnlyDirs && !i->dir)
		return;

	out.push_back(i->name.c_str());
}

bool FileSet::DoesFileExist(const CString &path) const
{
	return files.find(File(path.c_str())) != files.end();
}

bool FileSet::IsADirectory(const CString &path) const
{
	set<File>::const_iterator i = files.find(File(path.c_str()));
	if(i == files.end())
		return false;
	return i->dir;
}

bool FileSet::IsAFile(const CString &path) const
{
	set<File>::const_iterator i = files.find(File(path.c_str()));
	if(i == files.end())
		return false;
	return !i->dir;
}

int FileSet::GetFileSize(const CString &path) const
{
	set<File>::const_iterator i = files.find(File(path.c_str()));
	if(i == files.end())
		return -1;
	return i->size;
}

int FileSet::GetFileModTime(const CString &path) const
{
	set<File>::const_iterator i = files.find(File(path.c_str()));
	if(i == files.end())
		return -1;
	return i->mtime;
}

/* Given "foo/bar/baz/" or "foo/bar/baz", return "foo/bar/" and "baz". */
static void SplitPath( CString Path, CString &Dir, CString &Name )
{
	/* Strip off any trailing slashes. */
	if( Path.size() > 0 && Path.Right(1) == SLASH )
		Path.erase( Path.size()-1 );

#ifdef _XBOX
	static Regex split("(.*\\\\)([^\\\\]+)");
#else
	static Regex split("(.*/)([^/]+)");
#endif

	CStringArray match;
	if(split.Compare(Path, match)) {
		/* At least one slash. */
		Dir = match[0];
		Name = match[1];
	} else {
		/* No slash. */
#ifdef _XBOX
		Dir = "D:\\" ;
		Name = "" ;
#else
		Dir = "." SLASH;
		Name = Path;
#endif
	}
}



class FilenameDB
{
	FileSet &GetFileSet(CString dir, bool ResolveCase = true);

	/* Directories we have cached: */
	map<istring, FileSet *> dirs;

	void GetFilesEqualTo(const CString &dir, const CString &fn, vector<CString> &out, bool bOnlyDirs);
	void GetFilesMatching(const CString &dir, 
		const CString &beginning, const CString &containing, const CString &ending, 
		vector<CString> &out, bool bOnlyDirs);

public:
	/* This handles at most one * wildcard.  If we need anything more complicated,
	 * we'll need to use fnmatch or regex. */
	void GetFilesSimpleMatch(const CString &dir, const CString &fn, vector<CString> &out, bool bOnlyDirs);

	/* Search for "path" case-insensitively and replace it with the correct
	 * case.  If "path" doesn't exist at all, return false and don't change it. */
	bool ResolvePath(CString &path);

	bool DoesFileExist(const CString &path);
	bool IsAFile(const CString &path);
	bool IsADirectory(const CString &path);
	int GetFileSize(const CString &path);
	int GetFileModTime( const CString &sFilePath );

	void FlushDirCache();
};

bool FilenameDB::DoesFileExist( const CString &sPath )
{
	CString Dir, Name;
	SplitPath(sPath, Dir, Name);
	FileSet &fs = GetFileSet(Dir.c_str());
	return fs.DoesFileExist(Name);
}

bool FilenameDB::IsAFile( const CString &sPath )
{
	CString Dir, Name;
	SplitPath(sPath, Dir, Name);
	FileSet &fs = GetFileSet(Dir.c_str());
	return fs.IsAFile(Name);
}

bool FilenameDB::IsADirectory( const CString &sPath )
{
	CString Dir, Name;
	SplitPath(sPath, Dir, Name);
	FileSet &fs = GetFileSet(Dir.c_str());

#ifdef _XBOX
	if ( ( Dir == "D:\\" ) && ( Name == "" ) )
		return true ;
#endif

	return fs.IsADirectory(Name);
}

int FilenameDB::GetFileSize( const CString &sPath )
{
	CString Dir, Name;
	SplitPath(sPath, Dir, Name);
	FileSet &fs = GetFileSet(Dir.c_str());
	return fs.GetFileSize(Name);
}

int FilenameDB::GetFileModTime( const CString &sPath )
{
	CString Dir, Name;
	SplitPath(sPath, Dir, Name);
	FileSet &fs = GetFileSet(Dir.c_str());
	return fs.GetFileModTime(Name);
}

/* XXX: this won't work right for URIs, eg \\foo\bar */
bool FilenameDB::ResolvePath(CString &path)
{
	if(path == ".") return true;
	if(path == "") return true;

	/* Split path into components. */
	vector<CString> p;
	split(path, SLASH, p, true);

	/* If we have "/foo", then add a blank entry to the beginning to line things up. */
	if( path.Left( strlen(SLASH) ) == SLASH )
		p.insert( p.begin(), "" );

	/* Resolve each component.  Assume the first component is correct. XXX
	 * don't do that! "Songs/" vs "songs/" */
	CString ret = p[0];
	for(unsigned i = 1; i < p.size(); ++i)
	{
		ret += SLASH;

		vector<CString> lst;
		FileSet &fs = GetFileSet( ret );
		fs.GetFilesEqualTo(p[i], lst, false);

		/* If there were no matches, the path isn't found. */
		if(lst.empty()) return false;

		if( lst.size() > 1 && LOG )
			LOG->Warn("Ambiguous filenames '%s' and '%s'",
				lst[0].c_str(), lst[1].c_str());

		ret += lst[0];
	}

	if(path.Right(1) == SLASH)
		path = ret + SLASH;
	else
		path = ret;
	return true;
}

void FilenameDB::GetFilesMatching(const CString &dir, const CString &beginning, const CString &containing, const CString &ending, vector<CString> &out, bool bOnlyDirs)
{
	FileSet &fs = GetFileSet(dir.c_str());
	fs.GetFilesMatching(beginning, containing, ending, out, bOnlyDirs);
}

void FilenameDB::GetFilesEqualTo(const CString &dir, const CString &fn, vector<CString> &out, bool bOnlyDirs)
{
	FileSet &fs = GetFileSet(dir.c_str());
	fs.GetFilesEqualTo(fn, out, bOnlyDirs);
}


void FilenameDB::GetFilesSimpleMatch(const CString &dir, const CString &fn, vector<CString> &out, bool bOnlyDirs)
{
	/* Does this contain a wildcard? */
	unsigned first_pos = fn.find_first_of('*');
	if(first_pos == fn.npos)
	{
		/* No; just do a regular search. */
		GetFilesEqualTo(dir, fn, out, bOnlyDirs);
	} else {
		unsigned second_pos = fn.find_first_of('*', first_pos+1);
		if(second_pos == fn.npos)
		{
			/* Only one *: "A*B". */
			/* XXX: "_blank.png*.png" shouldn't match the file "_blank.png". */
			GetFilesMatching(dir, fn.substr(0, first_pos), "", fn.substr(first_pos+1), out, bOnlyDirs);
		} else {
			/* Two *s: "A*B*C". */
			GetFilesMatching(dir, 
				fn.substr(0, first_pos),
				fn.substr(first_pos+1, second_pos-first_pos-1),
				fn.substr(second_pos+1), out, bOnlyDirs);
		}
	}
}

FileSet &FilenameDB::GetFileSet(CString dir, bool ResolveCase)
{
	/* Normalize the path. */
	dir.Replace("\\", SLASH); /* foo\bar -> foo/bar */
	dir.Replace("/", SLASH); /* foo//bar -> foo/bar */
	dir.Replace("//", SLASH); /* foo//bar -> foo/bar */

	FileSet *ret;
	map<istring, FileSet *>::iterator i = dirs.find(dir.c_str());
	bool reload = false;
	if(i == dirs.end())
	{
		ret = new FileSet;
		dirs[dir.c_str()] = ret;
		reload = true;
	}
	else
	{
		ret = i->second;
		if(ret->age.PeekDeltaTime() > 30)
			reload = true;
	}

	if(reload)
	{
		CString RealDir = dir;
		if(ResolveCase)
		{
			/* Resolve path cases (path/Path -> PATH/path). */
			ResolvePath(RealDir);

			/* Alias this name, too. */
			dirs[RealDir.c_str()] = ret;
		}

		ret->LoadFromDir(RealDir);
	}
	return *ret;
}

void FilenameDB::FlushDirCache()
{
	set<FileSet *> freed;
	for( map<istring, FileSet *>::iterator i = dirs.begin(); i != dirs.end(); ++i )
	{
		if( freed.find(i->second) != freed.end() )
			continue;

		delete i->second;
		freed.insert( i->second );
	}
	dirs.clear();
}


FilenameDB FDB;

void GetDirListing( CString sPath, CStringArray &AddTo, bool bOnlyDirs, bool bReturnPathToo )
{
//	LOG->Trace( "GetDirListing( %s )", sPath.c_str() );

	/* If you want the CWD, use ".". */
	ASSERT(!sPath.empty());

	/* XXX: for case-insensitive resolving, we assume the first element is
	 * correct (we need a place to start from); so if sPath is relative,
	 * prepend "./" */

	/* Strip off the last path element and use it as a mask. */
	unsigned pos = sPath.find_last_of( SLASH );
	CString fn;
	if(pos != sPath.npos)
	{
		fn = sPath.substr(pos+1);
		sPath = sPath.substr(0, pos+1);
	}

	/* If there was only one path element, or if the last element was empty,
	 * use "*". */
	if(fn.size() == 0)
		fn = "*";

	unsigned start = AddTo.size();
	FDB.GetFilesSimpleMatch(sPath, fn, AddTo, bOnlyDirs);

	if(bReturnPathToo && start < AddTo.size())
	{
		FDB.ResolvePath(sPath);
		while(start < AddTo.size())
		{
			AddTo[start] = sPath + AddTo[start];
			start++;
		}
	}

//	LOG->Trace( "dir is '%s'", sPath.c_str() );
//	LOG->Trace( "Found:" );
//	for( unsigned i=0; i<AddTo.size(); i++ )
//		LOG->Trace( AddTo[i] );
}

bool ResolvePath(CString &path) { return FDB.ResolvePath(path); }

void FlushDirCache()
{
	FDB.FlushDirCache();
}


#if 1
bool DoesFileExist( const CString &sPath ) { return FDB.DoesFileExist(sPath); }
bool IsAFile( const CString &sPath ) { return FDB.IsAFile(sPath); }
bool IsADirectory( const CString &sPath ) { return FDB.IsADirectory(sPath); }
int GetFileModTime( const CString &sPath ) { return FDB.GetFileModTime(sPath); }
unsigned GetFileSizeInBytes( const CString &sPath )
{
	int ret = FDB.GetFileSize(sPath);
//	LOG->Trace("file '%s' size %i", sPath.c_str(), ret);
	if( ret == -1 )
		return 0;
	return ret;
}
#else
static bool DoStat(CString sPath, struct stat *st)
{
	TrimRight(sPath, "/\\");
    return stat(sPath.c_str(), st) != -1;
}
bool DoesFileExist( const CString &sPath )
{
	if(sPath.empty()) return false;
	struct stat st;
    return DoStat(sPath, &st);
}

bool IsAFile( const CString &sPath )
{
    return DoesFileExist(sPath) && !IsADirectory(sPath);
}

bool IsADirectory( const CString &sPath )
{
	if(sPath.empty()) return false;
	struct stat st;
    if (!DoStat(sPath, &st))
		return false;

	return !!(st.st_mode & S_IFDIR);
}

unsigned GetFileSizeInBytes( const CString &sFilePath )
{
	struct stat st;
	if(!DoStat(sFilePath, &st))
		return 0;
	
	return st.st_size;
}

int GetFileModTime( const CString &sPath )
{
	struct stat st;
	if(!DoStat(sFilePath, &st))
		return -1;
	
	return st.st_mtime;
}
#endif
