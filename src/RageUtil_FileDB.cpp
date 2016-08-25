#include "global.h"

#include "RageUtil_FileDB.h"
#include "RageUtil.h"
#include "RageLog.h"
#include "RageString.hpp"

using std::vector;
using std::string;

/* Search for "beginning*containing*ending". */
void FileSet::GetFilesMatching( std::string const &sBeginning_, std::string const &sContaining_, std::string const &sEnding_, vector<std::string> &asOut, bool bOnlyDirs ) const
{
	/* "files" is a case-insensitive mapping, by filename.  Use lower_bound to figure
	 * out where to start. */
	auto sBeginning = Rage::make_lower(sBeginning_);
	auto sContaining = Rage::make_lower(sContaining_);
	auto sEnding = Rage::make_lower(sEnding_);

	std::set<File>::const_iterator i = files.lower_bound( File(sBeginning) );
	for( ; i != files.end(); ++i )
	{
		const File &f = *i;

		if( bOnlyDirs && !f.dir )
			continue;

		const std::string &sPath = f.lname;

		/* Check sBeginning. Once we hit a filename that no longer matches sBeginning,
		 * we're past all possible matches in the sort, so stop. */
		if( sBeginning.size() > sPath.size() )
			break; /* can't start with it */
		if( sPath.compare(0, sBeginning.size(), sBeginning) )
			break; /* doesn't start with it */

		/* Position the end starts on: */
		int end_pos = int(sPath.size())-int(sEnding.size());

		/* Check end. */
		if( end_pos < 0 )
			continue; /* can't end with it */
		if( sPath.compare(end_pos, string::npos, sEnding) )
			continue; /* doesn't end with it */

		/* Check sContaining.  Do this last, since it's the slowest (substring
		 * search instead of string match). */
		if( !sContaining.empty() )
		{
			size_t pos = sPath.find( sContaining, sBeginning.size() );
			if( pos == sPath.npos )
				continue; /* doesn't contain it */
			if( pos + sContaining.size() > unsigned(end_pos) )
				continue; /* found it but it overlaps with the end */
		}

		asOut.push_back( f.name );
	}
}

void FileSet::GetFilesEqualTo( std::string const &sStr, vector<std::string> &asOut, bool bOnlyDirs ) const
{
	std::set<File>::const_iterator i = files.find( File(sStr) );
	if( i == files.end() )
	{
		return;
	}
	if( bOnlyDirs && !i->dir )
	{
		return;
	}
	asOut.push_back( i->name );
}

RageFileManager::FileType FileSet::GetFileType( const std::string &sPath ) const
{
	std::set<File>::const_iterator i = files.find( File(sPath) );
	if( i == files.end() )
	{
		return RageFileManager::TYPE_NONE;
	}
	return i->dir? RageFileManager::TYPE_DIR:RageFileManager::TYPE_FILE;
}

int FileSet::GetFileSize( const std::string &sPath ) const
{
	std::set<File>::const_iterator i = files.find( File(sPath) );
	if( i == files.end() )
	{
		return -1;
	}
	return i->size;
}

int FileSet::GetFileHash( const std::string &sPath ) const
{
	std::set<File>::const_iterator i = files.find( File(sPath) );
	if( i == files.end() )
	{
		return -1;
	}
	return i->hash + i->size;
}

void File::SetName(std::string const &fn)
{
	name = fn;
	lname = Rage::make_lower(name);
}

/*
 * Given "foo/bar/baz/" or "foo/bar/baz", return "foo/bar/" and "baz".
 * "foo" -> "", "foo"
 */
static void SplitPath( std::string sPath, std::string &sDir, std::string &sName )
{
	CollapsePath( sPath );
	if (Rage::ends_with(sPath, "/"))
	{
		sPath.erase(sPath.size() - 1);
	}
	size_t iSep = sPath.find_last_of( '/' );
	if( iSep == std::string::npos )
	{
		sDir = "";
		sName = sPath;
	}
	else
	{
		sDir = sPath.substr( 0, iSep+1 );
		sName = sPath.substr( iSep+1 );
	}
}


