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

Uint32 mySDL_SetRawRGBAV(const SDL_PixelFormat *fmt, const Uint8 *v);
void mySDL_SetRawRGBAV(Uint8 *p, const SDL_Surface *src, const Uint8 *v);

void mySDL_GetBitsPerChannel(const SDL_PixelFormat *fmt, Uint32 bits[4]);
void ConvertSDLSurface(SDL_Surface *&image,
		int width, int height, int bpp,
		Uint32 R, Uint32 G, Uint32 B, Uint32 A);
SDL_Surface *SDL_CreateRGBSurfaceSane
			(Uint32 flags, int width, int height, int depth, 
			Uint32 Rmask, Uint32 Gmask, Uint32 Bmask, Uint32 Amask);

void FixHiddenAlpha(SDL_Surface *img);

struct char_traits_Sint32: public char_traits<Sint32>
{
	static Sint32 *copy(Sint32 *s, const Sint32 *p, size_t n)
	{
		memcpy(s, p, n * sizeof(Sint32));
		return s;
	}
	static Sint32 *move(Sint32 *s, const Sint32 *p, size_t n)
	{
		memmove(s, p, n * sizeof(Sint32));
		return s;
	}
	static Sint32 *assign(Sint32 *s, unsigned cnt, int n)
	{
		while(cnt--) *(s++) = n;
		return s;
	}
	static void assign(Sint32 &c1, Sint32 c2)
	{
		c1 = c2;
	}
};

#endif

