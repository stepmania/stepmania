#include "global.h"
#include "RageLog.h"
#include "RageUtil.h"
#include "RageSoundReader_Resample_Good.h"

#include "libresample/include/libresample.h"
#ifdef _WINDOWS
#pragma comment(lib, "libresample/resample.lib")
#endif

#ifdef _XBOX

#ifdef _DEBUG
#pragma comment(lib, "libresample/xboxresample/debug/xboxresample.lib")
#else
#pragma comment(lib, "libresample/xboxresample/release/xboxresample.lib")
#endif

#endif

#include "RageTimer.h"

RageSoundReader_Resample_Good::RageSoundReader_Resample_Good()
{
	resamp[0] = resamp[1] = NULL;
	source = NULL;
	samplerate = -1;
	BufSamples = 0;
	eof = false;
	HighQuality = false;
}

/* Call this if the input position is changed or reset. */
void RageSoundReader_Resample_Good::Reset()
{
	BufSamples = 0;
	eof = false;

	/* Flush the resampler. */
	for( int i = 0; i < 2; ++i )
	{
		if( resamp[i] )
			resample_close( resamp[i] );

		resamp[i] = resample_dup( empty_resamp[i] );
	}
}


/* Call this if the sample factor changes. */
void RageSoundReader_Resample_Good::ReopenResampler()
{
	for( int i = 0; i < 2; ++i )
	{
		if( resamp[i] )
			resample_close( resamp[i] );

		resamp[i] = resample_open( HighQuality, GetFactor()-0.1f, GetFactor()+0.1f );
		empty_resamp[i] = resample_dup( resamp[i] );
	}
}

void RageSoundReader_Resample_Good::Open(SoundReader *source_)
{
	source = source_;
	ASSERT(source);

	samplerate = source->GetSampleRate();
}


RageSoundReader_Resample_Good::~RageSoundReader_Resample_Good()
{
	for( int i = 0; i < 2; ++i )
	{
		if( resamp[i] )
			resample_close( resamp[i] );
		if( empty_resamp[i] )
			resample_close( empty_resamp[i] );
	}

	delete source;
}

float RageSoundReader_Resample_Good::GetFactor() const
{
	return float(samplerate) / source->GetSampleRate();
}

void RageSoundReader_Resample_Good::SetSampleRate(int hz)
{
	samplerate = hz;
	ReopenResampler();
}

int RageSoundReader_Resample_Good::GetLength() const
{
	return source->GetLength();
}

int RageSoundReader_Resample_Good::GetLength_Fast() const
{
	return source->GetLength_Fast();
}

int RageSoundReader_Resample_Good::SetPosition_Accurate(int ms)
{
	Reset();
	return source->SetPosition_Accurate(ms);
}

int RageSoundReader_Resample_Good::SetPosition_Fast(int ms)
{
	Reset();
	return source->SetPosition_Fast(ms);
}

bool RageSoundReader_Resample_Good::FillBuf()
{
	int samples_free = BUFSIZE-BufSamples;
	if( eof )
		return true;
	if( !samples_free )
		return true;

	Sint16 tmpbuf[BUFSIZE*2];
	int cnt = source->Read((char *) tmpbuf, samples_free * sizeof(Sint16) * 2);

	if( cnt == -1 )
	{
		SetError(source->GetError());
		return false;
	}

	if( (unsigned) cnt < samples_free * sizeof(Sint16) * 2 )
		eof = true;

	cnt /= sizeof(Sint16);
	cnt /= 2;

	for( int c = 0; c < 2; ++c )
	{
		for( int s = 0; s < cnt; ++s )
			inbuf[c][s+BufSamples] = tmpbuf[s*2+c];
	}
	BufSamples += cnt;
	return true;
}

int RageSoundReader_Resample_Good::Read(char *bufp, unsigned len)
{
	Sint16 *buf = (Sint16 *) bufp;
	len /= 2;
	const float factor = GetFactor();

	int bytes_read = 0;
	while( 1 )
	{
		int samples_used = 0, samples_output = 0;
		if( BufSamples )
		{
			for( int c = 0; c < 2; ++c )
			{
				ASSERT( resamp[c] );
				float outbuf[BUFSIZE];
				samples_output = resample_process(resamp[c],
						factor,
						inbuf[c], BufSamples,
						eof,
						&samples_used,
						outbuf, len/2);
				if( samples_output == -1 )
					RageException::Throw( "Unexpected resample_process return value: -1" );

				memmove( inbuf[c], &inbuf[c][samples_used], sizeof(float) * (BufSamples-samples_used) );

				for( int s = 0; s < samples_output; ++s )
				{
					buf[s*2+c] = Sint16(clamp(outbuf[s], -32768, 32767));
				}
			}
		}

		BufSamples -= samples_used;

		if( !samples_output )
		{
			if( !len )
				return bytes_read; /* buffer full */
			if( eof )
				return bytes_read; /* EOF and we're completely flushed */
			if( !FillBuf() )
				return -1; /* source error */
		}

		len -= samples_output*2;
		buf += samples_output*2;
		bytes_read += samples_output*2*sizeof(Sint16);
	}
}

SoundReader *RageSoundReader_Resample_Good::Copy() const
{
	SoundReader *new_source = source->Copy();
	RageSoundReader_Resample_Good *ret = new RageSoundReader_Resample_Good;

	for( int c = 0; c < 2; ++c )
	{
		ASSERT( resamp[c] );
		ret->resamp[c] = resample_dup( resamp[c] );
		ret->empty_resamp[c] = resample_dup( empty_resamp[c] );
	}
	ret->source = new_source;
	ret->HighQuality = HighQuality;
	ret->samplerate = samplerate;
	memcpy( ret->inbuf, inbuf, sizeof(inbuf));
	ret->BufSamples = BufSamples;
	ret->eof = eof;

//	ret->Open(new_source);
//	ret->SetSampleRate(samplerate);
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