RageFileManager::FileType FilenameDB::GetFileType( const std::string &sPath )
{
	ASSERT( !m_Mutex.IsLockedByThisThread() );

	std::string sDir, sName;
	SplitPath( sPath, sDir, sName );

	if( sName == "/" )
		return RageFileManager::TYPE_DIR;

	const FileSet *fs = GetFileSet( sDir );
	RageFileManager::FileType ret = fs->GetFileType( sName );
	m_Mutex.Unlock(); /* locked by GetFileSet */
	return ret;
}


int FilenameDB::GetFileSize( const std::string &sPath )
{
	ASSERT( !m_Mutex.IsLockedByThisThread() );

	std::string sDir, sName;
	SplitPath( sPath, sDir, sName );

	const FileSet *fs = GetFileSet( sDir );
	int ret = fs->GetFileSize( sName );
	m_Mutex.Unlock(); /* locked by GetFileSet */
	return ret;
}

int FilenameDB::GetFileHash( const std::string &sPath )
{
	ASSERT( !m_Mutex.IsLockedByThisThread() );

	std::string sDir, sName;
	SplitPath( sPath, sDir, sName );

	const FileSet *fs = GetFileSet( sDir );
	int ret = fs->GetFileHash( sName );
	m_Mutex.Unlock(); /* locked by GetFileSet */
	return ret;
}

/* path should be fully collapsed, so we can operate in-place: no . or .. */
bool FilenameDB::ResolvePath( std::string &sPath )
{
	if( sPath == "/" || sPath == "" )
		return true;

	/* Split path into components. */
	int iBegin = 0, iSize = -1;

	/* Resolve each component. */
	std::string ret = "";
	const FileSet *fs = nullptr;

	static const std::string slash("/");
	for(;;)
	{
        Rage::split_in_place( sPath, slash, iBegin, iSize, Rage::EmptyEntries::skip );
		if( iBegin == (int) sPath.size() )
			break;

		if( fs == nullptr )
			fs = GetFileSet( ret );
		else
			m_Mutex.Lock(); /* for access to fs */

		std::string p = sPath.substr( iBegin, iSize );
		ASSERT_M( p.size() != 1 || p[0] != '.', sPath ); // no .
		ASSERT_M( p.size() != 2 || p[0] != '.' || p[1] != '.', sPath ); // no ..
		std::set<File>::const_iterator it = fs->files.find( File(p) );

		/* If there were no matches, the path isn't found. */
		if( it == fs->files.end() )
		{
			m_Mutex.Unlock(); /* locked by GetFileSet */
			return false;
		}

		ret += "/" + it->name;

		fs = it->dirp;

		m_Mutex.Unlock(); /* locked by GetFileSet */
	}

	if( sPath.size() && sPath[sPath.size()-1] == '/' )
		sPath = ret + "/";
	else
		sPath = ret;
	return true;
}

void FilenameDB::GetFilesMatching( std::string const &sDir, std::string const &sBeginning, std::string const &sContaining, std::string const &sEnding, vector<std::string> &asOut, bool bOnlyDirs )
{
	ASSERT( !m_Mutex.IsLockedByThisThread() );

	const FileSet *fs = GetFileSet( sDir );
	fs->GetFilesMatching( sBeginning, sContaining, sEnding, asOut, bOnlyDirs );
	m_Mutex.Unlock(); /* locked by GetFileSet */
}

void FilenameDB::GetFilesEqualTo( std::string const &sDir, std::string const &sFile, vector<std::string> &asOut, bool bOnlyDirs )
{
	ASSERT( !m_Mutex.IsLockedByThisThread() );

	const FileSet *fs = GetFileSet( sDir );
	fs->GetFilesEqualTo( sFile, asOut, bOnlyDirs );
	m_Mutex.Unlock(); /* locked by GetFileSet */
}


