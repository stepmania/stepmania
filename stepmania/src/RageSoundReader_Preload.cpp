/* This reader simply precaches all of the data from another reader. This
 * reduces CPU usage for sounds that are played several times at once. */

#include "global.h"
#include "RageSoundReader_Preload.h"
#include "PrefsManager.h"

#define samplesize (2 * channels) /* 16-bit */

/* If a sound is smaller than this, we'll load it entirely into memory. */
const unsigned max_prebuf_size = 1024*256;

int SoundReader_Preload::total_samples() const
{
	return buf.get().size() / samplesize;
}

bool SoundReader_Preload::Open(SoundReader *source)
{
	ASSERT(source);
	samplerate = source->GetSampleRate();
	channels = source->GetNumChannels();
	
	/* Check the length, and see if we think it'll fit in the buffer. */
	int len = source->GetLength_Fast();
	if(len != -1)
	{
		float secs = len / 1000.f;

		unsigned pcmsize = unsigned(secs * samplerate * samplesize); /* seconds -> bytes */
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
	{
		position = buf.get().size();
		return 0;
	}

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
 * Copyright (c) 2003 Glenn Maynard
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
