/* MAD is available from: http://www.mars.org/home/rob/proj/mpeg/ */

#include "global.h"
#include "RageSoundReader_MP3.h"
#include "RageLog.h"
#include "RageUtil.h"
#include "SDL_utils.h"

#include <stdio.h>
#include <errno.h>

#ifdef _WIN32
#include "mad-0.15.0b/mad.h"
#ifdef _XBOX
// not set up
// #pragma comment(lib, "mad-0.15.0b/msvc++/Xbox_Release/libmad.lib")
#else
#pragma comment(lib, "mad-0.15.0b/msvc++/Release/libmad.lib")
#endif
#else
#include <mad.h>
#endif

/* ID3 code from libid3: */
enum tagtype {
  TAGTYPE_NONE = 0,
  TAGTYPE_ID3V1,
  TAGTYPE_ID3V2,
  TAGTYPE_ID3V2_FOOTER
};

typedef unsigned long id3_length_t;
static const int ID3_TAG_FLAG_FOOTERPRESENT = 0x10;

static tagtype tagtype( const unsigned char *data, id3_length_t length )
{
	if (length >= 3 &&
		data[0] == 'T' && data[1] == 'A' && data[2] == 'G')
		return TAGTYPE_ID3V1;

	if (length >= 10 &&
	    ((data[0] == 'I' && data[1] == 'D' && data[2] == '3') ||
	     (data[0] == '3' && data[1] == 'D' && data[2] == 'I')) &&
	    data[3] < 0xff && data[4] < 0xff &&
	    data[6] < 0x80 && data[7] < 0x80 && data[8] < 0x80 && data[9] < 0x80)
		return data[0] == 'I' ? TAGTYPE_ID3V2 : TAGTYPE_ID3V2_FOOTER;

	return TAGTYPE_NONE;
}

static unsigned long id3_parse_uint( const unsigned char **ptr, unsigned int bytes )
{
	unsigned long value = 0;

	ASSERT(bytes >= 1 && bytes <= 4);

	switch (bytes)
	{
	case 4: value = (value << 8) | *(*ptr)++;
	case 3: value = (value << 8) | *(*ptr)++;
	case 2: value = (value << 8) | *(*ptr)++;
	case 1: value = (value << 8) | *(*ptr)++;
	}

	return value;
}

static unsigned long id3_parse_syncsafe( const unsigned char **ptr, unsigned int bytes )
{
	unsigned long value = 0;

	ASSERT(bytes == 4 || bytes == 5);

	switch (bytes)
	{
	case 5:
		value = (value << 4) | (*(*ptr)++ & 0x0f);
	case 4:
		value = (value << 7) | (*(*ptr)++ & 0x7f);
		value = (value << 7) | (*(*ptr)++ & 0x7f);
		value = (value << 7) | (*(*ptr)++ & 0x7f);
		value = (value << 7) | (*(*ptr)++ & 0x7f);
	}

	return value;
}


static void parse_header(const unsigned char **ptr,
		  unsigned int *version, int *flags, id3_length_t *size)
{
	*ptr += 3;

	*version = id3_parse_uint(ptr, 2);
	*flags   = id3_parse_uint(ptr, 1);
	*size    = id3_parse_syncsafe(ptr, 4);
}

/*
 * NAME:	tag->query()
 * DESCRIPTION:	if a tag begins at the given location, return its size
 */
signed long id3_tag_query( const unsigned char *data, id3_length_t length )
{
	unsigned int version;
	int flags;
	id3_length_t size;

	switch (tagtype(data, length))
	{
	case TAGTYPE_ID3V1:
		return 128;

	case TAGTYPE_ID3V2:
		parse_header(&data, &version, &flags, &size);

		if (flags & ID3_TAG_FLAG_FOOTERPRESENT)
			size += 10;

		return 10 + size;

	case TAGTYPE_ID3V2_FOOTER:
		parse_header(&data, &version, &flags, &size);
		return -(int)size - 10;

	case TAGTYPE_NONE:
		break;
	}

	return 0;
}




/* XING code ripped out of madplay (header)
 *
 * mad - MPEG audio decoder
 * Copyright (C) 2000-2001 Robert Leslie
 */
struct xing
{
	long flags;					/* valid fields (see below) */
	unsigned long frames;		/* total number of frames */
	unsigned long bytes;		/* total number of bytes */
	unsigned char toc[100];		/* 100-point seek table */
	long scale;					/* ?? */
};

