#include "global.h"
#include "SDL.h"
#include "SDL_utils.h"

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
typedef struct {
	struct jpeg::jpeg_destination_mgr pub;

	SDL_RWops *ctx;
	Uint8 buffer[OUTPUT_BUFFER_SIZE];
} my_destination_mgr;


/*
 * Initialize source --- called by jpeg_read_header
 * before any data is actually read.
 */
static void init_destination (jpeg::j_compress_ptr cinfo)
{
	/* We don't actually need to do anything */
	return;
}

/*
 * Empty the output buffer --- called whenever buffer is full.
 */
static jpeg::boolean empty_output_buffer (jpeg::j_compress_ptr cinfo)
{
	my_destination_mgr * dest = (my_destination_mgr *) cinfo->dest;
	int nbytes;

	nbytes = SDL_RWwrite(dest->ctx, dest->buffer, 1, OUTPUT_BUFFER_SIZE);
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
	my_destination_mgr * dest = (my_destination_mgr *) cinfo->dest;
	int nbytes;

	nbytes = SDL_RWwrite(dest->ctx, dest->buffer, 1, OUTPUT_BUFFER_SIZE - dest->pub.free_in_buffer);
	dest->pub.next_output_byte = dest->buffer;
	dest->pub.free_in_buffer = OUTPUT_BUFFER_SIZE;

	return;
}

/*
 * Prepare for output to a stdio stream.
 * The caller must have already opened the stream, and is responsible
 * for closing it after finishing decompression.
 */
static void jpeg_SDL_RW_dest(jpeg::j_compress_ptr cinfo, SDL_RWops *ctx)
{
  my_destination_mgr *dest;

  /* The source object and input buffer are made permanent so that a series
   * of JPEG images can be read from the same file by calling jpeg_stdio_src
   * only before the first one.  (If we discarded the buffer at the end of
   * one image, we'd likely lose the start of the next one.)
   * This makes it unsafe to use this manager and a different source
   * manager serially with the same JPEG object.  Caveat programmer.
   */
  if( cinfo->dest == NULL )
  {
	  /* first time for this JPEG object? */
    cinfo->dest = (struct jpeg::jpeg_destination_mgr *)
      (*cinfo->mem->alloc_small) ((jpeg::j_common_ptr) cinfo, JPOOL_PERMANENT,
				  sizeof(my_destination_mgr));
    dest = (my_destination_mgr *) cinfo->dest;
  }

  dest = (my_destination_mgr *) cinfo->dest;
  dest->pub.init_destination = init_destination;
  dest->pub.empty_output_buffer = empty_output_buffer;
  dest->pub.term_destination = term_destination;
  dest->ctx = ctx;
  dest->pub.free_in_buffer = OUTPUT_BUFFER_SIZE; /* forces fill_input_buffer on first read */
  dest->pub.next_output_byte = dest->buffer; /* until buffer loaded */
}

/* Save a JPEG to a file.  cjpeg.c and example.c from jpeglib were helpful in writing this. */
void IMG_SaveJPG_RW( SDL_Surface *surface, SDL_RWops *dest, bool bHighQual )
{
	SDL_Surface *dst_surface;
	if( ConvertSDLSurface( surface, dst_surface,
		surface->w, surface->h, 24, mySDL_SwapBE24(0xFF0000), mySDL_SwapBE24(0x00FF00), mySDL_SwapBE24(0x0000FF), 0 ) )
		surface = dst_surface;

  struct jpeg::jpeg_compress_struct cinfo;

  /* Step 1: allocate and initialize JPEG compression object */

  /* We have to set up the error handler first, in case the initialization
   * step fails.  (Unlikely, but it could happen if you are out of memory.)
   * This routine fills in the contents of struct jerr, and returns jerr's
   * address which we place into the link field in cinfo.
   */
  struct jpeg::jpeg_error_mgr jerr;
  cinfo.err = jpeg::jpeg_std_error(&jerr);

  /* Now we can initialize the JPEG compression object. */
  jpeg::jpeg_CreateCompress(&cinfo, JPEG_LIB_VERSION, \
                        (size_t) sizeof(struct jpeg::jpeg_compress_struct));

  /* Step 2: specify data destination (eg, a file) */
  /* Note: steps 2 and 3 can be done in either order. */

  /* Here we use the library-supplied code to send compressed data to a
   * stdio stream.  You can also write your own code to do something else.
   * VERY IMPORTANT: use "b" option to fopen() if you are on a machine that
   * requires it in order to write binary files.
   */
  jpeg_SDL_RW_dest(&cinfo, dest);

  /* Step 3: set parameters for compression */

  /* First we supply a description of the input image.
   * Four fields of the cinfo struct must be filled in:
   */
  cinfo.image_width = surface->w; 	/* image width and height, in pixels */
  cinfo.image_height = surface->h;
  cinfo.input_components = 3;		/* # of color components per pixel */
  cinfo.in_color_space = jpeg::JCS_RGB; 	/* colorspace of input image */
  /* Now use the library's routine to set default compression parameters.
   * (You must set at least cinfo.in_color_space before calling this,
   * since the defaults depend on the source color space.)
   */
  jpeg::jpeg_set_defaults(&cinfo);
  if( bHighQual )
	jpeg::jpeg_set_quality( &cinfo, 150, TRUE );
  else
	jpeg::jpeg_set_quality( &cinfo, 40, TRUE );

  /* Step 4: Start compressor */

  /* TRUE ensures that we will write a complete interchange-JPEG file.
   * Pass TRUE unless you are very sure of what you're doing.
   */
  jpeg::jpeg_start_compress(&cinfo, TRUE);

  /* Step 5: while (scan lines remain to be written) */
  /*           jpeg_write_scanlines(...); */

  /* Here we use the library's state variable cinfo.next_scanline as the
   * loop counter, so that we don't have to keep track ourselves.
   * To keep things simple, we pass one scanline per call; you can pass
   * more if you wish, though.
   */
  const int row_stride = surface->pitch;	/* JSAMPLEs per row in image_buffer */

  while( cinfo.next_scanline < cinfo.image_height )
  {
    /* jpeg_write_scanlines expects an array of pointers to scanlines.
     * Here the array is only one element long, but you could pass
     * more than one scanline at a time if that's more convenient.
     */
    jpeg::JSAMPROW row_pointer = & ((jpeg::JSAMPLE*)surface->pixels)[cinfo.next_scanline * row_stride];
    jpeg::jpeg_write_scanlines(&cinfo, &row_pointer, 1);
  }

  /* Step 6: Finish compression */

  jpeg::jpeg_finish_compress(&cinfo);
  /* After finish_compress, we can close the output file. */

  /* Step 7: release JPEG compression object */

  /* This is an important step since it will release a good deal of memory. */
  jpeg::jpeg_destroy_compress(&cinfo);

  /* And we're done! */
	if( dst_surface )
		SDL_FreeSurface( dst_surface );
}

