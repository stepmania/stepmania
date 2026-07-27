#include "global.h"

#include "ImageCache.h"
#include "RageDisplay.h"
#include "RageUtil.h"
#include "RageLog.h"
#include "RageSurface_Load.h"
#include "SongCacheIndex.h"
#include "Sprite.h"
#include "PrefsManager.h"
#include "RageDisplay.h"
#include "RageTexture.h"
#include "RageTextureManager.h"
#include "RageSurface.h"
#include "RageSurfaceUtils.h"
#include "RageSurfaceUtils_Palettize.h"
#include "RageSurfaceUtils_Dither.h"
#include "RageSurfaceUtils_Zoom.h"
#include "SpecialFiles.h"
#include "Banner.h"

#include <cmath>
#include <cstddef>
#include <cstdint>

static Preference<bool> g_bPalettedImageCache( "PalettedImageCache", false );

/* Neither a global or a file scope static can be used for this because
 * the order of initialization of nonlocal objects is unspecified. */
//const RString IMAGE_CACHE_INDEX = SpecialFiles::CACHE_DIR + "images.cache";
#define IMAGE_CACHE_INDEX (SpecialFiles::CACHE_DIR + "images.cache")

/* Call CacheImage to cache a image by path.  If the image is already
 * cached, it'll be recreated.  This is efficient if the image hasn't changed,
 * but we still only do this in TidyUpData for songs.
 *
 * Call LoadImage to load a cached image into main memory.  This will call
 * CacheImage only if needed.  This will not do a date/size check; call CacheImage
 * directly if you need that.
 *
 * Call LoadCachedImage to load a image into a texture and retrieve an ID
 * for it.  You can check if the image was actually preloaded by calling
 * TEXTUREMAN->IsTextureRegistered() on the ID; it might not be if the image cache
 * is missing or disabled.
 *
 * Note that each cache entries has two hashes.  The cache path is based soley
 * on the pathname; this way, loading the cache doesn't have to do a stat on every
 * image.  The full hash includes the file size and date, and is used only by
 * CacheImage to avoid doing extra work.
 */

ImageCache *IMAGECACHE; // global and accessible from anywhere in our program


static std::map<RString, RageSurface*> g_ImagePathToImage;
static int g_iDemandRefcount = 0;

RString ImageCache::GetImageCachePath( RString sImageDir ,RString sImagePath )
{
	return SongCacheIndex::GetCacheFilePath( sImageDir, sImagePath );
}

/* If in on-demand mode, load all cached images.  This must be fast, so
 * cache files will not be created if they don't exist; that should be done
 * by CacheImage or LoadImage on startup. */
void ImageCache::Demand( RString sImageDir )
{
	++g_iDemandRefcount;
	if( g_iDemandRefcount > 1 )
		return;

	if( PREFSMAN->m_ImageCache != IMGCACHE_LOW_RES_LOAD_ON_DEMAND )
		return;

	FOREACH_CONST_Child( &ImageData, p )
	{
		RString sImagePath = p->GetName();

		if( g_ImagePathToImage.find(sImagePath) != g_ImagePathToImage.end() )
			continue; /* already loaded */

		const RString sCachePath = GetImageCachePath(sImageDir,sImagePath);
		RageSurface *pImage = RageSurfaceUtils::LoadSurface( sCachePath );
		if( pImage == nullptr )
		{
			continue; /* doesn't exist */
		}

		g_ImagePathToImage[sImagePath] = pImage;
	}
}

/* Release images loaded on demand. */
void ImageCache::Undemand( RString sImageDir )
{
	--g_iDemandRefcount;
	if( g_iDemandRefcount != 0 )
		return;

	if( PREFSMAN->m_ImageCache != IMGCACHE_LOW_RES_LOAD_ON_DEMAND )
		return;

	UnloadAllImages();
}

/* If in a low-res image mode, load a low-res image into memory, creating
 * the cache file if necessary.  Unlike CacheImage(), the original file will
 * not be examined unless the cached image doesn't exist, so the image will
 * not be updated if the original file changes, for efficiency. */
