#include "global.h"

#include "SDL.h"
#include "SDL_utils.h"
#include "SDL_endian.h"
#include "SDL_rotozoom.h"	// for setting icon
#include "RageSurface_Load.h"
#include "RageFile.h"
#include "RageLog.h"

/* Pull in all of our SDL libraries here. */
#ifdef _XBOX
	#ifdef DEBUG
	#pragma comment(lib, "SDLx-0.02/SDLxd.lib")
	#else
	#pragma comment(lib, "SDLx-0.02/SDLx.lib")
	#endif
#elif defined _WINDOWS
	#ifdef DEBUG
	#pragma comment(lib, "SDL-1.2.6/lib/SDLd.lib")
	#else
	#pragma comment(lib, "SDL-1.2.6/lib/SDL.lib")
	#endif
#endif


Uint32 mySDL_Swap24(Uint32 x)
{
	return SDL_Swap32(x) >> 8; // xx223344 -> 443322xx -> 00443322
}

/* These conditionals in the inner loop are slow.  Templates? */
inline Uint32 decodepixel(const Uint8 *p, int bpp)
{
    switch(bpp)
	{
    case 1: return *p;
    case 2: return *(Uint16 *)p;
	case 3:
        if(SDL_BYTEORDER == SDL_BIG_ENDIAN)
            return p[0] << 16 | p[1] << 8 | p[2];
        else
            return p[0] | p[1] << 8 | p[2] << 16;

    case 4: return *(Uint32 *)p;
    default: return 0;       /* shouldn't happen, but avoids warnings */
    }
}

void encodepixel(Uint8 *p, int bpp, Uint32 pixel)
{
    switch(bpp)
	{
    case 1: *p = Uint8(pixel); break;
    case 2: *(Uint16 *)p = Uint16(pixel); break;
    case 3:
        if(SDL_BYTEORDER == SDL_BIG_ENDIAN) {
            p[0] = Uint8((pixel >> 16) & 0xff);
            p[1] = Uint8((pixel >> 8) & 0xff);
            p[2] = Uint8(pixel & 0xff);
        } else {
            p[0] = Uint8(pixel & 0xff);
            p[1] = Uint8((pixel >> 8) & 0xff);
            p[2] = Uint8((pixel >> 16) & 0xff);
        }
        break;
    case 4: *(Uint32 *)p = pixel; break;
    }
}

/* Get and set colors without scaling them to 0..255.  Get them into
 * an array, which is much easier to work with.  We need the surface
 * to get at flags, or we won't know if colorkey is valid.  (Why isn't
 * format self-contained?) Use mySDL_GetBitsPerChannel() to get the
 * number of bits per channel. */
void mySDL_GetRawRGBAV(Uint32 pixel, const SDL_Surface *src, Uint8 *v)
{
	const SDL_PixelFormat *fmt = src->format;
	if( src->format->BytesPerPixel == 1 )
	{
		v[0] = fmt->palette->colors[pixel].r;
		v[1] = fmt->palette->colors[pixel].g;
		v[2] = fmt->palette->colors[pixel].b;
		v[3] = fmt->palette->colors[pixel].unused;
	} else {
		v[0] = Uint8((pixel & fmt->Rmask) >> fmt->Rshift);
		v[1] = Uint8((pixel & fmt->Gmask) >> fmt->Gshift);
		v[2] = Uint8((pixel & fmt->Bmask) >> fmt->Bshift);
		v[3] = Uint8((pixel & fmt->Amask) >> fmt->Ashift);
	}
}

void mySDL_GetRawRGBAV(const Uint8 *p, const SDL_Surface *src, Uint8 *v)
{
	Uint32 pixel = decodepixel(p, src->format->BytesPerPixel);
	mySDL_GetRawRGBAV(pixel, src, v);	
}

void mySDL_GetRGBAV(Uint32 pixel, const SDL_Surface *src, Uint8 *v)
{
	mySDL_GetRawRGBAV(pixel, src, v);
	const SDL_PixelFormat *fmt = src->format;
	v[0] = v[0] << fmt->Rloss;
	v[1] = v[1] << fmt->Gloss;
	v[2] = v[2] << fmt->Bloss;
	// Correct for surfaces that don't have an alpha channel.
	if( fmt->Aloss == 8)
		v[3] = 255;
	else
		v[3] = v[3] << fmt->Aloss;
}

void mySDL_GetRGBAV(const Uint8 *p, const SDL_Surface *src, Uint8 *v)
{
	Uint32 pixel = decodepixel(p, src->format->BytesPerPixel);
	if( src->format->BytesPerPixel == 1 ) // paletted
	{
		memcpy( v, &src->format->palette->colors[pixel], sizeof(SDL_Color));
	}
	else	// RGBA
		mySDL_GetRGBAV(pixel, src, v);	
}


/* Inverse of mySDL_GetRawRGBAV. */
Uint32 mySDL_SetRawRGBAV(const SDL_PixelFormat *fmt, const Uint8 *v)
{
	return 	v[0] << fmt->Rshift |
			v[1] << fmt->Gshift |
			v[2] << fmt->Bshift |
			v[3] << fmt->Ashift;
}

void mySDL_SetRawRGBAV(Uint8 *p, const SDL_Surface *src, const Uint8 *v)
{
	Uint32 pixel = mySDL_SetRawRGBAV(src->format, v);
	encodepixel(p, src->format->BytesPerPixel, pixel);
}

/* Inverse of mySDL_GetRGBAV. */
Uint32 mySDL_SetRGBAV(const SDL_PixelFormat *fmt, const Uint8 *v)
{
	return 	(v[0] >> fmt->Rloss) << fmt->Rshift |
			(v[1] >> fmt->Gloss) << fmt->Gshift |
			(v[2] >> fmt->Bloss) << fmt->Bshift |
			(v[3] >> fmt->Aloss) << fmt->Ashift;
}