void FilenameDB::GetFilesSimpleMatch( std::string const &sDir, std::string const &sMask, vector<std::string> &asOut, bool bOnlyDirs )
{
	/* Does this contain a wildcard? */
	size_t first_pos = sMask.find_first_of( '*' );
	if( first_pos == sMask.npos )
	{
		/* No; just do a regular search. */
		GetFilesEqualTo( sDir, sMask, asOut, bOnlyDirs );
		return;
	}
	size_t second_pos = sMask.find_first_of( '*', first_pos+1 );
	if( second_pos == sMask.npos )
	{
		/* Only one *: "A*B". */
		/* XXX: "_blank.png*.png" shouldn't match the file "_blank.png". */
		GetFilesMatching( sDir, sMask.substr(0, first_pos), "", sMask.substr(first_pos+1), asOut, bOnlyDirs );
		return;
	}

	/* Two *s: "A*B*C". */
	GetFilesMatching( sDir,
		sMask.substr(0, first_pos),
		sMask.substr(first_pos+1, second_pos-first_pos-1),
		sMask.substr(second_pos+1), asOut, bOnlyDirs );
}

/*
 * Get the FileSet for dir; if create is true, create the FileSet if necessary.
 *
 * We want to unlock the object while we populate FileSets, so m_Mutex should not
 * be locked when this is called.  It will be locked on return; the caller must
 * unlock it.
 */
FileSet *FilenameDB::GetFileSet( const std::string &sDir_, bool bCreate )
{
	std::string sDir = sDir_;

	/* Creating can take a long time; don't hold the lock if we might do that. */
	if( bCreate && m_Mutex.IsLockedByThisThread() && LOG )
		LOG->Warn( "FilenameDB::GetFileSet: m_Mutex was locked" );

	/* Normalize the path. */
	Rage::replace(sDir, "\\", "/"); /* foo\bar -> foo/bar */
	Rage::replace(sDir, "//", "/"); /* foo//bar -> foo/bar */

	if (sDir == "")
	{
		sDir = "/";
	}
	std::string sLower = Rage::make_lower(sDir);

	m_Mutex.Lock();

	for(;;)
	{
		/* Look for the directory. */
		auto i = dirs.find( sLower );
		if( !bCreate )
		{
			if( i == dirs.end() )
				return nullptr;
			return i->second;
		}

		/* We're allowed to create.  If the directory wasn't found, break out and
		 * create it. */
		if( i == dirs.end() )
			break;

		/* This directory already exists.  If it's still being filled in by another
		 * thread, wait for it. */
		FileSet *pFileSet = i->second;
		if( !pFileSet->m_bFilled )
		{
			m_Mutex.Wait();

			/* Beware: when we unlock m_Mutex to wait for it to finish filling,
			 * we give up our claim to dirs, so i may be invalid.  Start over
			 * and re-search. */
			continue;
		}

		if( ExpireSeconds == -1 || pFileSet->age.PeekDeltaTime() < ExpireSeconds )
		{
			/* Found it, and it hasn't expired. */
			return pFileSet;
		}

		/* It's expired.  Delete the old entry. */
		this->DelFileSet( i );
		break;
	}

	/* Create the FileSet and insert it.  Set it to !m_bFilled, so if other threads
	 * happen to try to use this directory before we finish filling it, they'll wait. */
	FileSet *pRet = new FileSet;
	pRet->m_bFilled = false;
	dirs[sLower] = pRet;

	/* Unlock while we populate the directory.  This way, reads to other directories
	 * won't block if this takes a while. */
	m_Mutex.Unlock();
	ASSERT( !m_Mutex.IsLockedByThisThread() );
	PopulateFileSet( *pRet, sDir );

	/* If this isn't the root directory, we want to set the dirp pointer of our parent
	 * to the newly-created directory.  Find the pointer we need to set.  Be careful of
	 * order of operations, here: since we just unlocked, any this->dirs searches we did
	 * previously are no longer valid. */
	FileSet **pParentDirp = nullptr;
	if( sDir != "/" )
	{
		std::string sParent = Rage::dir_name( sDir );
		if( sParent == "./" )
			sParent = "";

		/* This also re-locks m_Mutex for us. */
		FileSet *pParent = GetFileSet( sParent );
		if( pParent != nullptr )
		{
			std::set<File>::iterator it = pParent->files.find( File(Rage::base_name(sDir)) );
			if( it != pParent->files.end() )
				pParentDirp = const_cast<FileSet **>(&it->dirp);
		}
	}
	else
	{
		m_Mutex.Lock();
	}

	if( pParentDirp != nullptr )
		*pParentDirp = pRet;

	pRet->age.Touch();
	pRet->m_bFilled = true;

	/* Signal the event, to wake up any other threads that might be waiting for this
	 * directory.  Leave the mutex locked; those threads will wake up when the current
	 * operation completes. */
	m_Mutex.Broadcast();

	return pRet;
}

