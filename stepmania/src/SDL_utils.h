#ifndef SM_SDL_UTILS
#define SM_SDL_UTILS 1

/* Hack to prevent X includes from messing with our namespace: */
#define Font X11___Font
#define Screen X11___Screen
#include "SDL.h"
#include "SDL_syswm.h"          // for SDL_SysWMinfo
#undef Font
#undef Screen

Uint32 decodepixel(const Uint8 *p, int bpp);
void encodepixel(Uint8 *p, int bpp, Uint32 pixel);

void mySDL_GetRawRGBAV(Uint32 pixel, const SDL_Surface *src, Uint8 *v);
void mySDL_GetRawRGBAV(const Uint8 *p, const SDL_Surface *src, Uint8 *v);
void mySDL_GetRGBAV(Uint32 pixel, const SDL_Surface *src, Uint8 *v);
void mySDL_GetRGBAV(const Uint8 *p, const SDL_Surface *src, Uint8 *v);

Uint32 mySDL_SetRawRGBAV(const SDL_PixelFormat *fmt, const Uint8 *v);
void mySDL_SetRawRGBAV(Uint8 *p, const SDL_Surface *src, const Uint8 *v);
Uint32 mySDL_SetRGBAV(const SDL_PixelFormat *fmt, const Uint8 *v);
void mySDL_SetRGBAV(Uint8 *p, const SDL_Surface *src, const Uint8 *v);

void mySDL_GetBitsPerChannel(const SDL_PixelFormat *fmt, Uint32 bits[4]);
void ConvertSDLSurface(SDL_Surface *&image,
		int width, int height, int bpp,
		Uint32 R, Uint32 G, Uint32 B, Uint32 A);
SDL_Surface *SDL_CreateRGBSurfaceSane
			(Uint32 flags, int width, int height, int depth, 
			Uint32 Rmask, Uint32 Gmask, Uint32 Bmask, Uint32 Amask);

void FixHiddenAlpha(SDL_Surface *img);

/* Check for an event; return true if one was waiting. */
bool SDL_GetEvent(SDL_Event &event, int mask);

/* The surface contains no transparent pixels and/or never uses its color
 * key, so it doesn't need any alpha bits at all. */
#define TRAIT_NO_TRANSPARENCY   0x0001 /* 0alpha */
/* The surface contains only transparent values of 0 or 1; no translucency.
 * It only needs one bit of alpha. */
#define TRAIT_BOOL_TRANSPARENCY 0x0002 /* 1alpha */
/* All opaque pixels in this image are completely white, so it doesn't need
 * any color bits at all; use a GL_ALPHA8 and only store the transparency. */
#define TRAIT_WHITE_ONLY		0x0004 /* 8alphaonly */
int FindSurfaceTraits(const SDL_Surface *img);

#endif