void mySDL_SetRGBAV(Uint8 *p, const SDL_Surface *src, const Uint8 *v)
{
	Uint32 pixel = mySDL_SetRGBAV(src->format, v);
	encodepixel(p, src->format->BytesPerPixel, pixel);
}


/* Get the number of bits representing each color channel in fmt. */
void mySDL_GetBitsPerChannel(const SDL_PixelFormat *fmt, Uint32 bits[4])
{
	if( fmt->BytesPerPixel == 1 )
	{
		/* If we're paletted, the palette is 8888. For some reason, the 
		 * *loss values are all 8 on paletted surfaces; they should be
		 * 0, to represent the palette.  Since they're not, we have to
		 * special case this. */
		bits[0] = bits[1] = bits[2] = bits[3] = 8;
		return;
	}

	/* The actual bits stored in each color is 8-loss.  */
	bits[0] = 8 - fmt->Rloss;
	bits[1] = 8 - fmt->Gloss;
	bits[2] = 8 - fmt->Bloss;
	bits[3] = 8 - fmt->Aloss;
}

/* SDL_SetPalette only works when SDL video has been initialized, even on software
 * surfaces. */
void mySDL_SetPalette(SDL_Surface *dst, const SDL_Color *colors, int start, int cnt)
{
	ASSERT( dst->format->palette );
	ASSERT( start+cnt <= dst->format->palette->ncolors );
	memcpy( dst->format->palette->colors + start, colors,
				cnt * sizeof(SDL_Color) );
}

void CopySDLSurface( SDL_Surface *src, SDL_Surface *dest )
{
	/* Copy the palette, if we have one. */
	if( src->format->BitsPerPixel == 8 && dest->format->BitsPerPixel == 8 )
	{
		ASSERT( dest->format->palette );
		mySDL_SetPalette( dest, src->format->palette->colors,
			0, src->format->palette->ncolors);
	}

	mySDL_BlitSurface( src, dest, -1, -1, false );
}

bool CompareSDLFormats( const SDL_PixelFormat *pf1, const SDL_PixelFormat *pf2 )
{
	if( pf1->BitsPerPixel != pf2->BitsPerPixel ) return false;
	if( pf1->Rmask == pf2->Rmask ) return false;
	if( pf1->Gmask == pf2->Gmask ) return false;
	if( pf1->Bmask == pf2->Bmask ) return false;
	if( pf1->Amask == pf2->Amask ) return false;
	return true;
}

bool ConvertSDLSurface( SDL_Surface *src, SDL_Surface *&dst,
		int width, int height, int bpp,
		Uint32 R, Uint32 G, Uint32 B, Uint32 A )
{
    dst = SDL_CreateRGBSurfaceSane( SDL_SWSURFACE, width, height, bpp, R, G, B, A );
	ASSERT( dst != NULL );

	/* If the formats are the same, no conversion is needed. */
	if( width == src->w && height == src->h && CompareSDLFormats( src->format, dst->format ) )
	{
		SDL_FreeSurface( dst );
		dst = NULL;
		return false;
	}

	CopySDLSurface( src, dst );
	return true;
}

void ConvertSDLSurface(SDL_Surface *&image,
		int width, int height, int bpp,
		Uint32 R, Uint32 G, Uint32 B, Uint32 A)
{
    SDL_Surface *ret_image;
	if( !ConvertSDLSurface( image, ret_image, width, height, bpp, R, G, B, A ) )
		return;

	SDL_FreeSurface( image );
	image = ret_image;
}

SDL_Surface *SDL_CreateRGBSurfaceSane
			(Uint32 flags, int width, int height, int depth, 
			Uint32 Rmask, Uint32 Gmask, Uint32 Bmask, Uint32 Amask)
{
	SDL_Surface *ret = SDL_CreateRGBSurface(flags, width, height, depth,
		Rmask, Gmask, Bmask, Amask);

	if(ret == NULL)
		RageException::Throw("SDL_CreateRGBSurface(%i, %i, %i, %i, %8x, %8x, %8x, %8x) failed: %s",
			flags, width, height, depth, Rmask, Gmask, Bmask, Amask, SDL_GetError());

	return ret;
}

static void FindAlphaRGB(const SDL_Surface *img, Uint8 &r, Uint8 &g, Uint8 &b, bool reverse)
{
	r = g = b = 0;
	
	/* If we have no alpha, there's no alpha color. */
	if( img->format->BitsPerPixel > 8 && !img->format->Amask )
		return;

	/* Eww.  Sorry.  Iterate front-to-back or in reverse. */
	for(int y = reverse? img->h-1:0;
		reverse? (y >=0):(y < img->h); reverse? (--y):(++y))
	{
		Uint8 *row = (Uint8 *)img->pixels + img->pitch*y;
		if(reverse)
			row += img->format->BytesPerPixel * (img->w-1);

		for(int x = 0; x < img->w; ++x)
		{
			Uint32 val = decodepixel(row, img->format->BytesPerPixel);
			if((img->format->BitsPerPixel == 8 && val != img->format->colorkey) ||
			   (img->format->BitsPerPixel != 8 && val & img->format->Amask))
			{
				/* This color isn't fully transparent, so grab it. */
				SDL_GetRGB(val, img->format, &r, &g, &b);
				return;
			}

			if( reverse )
				row -= img->format->BytesPerPixel;
			else
				row += img->format->BytesPerPixel;
		}
	}

	/* Huh?  The image is completely transparent. */
	r = g = b = 0;
}

/* Set the underlying RGB values of all pixels in 'img' that are
 * completely transparent. */
