#include "global.h"
#include "RageSoundResampler.h"
#include "RageUtil.h"
#include "RageLog.h"
#include <math.h>

/*
 * This class handles sound resampling.
 *
 * This isn't very efficient; we write to a static buffer instead of a circular
 * one.  I'll optimize it if it becomes an issue.
 */

const int channels = 2;

RageSoundResampler::RageSoundResampler()
{
	reset();
}

void RageSoundResampler::reset()
{
	at_eof = false;
	memset(prev, 0, sizeof(prev));
	memset(t, 0, sizeof(t));
	ipos = 0;
}


/* Write data to be converted. */
void RageSoundResampler::write(const void *data_, int bytes)
{
	ASSERT(!at_eof);

	const Sint16 *data = (const Sint16 *) data_;

	const unsigned samples = bytes / sizeof(Sint16);
	const unsigned frames = samples / channels;

	if(InputRate == OutputRate)
	{
		/* Optimization. */
		outbuf.insert(outbuf.end(), data, data+samples);
		return;
	}

#if 0
	/* Possibly slightly higher quality, but way too slow. */
	const int upsamp = 8;
	const float div = float(InputRate*upsamp) / OutputRate;
	for(unsigned f = 0; f < frames; ++f)
	{
		for(int u = 0; u < upsamp; ++u)
		{
			int ipos_begin = (int) roundf(ipos / div);
			int ipos_end = (int) roundf((ipos+1) / div);

			for(int c = 0; c < channels; ++c)
			{
				const float f = 0.5f;
				prev[c] = Sint16(prev[c] * (f) + data[c] * (1-f));
				for(int i = ipos_begin; i < ipos_end; ++i)
					outbuf.push_back(prev[c]);
			}
			ipos++;
		}
		data += channels;
	}
#else
	/* Lerp. */
	const float div = float(InputRate) / OutputRate;
	for(unsigned f = 0; f < frames; ++f)
	{
		for(int c = 0; c < channels; ++c)
		{
			while(t[c] < 1.0f)
			{
				Sint16 s = Sint16(prev[c]*(1-t[c]) + data[c]*t[c]);
				outbuf.push_back(s);
				t[c] += div;
			}

			t[c] -= 1;
			prev[c] = data[c];
		}

		ipos++;
		data += channels;
	}
#endif
}


void RageSoundResampler::eof()
{
	ASSERT(!at_eof);

	/* Write some silence to flush out the real data. */
	const int size = channels*16;
	Sint16 *data = new Sint16[size];
	memset(data, 0, size * sizeof(Sint16));
	write(data, size * sizeof(Sint16));
	delete [] data;

	at_eof = true;
}


int RageSoundResampler::read(void *data, unsigned bytes)
{
	/* Don't be silly. */
	ASSERT( (bytes % sizeof(Sint16)) == 0);

	/* If no data is available, and we're at_eof, return -1. */
	if(outbuf.size() == 0 && at_eof)
		return -1;

	/* Fill as much as we have. */
	int Avail = min(outbuf.size()*sizeof(Sint16), bytes);
	memcpy(data, &outbuf[0], Avail);
	outbuf.erase(outbuf.begin(), outbuf.begin() + Avail/sizeof(Sint16));
	return Avail;
}
