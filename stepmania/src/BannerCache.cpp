/*
 * BannerCache
 *
 * Maintains a cache of reduced-quality banners.
 */

#include "global.h"

#include "RageDisplay.h"
#include "RageUtil.h"
#include "RageLog.h"
#include "BannerCache.h"
#include "PrefsManager.h"
#include "SDL_utils.h"
#include "SDL_dither.h"
#include "SDL_image.h"
#include "SDL_rotozoom.h"

#include "Banner.h"

/* Call CacheBanner to cache a banner by path.  If the banner is already
 * cached, it'll be recreated.  This is efficient if the banner hasn't changed,
 * but we still only do this in TidyUpData for songs.
 *
 * Call LoadBanner to load a cached banner into main memory.  This will call
 * CacheBanner only if needed.  This will not do a date/size check; call CacheBanner
 * directly if you need that.
 *
 * Call LoadCachedBanner to load a banner into a texture and retrieve an ID
 * for it.  You can check if the banner was actually preloaded by calling
 * TEXTUREMAN->IsTextureRegistered() on the ID; it might not be if the banner cache
 * is missing or disabled.
 *
 * Note that each cache entries has two hashes.  The cache path is based soley
 * on the pathname; this way, loading the cache doesn't have to do a stat on every
 * banner.  The full hash includes the file size and date, and is used only by
 * CacheBanner to avoid doing extra work.
 */

BannerCache *BANNERCACHE;


static map<CString,SDL_Surface *> m_BannerPathToImage;

CString BannerCache::GetBannerCachePath( CString BannerPath )
{
	/* Use GetHashForString, not ForFile, since we don't want to spend time
	 * checking the file size and date. */
	return ssprintf( "Cache/Banners/%u", GetHashForString(BannerPath) );
}

void BannerCache::LoadBanner( CString BannerPath )
{
	if( !PREFSMAN->m_bBannerCache || BannerPath == "" )
		return;

	/* Load it. */
	const CString CachePath = GetBannerCachePath(BannerPath);

	for( int tries = 0; tries < 2; ++tries )
	{
		if( m_BannerPathToImage.find(BannerPath) != m_BannerPathToImage.end() )
			return; /* already loaded */

		Checkpoint( ssprintf( "BannerCache::LoadBanner: %s", CachePath.c_str() ) );
		SDL_Surface *img = mySDL_LoadSurface( CachePath );
		if( img == NULL )
		{
			if(tries == 0)
			{
				/* The file doesn't exist.  It's possible that the banner cache file is
				 * missing, so try to create it.  Don't do this first, for efficiency. */
				LOG->Trace( "Cached banner load of '%s' ('%s') failed, trying to cache ...", BannerPath.c_str(), CachePath.c_str() );
				/* Skip the up-to-date check; it failed to load, so it can't be up
				 * to date. */
				CacheBannerInternal( BannerPath );
				continue;
			}
			else
			{
				LOG->Trace( "Cached banner load of '%s' ('%s') failed", BannerPath.c_str(), CachePath.c_str() );
				return;
			}
		}

		m_BannerPathToImage[BannerPath] = img;
	}
}

void BannerCache::OutputStats() const
{
	map<CString,SDL_Surface *>::const_iterator ban;
	int total_size = 0;
	for( ban = m_BannerPathToImage.begin(); ban != m_BannerPathToImage.end(); ++ban )
	{
		SDL_Surface * const &img = ban->second;
		const int size = img->pitch * img->h;
		total_size += size;
	}
	LOG->Info( "%i bytes of banners loaded", total_size );
}

void BannerCache::UnloadAllBanners()
{
	map<CString,SDL_Surface *>::iterator it;
	for( it = m_BannerPathToImage.begin(); it != m_BannerPathToImage.end(); ++it )
		SDL_FreeSurface(it->second);

	m_BannerPathToImage.clear();
}

BannerCache::BannerCache()
{
	CreateDirectories("Cache/Banners/");
	BannerData.SetPath( "Cache/banners.cache" );
	BannerData.ReadFile();	// don't care if this fails
}

BannerCache::~BannerCache()
{
	UnloadAllBanners();
}

#include "SDL_utils.h"
#include "RageDisplay.h"
#include "RageTexture.h"
#include "RageTextureManager.h"

struct BannerTexture: public RageTexture
{
	unsigned m_uTexHandle;
	unsigned GetTexHandle() { return m_uTexHandle; };	// accessed by RageDisplay
	/* This is a reference to a pointer in m_BannerPathToImage. */
	SDL_Surface *&img;
	int width, height;

	BannerTexture( RageTextureID name, SDL_Surface *&img_, int width_, int height_ ):
		RageTexture(name), img(img_), width(width_), height(height_)
	{
		Create();
	}

	~BannerTexture()
	{ 
		Destroy();
	}
	
