#include "global.h"
#include "RageSurface_Load_PNG.h"
#include "RageUtil.h"
#include "RageLog.h"
#include "SDL_utils.h"
#include "SDL_endian.h"
#include "RageFile.h"

#if defined(WIN32)
#include "SDL_image-1.2/VisualC/graphics/include/png.h"
#pragma comment(lib, "SDL_image-1.2/VisualC/graphics/lib/libpng.lib")
#pragma warning(disable: 4611) /* interaction between '_setjmp' and C++ object destruction is non-portable */
#else
#include <png.h>
#endif

/*
 * The png_jmpbuf() macro, used in error handling, became available in
 * libpng version 1.0.6.  If you want to be able to run your code with older
 * versions of libpng, you must define the macro yourself (but only if it
 * is not already defined by libpng!).
 */
#ifndef png_jmpbuf
#define png_jmpbuf(png) ((png)->jmpbuf)
#endif

static void RageFile_png_read( png_struct *png, png_byte *p, png_size_t size )
{
	RageFile *f = (RageFile *) png->io_ptr;

	int got = f->Read( p, size );
	if( got == -1 )
		png_error( png, f->GetError() );
	else if( got != (int) size )
		png_error( png, "Unexpected EOF" );
}

struct error_info
{
	char *err;
	const char *fn;
};

static void PNG_Error( png_struct *png, const char *error )
{
	error_info *info = (error_info *) png->error_ptr;
	strncpy( info->err, error, 1024 );
	info->err[1023] = 0;
	LOG->Trace( "loading \"%s\": err: %s", info->fn, info->err );
	longjmp( png_jmpbuf(png), 1 );
}

static void PNG_Warning( png_struct *png, const char *warning )
{
	error_info *info = (error_info *) png->error_ptr;
	LOG->Trace( "loading \"%s\": warning: %s", info->fn, warning );
}

/* Since libpng forces us to use longjmp (gross!), this function shouldn't create any C++
 * objects, and needs to watch out for memleaks. */
