#include "global.h"
#include "RageSurface.h"
#include "RageSurfaceUtils.h"
#include "RageUtil.h"
#include "RageFile.h"

#undef FAR /* fix for VC */
namespace jpeg
{
	extern "C"
	{
#include <jpeglib.h>
	}
}

#include "SDL_SaveJPEG.h"

/* Pull in JPEG library here. */
#ifdef _XBOX
	// FIXME
#elif defined _WINDOWS
#pragma comment(lib, "libjpeg/jpeg.lib")
#endif




#define OUTPUT_BUFFER_SIZE	4096
typedef struct
{
	struct jpeg::jpeg_destination_mgr pub;

	RageFile *f;
	uint8_t buffer[OUTPUT_BUFFER_SIZE];
} my_destination_mgr;


/*
 * Initialize source --- called by jpeg_read_header
 * before any data is actually read.
 */
static void init_destination( jpeg::j_compress_ptr cinfo )
{
	/* nop */
	return;
}

/* Empty the output buffer; called whenever buffer is full. */
static jpeg::boolean empty_output_buffer( jpeg::j_compress_ptr cinfo )
{
	my_destination_mgr * dest = (my_destination_mgr *) cinfo->dest;
	dest->f->Write( dest->buffer, OUTPUT_BUFFER_SIZE );
	// XXX err
	dest->pub.next_output_byte = dest->buffer;
	dest->pub.free_in_buffer = OUTPUT_BUFFER_SIZE;

	return TRUE;
}


/*
 * Terminate source --- called by jpeg_finish_decompress
 * after all data has been read.
 */
static void term_destination (jpeg::j_compress_ptr cinfo)
{
	/* Write data remaining in the buffer */
	my_destination_mgr *dest = (my_destination_mgr *) cinfo->dest;
	dest->f->Write( dest->buffer, OUTPUT_BUFFER_SIZE - dest->pub.free_in_buffer );
	// XXX err
	dest->pub.next_output_byte = dest->buffer;
	dest->pub.free_in_buffer = OUTPUT_BUFFER_SIZE;
}

/*
 * Prepare for output to a stdio stream.
 * The caller must have already opened the stream, and is responsible
 * for closing it after finishing decompression.
 */
static void jpeg_RageFile_dest( jpeg::j_compress_ptr cinfo, RageFile &f )
{
	ASSERT( cinfo->dest == NULL );

	cinfo->dest = (struct jpeg::jpeg_destination_mgr *)
		(*cinfo->mem->alloc_small) ( (jpeg::j_common_ptr) cinfo, JPOOL_PERMANENT,
			sizeof(my_destination_mgr) );

	my_destination_mgr *dest = (my_destination_mgr *) cinfo->dest;
	dest->pub.init_destination = init_destination;
	dest->pub.empty_output_buffer = empty_output_buffer;
	dest->pub.term_destination = term_destination;
	dest->pub.free_in_buffer = OUTPUT_BUFFER_SIZE; /* forces fill_input_buffer on first read */
	dest->pub.next_output_byte = dest->buffer; /* until buffer loaded */

	dest->f = &f;
}

/* Save a JPEG to a file.  cjpeg.c and example.c from jpeglib were helpful in writing this. */
bool IMG_SaveJPG_RW( RageSurface *surface, RageFile &f, bool bHighQual )
{
	RageSurface *dst_surface;
	if( RageSurfaceUtils::ConvertSurface( surface, dst_surface,
		surface->w, surface->h, 24, Swap24BE(0xFF0000), Swap24BE(0x00FF00), Swap24BE(0x0000FF), 0 ) )
		surface = dst_surface;

	struct jpeg::jpeg_compress_struct cinfo;

	/* Set up the error handler. */
	struct jpeg::jpeg_error_mgr jerr;
	cinfo.err = jpeg::jpeg_std_error( &jerr );

	/* Now we can initialize the JPEG compression object. */
	jpeg::jpeg_CreateCompress(&cinfo, JPEG_LIB_VERSION, \
		(size_t) sizeof(struct jpeg::jpeg_compress_struct));

	cinfo.image_width = surface->w; 	/* image width and height, in pixels */
	cinfo.image_height = surface->h;
	cinfo.input_components = 3;		/* # of color components per pixel */
	cinfo.in_color_space = jpeg::JCS_RGB; 	/* colorspace of input image */

	/* Set compression parameters.  You must set at least cinfo.in_color_space before
	 * calling this.*/
	jpeg::jpeg_set_defaults(&cinfo);

	if( bHighQual )
		jpeg::jpeg_set_quality( &cinfo, 150, TRUE );
	else
		jpeg::jpeg_set_quality( &cinfo, 70, TRUE );

	jpeg_RageFile_dest( &cinfo, f );

	/* Start the compressor. */
	jpeg::jpeg_start_compress( &cinfo, TRUE );

	/* Here we use the library's state variable cinfo.next_scanline as the
	 * loop counter, so that we don't have to keep track ourselves.
	 * To keep things simple, we pass one scanline per call; you can pass
	 * more if you wish, though. */
	const int row_stride = surface->pitch;	/* JSAMPLEs per row in image_buffer */

	while( cinfo.next_scanline < cinfo.image_height )
	{
		/* jpeg_write_scanlines expects an array of pointers to scanlines.
		 * Here the array is only one element long, but you could pass
		 * more than one scanline at a time if that's more convenient. */
		jpeg::JSAMPROW row_pointer = & ((jpeg::JSAMPLE*)surface->pixels)[cinfo.next_scanline * row_stride];
		jpeg::jpeg_write_scanlines( &cinfo, &row_pointer, 1 );
	}

	/* Finish compression. */
	jpeg::jpeg_finish_compress( &cinfo );
	jpeg::jpeg_destroy_compress( &cinfo );

	delete dst_surface;
	return true;
}

/*
 * (c) 2004 Chris Danford
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
