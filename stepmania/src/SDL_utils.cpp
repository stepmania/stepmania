/*
 * Various small SDL tools.
 * 
 * Copyright (c) 2002 by the person(s) listed below.  All rights reserved.
 *	Glenn Maynard
 * 
 * Portions from SDL source and documentation.
 */

#include "stdafx.h"

#include "SDL.h"
#include "SDL_utils.h"
#include "SDL_endian.h"

#if defined(WIN32)

/* Pull in all of our SDL libraries here. */
#ifdef _DEBUG
#pragma comment(lib, "SDL-1.2.5/lib/SDLd.lib")
#else
#pragma comment(lib, "SDL-1.2.5/lib/SDL.lib")
#endif

#pragma comment(lib, "SDL_image-1.2/SDL_image.lib")

#endif

Uint32 mySDL_Swap24(Uint32 x)
{
	return SDL_Swap32(x) >> 8; // xx223344 -> 443322xx -> 00443322
}

#if SDL_BYTEORDER == SDL_BIG_ENDIAN
#define mySDL_SwapLE24(x) mySDL_Swap24(x)
#else
#define mySDL_SwapLE24(x) (x)
#endif

/* These conditionals in the inner loop are slow.  Templates? */
Uint32 decodepixel(const Uint8 *p, int bpp)
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

void ConvertSDLSurface(SDL_Surface *&image,
		int width, int height, int bpp,
		Uint32 R, Uint32 G, Uint32 B, Uint32 A)
{
    SDL_Surface *ret_image = SDL_CreateRGBSurfaceSane(
            SDL_SWSURFACE, width, height, bpp, R, G, B, A);
	ASSERT(ret_image != NULL);

	/* If the formats are the same, no conversion is needed. */
	if(width == image->w && height == image->h &&
	   !memcmp(image->format, ret_image->format, sizeof(SDL_PixelFormat)))
	{
		SDL_FreeSurface(ret_image);
		return;
	}

	if(!(image->flags & SDL_SRCCOLORKEY)) // XXX until HOT PINK color keys are gone
		SDL_SetColorKey( image, SDL_SRCCOLORKEY, SDL_MapRGB(image->format, 0xFF, 0, 0xFF));

	/* We don't want to actually blend the alpha channel over the destination converted
	 * surface; we want to simply blit it, so make sure SDL_SRCALPHA is not on. */
	SDL_SetAlpha( image, 0, SDL_ALPHA_OPAQUE );

	SDL_Rect area;
	area.x = area.y = 0;
	area.w = short(image->w);
	area.h = short(image->h);

	SDL_BlitSurface(image, &area, ret_image, &area);
	SDL_FreeSurface(image);
	image = ret_image;
}

/* With d3d, textures are stored little endian (local endian for x86).
 *
 * I'm not sure if we should store textures in big endian, little endian
 * or local endian with OpenGL.  It doesn't really impact anything except
 * the actual code that loads the texture itself, and OpenGL does have
 * byte order toggles, so maybe we can get rid of this. */
SDL_Surface *SDL_CreateRGBSurfaceSane
			(Uint32 flags, int width, int height, int depth, 
			Uint32 Rmask, Uint32 Gmask, Uint32 Bmask, Uint32 Amask)
{
	/* This is untested on big-endian machines. */
	if(depth == 16) {
		Rmask = SDL_SwapLE16((Uint16)Rmask);
		Gmask = SDL_SwapLE16((Uint16)Gmask);
		Bmask = SDL_SwapLE16((Uint16)Bmask);
		Amask = SDL_SwapLE16((Uint16)Amask);
	} else if(depth == 24) {  // completely untested
		Rmask = mySDL_SwapLE24(Rmask);
		Gmask = mySDL_SwapLE24(Gmask);
		Bmask = mySDL_SwapLE24(Bmask);
		Amask = mySDL_SwapLE24(Amask);
	} else if(depth == 32) {
		Rmask = SDL_SwapLE32(Rmask);
		Gmask = SDL_SwapLE32(Gmask);
		Bmask = SDL_SwapLE32(Bmask);
		Amask = SDL_SwapLE32(Amask);
	}

	return SDL_CreateRGBSurface(flags, width, height, depth,
		Rmask, Gmask, Bmask, Amask);
}