enum {
  XING_FRAMES = 0x00000001L,
  XING_BYTES  = 0x00000002L,
  XING_TOC    = 0x00000004L,
  XING_SCALE  = 0x00000008L
};

void xing_init( struct xing *xing );
int xing_parse( struct xing *xing, struct mad_bitptr ptr, unsigned int bitlen );

/* end XING header */


/* a -= b */
static void mad_timer_sub(mad_timer_t *a, mad_timer_t b)
{
	/* a = a - b -> a = a + -b */
	mad_timer_negate(&b);
	mad_timer_add(a, b);
}






/* internal->decoder_private field */
struct madlib_t
{
    Uint8 inbuf[16384], outbuf[8192];
    int outpos;
	unsigned outleft;
    int inbytes;

    struct mad_stream	Stream;
    struct mad_frame	Frame;
    struct mad_synth	Synth;
    /* Timestamp of the next frame. */
    mad_timer_t		Timer;

    /* Whether Timer is trusted; this is false after doing a quick seek. */
    int	timer_accurate;

    /* Frame index of each percentage of the file.  This is like the
     * Xing TOC, except it's actual byte indices, not percentages, so
     * it's accurate enough for "precise" seeking.  -1 indicates we don't
     * yet know; we always fill this in from beginning to end, so if n
     * is not -1, 0..n-1 are also not -1. 
     *
     * If we're VBR and have no Xing tag, we don't know the exact length
     * ahead of time, so we might end up going past the length we thought
     * the file was.  We don't want to change length after we've set it
     * (that would change all of the toc entries), so allow the TOC to
     * go up to 200%.  (Beyond that, we'll always hard seek; perhaps this
     * should be larger?) */
    int toc[200];

    /* Position in the file of inbuf: */
    int inbuf_filepos;

    /* File size. */
    int filesize;

    /* Number of bytes of header data at the beginning; used for seeking. */
    int header_bytes;

    /* This is set to true once we get a frame that isn't a header.  This
     * is to keep us from accidentally double-counting a header.  We do
     * reset these to 0 when we rewind, but it's possible we could 
     * seek_stream_to_byte in the middle of them. */
    int finished_header;

    /* This data is filled in when the first frame is decoded. */
    int has_xing; /* whether xingtag is valid */
    struct xing xingtag;

    /* If has_xing is true, this is filled in based on the xing header.
     * If it's false, this is filled in based on the size of the file
     * and the first frame. */
    int length;
    mad_timer_t framelength;

    /* If we have a Xing tag, this is the average bitrate; otherwise it's
     * the bitrate of the first frame. */
    int bitrate;
};








static signed int scale(mad_fixed_t sample)
{
    /* round */
    sample += (1L << (MAD_F_FRACBITS - 16));

    /* clip */
    if (sample >= MAD_F_ONE)
	sample = MAD_F_ONE - 1;
    else if (sample < -MAD_F_ONE)
        sample = -MAD_F_ONE;

    /* quantize */
    return sample >> (MAD_F_FRACBITS + 1 - 16);
}

/* Called on the first frame decoded.  Returns true if this frame
 * should be ignored. */
int RageSoundReader_MP3::handle_first_frame()
{
	int ret = 0;

	if( mad->finished_header )
		return ret;
	mad->finished_header = true;

	/* Check for a XING tag. */
	xing_init( &mad->xingtag );
	if( xing_parse(&mad->xingtag, mad->Stream.anc_ptr, mad->Stream.anc_bitlen) == 0 )
	{
		mad_timer_t tm;

		/* Add it to the header byte count. */
		mad->header_bytes += mad->Stream.next_frame - mad->Stream.this_frame;

		mad->has_xing = true;

		tm = mad->Frame.header.duration;
		/* XXX: does this include the Xing header itself? */
		mad_timer_multiply( &tm, mad->xingtag.frames );
		mad->length = mad_timer_count( tm, MAD_UNITS_MILLISECONDS );

		/* XXX: an id3v2 footer tag would throw this off a little. This also assumes
		 * the Xing tag is the last header; it always is, I think. */
		int bytes = mad->filesize - mad->header_bytes;
		mad->bitrate = (int)(bytes * 8 / (mad->length/1000.f));

		ret = 1;
	}

	/* If there's no Xing tag, mad->length will be filled in by _open. */

	return ret;
}