static void SetAlphaRGB(const SDL_Surface *img, Uint8 r, Uint8 g, Uint8 b)
{
	/* If it's a paletted surface, all we have to do is change the palette. */
	if( img->format->BitsPerPixel == 8 )
	{
		for( int c = 0; c < img->format->palette->ncolors; ++c )
		{
			if( img->format->palette->colors[c].unused )
				continue;
			img->format->palette->colors[c].r = r;
			img->format->palette->colors[c].g = g;
			img->format->palette->colors[c].b = b;
		}
		return;
	}


	/* If it's RGBA and there's no alpha channel, we have nothing to do. */
	if( img->format->BitsPerPixel > 8 && !img->format->Amask )
		return;

	Uint32 trans = SDL_MapRGBA(img->format, r, g, b, 0);
	for( int y = 0; y < img->h; ++y )
	{
		Uint8 *row = (Uint8 *)img->pixels + img->pitch*y;

		for( int x = 0; x < img->w; ++x )
		{
			Uint32 val = decodepixel( row, img->format->BytesPerPixel );
			if( val != trans && !(val&img->format->Amask) )
			{
				encodepixel( row, img->format->BytesPerPixel, trans );
			}

			row += img->format->BytesPerPixel;
		}
	}
}

/* When we scale up images (which we always do in high res), pixels
 * that are completely transparent can be blended with opaque pixels,
 * causing their RGB elements to show.  This is visible in many textures
 * as a pixel-wide border in the wrong color.  This is tricky to fix.
 * We need to set the RGB components of completely transparent pixels
 * to a reasonable color.
 *
 * Most images have a single border color.  For these, the transparent
 * color is easy: search through the image top-bottom-left-right,
 * find the first non-transparent pixel, and pull out its RGB.
 *
 * A few images don't.  We can only make a guess here.  What we'll do
 * is, after the above search, do the same in reverse (bottom-top-right-
 * left).  If the color we find is different, we'll just set the border
 * color to black.  
 */
void FixHiddenAlpha( SDL_Surface *img )
{
	Uint8 r, g, b;
	FindAlphaRGB(img, r, g, b, false);

	Uint8 cr, cg, cb; /* compare */
	FindAlphaRGB(img, cr, cg, cb, true);

	if( cr != r || cg != g || cb != b )
		r = g = b = 0;

	SetAlphaRGB(img, r, g, b);
}

/* Find various traits of a surface.  Do these all at once, so we only have to
 * iterate the surface once. */

/* We could theoretically do a test to see if an image fits in GL_ALPHA4,
 * by looking at the least significant bits of each alpha value.  This is
 * not likely to ever find a match, though, so don't bother; only use 8alphaonly
 * if it's explicitly enabled. 
 *
 * XXX: We could do the FindAlphaRGB search here, too, but we need that information
 * in a different place. */

int FindSurfaceTraits( const SDL_Surface *img )
{
	const int NEEDS_NO_ALPHA=0, NEEDS_BOOL_ALPHA=1, NEEDS_FULL_ALPHA=2;
	int alpha_type = NEEDS_NO_ALPHA;

	Uint32 max_alpha;
	if( img->format->BitsPerPixel == 8 )
		max_alpha = 0xFF;
	else
		max_alpha = img->format->Amask;

	for(int y = 0; y < img->h; ++y)
	{
		Uint8 *row = (Uint8 *)img->pixels + img->pitch*y;

		for(int x = 0; x < img->w; ++x)
		{
			Uint32 val = decodepixel(row, img->format->BytesPerPixel);

			Uint32 alpha;
			if( img->format->BitsPerPixel == 8 )
				alpha = img->format->palette->colors[val].unused;
			else
				alpha = (val & img->format->Amask);

			if( alpha == 0 )
				alpha_type = max( alpha_type, NEEDS_BOOL_ALPHA );
			else if( alpha != max_alpha )
				alpha_type = max( alpha_type, NEEDS_FULL_ALPHA );

			row += img->format->BytesPerPixel;
		}
	}

	int ret = 0;
	switch( alpha_type ) 
	{
	case NEEDS_NO_ALPHA:	ret |= TRAIT_NO_TRANSPARENCY;	break;
	case NEEDS_BOOL_ALPHA:	ret |= TRAIT_BOOL_TRANSPARENCY;	break;
	case NEEDS_FULL_ALPHA:									break;
	default:	ASSERT(0);
	}

	return ret;
}

bool SDL_GetEvent( SDL_Event &event, int mask )
{
	/* SDL_PeepEvents returns error if video isn't initialized. */
	if(!SDL_WasInit(SDL_INIT_VIDEO))
		return false;

	switch(SDL_PeepEvents(&event, 1, SDL_GETEVENT, mask))
	{
	case 1: return true;
	case 0: return false;
	default: RageException::Throw("SDL_PeepEvents returned unexpected error: %s", SDL_GetError());
	}
}

/* Reads all currently queued SDL events, clearing them from the queue. */
void mySDL_GetAllEvents( vector<SDL_Event> &events )
{
	while(1)
	{
		SDL_Event ev;
		if(SDL_PollEvent(&ev) <= 0)
			break;

		events.push_back(ev);
	}
}

/* Pushes the given events onto the SDL event queue. */
void mySDL_PushEvents( vector<SDL_Event> &events )
{
	for(unsigned i = 0; i < events.size(); ++i)
		SDL_PushEvent(&events[i]);
}

/* For some bizarre reason, SDL_EventState flushes all events.  This is a pain, so
 * avoid it. */
