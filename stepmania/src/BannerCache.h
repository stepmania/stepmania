#ifndef BANNER_CACHE_H
#define BANNER_CACHE_H

#include "IniFile.h"

#include "RageTexture.h"

class LoadingWindow;

class BannerCache
{
	IniFile BannerData;

	static CString GetBannerCachePath( CString BannerPath );
	void LoadAllBanners();
	void UnloadAllBanners();

public:
	BannerCache();
	~BannerCache();

	RageTextureID LoadCachedSongBanner( CString BannerPath );
	void CacheSongBanner( CString BannerPath );
};

extern BannerCache *BANNERCACHE; // global and accessable from anywhere in our program

#endif