static int get_this_frame_byte( const madlib_t *mad )
{
    int ret = mad->inbuf_filepos;

    /* If we have a frame, adjust. */
    if( mad->Stream.this_frame != NULL )
		ret += mad->Stream.this_frame-mad->inbuf;

    return ret;
}


int RageSoundReader_MP3::fill_buffer()
{
	/* Need more data. */
	if( mad->Stream.next_frame != NULL )
	{
		/* Pull out remaining data from the last buffer. */
		mad->inbytes = mad->Stream.bufend-mad->Stream.next_frame;
		memmove( mad->inbuf, mad->Stream.next_frame, mad->inbytes );
		mad->inbuf_filepos += mad->Stream.next_frame - mad->inbuf;
	} else {
		mad->inbytes = 0;
	}

	int rc = fread( mad->inbuf + mad->inbytes, 1, sizeof (mad->inbuf)-mad->inbytes, this->rw );
	if ( rc <= 0 )
		return rc;
	if ( rc == 0 )
		return 0;

	mad->inbytes += rc;
	mad_stream_buffer( &mad->Stream, mad->inbuf, mad->inbytes );
	return rc;
}

void fill_frame_index_cache( madlib_t *mad )
{
	int ms, percent, pos;

	/* Only update the frame cache if our timer is consistent. */
	if(!mad->timer_accurate) return;

	/* ms of the frame we just decoded: */
	ms = mad_timer_count(mad->Timer, MAD_UNITS_MILLISECONDS);
	percent = ms * 100 / mad->length;
	pos = get_this_frame_byte(mad);

	/* Fill in the TOC percent.  Also, fill in all values before it
	 * that are unknown; for example, if toc[4] is "50 -1 -1 -1", and filling
	 * in toc[3] value with "100", toc[1] and toc[2] are also 100.  This
	 * can probably only happen with a file with less than 100 frames, so
	 * this isn't too important. */
	while(percent >= 0 && percent < 200 && mad->toc[percent] == -1)
		mad->toc[percent--] = pos;
}

/* Handle first-stage decoding: extracting the MP3 frame data. */
int RageSoundReader_MP3::do_mad_frame_decode()
{
	int bytes_read = 0;

	while(1)
	{
		if( !mad_frame_decode(&mad->Frame,&mad->Stream) )
		{
			/* OK. */
			if( mad_timer_compare(mad->Timer, mad_timer_zero) == 0 )
			{
				/* We're at the beginning. */
				if(handle_first_frame())
				{
					/* The first frame contained a header. Continue searching. */
					continue;
				}
			}

			/* Do this now, while Timer is the time of this frame, not the next frame. */
			fill_frame_index_cache( mad );

			/* Set Timer to the time of the next frame. */
			mad_timer_add( &mad->Timer,mad->Frame.header.duration );
			return 1;
		}

		if(mad->Stream.error == MAD_ERROR_LOSTSYNC)
		{
			/* This might be an ID3V2 tag. */
			const int tagsize = id3_tag_query(mad->Stream.this_frame,
			mad->Stream.bufend - mad->Stream.this_frame);

			if(tagsize && !mad_timer_compare(mad->Timer, mad_timer_zero))
			{
				if( !mad->finished_header )
					mad->header_bytes += tagsize;
				mad_stream_skip(&mad->Stream, tagsize);

				/* Don't count the tagsize against the max-read-per-call figure. */
				bytes_read -= tagsize;
			}

			continue;
		}

		if( mad->Stream.error == MAD_ERROR_BADCRC )
		{
			/* XXX untested */
			mad_frame_mute(&mad->Frame);
			mad_synth_mute(&mad->Synth);

			continue;
		}

		if( mad->Stream.error == MAD_ERROR_BUFLEN || mad->Stream.error == MAD_ERROR_BUFPTR )
		{
			if( bytes_read > 25000 )
			{
				/* We've read this much without actually getting a frame; error. */
				SetError( "Can't find data" );
				return -1;
			}

			int ret = fill_buffer();
			if( ret <= 0 )
				return ret;
			bytes_read += ret;

			continue;
		}

		/* If it's some other recoverable error, just keep going. */
		if( MAD_RECOVERABLE(mad->Stream.error) )
			continue;

		SetError( mad_stream_errorstr(&mad->Stream) );
		return -1;
	}
}

