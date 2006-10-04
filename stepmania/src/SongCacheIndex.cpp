#include "global.h"

#include "SongCacheIndex.h"
#include "RageLog.h"
#include "RageUtil.h"
#include "RageFileManager.h"
#include "song.h"
#include "SpecialFiles.h"

/*
 * A quick explanation of song cache hashes: Each song has two hashes; a hash of the
 * song path, and a hash of the song directory.  The former is Song::GetCacheFilePath;
 * it stays the same if the contents of the directory change.  The latter is 
 * GetHashForDirectory(m_sSongDir), and changes on each modification.
 *
 * The file hash is used as the cache filename.  We don't want to use the directory
 * hash: if we do that, then we'll write a new cache file every time the song changes,
 * and they'll accumulate or we'll have to be careful to delete them.
 *
 * The directory hash is stored in here, indexed by the song path, and used to determine
 * if a song has changed.
 *
 * Another advantage of this system is that we can load songs from cache given only their
 * path; we don't have to actually look in the directory (to find out the directory hash)
 * in order to find the cache file.
 */
#define CACHE_INDEX SpecialFiles::CACHE_DIR + "index.cache"


SongCacheIndex *SONGINDEX;

RString SongCacheIndex::GetCacheFilePath( const RString &sGroup, const RString &sPath )
{
	/* Don't use GetHashForFile, since we don't want to spend time
	 * checking the file size and date. */
	RString s;
	
	if( sPath.size() > 2 && sPath[0] == '/' && sPath[sPath.size()-1] == '/' )
		s.assign( sPath, 1, sPath.size() - 2 );
	else if( sPath.size() > 1 && sPath[0] == '/' )
		s.assign( sPath, 1, sPath.size() - 1 );
	else
		s = sPath;
	/* Change slashes and invalid utf-8 characters to _.
	 * http://en.wikipedia.org/wiki/UTF-8
	 * Mac OS X doesn't support precomposed unicode characters in files names and
	 * so we should probably replace them with combining diacritics.
	 * XXX How do we do this and is it even worth it? */
	const char *invalid = "/\xc0\xc1\xfe\xff\xf8\xf9\xfa\xfb\xfc\xfd\xf5\xf6\xf7";
	for( size_t pos = s.find_first_of(invalid); pos != RString::npos; pos = s.find_first_of(invalid, pos) )
		s[pos] = '_';
	// CACHE_DIR ends with a /.
#if defined(XBOX)
	// Use CRC32 to make fatx compatible filenames.
	return ssprintf( "%s%s/%X", SpecialFiles::CACHE_DIR.c_str(), sGroup.c_str(), GetHashForString(s));
#else
	return ssprintf( "%s%s/%s", SpecialFiles::CACHE_DIR.c_str(), sGroup.c_str(), s.c_str() );
#endif
}

SongCacheIndex::SongCacheIndex()
{
	ReadCacheIndex();
}

SongCacheIndex::~SongCacheIndex()
{

}

void SongCacheIndex::ReadFromDisk()
{
	ReadCacheIndex();
}

static void EmptyDir( RString dir )
{
	ASSERT(dir[dir.size()-1] == '/');

	vector<RString> asCacheFileNames;
	GetDirListing( dir, asCacheFileNames );
	for( unsigned i=0; i<asCacheFileNames.size(); i++ )
	{
		if( !IsADirectory(dir + asCacheFileNames[i]) )
			FILEMAN->Remove( dir + asCacheFileNames[i] );
	}
}

void SongCacheIndex::ReadCacheIndex()
{
	CacheIndex.ReadFile( CACHE_INDEX );	// don't care if this fails

	int iCacheVersion = -1;
	CacheIndex.GetValue( "Cache", "CacheVersion", iCacheVersion );
	if( iCacheVersion == FILE_CACHE_VERSION )
		return; /* OK */

	LOG->Trace( "Cache format is out of date.  Deleting all cache files." );
	EmptyDir( SpecialFiles::CACHE_DIR );
	EmptyDir( SpecialFiles::CACHE_DIR+"Banners/" );
	EmptyDir( SpecialFiles::CACHE_DIR+"Songs/" );
	EmptyDir( SpecialFiles::CACHE_DIR+"Courses/" );

	CacheIndex.Clear();
}

void SongCacheIndex::AddCacheIndex(const RString &path, unsigned hash)
{
	if( hash == 0 )
		++hash; /* no 0 hash values */
	CacheIndex.SetValue( "Cache", "CacheVersion", FILE_CACHE_VERSION );
	CacheIndex.SetValue( "Cache", MangleName(path), hash );
	CacheIndex.WriteFile( CACHE_INDEX );
}

unsigned SongCacheIndex::GetCacheHash( const RString &path ) const
{
	unsigned iDirHash = 0;
	if( !CacheIndex.GetValue( "Cache", MangleName(path), iDirHash ) )
		return 0;
	if( iDirHash == 0 )
		++iDirHash; /* no 0 hash values */
	return iDirHash;
}

RString SongCacheIndex::MangleName( const RString &Name )
{
	/* We store paths in an INI.  We can't store '='. */
	RString ret = Name;
	ret.Replace( "=", "");
	return ret;
}

/*
 * (c) 2002-2003 Glenn Maynard
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
