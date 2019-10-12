#ifndef IMAGE_CACHE_H
#define IMAGE_CACHE_H

#include "IniFile.h"

#include "RageTexture.h"

class LoadingWindow;
/** @brief Maintains a cache of reduced-quality images. */
class ImageCache
{
public:
	ImageCache();
	~ImageCache();
	void ReadFromDisk();
	void WriteToDisk();

	RageTextureID LoadCachedImage( RString sImageDir, RString sImagePath );
	void CacheImage( RString sImageDir, RString sImagePath );
	void LoadImage( RString sImageDir, RString sImagePath );

	void Demand( RString sImageDir );
	void Undemand( RString sImageDir );

	void OutputStats() const;

	bool delay_save_cache;

private:
	static RString GetImageCachePath( RString sImageDir, RString sImagePath );
	void UnloadAllImages();
	void CacheImageInternal( RString sImageDir, RString sImagePath );

	IniFile ImageData;
};

extern ImageCache *IMAGECACHE; // global and accessible from anywhere in our program

#endif

/**
 * @file
 * @author Glenn Maynard (c) 2003
 * @section LICENSE
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