Uint8 mySDL_EventState( Uint8 type, int state )
{
	if(state == SDL_QUERY)
		return SDL_EventState(type, state);

	vector<SDL_Event> events;
	mySDL_GetAllEvents(events);

	/* Set the event mask. */
	Uint8 ret = SDL_EventState(type, state);

	/* Don't readding events that we just turned off; they'll just sit around
	 * in the buffer. */
	for(unsigned i = 0; i < events.size(); )
	{
		if(state == SDL_IGNORE && events[i].type == type)
			events.erase(events.begin()+i);
		else
			i++;
	}

	/* Put them back. */
	mySDL_PushEvents(events);

	return ret;
}

void mySDL_WM_SetIcon( CString sIconFile )
{
#if !defined(DARWIN)
	if( sIconFile.empty() )
	{
		SDL_WM_SetIcon(NULL, NULL);
		return;
	}

	SDL_Surface *srf = RageSurface::LoadFile(sIconFile);
	if( srf == NULL )
		return;

	/* Windows icons are 32x32 and SDL can't resize them for us, which
	 * causes mask corruption.  (Actually, the above icon *is* 32x32;
	 * this is here just in case it changes.) */
	ConvertSDLSurface(srf, srf->w, srf->h,
		32, 0xFF000000, 0x00FF0000, 0x0000FF00, 0x000000FF);
	zoomSurface(srf, 32, 32);

	SDL_SetAlpha( srf, SDL_SRCALPHA, SDL_ALPHA_OPAQUE );
	SDL_WM_SetIcon(srf, NULL /* derive from alpha */);
	SDL_FreeSurface(srf);
#endif
}


struct SurfaceHeader
{
	int width, height, pitch;
	int Rmask, Gmask, Bmask, Amask;
	int bpp;
};

/* Save and load SDL_Surfaces to disk.  This avoids problems with bitmaps. */
bool mySDL_SaveSurface( SDL_Surface *img, CString file )
{
	RageFile f;
	if( !f.Open( file, RageFile::WRITE ) )
		return false;

	SurfaceHeader h;
	memset( &h, 0, sizeof(h) );

	h.height = img->h;
	h.width = img->w;
	h.pitch = img->pitch;
	h.Rmask = img->format->Rmask;
	h.Gmask = img->format->Gmask;
	h.Bmask = img->format->Bmask;
	h.Amask = img->format->Amask;
	h.bpp = img->format->BitsPerPixel;

	f.Write( &h, sizeof(h) );

	if(h.bpp == 8)
		f.Write( img->format->palette->colors, 256 * sizeof(SDL_Color) );

	f.Write( img->pixels, img->h * img->pitch );
	
	return true;
}

SDL_Surface *mySDL_LoadSurface( CString file )
{
	RageFile f;
	if( !f.Open( file ) )
		return false;

	SurfaceHeader h;
	if( f.Read( &h, sizeof(h) ) != sizeof(h) )
		return NULL;
	
	SDL_Color palette[256];
	if(h.bpp == 8)
		if( f.Read( palette, 256 * sizeof(SDL_Color) != 256 * sizeof(SDL_Color) ) )
			return NULL;

	/* Create the surface. */
	SDL_Surface *img = SDL_CreateRGBSurface(
			SDL_SWSURFACE, h.width, h.height, h.bpp,
			h.Rmask, h.Gmask, h.Bmask, h.Amask);
	ASSERT( img );
	ASSERT( h.pitch == img->pitch );

	if( f.Read( img->pixels, h.height * h.pitch ) != h.height * h.pitch )
	{
		SDL_FreeSurface( img );
		return NULL;
	}

	/* Set the palette. */
	if( h.bpp == 8 )
		mySDL_SetPalette( img, palette, 0, 256 );

	return img;
}


/* Annoying: SDL_MapRGB will do a nearest-match if the specified color isn't found.
 * This breaks color keyed images that don't actually use the color key. */
bool mySDL_MapRGBExact( SDL_PixelFormat *fmt, Uint8 R, Uint8 G, Uint8 B, Uint32 &color )
{
	color = SDL_MapRGB(fmt, R, G, B);

	if( fmt->BitsPerPixel == 8 ) {
		if(fmt->palette->colors[color].r != R ||
		   fmt->palette->colors[color].g != G ||
		   fmt->palette->colors[color].b != B )
			return false;
	}

	return true;
}

inline static float scale( float x, float l1, float h1, float l2, float h2 )
{
	return ((x - l1) / (h1 - l1) * (h2 - l2) + l2);
}

inline void mySDL_GetRawRGBAV_XY( const SDL_Surface *src, Uint8 *v, int x, int y )
{
	const Uint8 *srcp = (const Uint8 *) src->pixels + (y * src->pitch);
	const Uint8 *srcpx = srcp + (x * src->format->BytesPerPixel);

	mySDL_GetRawRGBAV(srcpx, src, v);
}

#include "RageUtil.h"