static SDL_Surface *RageSurface_Load_PNG( RageFile *f, const char *fn, char errorbuf[1024] )
{
	error_info error;
	error.err = errorbuf;
	error.fn = fn;

	png_struct *png = png_create_read_struct( PNG_LIBPNG_VER_STRING, &error, PNG_Error, PNG_Warning );
	if( png == NULL )
	{
		sprintf( errorbuf, "creating png_create_read_struct failed");
		return NULL;
	}

	png_info *info_ptr = png_create_info_struct(png);
	if( info_ptr == NULL )
	{
		png_destroy_read_struct( &png, NULL, NULL );
		sprintf( errorbuf, "creating png_create_info_struct failed");
		return NULL;
	}

	if( setjmp(png_jmpbuf(png)) )
	{
		png_destroy_read_struct( &png, &info_ptr, png_infopp_NULL );
		return NULL;
	}

	png_set_read_fn( png, f, RageFile_png_read );

	png_read_info( png, info_ptr );

	png_uint_32 width, height;
	int bit_depth, color_type;
	png_get_IHDR( png, info_ptr, &width, &height, &bit_depth, &color_type, NULL, NULL, NULL );

	png_set_strip_16(png); /* 16bit->8bit */
	png_set_packing( png ); /* 1,2,4 bit->8 bit */

	/* Expand grayscale images to the full 8 bits from 1, 2, or 4 bits/pixel */
	if( color_type == PNG_COLOR_TYPE_GRAY && bit_depth < 8 )
		png_set_gray_1_2_4_to_8( png );

	/* These are set for type == PALETTE. */
	SDL_Color colors[256];
	int iColorKey = -1;

	/* We import three types of files: paletted, RGBX and RGBA.  The only difference
	 * between RGBX and RGBA is that RGBX won't set the alpha mask, so it's easier
	 * to tell later on that there's no alpha (without actually having to do a pixel scan). */
	enum { PALETTE, RGBX, RGBA } type;
	switch( color_type )
	{
	case PNG_COLOR_TYPE_GRAY:
		/* Fake PNG_COLOR_TYPE_GRAY. */
		for( int i = 0; i < 256; ++i )
		{
			colors[i].r = colors[i].g = colors[i].b = (int8_t) i;
			colors[i].unused = 0xFF;
		}

		type = PALETTE;
		break;

	case PNG_COLOR_TYPE_GRAY_ALPHA: 
		type = RGBA;
		png_set_gray_to_rgb( png );
		break;
	case PNG_COLOR_TYPE_PALETTE:
		type = PALETTE;
		break;
	case PNG_COLOR_TYPE_RGB:
		type = RGBX;
		break;
	case PNG_COLOR_TYPE_RGB_ALPHA:
		type = RGBA;
		break;
	default:
		FAIL_M(ssprintf( "%i", color_type) );
	}

	if( color_type == PNG_COLOR_TYPE_GRAY )
	{
		png_color_16 *trans;
		if( png_get_tRNS( png, info_ptr, NULL, NULL, &trans ) == PNG_INFO_tRNS )
			iColorKey = trans->gray;
	}
	else if( color_type != PNG_COLOR_TYPE_PALETTE )
	{
		/* If we have RGB image and tRNS, it's a color key.  Just convert it to RGBA. */
		if( png_get_valid(png, info_ptr, PNG_INFO_tRNS) )
		{
			/* We don't care about RGB color keys; just convert them to alpha. */
			png_set_tRNS_to_alpha( png );
			type = RGBA;
		}

		/* RGB->RGBX */
		png_set_filler( png, 0xff, PNG_FILLER_AFTER );
	}
	else if( color_type == PNG_COLOR_TYPE_PALETTE )
	{
		int num_palette;
		png_color *palette;
		int ret = png_get_PLTE( png, info_ptr, &palette, &num_palette );
		ASSERT( ret == PNG_INFO_PLTE );

		png_byte *trans = NULL;
		int num_trans = 0;
		png_get_tRNS( png, info_ptr, &trans, &num_trans, NULL );

		for( int i = 0; i < num_palette; ++i )
		{
			colors[i].r = palette[i].red;
			colors[i].g = palette[i].green;
			colors[i].b = palette[i].blue;
			colors[i].unused = 0xFF;
			if( i < num_trans )
				colors[i].unused = trans[i];
		}
	}

	png_read_update_info( png, info_ptr );

	SDL_Surface *surf;
	switch( type )
	{
	case PALETTE:
		surf = SDL_CreateRGBSurfaceSane( SDL_SWSURFACE, width, height, 8, 0, 0, 0, 0 );
		mySDL_SetPalette( surf, colors, 0, 256 );

		if( iColorKey != -1 )
			mySDL_AddColorKey( surf, iColorKey );

		break;
	case RGBX:
	case RGBA:
		surf = SDL_CreateRGBSurfaceSane( SDL_SWSURFACE, width, height, 32,
				SDL_SwapBE32( 0xFF000000 ),
				SDL_SwapBE32( 0x00FF0000 ),
				SDL_SwapBE32( 0x0000FF00 ),
				SDL_SwapBE32( type == RGBA? 0x000000FF:0x00000000 ) );
		break;
	default:
		FAIL_M(ssprintf( "%i", type) );
	}
	ASSERT( surf );

	/* alloca to prevent memleaks if libpng longjmps us */
	png_byte **row_pointers = (png_byte **) alloca( sizeof(png_byte*) * height );

	for( unsigned y = 0; y < height; ++y )
	{
		png_byte *p = (png_byte *) surf->pixels;
		row_pointers[y] = p + surf->pitch*y;
	}

	png_read_image( png, row_pointers );
	png_read_end( png, info_ptr );
	png_destroy_read_struct( &png, &info_ptr, png_infopp_NULL );

	return surf;
}


SDL_Surface *RageSurface_Load_PNG( const CString &sPath, CString &error )
{
	RageFile f;
	if( !f.Open( sPath ) )
	{
		error = f.GetError();
		return NULL;
	}

	char errorbuf[1024];
	SDL_Surface *ret = RageSurface_Load_PNG( &f, sPath, errorbuf );
	if( ret == NULL )
	{
		error = errorbuf;
		return ret;
	}

	return ret;
}

/*
 * (c) 2004 Glenn Maynard
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