void ImageCache::LoadImage( RString sImageDir, RString sImagePath )
{
	if( sImagePath == "" )
		return; // nothing to do
	if( PREFSMAN->m_ImageCache != IMGCACHE_LOW_RES_PRELOAD &&
	    PREFSMAN->m_ImageCache != IMGCACHE_LOW_RES_LOAD_ON_DEMAND )
		return;

	/* Load it. */
	const RString sCachePath = GetImageCachePath(sImageDir,sImagePath);

	for( int tries = 0; tries < 2; ++tries )
	{
		if( g_ImagePathToImage.find(sImagePath) != g_ImagePathToImage.end() )
			return; /* already loaded */

		CHECKPOINT_M( ssprintf( "ImageCache::LoadImage: %s", sCachePath.c_str() ) );
		RageSurface *pImage = RageSurfaceUtils::LoadSurface( sCachePath );
		if( pImage == nullptr )
		{
			if( tries == 0 )
			{
				/* The file doesn't exist.  It's possible that the image cache file is
				 * missing, so try to create it.  Don't do this first, for efficiency. */
				//LOG->Trace( "Cached image load of '%s' ('%s') failed, trying to cache ...", sImagePath.c_str(), sCachePath.c_str() );

				/* Skip the up-to-date check; it failed to load, so it can't be up
				 * to date. */
				CacheImageInternal( sImageDir, sImagePath );
				continue;
			}
			else
			{
				//LOG->Trace( "Cached image load of '%s' ('%s') failed", sImagePath.c_str(), sCachePath.c_str() );
				return;
			}
		}

		g_ImagePathToImage[sImagePath] = pImage;
	}
}

void ImageCache::OutputStats() const
{
	int iTotalSize = 0;
	for (auto const &it : g_ImagePathToImage)
	{
		const RageSurface *pImage = it.second;
		const int iSize = pImage->pitch * pImage->h;
		iTotalSize += iSize;
	}
	LOG->Info( "%i bytes of images loaded", iTotalSize );
}

void ImageCache::UnloadAllImages()
{
	for (auto &it: g_ImagePathToImage)
	{
		delete it.second;
	}

	g_ImagePathToImage.clear();
}

ImageCache::ImageCache()
	: delay_save_cache(false)
{
	ReadFromDisk();
}

ImageCache::~ImageCache()
{
	UnloadAllImages();
}

void ImageCache::ReadFromDisk()
{
	ImageData.ReadFile( IMAGE_CACHE_INDEX );	// don't care if this fails
}

struct ImageTexture: public RageTexture
{
	std::uintptr_t m_uTexHandle;
	std::uintptr_t GetTexHandle() const { return m_uTexHandle; };	// accessed by RageDisplay
	/* This is a reference to a pointer in g_ImagePathToImage. */
	RageSurface *&m_pImage;
	int m_iWidth, m_iHeight;

	ImageTexture( RageTextureID id, RageSurface *&pImage, int iWidth, int iHeight ):
		RageTexture(id), m_pImage(pImage), m_iWidth(iWidth), m_iHeight(iHeight)
	{
		Create();
	}

	~ImageTexture()
	{
		Destroy();
	}

	void Create()
	{
		ASSERT( m_pImage != nullptr );

		/* The image is preprocessed; do as little work as possible. */

		/* The source width is the width of the original file. */
		m_iSourceWidth = m_iWidth;
		m_iSourceHeight = m_iHeight;

		/* The image width (within the texture) is always the entire texture.
		 * Only resize if the max texture size requires it; since these images
		 * are already scaled down, this shouldn't happen often. */
		if( m_pImage->w > DISPLAY->GetMaxTextureSize() ||
			m_pImage->h > DISPLAY->GetMaxTextureSize() )
		{
			LOG->Warn( "Converted %s at runtime", GetID().filename.c_str() );
			int iWidth = std::min( m_pImage->w, DISPLAY->GetMaxTextureSize() );
			int iHeight = std::min( m_pImage->h, DISPLAY->GetMaxTextureSize() );
			RageSurfaceUtils::Zoom( m_pImage, iWidth, iHeight );
		}

		/* We did this when we cached it. */
		ASSERT( m_pImage->w == power_of_two(m_pImage->w) );
		ASSERT( m_pImage->h == power_of_two(m_pImage->h) );

		m_iTextureWidth = m_iImageWidth = m_pImage->w;
		m_iTextureHeight = m_iImageHeight = m_pImage->h;

		/* Find a supported texture format. If it happens to match the stored
		 * file, we won't have to do any conversion here, and that'll happen
		 * often with paletted images. */
		RagePixelFormat pf = m_pImage->format->BitsPerPixel == 8? RagePixelFormat_PAL: RagePixelFormat_RGB5A1;
		if( !DISPLAY->SupportsTextureFormat(pf) )
			pf = RagePixelFormat_RGBA4;

		ASSERT( DISPLAY->SupportsTextureFormat(pf) );

		ASSERT(m_pImage != nullptr);
		m_uTexHandle = DISPLAY->CreateTexture( pf, m_pImage, false );

		CreateFrameRects();
	}

