#include "global.h"

#include <sys/stat.h>
#include <sys/types.h>

#include "SongCacheIndex.h"
#include "RageLog.h"
#include "RageUtil.h"
#include "song.h"
#include "arch/arch.h"

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
#ifdef _XBOX
	ASSERT(dir[dir.size()-1] == '\\');
#else
	ASSERT(dir[dir.size()-1] == '/');
#endif

	/* XXX RageFile */
	CStringArray asCacheFileNames;
	GetDirListing( dir, asCacheFileNames );
	for( unsigned i=0; i<asCacheFileNames.size(); i++ )
	{
		if( !IsADirectory(dir + asCacheFileNames[i]) )
			remove( dir + asCacheFileNames[i] );
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
