/* Read from a Vorbisfile interface.  Currently targets the Tremor (integer)
 * decoder; could be adapted to the regular decoder if needed. */
#include "global.h"

#include "RageUtil.h"
#include "SDL_utils.h"
#include "SDL_endian.h"
#include "RageSoundReader_Vorbisfile.h"
#include "RageLog.h"

#if defined(INTEGER_OGG)
#include "tremor/ivorbisfile.h"
#else
#include "vorbis/vorbisfile.h"
#endif

#if defined(_XBOX)
#pragma comment(lib, "vorbis/xbox/ogg_static.lib")
#pragma comment(lib, "vorbis/xbox/vorbis_static.lib")
#pragma comment(lib, "vorbis/xbox/vorbisfile_static.lib")
#elif defined(_WINDOWS)
#pragma comment(lib, "vorbis/win32/ogg_static.lib")
#pragma comment(lib, "vorbis/win32/vorbis_static.lib")
#pragma comment(lib, "vorbis/win32/vorbisfile_static.lib")
#endif

#include <string.h>
#include <errno.h>


const int channels = 2;

/* The amount of data to read from SDL_sound at once. */
const int read_block_size = 1024;

static CString ov_ssprintf( int err, const char *fmt, ...)
{
    va_list	va;
    va_start(va, fmt);
    CString s = vssprintf( fmt, va );
    va_end(va);

	CString errstr;
	switch( err )
	{
	/* XXX: In the case of OV_EREAD, can we snoop at errno? */
	case OV_EREAD:		errstr = "Read error"; break;
	case OV_EFAULT:		errstr = "Internal error"; break;
	case OV_EIMPL:		errstr = "Feature not implemented"; break;
	case OV_EINVAL:		errstr = "Invalid argument"; break;
	case OV_ENOTVORBIS:	errstr = "Not Vorbis data"; break;
	case OV_EBADHEADER:	errstr = "Invalid Vorbis bitstream header"; break;
	case OV_EVERSION:	errstr = "Vorbis version mismatch"; break;
	case OV_ENOTAUDIO:	errstr = "OV_ENOTAUDIO"; break;
	case OV_EBADPACKET:	errstr = "OV_EBADPACKET"; break;
	case OV_EBADLINK:	errstr = "Link corrupted"; break;
	case OV_ENOSEEK:	errstr = "Stream is not seekable"; break;
	default:			errstr = ssprintf( "unknown error %i", err ); break;
	}

	return s + ssprintf( " (%s)", errstr.c_str() );
}

SoundReader_FileReader::OpenResult RageSoundReader_Vorbisfile::Open(CString filename_)
{
	filename=filename_;

	vf = new OggVorbis_File;
	FILE *f = fopen(filename, "rb");
	if(f == NULL)
	{
		SetError(ssprintf("ogg fopen(%s) failed: %s", filename.c_str(), strerror(errno)));
		return OPEN_NO_MATCH;
	}

	int ret = ov_open(f, vf, NULL, 0);
	if(ret < 0)
	{
		SetError( ov_ssprintf(ret, "ov_open failed") );
		fclose(f);
		switch( ret )
		{
		case OV_ENOTVORBIS:
			return OPEN_NO_MATCH;
		default:
			return OPEN_MATCH_BUT_FAIL;
		}
	}

	avail = 0;
	eof = false;

    return OPEN_OK;
}

int RageSoundReader_Vorbisfile::GetLength() const
{
#if defined(INTEGER_OGG)
	int len = ov_time_total(vf, -1);
#else
	int len = int(ov_time_total(vf, -1) * 1000);
#endif
	if(len == OV_EINVAL)
		RageException::Throw("RageSoundReader_Vorbisfile::GetLength: ov_time_total returned OV_EINVAL");

	return len; 
}

int RageSoundReader_Vorbisfile::GetLength_Fast() const
{
	return GetLength();
}

int RageSoundReader_Vorbisfile::SetPosition(int ms, bool accurate)
{
	avail = 0;
	eof = false;

	const ogg_int64_t sample = ogg_int64_t(ms) * GetSampleRate() / 1000;

	int ret = ov_pcm_seek( vf, sample );
	if(ret < 0)
	{
		/* Returns OV_EINVAL on EOF. */
		if( ret == OV_EINVAL )
		{
			eof = true;
			return 0;
		}
		SetError( ov_ssprintf(ret, "ogg: SetPosition failed") );
		return -1;
	}

	return ms;
}

int RageSoundReader_Vorbisfile::Read(char *buf, unsigned len)
{
	if( eof )
		return 0;

	int bytes_read = 0;
	while(len)
	{
		if(!avail)
		{
			vorbis_info *vi = ov_info(vf, -1);
			ASSERT(vi != NULL);
			
			char tmpbuf[4096];
			int bstream;

#if defined(INTEGER_OGG)
			int ret = ov_read(vf, tmpbuf, sizeof(tmpbuf), &bstream);
#else // float vorbis decoder
			int ret = ov_read(vf, tmpbuf, sizeof(tmpbuf), (SDL_BYTEORDER == SDL_BIG_ENDIAN)?1:0, 2, 1, &bstream);
#endif

			if(ret == OV_HOLE)
				continue;
			if(ret == OV_EBADLINK)
			{
				SetError(ssprintf("Read: OV_EBADLINK"));
				return -1;
			}
		
			if(ret == 0)
				return bytes_read;

			/* If we have a different number of channels, we need to convert. */
			ASSERT(vi->channels == 1 || vi->channels == 2);
			if(vi->channels == 1)
			{
				Sint16 *indata = (Sint16 *)tmpbuf;
				Sint16 *outdata = (Sint16 *)buffer;
				int size = ret / sizeof(Sint16);
				for(unsigned pos = 0; pos < unsigned(size); ++pos)
					outdata[pos*2] = outdata[pos*2+1] = indata[pos];
				ret *= 2;
			}
			else
				memcpy(buffer, tmpbuf, ret); /* XXX optimize */

			avail = ret;
		}

		unsigned size = min(avail, len);
		memcpy(buf, buffer, size);

		buf += size;
		len -= size;

		memmove(buffer, buffer+size, avail-size);
		avail -= size;
		bytes_read += size;
	}

	return bytes_read;
}

int RageSoundReader_Vorbisfile::GetSampleRate() const
{
	ASSERT(vf);

	vorbis_info *vi = ov_info(vf, -1);
	ASSERT(vi != NULL);

	return vi->rate;
}

RageSoundReader_Vorbisfile::RageSoundReader_Vorbisfile()
{
	vf = NULL;
}

RageSoundReader_Vorbisfile::~RageSoundReader_Vorbisfile()
{
	if(vf)
		ov_clear(vf);
	delete vf;
}

SoundReader *RageSoundReader_Vorbisfile::Copy() const
{
	RageSoundReader_Vorbisfile *ret = new RageSoundReader_Vorbisfile;
	ret->Open(filename);
	return ret;
}

/*
-----------------------------------------------------------------------------
 Copyright (c) 2003 by the person(s) listed below.  All rights reserved.
	Glenn Maynard
-----------------------------------------------------------------------------
*/