	void Destroy()
	{
		if( m_uTexHandle )
			DISPLAY->DeleteTexture( m_uTexHandle );
		m_uTexHandle = 0;
	}

	void Reload()
	{
		Destroy();
		Create();
	}

	void Invalidate()
	{
		m_uTexHandle = 0; /* don't Destroy() */
	}
};

/* If a image is cached, get its ID for use. */
RageTextureID ImageCache::LoadCachedImage( RString sImageDir, RString sImagePath )
{
	RageTextureID ID( GetImageCachePath(sImageDir,sImagePath) );

	std::size_t Found = sImagePath.find("_blank");
	if( sImagePath == "" || Found!=RString::npos )
		return ID;

	//LOG->Trace( "ImageCache::LoadCachedImage(%s): %s", sImagePath.c_str(), ID.filename.c_str() );

	/* Hack: make sure Image::Load doesn't change our return value and end up
	 * reloading. */
	if(sImageDir == "Banner")
		ID = Sprite::SongBannerTexture(ID);

	/* It's not in a texture.  Do we have it loaded? */
	if( g_ImagePathToImage.find(sImagePath) == g_ImagePathToImage.end() )
	{
		/* Oops, the image is missing.  Warn and continue. */
		if(PREFSMAN->m_ImageCache != IMGCACHE_OFF)
		{
			LOG->Warn( "Image cache for '%s' wasn't loaded", sImagePath.c_str() );
		}
		return ID;
	}

	/* This is a reference to a pointer.  ImageTexture's ctor may change it
	 * when converting; this way, the conversion will end up in the map so we
	 * only have to convert once. */
	RageSurface *&pImage = g_ImagePathToImage[sImagePath];
	ASSERT( pImage != nullptr );

	int iSourceWidth = 0, iSourceHeight = 0;
	ImageData.GetValue( sImagePath, "Width", iSourceWidth );
	ImageData.GetValue( sImagePath, "Height", iSourceHeight );
	if( iSourceWidth == 0 || iSourceHeight == 0 )
	{
		LOG->UserLog( "Cache file", sImagePath, "couldn't be loaded." );
		return ID;
	}

	/* Is the image already in a texture? */
	if( TEXTUREMAN->IsTextureRegistered(ID) )
		return ID; /* It's all set. */

	//LOG->Trace( "Loading image texture %s; src %ix%i; image %ix%i",
	//	    ID.filename.c_str(), iSourceWidth, iSourceHeight, pImage->w, pImage->h );
	RageTexture *pTexture = new ImageTexture( ID, pImage, iSourceWidth, iSourceHeight );

	ID.Policy = RageTextureID::TEX_VOLATILE;
	TEXTUREMAN->RegisterTexture( ID, pTexture );
	TEXTUREMAN->UnloadTexture( pTexture );

	return ID;
}

static inline int closest( int num, int n1, int n2 )
{
	if( std::abs(num - n1) > std::abs(num - n2) )
		return n2;
	return n1;
}

/* Create or update the image cache file as necessary.  If in preload mode,
 * load the cache file, too.  (This is done at startup.) */
void ImageCache::CacheImage( RString sImageDir, RString sImagePath )
{
	if( PREFSMAN->m_ImageCache != IMGCACHE_LOW_RES_PRELOAD &&
	    PREFSMAN->m_ImageCache != IMGCACHE_LOW_RES_LOAD_ON_DEMAND )
		return;

	CHECKPOINT_M( sImagePath );
	if( !DoesFileExist(sImagePath) )
		return;

	const RString sCachePath = GetImageCachePath(sImageDir, sImagePath);

	/* Check the full file hash.  If it's the loaded and identical, don't recache. */
	if( DoesFileExist(sCachePath) )
	{
		bool bCacheUpToDate = PREFSMAN->m_bFastLoad;
		if( !bCacheUpToDate )
		{
			unsigned CurFullHash;
			const unsigned FullHash = GetHashForFile( sImagePath );
			if( ImageData.GetValue( sImagePath, "FullHash", CurFullHash ) && CurFullHash == FullHash )
				bCacheUpToDate = true;
		}

		if( bCacheUpToDate )
		{
			/* It's identical.  Just load it, if in preload. */
			if( PREFSMAN->m_ImageCache == IMGCACHE_LOW_RES_PRELOAD )
				LoadImage( sImageDir, sImagePath );

			return;
		}
	}

	/* The cache file doesn't exist, or is out of date.  Cache it.  This
	 * will also load the cache into memory if in PRELOAD. */
	CacheImageInternal( sImageDir, sImagePath );
}