/* Completely unoptimized. */
void mySDL_BlitTransform( const SDL_Surface *src, SDL_Surface *dst, 
					const float fCoords[8] /* TL, BR, BL, TR */ )
{
	ASSERT( src->format->BytesPerPixel == dst->format->BytesPerPixel );

	const float Coords[8] = {
		(fCoords[0] * (src->w)), (fCoords[1] * (src->h)),
		(fCoords[2] * (src->w)), (fCoords[3] * (src->h)),
		(fCoords[4] * (src->w)), (fCoords[5] * (src->h)),
		(fCoords[6] * (src->w)), (fCoords[7] * (src->h))
	};

	const int TL_X = 0, TL_Y = 1, BL_X = 2, BL_Y = 3,
			  BR_X = 4, BR_Y = 5, TR_X = 6, TR_Y = 7;

	for( int y = 0; y < dst->h; ++y )
	{
		Uint8 *dstp = (Uint8 *) dst->pixels + (y * dst->pitch); /* line */
		Uint8 *dstpx = dstp; /* pixel */

		const float start_y = scale(float(y), 0, float(dst->h), Coords[TL_Y], Coords[BL_Y]);
		const float end_y = scale(float(y), 0, float(dst->h), Coords[TR_Y], Coords[BR_Y]);

		const float start_x = scale(float(y), 0, float(dst->h), Coords[TL_X], Coords[BL_X]);
		const float end_x = scale(float(y), 0, float(dst->h), Coords[TR_X], Coords[BR_X]);

		for( int x = 0; x < dst->w; ++x )
		{
			const float src_xp = scale(float(x), 0, float(dst->w), start_x, end_x);
			const float src_yp = scale(float(x), 0, float(dst->w), start_y, end_y);

			/* If the surface is two pixels wide, src_xp is 0..2.  .5 indicates
			 * pixel[0]; 1 indicates 50% pixel[0], 50% pixel[1]; 1.5 indicates
			 * pixel[1]; 2 indicates 50% pixel[1], 50% pixel[2] (which is clamped
			 * to pixel[1]). */
			int src_x[2], src_y[2];
			src_x[0] = (int) truncf(src_xp - 0.5f);
			src_x[1] = src_x[0] + 1;

			src_y[0] = (int) truncf(src_yp - 0.5f);
			src_y[1] = src_y[0] + 1;

			/* Emulate GL_REPEAT. */
			src_x[0] = clamp(src_x[0], 0, src->w);
			src_x[1] = clamp(src_x[1], 0, src->w);
			src_y[0] = clamp(src_y[0], 0, src->h);
			src_y[1] = clamp(src_y[1], 0, src->h);

			/* Decode our four pixels. */
			Uint8 v[4][4];
			mySDL_GetRawRGBAV_XY(src, v[0], src_x[0], src_y[0]);
			mySDL_GetRawRGBAV_XY(src, v[1], src_x[0], src_y[1]);
			mySDL_GetRawRGBAV_XY(src, v[2], src_x[1], src_y[0]);
			mySDL_GetRawRGBAV_XY(src, v[3], src_x[1], src_y[1]);

			/* Distance from the pixel chosen: */
			float weight_x = src_xp - (src_x[0] + 0.5f);
			float weight_y = src_yp - (src_y[0] + 0.5f);

			/* Filter: */
			Uint8 out[4] = { 0,0,0,0 };
			for(int i = 0; i < 4; ++i)
			{
				float sum = 0;
				sum += v[0][i] * (1-weight_x) * (1-weight_y);
				sum += v[1][i] * (1-weight_x) * (weight_y);
				sum += v[2][i] * (weight_x)   * (1-weight_y);
				sum += v[3][i] * (weight_x)   * (weight_y);
				out[i] = (Uint8) clamp( lrintf(sum), 0, 255 );
			}

			/* If the source has no alpha, set the destination to opaque. */
			if( src->format->Amask == 0 )
				out[3] = Uint8( dst->format->Amask >> dst->format->Ashift );

			mySDL_SetRawRGBAV(dstpx, dst, out);

			dstpx += dst->format->BytesPerPixel;
		}
	}
}

/*
 * Simplified:
 *
 * No source alpha.
 * Palette -> palette blits assume the palette is identical (no mapping).
 * No color key.
 * No general blitting rects.
 */

static void blit_same_type( SDL_Surface *src_surf, const SDL_Surface *dst_surf, int width, int height )
{
	const char *src = (const char *) src_surf->pixels;
	const char *dst = (const char *) dst_surf->pixels;

	/* Bytes to skip at the end of a line. */
	const int srcskip = src_surf->pitch - width*src_surf->format->BytesPerPixel;
	const int dstskip = dst_surf->pitch - width*dst_surf->format->BytesPerPixel;

	/* XXX: duff's this */
	while( height-- )
	{
		int x = 0;
		while( x++ < width )
		{
			/* (Relatively) fast. */
			switch( src_surf->format->BytesPerPixel )
			{
			case 1: *((Uint8 *)dst) = *((Uint8 *)src); break;
			case 2: *((Uint16 *)dst) = *((Uint16 *)src); break;
			case 3: ((Uint8 *)dst)[0] = ((Uint8 *)src)[0];
					((Uint8 *)dst)[1] = ((Uint8 *)src)[1];
					((Uint8 *)dst)[2] = ((Uint8 *)src)[2];
					break;
			case 4: *((Uint32 *)dst) = *((Uint32 *)src); break;
			}

			src += src_surf->format->BytesPerPixel;
			dst += dst_surf->format->BytesPerPixel;
		}

		src += srcskip;
		dst += dstskip;
	}
}

/* Rescaling blit with no ckey.  This is used to update movies in
 * D3D, so optimization is very important. */
