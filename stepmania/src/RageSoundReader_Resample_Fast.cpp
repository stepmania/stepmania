#include "global.h"
#include "RageSoundReader_Resample_Fast.h"

RageSoundReader_Resample_Fast::RageSoundReader_Resample_Fast()
{
	source = NULL;
	samplerate = -1;
}

void RageSoundReader_Resample_Fast::Open(SoundReader *source_)
{
	source = source_;
	ASSERT(source);

	samplerate = source->GetSampleRate();
	resamp.SetInputSampleRate(samplerate);
	resamp.SetChannels( source->GetNumChannels() );
}


RageSoundReader_Resample_Fast::~RageSoundReader_Resample_Fast()
{
	delete source;
}

void RageSoundReader_Resample_Fast::SetSampleRate(int hz)
{
	samplerate = hz;
	resamp.SetOutputSampleRate(samplerate);
}

int RageSoundReader_Resample_Fast::GetLength() const
{
	resamp.reset();
	return source->GetLength();
}

int RageSoundReader_Resample_Fast::GetLength_Fast() const
{
	resamp.reset();
	return source->GetLength_Fast();
}

int RageSoundReader_Resample_Fast::SetPosition_Accurate(int ms)
{
	resamp.reset();
	return source->SetPosition_Accurate(ms);
}

int RageSoundReader_Resample_Fast::SetPosition_Fast(int ms)
{
	resamp.reset();
	return source->SetPosition_Fast(ms);
}
static const int BUFSIZE = 1024*16;

int RageSoundReader_Resample_Fast::Read(char *buf, unsigned len)
{
	int bytes_read = 0;
	while(len)
	{
		int size = resamp.read(buf, len);

		if(size == -1)
			break;

		buf += size;
		len -= size;
		bytes_read += size;

		if(size == 0)
		{
			static char buf[BUFSIZE];

			int cnt = source->Read(buf, sizeof(buf));

			if(cnt == -1)
			{
				SetError(source->GetError());
				return -1;
			}
			if(cnt == 0)
				resamp.eof();
			else
				resamp.write(buf, cnt);
		}
	}

	return bytes_read;
}

SoundReader *RageSoundReader_Resample_Fast::Copy() const
{
	SoundReader *new_source = source->Copy();
	RageSoundReader_Resample_Fast *ret = new RageSoundReader_Resample_Fast;
	ret->Open(new_source);
	ret->SetSampleRate(samplerate);
	return ret;
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

