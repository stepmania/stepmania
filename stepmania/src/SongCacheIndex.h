#ifndef SONG_CACHE_INDEX_H
#define SONG_CACHE_INDEX_H

#include "IniFile.h"

class SongCacheIndex {
	IniFile CacheIndex;
	static CString MangleName( const CString &Name );

public:
	SongCacheIndex();
	~SongCacheIndex();

	void ReadCacheIndex();
	void AddCacheIndex( const CString &path, unsigned hash );
	unsigned GetCacheHash( const CString &path ) const;
};

extern SongCacheIndex *SONGINDEX;	// global and accessable from anywhere in our program

#endif
