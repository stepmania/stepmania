#include "global.h"

#include "BackgroundCache.h"
#include "Foreach.h"
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

#include "Background.h"

static Preference<bool> g_bPalettedBackgroundCache( "PalettedBackgroundCache", false );

/* Neither a global or a file scope static can be used for this because
 * the order of initialization of nonlocal objects is unspecified. */
#define BACKGROUND_CACHE_INDEX (SpecialFiles::CACHE_DIR + "backgrounds.cache")

/* Call CacheBackground to cache a background by path.  If the background is already
 * cached, it'll be recreated.  This is efficient if the background hasn't changed,
 * but we still only do this in TidyUpData for songs.
 *
 * Call LoadBackground to load a cached background into main memory.  This will call
 * CacheBackground only if needed.  This will not do a date/size check; call CacheBackground
 * directly if you need that.
 *
 * Call LoadCachedBackground to load a background into a texture and retrieve an ID
 * for it.  You can check if the background was actually preloaded by calling
 * TEXTUREMAN->IsTextureRegistered() on the ID; it might not be if the background cache
 * is missing or disabled.
 *
 * Note that each cache entries has two hashes.  The cache path is based soley
 * on the pathname; this way, loading the cache doesn't have to do a stat on every
 * background.  The full hash includes the file size and date, and is used only by
 * CacheBackground to avoid doing extra work.
 */

BackgroundCache *BACKGROUNDCACHE;


static map<RString,RageSurface *> g_BackgroundPathToImage;
static int g_iDemandRefcount = 0;

RString BackgroundCache::GetBackgroundCachePath( RString sBackgroundPath )
{
	return SongCacheIndex::GetCacheFilePath( "Backgrounds", sBackgroundPath );
}

/* If in on-demand mode, load all cached backgrounds.  This must be fast, so
 * cache files will not be created if they don't exist; that should be done
 * by CacheBackground or LoadBackground on startup. */
void BackgroundCache::Demand()
{
	++g_iDemandRefcount;
	if( g_iDemandRefcount > 1 )
		return;
	
	if( PREFSMAN->m_BackgroundCache != BNCACHE_LOW_RES_LOAD_ON_DEMAND )
		return;

	FOREACH_CONST_Child( &BackgroundData, p )
	{
		RString sBackgroundPath = p->GetName();

		if( g_BackgroundPathToImage.find(sBackgroundPath) != g_BackgroundPathToImage.end() )
			continue; /* already loaded */

		const RString sCachePath = GetBackgroundCachePath(sBackgroundPath);
		RageSurface *pImage = RageSurfaceUtils::LoadSurface( sCachePath );
		if( pImage == NULL )
		{
			continue; /* doesn't exist */
		}

		g_BackgroundPathToImage[sBackgroundPath] = pImage;
	}
}

/* Release backgrounds loaded on demand. */
void BackgroundCache::Undemand()
{
	--g_iDemandRefcount;
	if( g_iDemandRefcount != 0 )
		return;
	
	if( PREFSMAN->m_BackgroundCache != BNCACHE_LOW_RES_LOAD_ON_DEMAND )
		return;

	UnloadAllBackgrounds();
}

/* If in a low-res background mode, load a low-res background into memory, creating
 * the cache file if necessary.  Unlike CacheBackground(), the original file will
 * not be examined unless the cached background doesn't exist, so the background will
 * not be updated if the original file changes, for efficiency. */
void BackgroundCache::LoadBackground( RString sBackgroundPath )
{
	if( sBackgroundPath == "" )
		return; // nothing to do
	if( PREFSMAN->m_BackgroundCache != BNCACHE_LOW_RES_PRELOAD &&
	    PREFSMAN->m_BackgroundCache != BNCACHE_LOW_RES_LOAD_ON_DEMAND )
		return;

	/* Load it. */
	const RString sCachePath = GetBackgroundCachePath(sBackgroundPath);

	for( int tries = 0; tries < 2; ++tries )
	{
		if( g_BackgroundPathToImage.find(sBackgroundPath) != g_BackgroundPathToImage.end() )
			return; /* already loaded */

		CHECKPOINT_M( ssprintf( "BackgroundCache::LoadBackground: %s", sCachePath.c_str() ) );
		RageSurface *pImage = RageSurfaceUtils::LoadSurface( sCachePath );
		if( pImage == NULL )
		{
			if( tries == 0 )
			{
				/* The file doesn't exist.  It's possible that the background cache file is
				 * missing, so try to create it.  Don't do this first, for efficiency. */
				LOG->Trace( "Cached background load of '%s' ('%s') failed, trying to cache ...", sBackgroundPath.c_str(), sCachePath.c_str() );

				/* Skip the up-to-date check; it failed to load, so it can't be up
				 * to date. */
				CacheBackgroundInternal( sBackgroundPath );
				continue;
			}
			else
			{
				LOG->Trace( "Cached background load of '%s' ('%s') failed", sBackgroundPath.c_str(), sCachePath.c_str() );
				return;
			}
		}

		g_BackgroundPathToImage[sBackgroundPath] = pImage;
	}
}

