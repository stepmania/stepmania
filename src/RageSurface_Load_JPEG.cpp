#include "global.h"
#include "RageSurface_Load_JPEG.h"
#include "RageUtil.h"
#include "RageLog.h"
#include "RageFile.h"
#include "RageSurface.h"

#include <setjmp.h>

// Don't let jpeglib.h define the boolean type on Xbox.
#if defined(_XBOX)
#  define HAVE_BOOLEAN
#endif

#if defined(WIN32)
/* work around namespace bugs in win32/libjpeg: */
#define XMD_H
#undef FAR
#include "libjpeg/jpeglib.h"
#include "libjpeg/jerror.h"

#if defined(_MSC_VER)
#if !defined(XBOX)
#pragma comment(lib, "libjpeg/jpeg.lib")
#else
#pragma comment(lib, "libjpeg/xboxjpeg.lib")
#endif
#endif

#pragma warning(disable: 4611) /* interaction between '_setjmp' and C++ object destruction is non-portable */
#else
extern "C" {
#include <jpeglib.h>
#include <jerror.h>
}
#endif

struct my_jpeg_error_mgr
{
  struct jpeg_error_mgr pub;    /* "public" fields */

  jmp_buf setjmp_buffer;        /* for return to caller */
  char errorbuf[JMSG_LENGTH_MAX];
};


void my_output_message( j_common_ptr cinfo )
{
	my_jpeg_error_mgr *myerr = (my_jpeg_error_mgr *) cinfo->err;
	(*cinfo->err->format_message)( cinfo, myerr->errorbuf );
}


void my_error_exit( j_common_ptr cinfo )
{
	my_jpeg_error_mgr *myerr = (my_jpeg_error_mgr *) cinfo->err;
	(*cinfo->err->output_message)(cinfo);

	longjmp( myerr->setjmp_buffer, 1 );
}

struct RageFile_source_mgr
{
	struct jpeg_source_mgr pub;   /* public fields */

	RageFile *file;		/* source stream */
	JOCTET buffer[1024*4];
	bool start_of_file;	/* have we gotten any data yet? */
};

void RageFile_JPEG_init_source( j_decompress_ptr cinfo )
{
	RageFile_source_mgr *src = (RageFile_source_mgr *) cinfo->src;
	src->start_of_file = true;
	src->pub.next_input_byte = NULL;
	src->pub.bytes_in_buffer = 0;
}

boolean RageFile_JPEG_fill_input_buffer( j_decompress_ptr cinfo )
{
	RageFile_source_mgr *src = (RageFile_source_mgr *) cinfo->src;
	size_t nbytes = src->file->Read( src->buffer, sizeof(src->buffer) );

	if( nbytes <= 0 )
	{
		if( src->start_of_file )     /* Treat empty input file as fatal error */
			ERREXIT( cinfo, JERR_INPUT_EMPTY );

		WARNMS( cinfo, JWRN_JPEG_EOF );

		/* Insert a fake EOI marker */
		src->buffer[0] = (JOCTET) 0xFF;
		src->buffer[1] = (JOCTET) JPEG_EOI;
		nbytes = 2;
	}

	src->pub.next_input_byte = src->buffer;
	src->pub.bytes_in_buffer = nbytes;
	src->start_of_file = FALSE;

	return TRUE;
}

void RageFile_JPEG_skip_input_data( j_decompress_ptr cinfo, long num_bytes )
{
	RageFile_source_mgr *src = (RageFile_source_mgr *) cinfo->src;

	int in_buffer = min( (long) src->pub.bytes_in_buffer, num_bytes );
	src->pub.next_input_byte += in_buffer;
	src->pub.bytes_in_buffer -= in_buffer;
	num_bytes -= in_buffer;

	if( num_bytes )
		src->file->Seek( src->file->Tell() + num_bytes );
}

void RageFile_JPEG_term_source( j_decompress_ptr cinfo )
{
}