static void blit_rgba_to_rgba( SDL_Surface *src_surf, const SDL_Surface *dst_surf, int width, int height )
{
	const char *src = (const char *) src_surf->pixels;
	const char *dst = (const char *) dst_surf->pixels;

	/* Bytes to skip at the end of a line. */
	const int srcskip = src_surf->pitch - width*src_surf->format->BytesPerPixel;
	const int dstskip = dst_surf->pitch - width*dst_surf->format->BytesPerPixel;

	Uint32 src_bits[4], dst_bits[4];
	mySDL_GetBitsPerChannel(src_surf->format, src_bits);
	mySDL_GetBitsPerChannel(dst_surf->format, dst_bits);

	const int rshifts[4] = {
		src_surf->format->Rshift + src_bits[0] - dst_bits[0],
		src_surf->format->Gshift + src_bits[1] - dst_bits[1],
		src_surf->format->Bshift + src_bits[2] - dst_bits[2],
		src_surf->format->Ashift + src_bits[3] - dst_bits[3],
	};
	const int lshifts[4] = {
		dst_surf->format->Rshift,
		dst_surf->format->Gshift,
		dst_surf->format->Bshift,
		dst_surf->format->Ashift,
	};

	const Uint32 masks[4] = {
		src_surf->format->Rmask,
		src_surf->format->Gmask,
		src_surf->format->Bmask,
		src_surf->format->Amask
	};

	int ormask = 0;
	if( src_surf->format->Amask == 0 )
		ormask = dst_surf->format->Amask;

	while( height-- )
	{
		int x = 0;
		while( x++ < width )
		{
			unsigned int pixel = decodepixel((Uint8 *) src, src_surf->format->BytesPerPixel);

			/* Convert pixel to the destination RGBA. */
			unsigned int opixel = 0;
			opixel |= (pixel & masks[0]) >> rshifts[0] << lshifts[0];
			opixel |= (pixel & masks[1]) >> rshifts[1] << lshifts[1];
			opixel |= (pixel & masks[2]) >> rshifts[2] << lshifts[2];
			opixel |= (pixel & masks[3]) >> rshifts[3] << lshifts[3];

			// Correct surfaces that don't have an alpha channel.
			opixel |= ormask;

			/* Store it. */
			encodepixel((Uint8 *) dst, dst_surf->format->BytesPerPixel, opixel);

			src += src_surf->format->BytesPerPixel;
			dst += dst_surf->format->BytesPerPixel;
		}

		src += srcskip;
		dst += dstskip;
	}
}

static void blit_generic( SDL_Surface *src_surf, const SDL_Surface *dst_surf, int width, int height )
{
	const char *src = (const char *) src_surf->pixels;
	const char *dst = (const char *) dst_surf->pixels;

	/* Bytes to skip at the end of a line. */
	const int srcskip = src_surf->pitch - width*src_surf->format->BytesPerPixel;
	const int dstskip = dst_surf->pitch - width*dst_surf->format->BytesPerPixel;

	while( height-- )
	{
		int x = 0;
		while( x++ < width )
		{
			unsigned int pixel = decodepixel((Uint8 *) src, src_surf->format->BytesPerPixel);

			Uint8 colors[4];
				/* Convert pixel to the destination RGBA. */
				colors[0] = src_surf->format->palette->colors[pixel].r;
				colors[1] = src_surf->format->palette->colors[pixel].g;
				colors[2] = src_surf->format->palette->colors[pixel].b;
				colors[3] = src_surf->format->palette->colors[pixel].unused;
			pixel = mySDL_SetRGBAV(dst_surf->format, colors);

			/* Store it. */
			encodepixel((Uint8 *) dst, dst_surf->format->BytesPerPixel, pixel);

			src += src_surf->format->BytesPerPixel;
			dst += dst_surf->format->BytesPerPixel;
		}

		src += srcskip;
		dst += dstskip;
	}

}

void mySDL_BlitSurface( 
	SDL_Surface *src, SDL_Surface *dst, int width, int height, bool ckey)
{
	if(width == -1)
		width = src->w;
	if(height == -1)
		height = src->h;
	width = min(src->w, dst->w);
	height = min(src->h, dst->h);

	/* Types of blits:
	 * RGBA->RGBA, same format without colorkey
	 * RGBA->RGBA, same format with colorkey
	 * PAL->PAL; ignore colorkey flag
	 * RGBA->RGBA different format without colorkey
	 * RGBA->RGBA different format with colorkey
	 * PAL->RGBA with colorkey
	 * PAL->RGBA without colorkey
	 */
	if( src->format->BytesPerPixel == dst->format->BytesPerPixel &&
		src->format->Rmask == dst->format->Rmask &&
		src->format->Gmask == dst->format->Gmask &&
		src->format->Bmask == dst->format->Bmask &&
		src->format->Amask == dst->format->Amask )
	{
		/* RGBA->RGBA with the same format, or PAL->PAL.  Simple copy. */
		blit_same_type(src, dst, width, height);
	}

	else if( src->format->BytesPerPixel != 1 && dst->format->BytesPerPixel != 1 )
	{
		/* RGBA->RGBA with different formats. */
		blit_rgba_to_rgba(src, dst, width, height);
	}

	else if( src->format->BytesPerPixel == 1 && dst->format->BytesPerPixel != 1 )
	{
		/* PAL->RGBA. */
		blit_generic(src, dst, width, height);
	}
	else
		/* We don't do RGBA->PAL. */
		ASSERT(0);
}

/* This converts an image to a special 8-bit paletted format.  The palette is set up
 * so that palette indexes look like regular, packed components.
 *
 * For example, an image with 8 bits of grayscale and 0 bits of alpha has a palette
 * that looks like { 0,0,0,255 }, { 1,1,1,255 }, { 2,2,2,255 }, ... { 255,255,255,255 }.
 * This results in index components that can be treated as grayscale values.
 *
 * An image with 2 bits of grayscale and 2 bits of alpha look like
 * { 0,0,0,0  }, { 85,85,85,0  }, { 170,170,170,0  }, { 255,255,255,0  },
 * { 0,0,0,85 }, { 85,85,85,85 }, { 170,170,170,85 }, { 255,255,255,85 }, ...
 *
 * This results in index components that can be pulled apart like regular packed
 * values: the first two bits of the index are the grayscale component, and the next
 * two bits are the alpha component.
 *
 * This gives us a generic way to handle arbitrary 8-bit texture formats.  It could
 * possibly be used for 16-bit texture formats, but I doubt those are well-supported
 * in hardware, and SDL blits only support 8-bit paletted surfaces. */