void BackgroundCache::OutputStats() const
{
	int iTotalSize = 0;
	FOREACHM_CONST( RString, RageSurface *, g_BackgroundPathToImage, it )
	{
		const RageSurface *pImage = it->second;
		const int iSize = pImage->pitch * pImage->h;
		iTotalSize += iSize;
	}
	LOG->Info( "%i bytes of backgrounds loaded", iTotalSize );
}

void BackgroundCache::UnloadAllBackgrounds()
{
	FOREACHM( RString, RageSurface *, g_BackgroundPathToImage, it )
		delete it->second;

	g_BackgroundPathToImage.clear();
}

BackgroundCache::BackgroundCache()
{
	ReadFromDisk();
}

BackgroundCache::~BackgroundCache()
{
	UnloadAllBackgrounds();
}

void BackgroundCache::ReadFromDisk()
{
	BackgroundData.ReadFile( BACKGROUND_CACHE_INDEX );	// don't care if this fails
}
// Archer calls this "BackgroundSSMTexture". -aj
struct BackgroundTexture: public RageTexture
{
	unsigned m_uTexHandle;
	unsigned GetTexHandle() const { return m_uTexHandle; };	// accessed by RageDisplay
	/* This is a reference to a pointer in g_BackgroundPathToImage. */
	RageSurface *&m_pImage;
	int m_iWidth, m_iHeight;

	BackgroundTexture( RageTextureID id, RageSurface *&pImage, int iWidth, int iHeight ):
		RageTexture(id), m_pImage(pImage), m_iWidth(iWidth), m_iHeight(iHeight)
	{
		Create();
	}

	~BackgroundTexture()
	{ 
		Destroy();
	}

	void Create()
	{
		ASSERT( m_pImage );

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
			int iWidth = min( m_pImage->w, DISPLAY->GetMaxTextureSize() );
			int iHeight = min( m_pImage->h, DISPLAY->GetMaxTextureSize() );
			RageSurfaceUtils::Zoom( m_pImage, iWidth, iHeight );
		}

		/* We did this when we cached it. */
		ASSERT( m_pImage->w == power_of_two(m_pImage->w) );
		ASSERT( m_pImage->h == power_of_two(m_pImage->h) );

		m_iTextureWidth = m_iImageWidth = m_pImage->w;
		m_iTextureHeight = m_iImageHeight = m_pImage->h;

		/* Find a supported texture format.  If it happens to match the stored
		 * file, we won't have to do any conversion here, and that'll happen often
		 * with paletted images. */
#if !defined(XBOX)
		PixelFormat pf = m_pImage->format->BitsPerPixel == 8? PixelFormat_PAL: PixelFormat_RGB5A1;
		if( !DISPLAY->SupportsTextureFormat(pf) )
			pf = PixelFormat_RGBA4;
#else
		// xbox display currently supports only rgba8
		PixelFormat pf = PixelFormat_RGBA8;
#endif
		ASSERT( DISPLAY->SupportsTextureFormat(pf) );