static RageSurface *RageSurface_Load_JPEG( RageFile *f, const char *fn, char errorbuf[JMSG_LENGTH_MAX] )
{
	struct jpeg_decompress_struct cinfo;

	struct my_jpeg_error_mgr jerr;
	cinfo.err = jpeg_std_error(&jerr.pub);
	jerr.pub.error_exit = my_error_exit;
	jerr.pub.output_message = my_output_message;
	
	RageSurface *volatile img = NULL; /* volatile to prevent possible problems with setjmp */

	if( setjmp(jerr.setjmp_buffer) )
	{
		my_jpeg_error_mgr *myerr = (my_jpeg_error_mgr *) cinfo.err;
		memcpy( errorbuf, myerr->errorbuf, JMSG_LENGTH_MAX );
		
		jpeg_destroy_decompress( &cinfo );
		delete img;
		return NULL;
	}

	/* Now we can initialize the JPEG decompression object. */
	jpeg_create_decompress( &cinfo );

	/* Step 2: specify data source (eg, a file) */
	RageFile_source_mgr RageFileJpegSource;
	RageFileJpegSource.pub.init_source = RageFile_JPEG_init_source;
	RageFileJpegSource.pub.fill_input_buffer = RageFile_JPEG_fill_input_buffer;
	RageFileJpegSource.pub.skip_input_data = RageFile_JPEG_skip_input_data;
	RageFileJpegSource.pub.resync_to_restart = jpeg_resync_to_restart; /* use default method */
	RageFileJpegSource.pub.term_source = RageFile_JPEG_term_source;
	RageFileJpegSource.file = f;

	cinfo.src = (jpeg_source_mgr *) &RageFileJpegSource;

	jpeg_read_header( &cinfo, TRUE );

	switch( cinfo.jpeg_color_space )
	{
	case JCS_GRAYSCALE:
		cinfo.out_color_space = JCS_GRAYSCALE;
		break;

	case JCS_YCCK:
	case JCS_CMYK:
		sprintf( errorbuf, "Color format \"%s\" not supported", cinfo.jpeg_color_space == JCS_YCCK? "YCCK":"CMYK" );
		jpeg_destroy_decompress( &cinfo );
		return NULL;

	default:
		cinfo.out_color_space = JCS_RGB;
		break;
	}

	jpeg_start_decompress( &cinfo );

	if( cinfo.out_color_space == JCS_GRAYSCALE )
	{
		img = CreateSurface( cinfo.output_width, cinfo.output_height, 8, 0, 0, 0, 0 );

		for( int i = 0; i < 256; ++i )
		{
			RageSurfaceColor color;
			color.r = color.g = color.b = (int8_t) i;
			color.a = 0xFF;
			img->fmt.palette->colors[i] = color;
		}
	} else {
		img = CreateSurface( cinfo.output_width, cinfo.output_height, 24,
				Swap24BE( 0xFF0000 ),
				Swap24BE( 0x00FF00 ),
				Swap24BE( 0x0000FF ),
				Swap24BE( 0x000000 ) );
	}

	while( cinfo.output_scanline < cinfo.output_height )
	{
		JSAMPROW p = (JSAMPROW) img->pixels;
		p += cinfo.output_scanline * img->pitch;
		jpeg_read_scanlines(&cinfo, &p, 1);
	}

	jpeg_finish_decompress( &cinfo );
	jpeg_destroy_decompress( &cinfo );

	return img;
}


RageSurfaceUtils::OpenResult RageSurface_Load_JPEG( const RString &sPath, RageSurface *&ret, bool bHeaderOnly, RString &error )
{
	RageFile f;
	if( !f.Open( sPath ) )
	{
		error = f.GetError();
		return RageSurfaceUtils::OPEN_FATAL_ERROR;
	}

	char errorbuf[1024];
	ret = RageSurface_Load_JPEG( &f, sPath, errorbuf );
	if( ret == NULL )
	{
		error = errorbuf;
		return RageSurfaceUtils::OPEN_UNKNOWN_FILE_FORMAT; // XXX
	}

	return RageSurfaceUtils::OPEN_OK;
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