	void Create()
	{
		/* The image is preprocessed; do as little work as possible. */

		/* The source width is the width of the original file. */
		m_iSourceWidth = width;
		m_iSourceHeight = height;

		/* We did this when we cached it. */
		ASSERT( img->w == power_of_two(img->w) );
		ASSERT( img->h == power_of_two(img->h) );

		/* The image width (within the texture) is always the entire texture. 
		 * Only resize if the max texture size requires it; since these images
		 * are already scaled down, this shouldn't happen often. */
		if( img->w > DISPLAY->GetMaxTextureSize() || 
			img->h > DISPLAY->GetMaxTextureSize() )
		{
			LOG->Warn("Converted %s at runtime", GetID().filename.c_str() );
			ConvertSDLSurface(img, img->w, img->h, 32, 0x000000FF, 0x0000FF00, 0x00FF0000, 0xFF000000);
			zoomSurface(img, width, height);
		}

		m_iTextureWidth = m_iImageWidth = img->w;
		m_iTextureHeight = m_iImageHeight = img->h;

		/* Find a supported texture format.  If it happens to match the stored
		 * file, we won't have to do any conversion here, and that'll happen often
		 * with paletted images. */
		PixelFormat pf = img->format->BitsPerPixel == 8? FMT_PAL: FMT_RGB5A1;
		if( !DISPLAY->SupportsTextureFormat(pf) )
			pf = FMT_RGBA4;
		ASSERT( DISPLAY->SupportsTextureFormat(pf) );

		const PixelFormatDesc *pfd = DISPLAY->GetPixelFormatDesc(pf);
		ASSERT(pfd);
		ASSERT(img);
		ConvertSDLSurface(img, m_iTextureWidth, m_iTextureHeight,
				pfd->bpp, pfd->masks[0], pfd->masks[1], pfd->masks[2], pfd->masks[3]);
	
		m_uTexHandle = DISPLAY->CreateTexture( FMT_RGB5A1, img );

		CreateFrameRects();
	}

