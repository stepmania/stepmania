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
 * Copyright (c) 2003 by the person(s) listed below.  All rights reserved.
 *	Glenn Maynard
 */
