/* Read from a Vorbisfile interface.  Currently targets the Tremor (integer)
 * decoder; could be adapted to the regular decoder if needed. */
#include "global.h"

#include "RageUtil.h"
#include "SDL_utils.h"
#include "RageSoundReader_Vorbisfile.h"
#include "tremor/ivorbisfile.h"

#include <string.h>
#include <sys/errno.h>


const int channels = 2;

/* The amount of data to read from SDL_sound at once. */
const int read_block_size = 1024;

bool RageSoundReader_Vorbisfile::Open(CString filename_)
{
	filename=filename_;

	vf = new OggVorbis_File;
	FILE *f = fopen(filename, "r");
	if(f == NULL)
	{
		SetError(ssprintf("ogg fopen(%s) failed: %s", filename.c_str(), strerror(errno)));
		return false;
	}

	int ret = ov_open(f, vf, NULL, 0);
	if(ret < 0)
	{
		SetError(ssprintf("ogg failed %i", ret));
		fclose(f);
		return false;
	}
		
	avail = 0;

    return true;
}

int RageSoundReader_Vorbisfile::GetLength() const
{
	int len = ov_time_total(vf, -1);

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
	int ret = ov_time_seek(vf, ms);
	if(ret < 0)
	{
		SetError(ssprintf("ogg: SetPosition failed: %i", ret));
		return false;
	}

	return ms;
}

int RageSoundReader_Vorbisfile::Read(char *buf, unsigned len)
{
	int bytes_read = 0;
	while(len)
	{
		if(!avail)
		{
			vorbis_info *vi = ov_info(vf, -1);
			ASSERT(vi != NULL);
			
			char tmpbuf[4096];
			int bstream;

			int ret = ov_read(vf, tmpbuf, sizeof(tmpbuf), &bstream);
//int ret = 4096;
//memset(tmpbuf, 0, sizeof(tmpbuf));
			if(ret == OV_HOLE)
				continue;
			if(ret == OV_EBADLINK)
			{
				SetError(ssprintf("Read: OV_EBADLINK"));
				return -1;
			}
		
			if(ret == 0)
				return bytes_read; /* EOF */

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