/* Previously, there was a bug in MADLIB_rewind: it didn't clear synth.  This
 * resulted in some files having a few frames of silence at the beginning, throwing
 * off sync.  Figure out how far ahead the file was offset under those conditions,
 * so we can emulate that sync offset. */
int RageSoundReader_MP3::FindOffsetFix()
{
	/* Do a fake rewind. */
	if( fseek(this->rw, 0, SEEK_SET) == -1 )
	{
		SetError( strerror(errno) );
		return 0;
	}

	mad_frame_mute(&mad->Frame);
	mad_synth_mute(&mad->Synth);
	mad_timer_reset(&mad->Timer);
	mad->outpos = mad->outleft = 0;

//	mad_stream_finish(&mad->Stream); // fake
//	mad_stream_init(&mad->Stream);   // fake
	mad_stream_buffer(&mad->Stream, NULL, 0);
	mad->inbuf_filepos = 0;
	mad->header_bytes = 0;
	mad->finished_header = false;

	/* Read a couple frames, to make sure we're synced. */
	for( int i = 0; i < 5; ++i )
	{
		int ret = do_mad_frame_decode();
		if( ret == 0 )
		{
			SetError("Unexpected EOF");
			return false;
		}
		if( ret == -1 )
			return false; /* it set the error */

		mad->outleft = 0;
		synth_output();
	}

	/* Clear the TOC cache.  We might have cached bogus values. */
	for(int i = 0; i < 200; ++i)
		mad->toc[i] = -1;

	/* Save the current timestamp.  This is the time we thought we were at when
	 * we decoded the last frame. */
	mad_timer_t Apparent = mad->Timer;

	/* Save the last frame's PCM data. */
	unsigned size = mad->outleft;
	char *cpy = new char[size];
	memcpy( cpy, mad->outbuf, size );

	/* Do a real rewind. */
	MADLIB_rewind();

	/* Search for the frame we just saved. */
	mad_timer_t Actual;
	bool found = false;
	for( int i = 0; i < 10; ++i )
	{
		int ret = do_mad_frame_decode();
		if( ret == 0 )
		{
			SetError("Unexpected EOF");
			return false;
		}
		if( ret == -1 )
			return false; /* it set the error */

		mad->outleft = 0;
		synth_output();
		if( mad->outleft == size && !memcmp( cpy, mad->outbuf, mad->outleft ) )
		{
			/* Found it.  Record that frame's real timestamp. */
			Actual = mad->Timer;
			found = true;
			break;

		}
	}

	if( found )
	{
		/* Save the offset, so timestamps can be corrected. */
		mad_timer_t Offset = Apparent;
		mad_timer_sub( &Offset, Actual );
		this->OffsetFix = mad_timer_count( Offset, (mad_units) 1000) / 1000.0f;
	}
	else
		this->OffsetFix = 0;


	delete [] cpy;
	MADLIB_rewind();
	return true;
}

void RageSoundReader_MP3::synth_output()
{
	if( MAD_NCHANNELS(&mad->Frame.header) != this->Channels )
	{
		/* This frame contains a different number of channels than the first.
		 * I've never actually seen this--it's just to prevent exploding if
		 * it happens--and we could continue on easily enough, so if it happens,
		 * just continue. */
		return;
	}

	mad_synth_frame(&mad->Synth, &mad->Frame);
	for(int i=0; i < mad->Synth.pcm.length; i++)
	{
		for(int chan = 0; chan < this->Channels; ++chan)
		{
			short Sample = (short) scale(mad->Synth.pcm.samples[chan][i]);
			*((short *) (mad->outbuf + mad->outleft)) = Sample;
			mad->outleft += 2;
		}
	}
}

/* Seek to a byte in the file.  If you're going to put the file position
 * back when you're done, and not going to read any data, you don't have
 * to use this. */
int RageSoundReader_MP3::seek_stream_to_byte( int byte )
{
	if( fseek(this->rw, byte, SEEK_SET) == -1 )
	{
		SetError( strerror(errno) );
		return 0;
	}

	mad_frame_mute(&mad->Frame);
	mad_synth_mute(&mad->Synth);
	/* This is the only way I know of to reset a stream.  (If we don't
	 * do this, then seeking to a position and decoding might not do the
	 * exact same thing every time, which will throw resync() off.) */
	mad_stream_finish(&mad->Stream);
	mad_stream_init(&mad->Stream);

	mad->inbytes = mad->outleft = mad->outpos = 0;

	mad->inbuf_filepos = byte;
	return 1;
}