	void Destroy()
	{
		DISPLAY->DeleteTexture( m_uTexHandle );
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

RageTextureID BannerCache::LoadCachedBanner( CString BannerPath )
{
	RageTextureID ID( GetBannerCachePath(BannerPath) );

	if( BannerPath == "" )
		return ID;

	LOG->Trace("BannerCache::LoadCachedBanner(%s): %s", BannerPath.c_str(), ID.filename.c_str() );

	/* Hack: make sure Banner::Load doesn't change our return value and end up
	 * reloading. */
	ID = Banner::BannerTex(ID);

	/* It's not in a texture.  Do we have it loaded? */
	if( m_BannerPathToImage.find(BannerPath) == m_BannerPathToImage.end() )
	{
		/* Oops, the image is missing.  Warn and continue. */
		LOG->Warn( "Banner cache for '%s' wasn't loaded", BannerPath.c_str() );
		return ID;
	}

	/* This is a reference to a pointer.  BannerTexture's ctor may change it
	 * when converting; this way, the conversion will end up in the map so we
	 * only have to convert once. */
	SDL_Surface *&img = m_BannerPathToImage[BannerPath];
	ASSERT( img );

	int src_width = 0, src_height = 0;
	bool WasRotatedBanner = false;
	BannerData.GetValueI( BannerPath, "Width", src_width );
	BannerData.GetValueI( BannerPath, "Height", src_height );
	BannerData.GetValueB( BannerPath, "Rotated", WasRotatedBanner );
	if(src_width == 0 || src_height == 0)
	{
		LOG->Warn("Couldn't load '%s'", BannerPath.c_str() );
		return ID;
	}

	if( WasRotatedBanner )
	{
		/* We need to tell CroppedSprite that this was originally a rotated
		 * sprite. */
		ID.filename += "(was rotated)";
	}

	/* Is the banner already in a texture? */
	if( TEXTUREMAN->IsTextureRegistered(ID) )
		return ID; /* It's all set. */

	RageTexture *pTexture = new BannerTexture( ID, img, src_width, src_height );

	TEXTUREMAN->RegisterTexture( ID, pTexture );

	return ID;
}

static inline int closest( int num, int n1, int n2 )
{
	if( abs(num - n1) > abs(num - n2) )
		return n2;
	return n1;
}

/* We write the cache even if we won't use it, so we don't have to recache everything
 * if the memory or settings change. */
void BannerCache::CacheBanner( CString BannerPath )
{
	if( !DoesFileExist(BannerPath) )
		return;

	const CString CachePath = GetBannerCachePath(BannerPath);

	/* Check the full file hash.  If it's the loaded and identical, don't recache. */
	if( DoesFileExist(CachePath) )
	{
		unsigned CurFullHash;
		const unsigned FullHash = GetHashForFile( BannerPath );
		if( BannerData.GetValueU( BannerPath, "FullHash", CurFullHash ) &&
			CurFullHash == FullHash )
		{
			/* It's identical.  Just load it. */
			LoadBanner( BannerPath );
			return;
		}
	}

	CacheBannerInternal( BannerPath );
}

void BannerCache::CacheBannerInternal( CString BannerPath )
{
	SDL_Surface *img = IMG_Load( BannerPath );
	if(img == NULL)
		RageException::Throw( "BannerCache::CacheBanner: Couldn't load %s: %s", BannerPath.c_str(), SDL_GetError() );

	bool WasRotatedBanner = false;

	if( img->w == img->h )
	{
		/* Ack.  It's a diagonal banner.  Problem: if we resize a diagonal banner, we
		 * get ugly checker patterns.  We need to un-rotate it.
		 *
		 * If we spin the banner by hand, we need to do a linear filter, or the
		 * fade to the full resolution banner is misaligned, which looks strange.
		 *
		 * To do a linear filter, we need to lose the palette.  Oh well.
		 *
		 * This also makes the banner take less memory, though that could also be
		 * done by RLEing the surface.
		 */
		ConvertSDLSurface(img, img->w, img->h, 32, 0x000000FF, 0x0000FF00, 0x00FF0000, 0xFF000000);
		
		SDL_Surface *dst = SDL_CreateRGBSurface(
            SDL_SWSURFACE, 256, 64, img->format->BitsPerPixel, 
			img->format->Rmask, img->format->Gmask, img->format->Bmask, img->format->Amask );

		if( img->format->BitsPerPixel == 8 ) 
		{
			ASSERT( img->format->palette );
			mySDL_SetPalette(dst, img->format->palette->colors, 0, 256);
		}
		if( img->flags & SDL_SRCCOLORKEY )
			SDL_SetColorKey( dst, SDL_SRCCOLORKEY, img->format->colorkey);

		const float fCustomImageCoords[8] = {
			0.02f,	0.78f,	// top left
			0.22f,	0.98f,	// bottom left
			0.98f,	0.22f,	// bottom right
			0.78f,	0.02f,	// top right
		};

		mySDL_BlitTransform( img, dst, fCustomImageCoords );

//		SDL_SaveBMP( dst, BannerPath + "-test.bmp" );
		SDL_FreeSurface( img );
		img = dst;

		WasRotatedBanner = true;
	}



	const int src_width = img->w, src_height = img->h;

	int width = img->w / 2, height = img->h / 2;
//	int width = img->w, height = img->h;

	/* Round to the nearest power of two.  This simplifies the actual texture load. */
	width = closest(width, power_of_two(width), power_of_two(width) / 2);
	height = closest(height, power_of_two(height), power_of_two(height) / 2);

	/* Don't resize the image to less than 32 pixels in either dimension or the next
	 * power of two of the source (whichever is smaller); it's already very low res. */
	width = max( width, min(32, power_of_two(src_width)) );
	height = max( height, min(32, power_of_two(src_height)) );

	{
		const int color = mySDL_MapRGBExact(img->format, 0xFF, 0, 0xFF);
		if( color != -1 )
			SDL_SetColorKey( img, SDL_SRCCOLORKEY, color );
	}

	{
		ConvertSDLSurface(img, img->w, img->h, 32, 0x000000FF, 0x0000FF00, 0x00FF0000, 0xFF000000);
		zoomSurface(img, width, height);

		/* XXX: Query DISPLAY to find out if we should use FMT_RGB5A1 or FMT_RGBA4. 
		 * But then we'd have to recache if the display changes, and this gets called
		 * during the song cache phase, before we even have a display ... */
/*		const PixelFormatDesc *pfd = DISPLAY->GetPixelFormatDesc(FMT_RGB5A1);
		SDL_Surface *dst = SDL_CreateRGBSurfaceSane(SDL_SWSURFACE, img->w, img->h,
					pfd->bpp, pfd->masks[0], pfd->masks[1], pfd->masks[2], pfd->masks[3]); */

		/* Dither: */
		SDL_Surface *dst = SDL_CreateRGBSurfaceSane(SDL_SWSURFACE, img->w, img->h,
					16, 0xF800, 0x07C0, 0x003E, 0x0001 );

		SM_SDL_ErrorDiffusionDither(img, dst);
		SDL_FreeSurface( img );
		img = dst;
	}

	const CString CachePath = GetBannerCachePath(BannerPath);
	mySDL_SaveSurface( img, CachePath );

	if( PREFSMAN->m_bBannerCache )
	{
		/* If an old image is loaded, free it. */
		if( m_BannerPathToImage.find(BannerPath) != m_BannerPathToImage.end() )
		{
			SDL_Surface *img = m_BannerPathToImage[BannerPath];
			SDL_FreeSurface( img );
			m_BannerPathToImage.erase(BannerPath);
		}

		/* Keep it; we're just going to load it anyway. */
		m_BannerPathToImage[BannerPath] = img;
	}
	else
		SDL_FreeSurface(img);

	/* Remember the original size. */
	BannerData.SetValue ( BannerPath, "Path", CachePath );
	BannerData.SetValueI( BannerPath, "Width", src_width );
	BannerData.SetValueI( BannerPath, "Height", src_height );
	BannerData.SetValueU( BannerPath, "FullHash", GetHashForFile( BannerPath ) );
	/* Remember this, so we can hint CroppedSprite. */
	BannerData.SetValueB( BannerPath, "Rotated", WasRotatedBanner );
	BannerData.WriteFile();
}

/*
   Copyright (c) 2003 by the person(s) listed below.  All rights reserved.
	Glenn Maynard
*/
