#ifndef SM_SDL_UTILS
#define SM_SDL_UTILS 1

/* Hack to prevent X includes from messing with our namespace: */
#define Font X11___Font
#define Screen X11___Screen
#include "SDL.h"
#include "SDL_syswm.h"          // for SDL_SysWMinfo
#undef Font
#undef Screen

Uint32 mySDL_Swap24(Uint32 x);
#if SDL_BYTEORDER == SDL_LIL_ENDIAN
#define mySDL_SwapLE24(n) (n)
#define mySDL_SwapBE24(n) (mySDL_Swap24(n))
#else
#define mySDL_SwapLE24(n) (mySDL_Swap24(n))
#define mySDL_SwapBE24(n) (n)
#endif

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
void mySDL_SetPalette(SDL_Surface *dst, SDL_Color *colors, int start, int cnt);
void CopySDLSurface( SDL_Surface *src, SDL_Surface *dest );
bool ConvertSDLSurface( SDL_Surface *src, SDL_Surface *&dst,
		int width, int height, int bpp,
		Uint32 R, Uint32 G, Uint32 B, Uint32 A );
void ConvertSDLSurface(SDL_Surface *&image,
		int width, int height, int bpp,
		Uint32 R, Uint32 G, Uint32 B, Uint32 A);
SDL_Surface *SDL_CreateRGBSurfaceSane
			(Uint32 flags, int width, int height, int depth, 
			Uint32 Rmask, Uint32 Gmask, Uint32 Bmask, Uint32 Amask);

void FixHiddenAlpha(SDL_Surface *img);

/* Check for an event; return true if one was waiting. */
bool SDL_GetEvent(SDL_Event &event, int mask);

/* Change the event state without dropping extra events. */
Uint8 mySDL_EventState(Uint8 type, int state);

void mySDL_GetAllEvents(vector<SDL_Event> &events);
void mySDL_PushEvents(vector<SDL_Event> &events);

/* The surface contains no transparent pixels and/or never uses its color
 * key, so it doesn't need any alpha bits at all. */
#define TRAIT_NO_TRANSPARENCY   0x0001 /* 0alpha */
/* The surface contains only transparent values of 0 or 1; no translucency.
 * It only needs one bit of alpha. */
#define TRAIT_BOOL_TRANSPARENCY 0x0002 /* 1alpha */
int FindSurfaceTraits(const SDL_Surface *img);


void mySDL_WM_SetIcon( CString sIconFile );

bool mySDL_SaveSurface( SDL_Surface *img, CString file );
SDL_Surface *mySDL_LoadSurface( CString file );

int mySDL_MapRGBExact(SDL_PixelFormat *fmt, Uint8 R, Uint8 G, Uint8 B);

void mySDL_BlitTransform( const SDL_Surface *src, SDL_Surface *dst, 
					const float fCoords[8] /* TL, BR, BL, TR */ );
void mySDL_BlitSurface( 
	SDL_Surface *src, SDL_Surface *dst, int width, int height, bool ckey);

/* Use the "unused1" field in surfaces to mark paletted surfaces that only use
 * 16 palette values. */
enum { FOUR_BIT_PALETTE = 0x1 };

SDL_Surface *mySDL_Palettize( SDL_Surface *src_surf, int GrayBits, int AlphaBits );

SDL_Surface *SDL_LoadImage( const CString &sPath );

SDL_RWops *OpenRWops( const CString &sPath, bool Write=false );
SDL_RWops *OpenRWops( CString &sBuf );
SDL_Surface *mySDL_MakeDummySurface( int height, int width );

#endif