/* Call this after seeking the stream.  We'll back up a bit and reread
 * frames until we're back where we started, so the next read is aligned
 * to a frame and synced.  This must never leave the position ahead of where
 * it way, since that can confuse the seek optimizations. */
int RageSoundReader_MP3::resync()
{
	/* Save the timer; decoding will change it, and we need to put it back. */
	mad_timer_t orig = mad->Timer;

	int reads = 0;

	/* Seek backwards up to 4k. */
	int origpos = mad->inbuf_filepos;
	int seekpos = origpos - 1024*4;
	if( seekpos < 0 )
		seekpos = 0;
	seek_stream_to_byte( seekpos );

	/* We're already synced if we're at the beginning. */
	/* ... but we need to resync anyway; even if seekpos < 0, origpos might
	 * not have been. */
//	if( mad->inbuf_filepos == 0 )
//		return 1;

	/* Agh.  This is annoying.  We want to decode enough so that the next frame
	 * read will be the first frame after the current file pointer.  If we just
	 * read until the file pointer is >= what it was, we've passed it already.
	 * So, read until it's >= what it was, counting the number of times we had
	 * to read; then back up again and read n-1 times.  Gross. */
	do
	{
		if( do_mad_frame_decode() <= 0 ) /* XXX eof */
			return -1; /* it set the error */

		reads++;
	} while( get_this_frame_byte(mad) < origpos );

	seek_stream_to_byte( seekpos );

	reads--;
	while( reads-- > 0 )
	{
		if( do_mad_frame_decode() <= 0 ) /* XXX eof */
			return -1; /* it set the error */
	}

	/* Restore the timer. */
	mad->Timer = orig;
	mad->outpos = mad->outleft = 0;

	return 1;
}


RageSoundReader_MP3::RageSoundReader_MP3()
{
	mad = new madlib_t;
	memset(mad, '\0', sizeof (madlib_t));

	mad_stream_init( &mad->Stream );
	mad_frame_init( &mad->Frame );
	mad_synth_init( &mad->Synth );
}

RageSoundReader_MP3::~RageSoundReader_MP3()
{
    mad_synth_finish( &mad->Synth );
    mad_frame_finish( &mad->Frame );
    mad_stream_finish( &mad->Stream );
	if( rw )
		fclose( rw );

	delete mad;
}

SoundReader_FileReader::OpenResult RageSoundReader_MP3::Open( CString filename_ )
{
	filename = filename_;
    rw = fopen(filename, "rb");
	ASSERT( rw );

	mad_frame_mute( &mad->Frame );
	mad_timer_reset( &mad->Timer );
	mad->length = -1;
	mad->inbuf_filepos = 0;
	mad->header_bytes = 0;
	mad->finished_header = false;
	mad->has_xing = false;
	mad->timer_accurate = 1;
	mad->bitrate = -1;

	for(int i = 0; i < 200; ++i)
		mad->toc[i] = -1;

	int ret = fseek( this->rw, 0, SEEK_END );
	ASSERT( ret != -1 );
	mad->filesize = ftell( this->rw );
	ASSERT( mad->filesize != -1 );
	fseek( this->rw, 0, SEEK_SET );

	/* Make sure we can decode at least one frame.  This will also fill in header info. */
	mad->outpos = 0;

	ret = do_mad_frame_decode();
	if( ret == 0 )
	{
		SetError( "Failed to read any data at all" );
		return OPEN_NO_MATCH;
	}
	if( ret == -1 )
	{
		SetError( ssprintf("%s (not an MP3 stream?)", GetError().c_str()) );
		return OPEN_NO_MATCH;
	}

	LOG->Trace("Accepting MP3 stream.");

	/* Store the bitrate of the frame we just got. */
	if(mad->bitrate == -1)
		mad->bitrate = mad->Frame.header.bitrate;

	SampleRate = mad->Frame.header.samplerate;
	mad->framelength = mad->Frame.header.duration;
	this->Channels = MAD_NCHANNELS( &mad->Frame.header );

	/* Since we've already decoded a frame, just synth it instead of rewinding
	 * the stream. */
	synth_output();

	ret = FindOffsetFix();
	ASSERT( ret != -1 );

    if(mad->length == -1)
    {
		/* If vbr and !xing, this is just an estimate. */
		int bps = mad->bitrate / 8;
		float secs = (float)(mad->filesize - mad->header_bytes) / bps;
		mad->length = (int)(secs * 1000.f);
	}

	return OPEN_OK;
}