/* Add the file or directory "sPath".  sPath is a directory if it ends with
 * a slash. */
void FilenameDB::AddFile( const std::string &sPath_, int iSize, int iHash, void *pPriv )
{
	std::string sPath(sPath_);

	if( sPath == "" || sPath == "/" )
		return;

	if( sPath[0] != '/' )
		sPath = "/" + sPath;

	auto asParts = Rage::split(sPath, "/", Rage::EmptyEntries::include);

	auto begin = asParts.begin();
	auto end = asParts.end();

	bool IsDir = true;
	if( sPath[sPath.size()-1] != '/' )
	{
		IsDir = false;
	}
	else
	{
		--end;
	}
	/* Skip the leading slash. */
	++begin;

	do
	{
		/* Combine all but the last part. */
		std::string dir = "/" + Rage::join( "/", begin, end-1 );
		if( dir != "/" )
		{
			dir += "/";
		}
		const std::string &fn = *(end-1);
		FileSet *fs = GetFileSet( dir );
		ASSERT( m_Mutex.IsLockedByThisThread() );

		// const_cast to cast away the constness that is only needed for the name
		File &f = const_cast<File&>(*fs->files.insert( fn ).first);
		f.dir = IsDir;
		if( !IsDir )
		{
			f.size = iSize;
			f.hash = iHash;
			f.priv = pPriv;
		}
		m_Mutex.Unlock(); /* locked by GetFileSet */
		IsDir = true;

		--end;
	} while( begin != end );
}

/* Remove the given FileSet, and all dirp pointers to it.  This means the cache has
 * expired, not that the directory is necessarily gone; don't actually delete the file
 * from the parent. */
void FilenameDB::DelFileSet( std::map<std::string, FileSet *>::iterator dir )
{
	/* If this isn't locked, dir may not be valid. */
	ASSERT( m_Mutex.IsLockedByThisThread() );

	if( dir == dirs.end() )
		return;

	FileSet *fs = dir->second;

	/* Remove any stale dirp pointers. */
	for( auto it = dirs.begin(); it != dirs.end(); ++it )
	{
		FileSet *Clean = it->second;
		for( auto f = Clean->files.begin(); f != Clean->files.end(); ++f )
		{
			File &ff = (File &) *f;
			if( ff.dirp == fs )
				ff.dirp = nullptr;
		}
	}

	delete fs;
	dirs.erase( dir );
}

void FilenameDB::DelFile( const std::string &sPath )
{
	LockMut(m_Mutex);
	std::string lower = Rage::make_lower(sPath);

	auto fsi = dirs.find( lower );
	DelFileSet( fsi );

	/* Delete sPath from its parent. */
	std::string Dir, Name;
	SplitPath(sPath, Dir, Name);
	FileSet *Parent = GetFileSet( Dir, false );
	if( Parent )
		Parent->files.erase( Name );

	m_Mutex.Unlock(); /* locked by GetFileSet */
}

