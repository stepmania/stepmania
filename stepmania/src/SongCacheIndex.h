#ifndef SONG_CACHE_INDEX_H
#define SONG_CACHE_INDEX_H

#include "IniFile.h"

class SongCacheIndex {
	IniFile CacheIndex;
public:
	SongCacheIndex();
	~SongCacheIndex();

	void ReadCacheIndex();
	void AddCacheIndex( const CString &path, int hash );
	int GetCacheHash( const CString &path );
};

extern SongCacheIndex *SONGINDEX;	// global and accessable from anywhere in our program

#endif