void ImageCache::CacheImageInternal( RString sImageDir, RString sImagePath )
{
	RString sError;
	RageSurface *pImage = RageSurfaceUtils::LoadFile( sImagePath, sError );
	if( pImage == nullptr )
	{
		LOG->UserLog( "Cache file", sImagePath, "couldn't be loaded: %s", sError.c_str() );
		return;
	}

	const int iSourceWidth = pImage->w, iSourceHeight = pImage->h;

	int iWidth = pImage->w / 2, iHeight = pImage->h / 2;
//	int iWidth = pImage->w, iHeight = pImage->h;

	/* Round to the nearest power of two.  This simplifies the actual texture load. */
	iWidth = closest( iWidth, power_of_two(iWidth), power_of_two(iWidth) / 2 );
	iHeight = closest( iHeight, power_of_two(iHeight), power_of_two(iHeight) / 2 );

	/* Don't resize the image to less than 32 pixels in either dimension or the next
	 * power of two of the source (whichever is smaller); it's already very low res. */
	iWidth = std::max( iWidth, std::min(32, power_of_two(iSourceWidth)) );
	iHeight = std::max( iHeight, std::min(32, power_of_two(iSourceHeight)) );

	//RageSurfaceUtils::ApplyHotPinkColorKey( pImage );

	RageSurfaceUtils::Zoom( pImage, iWidth, iHeight );

	/*
	 * When paletted image cache is enabled, cached images are paletted.  Cached
	 * 32-bit images take 1/16 as much memory, 16-bit images take 1/8, and paletted
	 * images take 1/4.
	 *
	 * When paletted image cache is disabled, cached images are stored in 16-bit
	 * RGBA.  Cached 32-bit images take 1/8 as much memory, cached 16-bit images
	 * take 1/4, and cached paletted images take 1/2.
	 *
	 * Paletted cache is disabled by default because palettization takes time, causing
	 * the initial cache run to take longer.  Also, newer ATI hardware doesn't supported
	 * paletted textures, which would slow down runtime, because we have to depalettize
	 * on use.  They'd still have the same memory benefits, though, since we only load
	 * one cached image into a texture at once, and the speed hit may not matter on
	 * newer ATI cards.  RGBA is safer, though.
	 */
	if( g_bPalettedImageCache )
	{
		if( pImage->fmt.BytesPerPixel != 1 )
			RageSurfaceUtils::Palettize( pImage );
	}
	else
	{
		/* Dither to the final format.  We use A1RGB5, since that's usually supported
		 * natively by both OpenGL and D3D. */
		RageSurface *dst = CreateSurface( pImage->w, pImage->h, 16,
			0x7C00, 0x03E0, 0x001F, 0x8000 );

		/* OrderedDither is still faster than ErrorDiffusionDither, and
		 * these images are very small and only displayed briefly. */
		RageSurfaceUtils::OrderedDither( pImage, dst );
		delete pImage;
		pImage = dst;
	}

	const RString sCachePath = GetImageCachePath(sImageDir,sImagePath);
	RageSurfaceUtils::SaveSurface( pImage, sCachePath );

	/* If an old image is loaded, free it. */
	if( g_ImagePathToImage.find(sImagePath) != g_ImagePathToImage.end() )
	{
		RageSurface *oldimg = g_ImagePathToImage[sImagePath];
		delete oldimg;
		g_ImagePathToImage.erase(sImagePath);
	}

	if( PREFSMAN->m_ImageCache == IMGCACHE_LOW_RES_PRELOAD )
	{
		/* Keep it; we're just going to load it anyway. */
		g_ImagePathToImage[sImagePath] = pImage;
	}
	else
		delete pImage;

	/* Remember the original size. */
	ImageData.SetValue( sImagePath, "Path", sCachePath );
	ImageData.SetValue( sImagePath, "Width", iSourceWidth );
	ImageData.SetValue( sImagePath, "Height", iSourceHeight );
	ImageData.SetValue( sImagePath, "FullHash", GetHashForFile( sImagePath ) );
	if (!delay_save_cache)
		WriteToDisk();
}

void ImageCache::WriteToDisk()
{
	ImageData.WriteFile(IMAGE_CACHE_INDEX);
}


/*
 * (c) 2003 Glenn Maynard
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
