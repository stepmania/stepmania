#include "global.h"
#include "RageSurface_Save_PNG.h"
#include "RageSurface.h"
#include "RageSurfaceUtils.h"
#include "RageFile.h"
#include "RageLog.h"
#include "RageUtil.h"

#if defined(WINDOWS) || defined(_XBOX)
#include "libpng/include/png.h"
#if defined(_MSC_VER)
#  if defined(_XBOX)
#    pragma comment(lib, "libpng/lib/xboxlibpng.lib")
#  else
#    pragma comment(lib, "libpng/lib/libpng.lib")
#  endif
#pragma warning(disable: 4611) /* interaction between '_setjmp' and C++ object destruction is non-portable */
#endif
#else
#include <png.h>
#endif

static void SafePngError( png_struct *pPng, const RString &sStr )
{
	/* png_error will call PNG_Error, which will longjmp.  If we just pass
	 * GetError().c_str() to it, a temporary may be created; since control
	 * never returns, it may never be destructed and leak. */
	static char error[256];
	strncpy( error, sStr, sizeof(error) );
	error[sizeof(error)-1] = 0;
	png_error( pPng, error );
}

static void RageFile_png_write( png_struct *pPng, png_byte *pData, png_size_t iSize )
{
	RageFile *pFile = (RageFile *) pPng->io_ptr;

	int iGot = pFile->Write( pData, iSize );
	if( iGot == -1 )
		SafePngError( pPng, pFile->GetError() );
}

static void RageFile_png_flush( png_struct *pPng )
{
	RageFile *pFile = (RageFile *) pPng->io_ptr;

	int iGot = pFile->Flush();
	if( iGot == -1 )
		SafePngError( pPng, pFile->GetError() );
}

struct error_info
{
	char *szErr;
};

static void PNG_Error( png_struct *pPng, const char *szError )
{
	error_info *pInfo = (error_info *) pPng->error_ptr;
	strncpy( pInfo->szErr, szError, 1024 );
	pInfo->szErr[1023] = 0;
	longjmp( pPng->jmpbuf, 1 );
}

static void PNG_Warning( png_struct *png, const char *warning )
{
	LOG->Trace( "saving PNG: warning: %s", warning );
}

/* Since libpng forces us to use longjmp, this function shouldn't create any C++
 * objects, and needs to watch out for memleaks. */
static bool RageSurface_Save_PNG( RageFile &f, char szErrorbuf[1024], RageSurface *pImgIn )
{
	RageSurface *pImg;
	bool bDeleteImg = RageSurfaceUtils::ConvertSurface( pImgIn, pImg, pImgIn->w, pImgIn->h, 32,
			Swap32BE( 0xFF000000 ),
			Swap32BE( 0x00FF0000 ),
			Swap32BE( 0x0000FF00 ),
			Swap32BE( 0x00000000 ) );
	if( !bDeleteImg )
		pImg = pImgIn;

	error_info error;
	error.szErr = szErrorbuf;

	png_struct *pPng = png_create_write_struct( PNG_LIBPNG_VER_STRING, &error, PNG_Error, PNG_Warning );
	if( pPng == NULL )
	{
		sprintf( szErrorbuf, "creating png_create_write_struct failed");
		return false;
	}

	png_info *pInfo = png_create_info_struct(pPng);
	if( pInfo == NULL )
	{
		png_destroy_read_struct( &pPng, NULL, NULL );
		if( bDeleteImg )
			delete pImg;
		sprintf( szErrorbuf, "creating png_create_info_struct failed");
		return false;
	}

	if( setjmp(pPng->jmpbuf) )
	{
		png_destroy_read_struct( &pPng, &pInfo, png_infopp_NULL );
		return false;
	}

	png_set_write_fn( pPng, &f, RageFile_png_write, RageFile_png_flush );
	png_set_compression_level( pPng, 1 );

	png_set_IHDR( pPng, pInfo, pImg->w, pImg->h, 8, PNG_COLOR_TYPE_RGB,
		PNG_INTERLACE_NONE, PNG_COMPRESSION_TYPE_BASE, PNG_FILTER_TYPE_BASE );

	png_write_info( pPng, pInfo );
	png_set_filler( pPng, 0, PNG_FILLER_AFTER );

	png_byte *pixels = (png_byte *) pImg->pixels;
	for( int y = 0; y < pImg->h; y++ )
		png_write_row( pPng, pixels + pImg->pitch*y );

	png_write_end( pPng, pInfo );
	png_destroy_write_struct( &pPng, &pInfo );

	/* Free the converted image. */
	if( bDeleteImg )
		delete pImg;

	return true;
}

bool RageSurfaceUtils::SavePNG( RageSurface *pImg, RageFile &f, RString &sError )
{
	char szErrorBuf[1024];
	if( !RageSurface_Save_PNG(f, szErrorBuf, pImg) )
	{
		sError = szErrorBuf;
		return false;
	}

	return true;
}

/*
 * (c) 2004-2006 Glenn Maynard
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