/* dst and src are buffers of 16-bit samples.  len is the number of bytes
 * to copy from src.  len must be even. */
static void mono_to_stereo( char *dst, const char *src, unsigned len )
{
	ASSERT( (len % 2) == 0 );
	const char *src_end = src + len;

	while( src != src_end )
	{
		dst[0] = dst[2] = src[0];
		dst[1] = dst[3] = src[1];
		dst += 4;
		src += 2;
	}
}

int RageSoundReader_MP3::Read( char *buf, unsigned len )
{
	Uint32 bw = 0;

	ASSERT( (len % (sizeof(Sint16)*2)) == 0 );

	while( bw < len )
	{
		if( mad->outleft > 0 )
		{
			/* Ratio of input bytes to output bytes. */
			const int Ratio = Channels == 1? 2:1;

			/* Input bytes to be copied: */
			const unsigned cpysize = min( (len - bw)/Ratio, mad->outleft );

			if( Channels == 1 )
				mono_to_stereo( (char *) buf + bw, (const char *) mad->outbuf + mad->outpos, cpysize );
			else
				memcpy( buf + bw, mad->outbuf + mad->outpos, cpysize );

			bw += cpysize * Ratio;
			mad->outpos += cpysize;
			mad->outleft -= cpysize;
			continue;
		}

		/* Decode more from the MP3 stream. */
		mad->outpos = 0;
		int ret = do_mad_frame_decode();
		if( ret == 0 )
			return bw;
		if( ret == -1 )
			return -1;

		synth_output();
	}

	return bw;
}

bool RageSoundReader_MP3::MADLIB_rewind()
{
	if( fseek(this->rw, 0, SEEK_SET) == -1 )
	{
		SetError( strerror(errno) );
		return 0;
	}

	mad_frame_mute(&mad->Frame);
	mad_synth_mute(&mad->Synth);
	mad_timer_reset(&mad->Timer);
	mad->outpos = mad->outleft = 0;

	mad_stream_finish(&mad->Stream);
	mad_stream_init(&mad->Stream);
	mad_stream_buffer(&mad->Stream, NULL, 0);
	mad->inbuf_filepos = 0;
	mad->header_bytes = 0;
	mad->finished_header = false;

	/* Decode and synth a frame, so we'll skip any id3/xing tags and the
	 * next_frame pointer will be valid--in case we seek immediately after
	 * this. */
	int ret = do_mad_frame_decode();
	if( ret == 0 )
	{
		SetError("Unexpected EOF");
		return false;
	}
	if( ret == -1 )
		return false; /* it set the error */

	synth_output();

	return true;
}

/* Methods of seeking:
 *
 * 1. We can jump based on a TOC.  We potentially have two; the Xing TOC and our
 *    own index.  The Xing TOC is only accurate to 1/256th of the file size,
 *    so it's unsuitable for precise seeks.  Our own TOC is byte-accurate.
 *    (SetPosition_toc)
 *
 * 2. We can jump based on the bitrate.  This is fast, but not accurate.
 *    (SetPosition_estimate)
 *
 * 3. We can seek from any position to any higher position by decoding headers.
 *    (SetPosition_hard)
 *
 * Both 1 and 2 will leave the position behind the actual requested position; 
 * combine them with 3 to catch up. Never do 3 alone in "fast" mode, since it's
 * slow if it ends up seeking from the beginning of the file.  Never do 2 in
 * "precise" mode.
 */

/* Returns 1 on success, 0 on error, -1 if we couldn't it (don't have
 * a Xing tag or a length). */
