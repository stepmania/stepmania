#ifndef SM_SDL_UTILS
#define SM_SDL_UTILS 1

#include "SDL.h"

Uint32 decodepixel(const Uint8 *p, int bpp);
void encodepixel(Uint8 *p, int bpp, Uint32 pixel);

void mySDL_GetRawRGBAV(Uint32 pixel, const SDL_Surface *src, Uint8 *v);
void mySDL_GetRawRGBAV(const Uint8 *p, const SDL_Surface *src, Uint8 *v);

Uint32 mySDL_SetRawRGBAV(const SDL_PixelFormat *fmt, const Uint8 *v);
void mySDL_SetRawRGBAV(Uint8 *p, const SDL_Surface *src, const Uint8 *v);

void mySDL_GetBitsPerChannel(const SDL_PixelFormat *fmt, Uint32 bits[4]);
void ConvertSDLSurface(SDL_Surface *&image,
		int width, int height, int bpp,
		Uint32 R, Uint32 G, Uint32 B, Uint32 A);
SDL_Surface *SDL_CreateRGBSurfaceSane
			(Uint32 flags, int width, int height, int depth, 
			Uint32 Rmask, Uint32 Gmask, Uint32 Bmask, Uint32 Amask);

void SetAlphaRGB(SDL_Surface *img, Uint8 r, Uint8 g, Uint8 b);

#endif

