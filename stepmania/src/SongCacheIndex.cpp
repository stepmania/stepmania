#include "global.h"

#include <sys/stat.h>
#include <sys/types.h>

#include "SongCacheIndex.h"
#include "RageLog.h"
#include "RageUtil.h"
#include "song.h"

SongCacheIndex *SONGINDEX;

SongCacheIndex::SongCacheIndex()
{
	mkdir("Cache", 0755);
	CacheIndex.SetPath( "Cache/index.cache" );
	ReadCacheIndex();
}

SongCacheIndex::~SongCacheIndex()
{

}

void SongCacheIndex::ReadCacheIndex()
{
	CacheIndex.ReadFile();	// don't care if this fails

	int iCacheVersion;
	CacheIndex.GetValueI( "Cache", "CacheVersion", iCacheVersion );
	if( iCacheVersion == FILE_CACHE_VERSION )
		return; /* OK */

	LOG->Trace( "Cache format is out of date.  Deleting all cache files." );
	CStringArray asCacheFileNames;
	GetDirListing( "Cache/*", asCacheFileNames );
	for( unsigned i=0; i<asCacheFileNames.size(); i++ )
		remove( "Cache/" + asCacheFileNames[i] );
	CacheIndex.Reset();
}

void SongCacheIndex::AddCacheIndex(const CString &path, unsigned hash)
{
	CacheIndex.SetValueI( "Cache", "CacheVersion", FILE_CACHE_VERSION );
	CacheIndex.SetValueU( "Cache", path, hash );
	CacheIndex.WriteFile();
}

unsigned SongCacheIndex::GetCacheHash( const CString &path ) const
{
	unsigned iDirHash;
	CacheIndex.GetValueU( "Cache", path, iDirHash );
	return iDirHash;
}