		ASSERT(m_pImage);
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

/* If a background is cached, get its ID for use. */
RageTextureID BackgroundCache::LoadCachedBackground( RString sBackgroundPath )
{
	RageTextureID ID( GetBackgroundCachePath(sBackgroundPath) );

	if( sBackgroundPath == "" )
		return ID;

	LOG->Trace( "BackgroundCache::LoadCachedBackground(%s): %s", sBackgroundPath.c_str(), ID.filename.c_str() );

	/* Hack: make sure Background::Load doesn't change our return value and end up
	 * reloading. */
	ID = Sprite::SongBGTexture(ID);

	/* It's not in a texture.  Do we have it loaded? */
	if( g_BackgroundPathToImage.find(sBackgroundPath) == g_BackgroundPathToImage.end() )
	{
		/* Oops, the image is missing.  Warn and continue. */
		LOG->Warn( "Background cache for '%s' wasn't loaded", sBackgroundPath.c_str() );
		return ID;
	}

	/* This is a reference to a pointer.  BackgroundTexture's ctor may change it
	 * when converting; this way, the conversion will end up in the map so we
	 * only have to convert once. */
	RageSurface *&pImage = g_BackgroundPathToImage[sBackgroundPath];
	ASSERT( pImage );

	int iSourceWidth = 0, iSourceHeight = 0;
	BackgroundData.GetValue( sBackgroundPath, "Width", iSourceWidth );
	BackgroundData.GetValue( sBackgroundPath, "Height", iSourceHeight );
	if( iSourceWidth == 0 || iSourceHeight == 0 )
	{
		LOG->UserLog( "Cache file", sBackgroundPath, "couldn't be loaded." );
		return ID;
	}

	/* Is the background already in a texture? */
	if( TEXTUREMAN->IsTextureRegistered(ID) )
		return ID; /* It's all set. */

	LOG->Trace( "Loading background texture %s; src %ix%i; image %ix%i",
		    ID.filename.c_str(), iSourceWidth, iSourceHeight, pImage->w, pImage->h );
	RageTexture *pTexture = new BackgroundTexture( ID, pImage, iSourceWidth, iSourceHeight );

	ID.Policy = RageTextureID::TEX_VOLATILE;
	TEXTUREMAN->RegisterTexture( ID, pTexture );
	TEXTUREMAN->UnloadTexture( pTexture );

	return ID;
}

static inline int closest( int num, int n1, int n2 )
{
	if( abs(num - n1) > abs(num - n2) )
		return n2;
	return n1;
}

/* Create or update the background cache file as necessary.  If in preload mode,
 * load the cache file, too.  (This is done at startup.) */
void BackgroundCache::CacheBackground( RString sBackgroundPath )
{
	if( PREFSMAN->m_BackgroundCache != BNCACHE_LOW_RES_PRELOAD &&
	    PREFSMAN->m_BackgroundCache != BNCACHE_LOW_RES_LOAD_ON_DEMAND )
		return;

	CHECKPOINT_M( sBackgroundPath );
	if( !DoesFileExist(sBackgroundPath) )
		return;

	const RString sCachePath = GetBackgroundCachePath(sBackgroundPath);

	/* Check the full file hash.  If it's the loaded and identical, don't recache. */
	if( DoesFileExist(sCachePath) )
	{
		bool bCacheUpToDate = PREFSMAN->m_bFastLoad;
		if( !bCacheUpToDate )
		{
			unsigned CurFullHash;
			const unsigned FullHash = GetHashForFile( sBackgroundPath );
			if( BackgroundData.GetValue( sBackgroundPath, "FullHash", CurFullHash ) && CurFullHash == FullHash )
				bCacheUpToDate = true;
		}

		if( bCacheUpToDate )
		{
			/* It's identical.  Just load it, if in preload. */
			if( PREFSMAN->m_BackgroundCache == BNCACHE_LOW_RES_PRELOAD )
				LoadBackground( sBackgroundPath );

			return;
		}
	}

	/* The cache file doesn't exist, or is out of date.  Cache it.  This
	 * will also load the cache into memory if in PRELOAD. */
	CacheBackgroundInternal( sBackgroundPath );
}

void BackgroundCache::CacheBackgroundInternal( RString sBackgroundPath )
{
	RString sError;
	RageSurface *pImage = RageSurfaceUtils::LoadFile( sBackgroundPath, sError );
	if( pImage == NULL )
	{
		LOG->UserLog( "Cache file", sBackgroundPath, "couldn't be loaded: %s", sError.c_str() );
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
	iWidth = max( iWidth, min(32, power_of_two(iSourceWidth)) );
	iHeight = max( iHeight, min(32, power_of_two(iSourceHeight)) );

	RageSurfaceUtils::ApplyHotPinkColorKey( pImage );

	RageSurfaceUtils::Zoom( pImage, iWidth, iHeight );

	/*
	 * When paletted background cache is enabled, cached backgrounds are paletted.  Cached
	 * 32-bit backgrounds take 1/16 as much memory, 16-bit backgrounds take 1/8, and paletted
	 * backgrounds take 1/4.
	 *
	 * When paletted background cache is disabled, cached backgrounds are stored in 16-bit
	 * RGBA.  Cached 32-bit backgrounds take 1/8 as much memory, cached 16-bit backgrounds
	 * take 1/4, and cached paletted backgrounds take 1/2.
	 *
	 * Paletted cache is disabled by default because palettization takes time, causing
	 * the initial cache run to take longer.  Also, newer ATI hardware doesn't supported
	 * paletted textures, which would slow down runtime, because we have to depalettize
	 * on use.  They'd still have the same memory benefits, though, since we only load
	 * one cached background into a texture at once, and the speed hit may not matter on
	 * newer ATI cards.  RGBA is safer, though.
	 */
	if( g_bPalettedBackgroundCache )
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

	const RString sCachePath = GetBackgroundCachePath(sBackgroundPath);
	RageSurfaceUtils::SaveSurface( pImage, sCachePath );

	/* If an old image is loaded, free it. */
	if( g_BackgroundPathToImage.find(sBackgroundPath) != g_BackgroundPathToImage.end() )
	{
		RageSurface *oldimg = g_BackgroundPathToImage[sBackgroundPath];
		delete oldimg;
		g_BackgroundPathToImage.erase(sBackgroundPath);
	}

	if( PREFSMAN->m_BackgroundCache == BNCACHE_LOW_RES_PRELOAD )
	{
		/* Keep it; we're just going to load it anyway. */
		g_BackgroundPathToImage[sBackgroundPath] = pImage;
	}
	else
		delete pImage;

	/* Remember the original size. */
	BackgroundData.SetValue( sBackgroundPath, "Path", sCachePath );
	BackgroundData.SetValue( sBackgroundPath, "Width", iSourceWidth );
	BackgroundData.SetValue( sBackgroundPath, "Height", iSourceHeight );
	BackgroundData.SetValue( sBackgroundPath, "FullHash", GetHashForFile( sBackgroundPath ) );
	BackgroundData.WriteFile( BACKGROUND_CACHE_INDEX );
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
