#include "stdafx.h"

#include <sys/stat.h>
#include <sys/types.h>

#include "SongCacheIndex.h"
#include "RageLog.h"
#include "RageUtil.h"
#include "Song.h"

SongCacheIndex *SONGINDEX;

SongCacheIndex::SongCacheIndex()
{
	mkdir("Cache", 0755);
	CacheIndex.SetPath( "Cache\\index.cache" );
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

void SongCacheIndex::AddCacheIndex(const CString &path, int hash)
{
	CacheIndex.SetValueI( "Cache", "CacheVersion", FILE_CACHE_VERSION );
	CacheIndex.SetValueI( "Cache", path, hash );
	CacheIndex.WriteFile();
}

int SongCacheIndex::GetCacheHash( const CString &path )
{
	int iDirHash;
	CacheIndex.GetValueI( "Cache", path, iDirHash );
	return iDirHash;
}