int RageSoundReader_MP3::SetPosition_toc( int ms, bool Xing )
{
    int percent;
    
    ASSERT( !Xing || mad->has_xing );
    ASSERT( mad->length != -1 );

    /* This leaves our timer accurate if we're using our own TOC, and inaccurate
     * if we're using Xing. */
    mad->timer_accurate = !Xing;

    /* We can speed up the seek using the XING tag.  First, figure
     * out what percentage the requested position falls in. */
    percent = ms * 100 / mad->length;
    
	if(percent >= 0)
	{
		int bytepos = -1;
		if( Xing && percent < 100 )
		{
			int jump = mad->xingtag.toc[percent];
			bytepos = mad->filesize * jump / 256;
		} else if(percent < 200) {
			/* Find the last entry <= percent that we actually have an entry for;
			 * this will get us as close as possible. */
			while( percent >= 0 && mad->toc[percent] == -1 )
				percent--;
			if( percent == -1 )
				return ms; /* don't have any info */

			bytepos = mad->toc[percent];
		}

		if( bytepos != -1 )
		{
			seek_stream_to_byte( bytepos );

			mad_timer_set( &mad->Timer, 0, percent * mad->length / 1000, 100 );

			/* We've jumped across the file, so the decoder is currently desynced. */
			resync();
		}
	}

    return ms;
}

int RageSoundReader_MP3::SetPosition_hard( int ms )
{
	mad_timer_t desired;
	mad_timer_set( &desired, 0, ms, 1000 );

	/* This seek doesn't change the accuracy of our timer. */

	/* If we're already past the requested position, rewind. */
	mad_timer_set(&desired, 0, ms, 1000);
	if(mad_timer_compare(mad->Timer, desired) > 0)
		MADLIB_rewind();

	/* Decode frames until the current frame contains the desired offset. */
	while(1)
	{
		/* If cur > desired, this frame contains the position.  Since we've
		 * already decoded the frame, synth it, too. */
		if( mad_timer_compare(mad->Timer, desired) > 0 )
		{
			mad->outleft = 0;
			synth_output();

			/* mad->Timer is the timestamp of the next frame.  Subtract the frame length
			 * to find out the timestamp of the frame we just synthed. */
			mad_timer_t ts = mad->Timer;
			mad_timer_sub( &ts, mad->framelength );

			/* We just synthed data starting at ts, containing the desired offset.
			 * Skip (desired - ts) worth of frames in the output to line up. */
			mad_timer_t skip = desired;
			mad_timer_sub( &skip, ts );

			int samples = mad_timer_count( skip, (mad_units) SampleRate );

			/* Skip 'samples' samples; each sample is 2 * channels bytes, since
			 * we're currently always using AUDIO_S16SYS. */
			mad->outpos = samples * 2 * this->Channels;
			mad->outleft -= samples * 2 * this->Channels;
			return ms;
		}

		if( do_mad_frame_decode() == -1 )
			return -1; /* it set the error */
	}
}

/* Do a seek based on the bitrate. */
int RageSoundReader_MP3::SetPosition_estimate( int ms )
{
	/* This doesn't leave us accurate. */
	mad->timer_accurate = 0;

	mad_timer_t seekamt;
	mad_timer_set( &seekamt, 0, ms, 1000 );
	{
		/* We're going to skip ahead three samples below plus a little,
		 * as the decoder resyncs, so seek four frames earlier htan
		 * we were asked to. */
		mad_timer_t back_len = mad->framelength;
		mad_timer_multiply(&back_len, -4);
		mad_timer_add(&seekamt, back_len);
		if( mad_timer_compare(seekamt, mad_timer_zero) < 0 )
			seekamt = mad_timer_zero;
	}

	int seekpos = mad_timer_count( seekamt, MAD_UNITS_MILLISECONDS ) * (mad->bitrate / 8 / 1000);
	seekpos += mad->header_bytes;
	seek_stream_to_byte( seekpos );

	/* We've jumped across the file, so the decoder is currently desynced. 
	 * Don't use resync(); it's slow.  Just decode a few frames. */
	if( do_mad_frame_decode() == -1 ) return -1;
	if( do_mad_frame_decode() == -1 ) return -1;
	if( do_mad_frame_decode() == -1 ) return -1;

	/* Find out where we really seeked to. */
	ms = (get_this_frame_byte(mad) - mad->header_bytes) / (mad->bitrate / 8 / 1000);
	mad_timer_set(&mad->Timer, 0, ms, 1000);

	return ms;
}