SDL_Surface *mySDL_Palettize( SDL_Surface *src_surf, int GrayBits, int AlphaBits )
{
	AlphaBits = min( AlphaBits, 8-src_surf->format->Aloss );
	
	const int TotalBits = GrayBits + AlphaBits;
	ASSERT( TotalBits <= 8 );

	SDL_Surface *dst_surf = SDL_CreateRGBSurfaceSane(SDL_SWSURFACE, src_surf->w, src_surf->h,
		8, 0,0,0,0 );

	/* Set up the palette. */
	const int TotalColors = 1 << TotalBits;
	const int Ivalues = 1 << GrayBits;					// number of intensity values
	const int Ishift = 0;								// intensity shift
	const int Imask = ((1 << GrayBits) - 1) << Ishift;	// intensity mask
	const int Iloss = 8-GrayBits;

	const int Avalues = 1 << AlphaBits;					// number of alpha values
	const int Ashift = GrayBits;						// alpha shift
	const int Amask = ((1 << AlphaBits) - 1) << Ashift;	// alpha mask
	const int Aloss = 8-AlphaBits;

	for( int index = 0; index < TotalColors; ++index )
	{
		const int I = (index & Imask) >> Ishift;
		const int A = (index & Amask) >> Ashift;

		int ScaledI;
		if( Ivalues == 1 )
			ScaledI = 255; // if only one intensity value, always fullbright
		else
			ScaledI = clamp( int(roundf(I * (255.0f / (Ivalues-1)))), 0, 255 );

		int ScaledA;
		if( Avalues == 1 )
			ScaledA = 255; // if only one alpha value, always opaque
		else
			ScaledA = clamp( int(roundf(A * (255.0f / (Avalues-1)))), 0, 255 );

		SDL_Color c;
		c.r = Uint8(ScaledI);
		c.g = Uint8(ScaledI);
		c.b = Uint8(ScaledI);
		c.unused = Uint8(ScaledA);

		SDL_SetColors( dst_surf, &c, index, 1);
	}

	const char *src = (const char *) src_surf->pixels;
	const char *dst = (const char *) dst_surf->pixels;

	int height = src_surf->h;
	int width = src_surf->w;

	/* Bytes to skip at the end of a line. */
	const int srcskip = src_surf->pitch - width*src_surf->format->BytesPerPixel;
	const int dstskip = dst_surf->pitch - width*dst_surf->format->BytesPerPixel;

	while( height-- )
	{
		int x = 0;
		while( x++ < width )
		{
			unsigned int pixel = decodepixel((Uint8 *) src, src_surf->format->BytesPerPixel);

			Uint8 colors[4];
			mySDL_GetRGBAV(pixel, src_surf, colors);

			int Ival = 0;
			Ival += colors[0];
			Ival += colors[1];
			Ival += colors[2];
			Ival /= 3;

			pixel = (Ival >> Iloss) << Ishift |
					(colors[3] >> Aloss) << Ashift;

			/* Store it. */
			*((Uint8 *) dst) = Uint8(pixel);

			src += src_surf->format->BytesPerPixel;
			dst += dst_surf->format->BytesPerPixel;
		}

		src += srcskip;
		dst += dstskip;
	}

	return dst_surf;
}

int RWRageFile_Seek( struct SDL_RWops *context, int offset, int whence )
{
	RageFile *f = (RageFile *) context->hidden.unknown.data1;
	return f->Seek( (int) offset, whence );
}

int RWRageFile_Read( struct SDL_RWops *context, void *ptr, int size, int nmemb )
{
	RageFile *f = (RageFile *) context->hidden.unknown.data1;
	return f->Read( ptr, size, nmemb );
}

int RWRageFile_Write( struct SDL_RWops *context, const void *ptr, int size, int nmemb )
{
	RageFile *f = (RageFile *) context->hidden.unknown.data1;
	return f->Write( ptr, size, nmemb );
}

/* Close and free an allocated SDL_FSops structure */
int RWRageFile_Close(struct SDL_RWops *context)
{
	return 0;
}

void OpenRWops( SDL_RWops *rw, RageFile *f )
{
	rw->hidden.unknown.data1 = f;
	rw->seek = RWRageFile_Seek;
	rw->read = RWRageFile_Read;
	rw->write = RWRageFile_Write;
	rw->close = RWRageFile_Close;
}

struct RWString
{
	CString *buf;
	int fp;
	RWString( CString *b ): buf(b), fp(0) { }
};

int RWString_Seek( struct SDL_RWops *context, int offset, int whence )
{
	RWString *f = (RWString *) context->hidden.unknown.data1;

	switch(whence)
	{
	case SEEK_CUR: offset += f->fp; break;
	case SEEK_END: offset += f->buf->size(); break;
	}
	f->fp = min( offset, (int) f->buf->size() );
	return f->fp;
}

int RWString_Read( struct SDL_RWops *context, void *ptr, int size, int nmemb )
{
	RWString *f = (RWString *) context->hidden.unknown.data1;
	CString &buf = *f->buf;

	size *= nmemb;
	size = min( size, (int) buf.size() - f->fp );
	memcpy( ptr, buf.data()+f->fp, size );

	f->fp += size;
	return size;
}

int RWString_Write( struct SDL_RWops *context, const void *ptr, int size, int nmemb )
{
	RWString *f = (RWString *) context->hidden.unknown.data1;
	CString &buf = *f->buf;

	size *= nmemb;
	const int ahead = buf.size() - f->fp;
	const int overwrite = min( size, ahead );
	buf.replace( buf.begin() + f->fp, buf.begin() + f->fp + overwrite,
		(const char*)ptr, size );

	f->fp += size;
	return size;
}

/* Close and free an allocated SDL_FSops structure */
int RWString_Close(struct SDL_RWops *context)
{
	RWString *f = (RWString *) context->hidden.unknown.data1;
	delete f;
	return 0;
}

