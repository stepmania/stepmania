#include "global.h"

#include "SongCacheIndex.h"
#include "RageLog.h"
#include "RageUtil.h"
#include "RageFileManager.h"
#include "song.h"

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
#define CACHE_DIR "Cache/"

SongCacheIndex *SONGINDEX;

SongCacheIndex::SongCacheIndex()
{
	CacheIndex.SetPath( CACHE_DIR "index.cache" );
	ReadCacheIndex();
}

SongCacheIndex::~SongCacheIndex()
{

}

static void EmptyDir( CString dir )
{
	ASSERT(dir[dir.size()-1] == '/');

	CStringArray asCacheFileNames;
	GetDirListing( dir, asCacheFileNames );
	for( unsigned i=0; i<asCacheFileNames.size(); i++ )
	{
		if( !IsADirectory(dir + asCacheFileNames[i]) )
			FILEMAN->Remove( dir + asCacheFileNames[i] );
	}
}

void SongCacheIndex::ReadCacheIndex()
{
	CacheIndex.ReadFile();	// don't care if this fails

	int iCacheVersion;
	CacheIndex.GetValue( "Cache", "CacheVersion", iCacheVersion );
	if( iCacheVersion == FILE_CACHE_VERSION )
		return; /* OK */

	LOG->Trace( "Cache format is out of date.  Deleting all cache files." );
	EmptyDir( CACHE_DIR );
	EmptyDir( CACHE_DIR "Banners/" );
	EmptyDir( CACHE_DIR "Songs/" );

	CacheIndex.Reset();
}

void SongCacheIndex::AddCacheIndex(const CString &path, unsigned hash)
{
	CacheIndex.SetValue( "Cache", "CacheVersion", FILE_CACHE_VERSION );
	CacheIndex.SetValue( "Cache", MangleName(path), hash );
	CacheIndex.WriteFile();
}

unsigned SongCacheIndex::GetCacheHash( const CString &path ) const
{
	unsigned iDirHash;
	CacheIndex.GetValue( "Cache", MangleName(path), iDirHash );
	return iDirHash;
}

CString SongCacheIndex::MangleName( const CString &Name )
{
	/* We store paths in an INI.  We can't store '='. */
	CString ret = Name;
	ret.Replace( "=", "");
	return ret;
}
