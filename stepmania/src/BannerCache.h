#ifndef BANNER_CACHE_H
#define BANNER_CACHE_H

#include "IniFile.h"

#include "RageTexture.h"

class LoadingWindow;

class BannerCache
{
	IniFile BannerData;

	static CString GetBannerCachePath( CString BannerPath );
	void UnloadAllBanners();

public:
	BannerCache();
	~BannerCache();

	RageTextureID LoadCachedBanner( CString BannerPath );

	void CacheBanner( CString BannerPath );
	void UncacheBanner( CString BannerPath );
	void LoadBanner( CString BannerPath );
	void OutputStats() const;
};

extern BannerCache *BANNERCACHE; // global and accessable from anywhere in our program

#endif
