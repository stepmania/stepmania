#include "global.h"
#include "RageSoundReader_Preload.h"
#include "PrefsManager.h"

const int channels = 2;
const int samplesize = 2 * channels; /* 16-bit */

/* If a sound is smaller than this, we'll load it entirely into memory. */
const int max_prebuf_size = 1024*256;

int SoundReader_Preload::total_samples() const
{
	return buf.get().size() / samplesize;
}

bool SoundReader_Preload::Open(SoundReader *source)
{
	ASSERT(source);
	samplerate = source->GetSampleRate();

	/* Check the length, and see if we think it'll fit in the buffer. */
	int len = source->GetLength_Fast();
	if(len != -1)
	{
		float secs = len / 1000.f;

		int pcmsize = int(secs * samplerate * samplesize); /* seconds -> bytes */
		if(pcmsize > max_prebuf_size)
			return false; /* Don't bother trying to preload it. */

		buf.get_owned().reserve(pcmsize);
	}

	while(1) {
		char buffer[1024];
		int cnt = source->Read(buffer, sizeof(buffer));

		if(cnt < 0) {
			/* XXX untested */
			SetError(source->GetError());
			return false;
		}

		if(!cnt) break; /* eof */

		/* Add the buffer. */
		buf.get_owned().append(buffer, buffer+cnt);

		if(buf.get_owned().size() > max_prebuf_size)
			return false; /* too big */
	}

	position = 0;
	delete source;
	return true;
}

int SoundReader_Preload::GetLength() const
{
	return int(float(total_samples()) * 1000.f / samplerate);
}

int SoundReader_Preload::GetLength_Fast() const
{
	return GetLength();
}

int SoundReader_Preload::SetPosition_Accurate(int ms)  
{
	const int sample = int((ms / 1000.0f) * samplerate);
	position = sample * samplesize;

	if(position >= int(buf.get().size()))
		position = 0;

	return position;
}

int SoundReader_Preload::SetPosition_Fast(int ms)
{
	return SetPosition_Accurate(ms); 
}

int SoundReader_Preload::Read(char *buffer, unsigned len)
{
	const unsigned bytes_avail = buf.get().size() - position;

	len = min(len, bytes_avail);
	memcpy(buffer, buf.get().data()+position, len);
	position += len;
	
	return len;
}

SoundReader *SoundReader_Preload::Copy() const
{
	return new SoundReader_Preload(*this);
}

rc_string::rc_string()
{
	buf = new string;
	cnt = new int(1);
}

rc_string::rc_string(const rc_string &rhs)
{
	buf = rhs.buf;
	cnt = rhs.cnt;
	(*cnt)++;
}

rc_string::~rc_string()
{
	(*cnt)--;
	if(!*cnt)
	{
		delete buf;
		delete cnt;
	}
}

string &rc_string::get_owned()
{
	if(*cnt != 1)
	{
		(*cnt)--;
		buf = new string(*buf);
		cnt = new int(1);
	}

	return *buf;
}

const string &rc_string::get() const
{
	return *buf;
}

/*
-----------------------------------------------------------------------------
 Copyright (c) 2003 by the person(s) listed below.  All rights reserved.
	Glenn Maynard
-----------------------------------------------------------------------------
*/
