#ifndef UTILS_H
#define UTILS_H

inline float truncf( float f )	{ return float(int(f)); };
inline float roundf( float f )	{ if(f < 0) return truncf(f-0.5f); return truncf(f+0.5f); };

inline long int lrintf( float f )
{
	int retval;
	
	_asm fld f;
	_asm fistp retval;

	return retval;
}

struct Surface
{
	Surface() { pRGBA = NULL; }
	~Surface() { delete [] pRGBA; }
	Surface( const Surface &cpy );
	int iWidth;
	int iHeight;
	int iPitch;
	unsigned char *pRGBA;
};

void BitmapToSurface( HBITMAP hBitmap, Surface *pSurf );
void GrayScaleToAlpha( Surface *pSurf );
void GetBounds( const Surface *pSurf, RECT *out );

bool SavePNG( FILE *f, char szErrorbuf[1024], const Surface *pSurf );

#endif

/*
 * Copyright (c) 2003-2007 Glenn Maynard
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