int RageSoundReader_MP3::SetPosition_Accurate( int ms )
{
	/* Seek using our own internal (accurate) TOC. */
	if( SetPosition_toc( ms, false ) == -1 )
		return -1; /* error */

	/* Align exactly. */
	return SetPosition_hard( ms );
}

int RageSoundReader_MP3::SetPosition_Fast( int ms )
{
	/* Rewinding is always fast and accurate, and SetPosition_estimate is bad at 0. */
	if( !ms )
	{
		if( !MADLIB_rewind() )
			return -1; /* error */
		return 0; /* error */
	}

	/* We can do a fast jump in VBR with Xing with more accuracy than without Xing. */
	if( mad->has_xing )
		return SetPosition_toc( ms, true );

	/* Guess.  This is only remotely accurate when we're not VBR, but also
	 * do it if we have no Xing tag. */
	return SetPosition_estimate( ms );
}

int RageSoundReader_MP3::GetLengthInternal( bool fast )
{
	if( mad->has_xing && mad->length != -1 )
		return mad->length; /* should be accurate */

	/* Check to see if a frame in the middle of the file is the same
	 * bitrate as the first frame.  If it is, assume the file is really CBR. */
	seek_stream_to_byte( mad->filesize / 2 );

	if(mad->length != -1 &&
	   do_mad_frame_decode() &&
	   mad->bitrate == (int) mad->Frame.header.bitrate)
	{
		return mad->length;
	}

	/* We have a filesize, so we're seekable. */
	if( !MADLIB_rewind() )
		return 0;

	/* Worst-case: vbr && !xing.  We've made a guess at the length, but let's actually
	 * scan the size, since the guess is probably wrong. */
	if( fast )
	{
		SetError("Can't estimate file length");
		return -1;
	}

	MADLIB_rewind();
	while(1)
	{
		int ret = do_mad_frame_decode();
		if( ret == -1 )
			return -1; /* it set the error */
		if( ret == 0 ) /* EOF */
			break;
	}

	/* Count milliseconds. */
	return mad_timer_count( mad->Timer, MAD_UNITS_MILLISECONDS );
}

int RageSoundReader_MP3::GetLengthConst( bool fast ) const
{
	RageSoundReader_MP3 *cpy = new RageSoundReader_MP3;
	SoundReader_FileReader::OpenResult  ret = cpy->Open( filename );
	ASSERT( ret == OPEN_OK );

	int length = cpy->GetLengthInternal( fast );

	delete cpy;
	return length;
}

SoundReader *RageSoundReader_MP3::Copy() const
{
	RageSoundReader_MP3 *ret = new RageSoundReader_MP3;
	ret->Open( filename );
	return ret;
}






/* begin XING code ripped out of madplay */

/*
 * NAME:	xing->init()
 * DESCRIPTION:	initialize Xing structure
 */
void xing_init(struct xing *xing)
{
  xing->flags = 0;
}

/*
 * NAME:	xing->parse()
 * DESCRIPTION:	parse a Xing VBR header
 */
int xing_parse(struct xing *xing, struct mad_bitptr ptr, unsigned int bitlen)
{
	const unsigned XING_MAGIC = (('X' << 24) | ('i' << 16) | ('n' << 8) | 'g');
	if (bitlen < 64 || mad_bit_read(&ptr, 32) != XING_MAGIC)
	goto fail;

	xing->flags = mad_bit_read(&ptr, 32);
	bitlen -= 64;

	if (xing->flags & XING_FRAMES)
	{
		if (bitlen < 32)
			goto fail;

		xing->frames = mad_bit_read(&ptr, 32);
		bitlen -= 32;
	}

	if (xing->flags & XING_BYTES)
	{
		if (bitlen < 32)
			goto fail;

		xing->bytes = mad_bit_read(&ptr, 32);
		bitlen -= 32;
	}

	if (xing->flags & XING_TOC)
	{
		int i;

		if (bitlen < 800)
			goto fail;

		for (i = 0; i < 100; ++i)
			xing->toc[i] = (unsigned char) mad_bit_read(&ptr, 8);

		bitlen -= 800;
	}

	if (xing->flags & XING_SCALE)
	{
		if (bitlen < 32)
			goto fail;

		xing->scale = mad_bit_read(&ptr, 32);
		bitlen -= 32;
	}

	return 0;

fail:
	xing->flags = 0;
	return -1;
}

/* end XING code ripped out of madplay */