void FilenameDB::FlushDirCache( const std::string & /* sDir */ )
{
	m_Mutex.Lock();

	for(;;)
	{
		if( dirs.empty() )
			break;

		/* Grab the first entry.  Take it out of the list while we hold the
		 * lock, to guarantee that we own it. */
		FileSet *pFileSet = dirs.begin()->second;

		dirs.erase( dirs.begin() );

		/* If it's being filled, we don't really own it until it's finished being
		 * filled, so wait. */
		while( !pFileSet->m_bFilled )
			m_Mutex.Wait();
		delete pFileSet;
	}

#if 0
	/* XXX: This is tricky, we want to flush all of the subdirectories of
	 * sDir, but once we unlock the mutex, we basically have to start over.
	 * It's just an optimization though, so it can wait. */
	{
		if( it != dirs.end() )
		{
			pFileSet = it->second;
			dirs.erase( it );
			while( !pFileSet->m_bFilled )
				m_Mutex.Wait();
			delete pFileSet;

			if( sDir != "/" )
			{
				std::string sParent = Rage::dir_name( sDir );
				if( sParent == "./" )
				{
					sParent = "";
				}
				else
				{
					sParent = Rage::make_lower(sParent);
				}
				it = dirs.find( sParent );
				if( it != dirs.end() )
				{
					FileSet *pParent = it->second;
					std::set<File>::iterator fileit = pParent->files.find( File(Rage::base_name(sDir)) );
					if( fileit != pParent->files.end() )
						fileit->dirp = nullptr;
				}
			}
		}
		else
		{
			LOG->Warn( "Trying to flush an unknown directory %s.", sDir.c_str() );
		}
#endif
		m_Mutex.Unlock();
}

const File *FilenameDB::GetFile( const std::string &sPath )
{
	if( m_Mutex.IsLockedByThisThread() && LOG )
		LOG->Warn( "FilenameDB::GetFile: m_Mutex was locked" );

	std::string Dir, Name;
	SplitPath(sPath, Dir, Name);
	FileSet *fs = GetFileSet( Dir );

	std::set<File>::iterator it = fs->files.find( File(Name) );
	if( it == fs->files.end() )
		return nullptr;

	return &*it;
}

void *FilenameDB::GetFilePriv( const std::string &path )
{
	ASSERT( !m_Mutex.IsLockedByThisThread() );

	const File *pFile = GetFile( path );
	void *pRet = nullptr;
	if( pFile != nullptr )
		pRet = pFile->priv;

	m_Mutex.Unlock(); /* locked by GetFileSet */
	return pRet;
}



void FilenameDB::GetDirListing( std::string const &sPath_, vector<std::string> &asAddTo, bool bOnlyDirs, bool bReturnPathToo )
{
	std::string sPath = sPath_;
//	LOG->Trace( "GetDirListing( %s )", sPath.c_str() );

	ASSERT( !sPath.empty() );

	/* Strip off the last path element and use it as a mask. */
	size_t  pos = sPath.find_last_of( '/' );
	std::string fn;
	if( pos == sPath.npos )
	{
		fn = sPath;
		sPath = "";
	}
	else
	{
		fn = sPath.substr(pos+1);
		sPath = sPath.substr(0, pos+1);
	}

	/* If the last element was empty, use "*". */
	if( fn.size() == 0 )
	{
		fn = "*";
	}
	unsigned iStart = asAddTo.size();
	GetFilesSimpleMatch( sPath, fn, asAddTo, bOnlyDirs );

	if( bReturnPathToo && iStart < asAddTo.size() )
	{
		while( iStart < asAddTo.size() )
		{
			asAddTo[iStart] = sPath + asAddTo[iStart];
			iStart++;
		}
	}
}

/* Get a complete copy of a FileSet.  This isn't very efficient, since it's a deep
 * copy, but allows retrieving a copy from elsewhere without having to worry about
 * our locking semantics. */
void FilenameDB::GetFileSetCopy( const std::string &sDir, FileSet &out )
{
	FileSet *pFileSet = GetFileSet( sDir );
	out = *pFileSet;
	m_Mutex.Unlock(); /* locked by GetFileSet */
}

void FilenameDB::CacheFile( const std::string &sPath )
{
	LOG->Warn( "Slow cache due to: %s", sPath.c_str() );
	FlushDirCache( Rage::dir_name(sPath) );
}

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
