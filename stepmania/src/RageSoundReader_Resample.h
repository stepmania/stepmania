#ifndef RAGE_SOUND_READER_RESAMPLE
#define RAGE_SOUND_READER_RESAMPLE

#include "RageSoundReader.h"
#include "RageSoundResampler.h"

/* This class changes the sampling rate of a sound. */
class RageSoundReader_Resample: public SoundReader
{
public:
	/* We own source. */
	virtual void Open(SoundReader *source) = 0;

	/* Change the actual sample rate of a sound. */
	virtual void SetSampleRate(int hz) = 0;

	enum ResampleQuality { RESAMP_FAST, RESAMP_NORMAL, RESAMP_HIGHQUALITY };
	static RageSoundReader_Resample *MakeResampler( ResampleQuality q );
};

#endif
/*
 * Copyright (c) 2003 by the person(s) listed below.  All rights reserved.
 *	Glenn Maynard
 */
