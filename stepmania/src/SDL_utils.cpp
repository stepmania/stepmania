/*
 * Various small SDL tools.
 * 
 * Copyright (c) 2002 by the person(s) listed below.  All rights reserved.
 *	Glenn Maynard
 * 
 * Portions from SDL source and documentation.
 */

#include "global.h"

#include "SDL.h"
#include "SDL_utils.h"
#include "SDL_endian.h"

#if defined(WIN32)

/* Pull in all of our SDL libraries here. */
#ifdef DEBUG
#pragma comment(lib, "SDL-1.2.5/lib/SDLd.lib")
#pragma comment(lib, "SDL_image-1.2/SDL_imaged.lib")
#else
#pragma comment(lib, "SDL-1.2.5/lib/SDL.lib")
#pragma comment(lib, "SDL_image-1.2/SDL_image.lib")
#endif


#endif

Uint32 mySDL_Swap24(Uint32 x)
{
	return SDL_Swap32(x) >> 8; // xx223344 -> 443322xx -> 00443322
}

/* These conditionals in the inner loop are slow.  Templates? */
inline Uint32 decodepixel(const Uint8 *p, int bpp)
{
    switch(bpp) {
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
    switch(bpp) {
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
	if(src->format->BytesPerPixel == 1) {
		v[0] = fmt->palette->colors[pixel].r;
		v[1] = fmt->palette->colors[pixel].g;
		v[2] = fmt->palette->colors[pixel].b;
		v[3] = 0xFF;
	} else {
		v[0] = Uint8((pixel & fmt->Rmask) >> fmt->Rshift);
		v[1] = Uint8((pixel & fmt->Gmask) >> fmt->Gshift);
		v[2] = Uint8((pixel & fmt->Bmask) >> fmt->Bshift);
		v[3] = Uint8((pixel & fmt->Amask) >> fmt->Ashift);
	}

	if(src->flags & SDL_SRCCOLORKEY) {
		if((fmt->colorkey & ~fmt->Amask) == (pixel & ~fmt->Amask))
			v[3] = 0;
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
		v[3] = 0xFF;	// full alpha
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
	if(fmt->BytesPerPixel == 1) {
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
void mySDL_SetPalette(SDL_Surface *dst, SDL_Color *colors, int start, int cnt)
{
	ASSERT( dst->format->palette );
	ASSERT( start+cnt <= dst->format->palette->ncolors );
	memcpy( dst->format->palette->colors + start, colors,
				cnt * sizeof(SDL_Color) );
}

void ConvertSDLSurface(SDL_Surface *&image,
		int width, int height, int bpp,
		Uint32 R, Uint32 G, Uint32 B, Uint32 A)
{
    SDL_Surface *ret_image = SDL_CreateRGBSurfaceSane(
            SDL_SWSURFACE, width, height, bpp, R, G, B, A);
	ASSERT(ret_image != NULL);

	/* If the formats are the same, no conversion is needed. */
	if(width == image->w && height == image->h && bpp == image->format->BitsPerPixel &&
	   image->format->Rmask == ret_image->format->Rmask &&
	   image->format->Gmask == ret_image->format->Gmask &&
	   image->format->Bmask == ret_image->format->Bmask &&
	   image->format->Amask == ret_image->format->Amask)
	{
		/* One exception: if we have a color key and we're not paletted (8-bit). 
		 * In this case, we need to do the blit to get rid of the color key. */
		if(!( image->flags & SDL_SRCCOLORKEY && image->format->BitsPerPixel != 8) )
		{
			SDL_FreeSurface(ret_image);
			return;
		}
	}

	/* We don't want to actually blend the alpha channel over the destination converted
	 * surface; we want to simply blit it, so make sure SDL_SRCALPHA is not on. */
	SDL_SetAlpha( image, 0, SDL_ALPHA_OPAQUE );

	/* Copy the palette, if we have one. */
	if( image->format->BitsPerPixel == 8 && ret_image->format->BitsPerPixel == 8 )
	{
		ASSERT( ret_image->format->palette );
		mySDL_SetPalette(ret_image, image->format->palette->colors,
			0, image->format->palette->ncolors);
	}

	if(image->format->BitsPerPixel == 8 && ret_image->format->BitsPerPixel == 8 &&
	   image->flags & SDL_SRCCOLORKEY)
	{
		/* The source and dest are both paletted, and we have a color key. 
		 * First, make sure that the image we're blitting to has a default
		 * color of the color key, so any places we don't blit to will
		 * be transparent.  (The default color in the image is 0, so we're
		 * all set if the color key is 0.) */
		if(image->format->colorkey != 0)
			SDL_FillRect(ret_image, NULL, image->format->colorkey);

		/* Copy over the color key mode, and then turn off color keying in the
		 * source so the color key index gets copied like any other color. */
		SDL_SetColorKey( ret_image, SDL_SRCCOLORKEY, image->format->colorkey);
		SDL_SetColorKey( image, 0, 0 );
	}

	SDL_Rect area;
	area.x = area.y = 0;
	area.w = short(image->w);
	area.h = short(image->h);

	SDL_BlitSurface(image, &area, ret_image, &area);
	SDL_FreeSurface(image);
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
	/* If we have no alpha or no color key, there's no alpha color. */
	if(img->format->BitsPerPixel == 8 && !(img->flags & SDL_SRCCOLORKEY))
		return;
	if(img->format->BitsPerPixel > 8 && !img->format->Amask)
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

			if(reverse)
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
	/* If it's a paletted surface, all we have to do is change the
	 * colorkey, if any. */
	if(img->format->BitsPerPixel == 8)
	{
		if(img->flags & SDL_SRCCOLORKEY)
		{
			img->format->palette->colors[img->format->colorkey].r = r;
			img->format->palette->colors[img->format->colorkey].g = g;
			img->format->palette->colors[img->format->colorkey].b = b;
		}

		return;
	}

	/* It's RGBA.  If there's no alpha channel, we have nothing to do. */
	if(!img->format->Amask)
		return;

	Uint32 trans = SDL_MapRGBA(img->format, r, g, b, 0);
	for(int y = 0; y < img->h; ++y)
	{
		Uint8 *row = (Uint8 *)img->pixels + img->pitch*y;

		for(int x = 0; x < img->w; ++x)
		{
			Uint32 val = decodepixel(row, img->format->BytesPerPixel);
			if(val != trans && !(val&img->format->Amask))
			{
				encodepixel(row, img->format->BytesPerPixel, trans);
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
void FixHiddenAlpha(SDL_Surface *img)
{
	Uint8 r, g, b;
	FindAlphaRGB(img, r, g, b, false);

	Uint8 cr, cg, cb; /* compare */
	FindAlphaRGB(img, cr, cg, cb, true);

	if(cr != r || cg != g || cb != b)
		r = g = b = 0;

	SetAlphaRGB(img, r, g, b);
}

/* XXX: currently only TRAIT_NO_TRANSPARENCY and TRAIT_BOOL_TRANSPARENCY work. */
/* Find various traits of a surface.  Do these all at once, so we only have to
 * iterate the surface once. */

/* We could theoretically do a test to see if an image fits in GL_ALPHA4,
 * by looking at the least significant bits of each alpha value.  This is
 * not likely to ever find a match, though, so don't bother; only use 8alphaonly
 * if it's explicitly enabled. 
 *
 * XXX: We could do the FindAlphaRGB search here, too, but we need that information
 * in a different place. */
// #define TRAIT_CONSISTENT_TRANSPARENT_COLOR	0x0008

int FindSurfaceTraits(const SDL_Surface *img)
//							  Uint8 AlphaColor[3])
{
//	bool HaveAlphaValue = false; /* whether ar, ag, ab is valid */
//	bool HaveConsistentAlphaValue = true;
	const int NEEDS_NO_ALPHA=0, NEEDS_BOOL_ALPHA=1, NEEDS_FULL_ALPHA=2;
	int alpha_type = NEEDS_NO_ALPHA;
//	bool HaveTransparency = false;
//	bool HaveTranslucensy = false;
//	bool WhiteOnly = true;

	for(int y = 0; y < img->h; ++y)
	{
		Uint8 *row = (Uint8 *)img->pixels + img->pitch*y;
//		bool FirstVisible = true;

		for(int x = 0; x < img->w; ++x)
		{
			Uint32 val = decodepixel(row, img->format->BytesPerPixel);

			if( img->format->BitsPerPixel == 8 ) {
				if(img->flags & SDL_SRCCOLORKEY && val == img->format->colorkey )
					alpha_type = max( alpha_type, NEEDS_BOOL_ALPHA );
			} else if(img->format->Amask) {
				Uint32 masked_alpha = val & img->format->Amask;
				if(masked_alpha == 0) 
					alpha_type = max( alpha_type, NEEDS_BOOL_ALPHA );
				else if(masked_alpha != img->format->Amask)
					alpha_type = max( alpha_type, NEEDS_FULL_ALPHA );
			}

#if 0
			/* Hmm.  This doesn't quite work.  For example, the ScreenCompany
			 * shadow is actually black; we load it as 8alphaonly, which discards
			 * the black completely (making it a white shadow), and then we make
			 * it black again by setting the diffuse color to black.  This is hard
			 * to generalize.  I guess we could just make the shadow white, but
			 * that's a little ugly to edit.
			 *
			 * Also, for some reason, the font borders are actually a combination
			 * of a shade of gray and some alpha value.  Those need to be simplified
			 * to white and some alpha value (multiply the luma by alpha), but
			 * that's another special case.
			 *
			 * So, this doesn't actually help anything right now, and we still
			 * need 8alphaonly. */
			if( img->format->BitsPerPixel != 8 ) {
				/* If the pixel isn't transparent, and isn't completely white: */
				if(!Transparent &&
					(val & img->format->Rmask) != img->format->Rmask &&
					(val & img->format->Gmask) != img->format->Gmask &&
					(val & img->format->Bmask) != img->format->Bmask)
					WhiteOnly=false;

			}
#endif

#if 0
			/* Is this the first non-invisible pixel on this row? */
			if(!Invisible && FirstVisible)
			{
				FirstVisible = false;

				Uint8 r, g, b;
				SDL_GetRGB(val, img->format, &r, &g, &b);

				if(!HaveAlphaValue) {
					/* We don't have the border color yet; set it. */
					HaveAlphaValue = true;
					AlphaColor[0] = r;
					AlphaColor[1] = g;
					AlphaColor[2] = b;
				} else if(HaveConsistentAlphaValue) {
					/* We already have an alpha color from the previous row;
					 * if it's not the same, it's inconsistent. */
					if(r != AlphaColor[0] || g != AlphaColor[1] || b != AlphaColor[2])
						HaveConsistentAlphaValue = false;
				}
			}
#endif

			row += img->format->BytesPerPixel;
		}
	}

	int ret = 0;
//	if(HaveConsistentAlphaValue) ret |= TRAIT_CONSISTENT_TRANSPARENT_COLOR;
	switch( alpha_type ) 
	{
	case NEEDS_NO_ALPHA:	ret |= TRAIT_NO_TRANSPARENCY;	break;
	case NEEDS_BOOL_ALPHA:	ret |= TRAIT_BOOL_TRANSPARENCY;	break;
	case NEEDS_FULL_ALPHA:									break;
	default:	ASSERT(0);
	}
//	if(WhiteOnly) ret |= TRAIT_WHITE_ONLY;
	return ret;
}

bool SDL_GetEvent(SDL_Event &event, int mask)
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
void mySDL_GetAllEvents(vector<SDL_Event> &events)
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
void mySDL_PushEvents(vector<SDL_Event> &events)
{
	for(unsigned i = 0; i < events.size(); ++i)
		SDL_PushEvent(&events[i]);
}

/* For some bizarre reason, SDL_EventState flushes all events.  This is a pain, so
 * avoid it. */
Uint8 mySDL_EventState(Uint8 type, int state)
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

#include "SDL_image.h"	// for setting icon
#include "SDL_rotozoom.h"	// for setting icon

#if defined(DARWIN) && DARWIN
void mySDL_WM_SetIcon( CString sIconFile )
{
#pragma unused(sIconFile)
}
#else /* !defined(DARWIN) || !DARWIN */
void mySDL_WM_SetIcon( CString sIconFile )
{
	if( sIconFile.empty() )
	{
		SDL_WM_SetIcon(NULL, NULL);
		return;
	}

	SDL_Surface *srf = IMG_Load(sIconFile);

	// Why is this needed?  It's goofing up paletted images 
	// that use a color key other than pink. -Chris
	//	SDL_SetColorKey( srf, SDL_SRCCOLORKEY, SDL_MapRGB(srf->format, 0xFF, 0, 0xFF));

	/* Windows icons are 32x32 and SDL can't resize them for us, which
	 * causes mask corruption.  (Actually, the above icon *is* 32x32;
	 * this is here just in case it changes.) */
	ConvertSDLSurface(srf, srf->w, srf->h,
		32, 0xFF000000, 0x00FF0000, 0x0000FF00, 0x000000FF);
	zoomSurface(srf, 32, 32);

	SDL_SetAlpha( srf, SDL_SRCALPHA, SDL_ALPHA_OPAQUE );
	SDL_WM_SetIcon(srf, NULL /* derive from alpha */);
	SDL_FreeSurface(srf);
}
#endif /* !defined(DARWIN) || !DARWIN */


struct SurfaceHeader
{
	int width, height, pitch;
	int Rmask, Gmask, Bmask, Amask;
	int bpp;

	/* Relevant only if bpp = 8: */
	Uint8 colorkey; 
	bool colorkeyed;
};

/* Save and load SDL_Surfaces to disk.  This avoids problems with bitmaps. */
bool mySDL_SaveSurface( SDL_Surface *img, CString file )
{
	FILE *f = fopen(file.c_str(), "wb+");
	if(f == NULL)
		return false;

	SurfaceHeader h;
	h.height = img->h;
	h.width = img->w;
	h.pitch = img->pitch;
	h.Rmask = img->format->Rmask;
	h.Gmask = img->format->Gmask;
	h.Bmask = img->format->Bmask;
	h.Amask = img->format->Amask;
	h.bpp = img->format->BitsPerPixel;
	/* We lose data in the conversion, which makes matching the color key after loading
	 * tricky.  Just store it. */
	if( img->flags & SDL_SRCCOLORKEY )
	{
		h.colorkeyed = true;
		ASSERT( img->format->colorkey < 256 );
		h.colorkey = (Uint8) img->format->colorkey;
	} else {
		h.colorkeyed = false;
		h.colorkey = 0;
	}

	fwrite(&h, sizeof(h), 1, f);

	if(h.bpp == 8)
		fwrite(img->format->palette->colors, 256 * sizeof(SDL_Color), 1, f);

	fwrite(img->pixels, img->h * img->pitch, 1, f);
	
	fclose(f);
	return true;
}

SDL_Surface *mySDL_LoadSurface( CString file )
{
	FILE *f = fopen(file.c_str(), "rb");
	if(f == NULL)
		return NULL;

	SurfaceHeader h;
	if(fread(&h, sizeof(h), 1, f) != 1) return NULL;

	SDL_Color palette[256];
	if(h.bpp == 8)
		if(fread(palette, 256 * sizeof(SDL_Color), 1, f) != 1) return NULL;

	int size = h.height * h.pitch;
	char *pixels = (char *) malloc(size);
	if(fread(pixels, size, 1, f) != 1) return NULL;

	/* Create the surface. */
	SDL_Surface *img = SDL_CreateRGBSurfaceFrom(pixels,
			h.width, h.height, h.bpp, h.pitch,
			h.Rmask, h.Gmask, h.Bmask, h.Amask);

	/* Set the palette. */
	if( h.bpp == 8 )
	{
		SDL_SetColors( img, palette, 0, 256 );
		if( h.colorkeyed )
			SDL_SetColorKey( img, SDL_SRCCOLORKEY, h.colorkey );
	}

	fclose(f);
	return img;
}


/* Annoying: SDL_MapRGB will do a nearest-match if the specified color isn't found.
 * This breaks color keyed images that don't actually use the color key. */
int mySDL_MapRGBExact(SDL_PixelFormat *fmt, Uint8 R, Uint8 G, Uint8 B)
{
	Uint32 color = SDL_MapRGB(fmt, R, G, B);

	if( fmt->BitsPerPixel == 8 ) {
		if(fmt->palette->colors[color].r != R ||
		   fmt->palette->colors[color].g != G ||
		   fmt->palette->colors[color].b != B )
			return -1;
	}

	return color;
}

inline static float scale(float x, float l1, float h1, float l2, float h2)
{
	return ((x - l1) / (h1 - l1) * (h2 - l2) + l2);
}

inline void mySDL_GetRawRGBAV_XY(const SDL_Surface *src, Uint8 *v, int x, int y)
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
				Uint32 sum = 0;
				sum += Uint8(v[0][i] * (1-weight_x) * (1-weight_y));
				sum += Uint8(v[1][i] * (1-weight_x) * (weight_y));
				sum += Uint8(v[2][i] * (weight_x)   * (1-weight_y));
				sum += Uint8(v[3][i] * (weight_x)   * (weight_y));
				out[i] = (Uint8) clamp(sum, 0u, 255u);
			}

			mySDL_SetRawRGBAV(dstpx, dst, out);

			dstpx += dst->format->BytesPerPixel;
		}
	}
}

/* Templated blitter.  This has a couple advantages:
 *
 * The generic implementations will see the parameters at compile-time, and
 * the compiler can elide them completely, giving a smaller loop.
 *
 * Also, we can enable and disable specializations simply by commenting out
 * a blit function; if it's not there, it'll fall back on the generic version
 * automatically. 
 *
 * This is not yet intended for widespread use; it's currently only used in
 * the RageDisplay Create/UpdateTexture functions.  I wrote this because
 * I'm tired of fighting with SDL's blit.  This can replace the SDL blits
 * completely after the release, but that'll take a lot of testing.
 *
 * Simplified:
 *
 * No source alpha.
 * Palette -> palette blits assume the palette is identical (no mapping).
 * Color key removal controlled by a parameter, not a flag.
 * No general blitting rects.
 */
struct blit_traits_true {  enum { val = true };  };
struct blit_traits_false { enum { val = false }; };
enum { IDENTITY, DIFFERENT_RGBA, PAL_TO_RGBA };
struct blit_traits_identity { enum { convert = IDENTITY }; };
/* Nonidentical RGBA->RGBA; convert. */
struct blit_traits_rescale { enum { convert = DIFFERENT_RGBA }; };
/* PAL->RGBA; convert. */
struct blit_traits_depallete { enum { convert = PAL_TO_RGBA }; };

template<class ckey,class blit_traits>
static void blit( SDL_Surface *src_surf, const SDL_Surface *dst_surf, int width, int height );

template<>
static void blit<blit_traits_false,blit_traits_identity>( SDL_Surface *src_surf, const SDL_Surface *dst_surf, int width, int height )
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

/* Rescaling blit with no ckey.  This is what gets used to update moveis in
 * D3D, so optimization is very important. */
template<>
static void blit<blit_traits_false,blit_traits_rescale>( SDL_Surface *src_surf, const SDL_Surface *dst_surf, int width, int height )
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

template<class ckey,class blit_traits>
static void blit( SDL_Surface *src_surf, const SDL_Surface *dst_surf, int width, int height )
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

			if( ckey::val == true )
			{
				/* dst_surf->format->BytesPerPixel is not 1 (not paletted). 
				 * We don't check this, for efficiency. */
				// ASSERT(dst_surf->format->BytesPerPixel != 1);

				if( pixel == src_surf->format->colorkey )
				{
					pixel = 0;

					/* Skip the conversion. */
					goto skip_convert;
				}
			}

			if( blit_traits::convert )
			{
				Uint8 colors[4];
				if( blit_traits::convert == DIFFERENT_RGBA )
				{
					/* Convert pixel to the destination RGBA. */
					mySDL_GetRGBAV(pixel, src_surf, colors);
				} else if( blit_traits::convert == PAL_TO_RGBA ) {
					/* Convert pixel to the destination RGBA. */
					colors[0] = src_surf->format->palette->colors[pixel].r;
					colors[1] = src_surf->format->palette->colors[pixel].g;
					colors[2] = src_surf->format->palette->colors[pixel].b;
					colors[3] = 0xFF;
				}
				pixel = mySDL_SetRGBAV(dst_surf->format, colors);
			}
skip_convert:

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
		if(src->format->BitsPerPixel != 8 && ckey)
			blit<blit_traits_true,blit_traits_identity>(src, dst, width, height);
		else
			blit<blit_traits_false,blit_traits_identity>(src, dst, width, height);
	}

	else if( src->format->BytesPerPixel != 1 && dst->format->BytesPerPixel != 1 )
	{
		/* RGBA->RGBA with different formats. */
		if(ckey)
			blit<blit_traits_true,blit_traits_rescale>(src, dst, width, height);
		else
			blit<blit_traits_false,blit_traits_rescale>(src, dst, width, height);
	}

	else if( src->format->BytesPerPixel == 1 && dst->format->BytesPerPixel != 1 )
	{
		/* RGBA->PAL. */
		if(ckey)
			blit<blit_traits_true,blit_traits_depallete>(src, dst, width, height);
		else
			blit<blit_traits_false,blit_traits_depallete>(src, dst, width, height);
	}
	else
		/* We don't do RGBA->PAL. */
		ASSERT(0);
}