void OpenRWops( SDL_RWops *rw, CString *sBuf )
{
	rw->hidden.unknown.data1 = new RWString( sBuf );
	rw->seek = RWString_Seek;
	rw->read = RWString_Read;
	rw->write = RWString_Write;
	rw->close = RWString_Close;
}

SDL_Surface *mySDL_MakeDummySurface( int height, int width )
{
    SDL_Surface *ret_image = SDL_CreateRGBSurfaceSane(
            SDL_SWSURFACE, width, height, 8, 0,0,0,0);
	ASSERT( ret_image != NULL );

	SDL_Color pink = { 0xFF, 0x10, 0xFF, 0xFF };
	mySDL_SetPalette( ret_image, &pink, 0, 1 );

	memset( ret_image->pixels, 0, ret_image->h*ret_image->pitch );

	return ret_image;
}

/* SDL sometimes fails to set an error, in which case we get the null string.  We
 * sometimes use that as a sentinel return value.  This function returns "(none)"
 * if no error is set. */
CString mySDL_GetError()
{
	CString error = SDL_GetError();
	if( error == "" )
		return "(none)"; /* SDL sometimes fails to set an error */
	return error;
}

/* When we receive surfaces from other libraries, the "unused" palette entry is
 * undefined.  We use it for alpha, so set it to 255. */
void mySDL_FixupPalettedAlpha( SDL_Surface *img )
{
	if( img->format->BitsPerPixel != 8 )
		return;

	for( int index = 0; index < img->format->palette->ncolors; ++index )
		img->format->palette->colors[index].unused = 0xFF;

	/* If the surface had a color key set, transfer it. */
	if( img->flags & SDL_SRCCOLORKEY )
	{
		img->flags &= ~SDL_SRCCOLORKEY;
		ASSERT_M( (int)img->format->colorkey < img->format->palette->ncolors, ssprintf("%i",img->format->colorkey) );
		img->format->palette->colors[ img->format->colorkey ].unused = 0;
	}
}

void mySDL_AddColorKey( SDL_Surface *img, Uint32 color )
{
	ASSERT_M( img->format->BitsPerPixel == 8, ssprintf( "%i", img->format->BitsPerPixel) );
	ASSERT_M( color < (Uint32) img->format->palette->ncolors, ssprintf("%i < %i", color, img->format->palette->ncolors ) );
	img->format->palette->colors[ color ].unused = 0;
}

/* HACK:  Some banners and textures have #F800F8 as the color key. Search the edge
 * it; if we find it, use that as the color key. */
static bool ImageUsesOffHotPink( const SDL_Surface *img )
{
	Uint32 OffHotPink;
	if( !mySDL_MapRGBExact( img->format, 0xF8, 0, 0xF8, OffHotPink ) )
		return false;

	const uint8_t *p = (uint8_t*) img->pixels;
	for( int x = 0; x < img->w; ++x )
	{
		Uint32 val = decodepixel( p, img->format->BytesPerPixel );
		if( val == OffHotPink )
			return true;
		p += img->format->BytesPerPixel;
	}

	p = (uint8_t*)img->pixels;
	p += img->pitch * (img->h-1);
	for( int i=0; i < img->w; i++ )
	{
		Uint32 val = decodepixel( p, img->format->BytesPerPixel );
		if( val == OffHotPink )
			return true;
		p += img->format->BytesPerPixel;
	}
	return false;
}

/* Set #FF00FF and #F800F8 to transparent.  img may be reallocated if it has no alpha
 * bits. */
void ApplyHotPinkColorKey( SDL_Surface *&img )
{
	if( img->format->BitsPerPixel == 8 )
	{
		Uint32 color;
		if( mySDL_MapRGBExact( img->format, 0xF8, 0, 0xF8, color ) )
			mySDL_AddColorKey( img, color );
		if( mySDL_MapRGBExact( img->format, 0xFF, 0, 0xFF,  color ) )
			mySDL_AddColorKey( img, color );
		return;
	}

	/* RGBA. */
	/* Make sure we have alpha. */
	if( !img->format->Amask )
	{
		/* We don't have any alpha.  Try to enable it without copying. */
		int usable_bits = (1<<img->format->BitsPerPixel)-1;
		usable_bits &= ~img->format->Rmask;
		usable_bits &= ~img->format->Gmask;
		usable_bits &= ~img->format->Bmask;
		
		for( int i = 0; img->format->Amask == 0 && i < 32; ++i )
		{
			if( usable_bits & (1<<i) )
			{
				img->format->Amask = 1<<i;
				img->format->Aloss = 7;
				img->format->Ashift = (Uint8) i;
			}
		}

		if( img->format->Amask == 0  )
			ConvertSDLSurface( img, img->w, img->h,
				32, 0xFF000000, 0x00FF0000, 0x0000FF00, 0x000000FF );
	}

	Uint32 HotPink;

	bool bHaveColorKey;
	if( ImageUsesOffHotPink(img) )
		bHaveColorKey = mySDL_MapRGBExact( img->format, 0xF8, 0, 0xF8, HotPink );
	else
		bHaveColorKey = mySDL_MapRGBExact( img->format, 0xFF, 0, 0xFF, HotPink );
	if( !bHaveColorKey )
		return;

	for( int y = 0; y < img->h; ++y )
	{
		Uint8 *row = (Uint8 *)img->pixels + img->pitch*y;

		for( int x = 0; x < img->w; ++x )
		{
			Uint32 val = decodepixel( row, img->format->BytesPerPixel );
			if( val == HotPink )
				encodepixel( row, img->format->BytesPerPixel, 0 );

			row += img->format->BytesPerPixel;
		}
	}
}

/*
 * (c) 2002-2004 Glenn Maynard
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
