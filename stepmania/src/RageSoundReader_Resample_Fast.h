#ifndef RAGE_SOUND_READER_RESAMPLE_FAST
#define RAGE_SOUND_READER_RESAMPLE_FAST

#include "RageSoundReader.h"
#include "RageSoundReader_Resample.h"
#include "RageSoundResampler.h"

/* This class changes the sampling rate of a sound. */
class RageSoundReader_Resample_Fast: public RageSoundReader_Resample
{
	int samplerate;

	bool FillBuf();

	SoundReader *source;

	mutable RageSoundResampler resamp;

public:
	/* We own source. */
	void Open(SoundReader *source);
	int GetLength() const;
	int GetLength_Fast() const;
	int SetPosition_Accurate(int ms);
	int SetPosition_Fast(int ms);
	int Read(char *buf, unsigned len);
	RageSoundReader_Resample_Fast();
	virtual ~RageSoundReader_Resample_Fast();
	SoundReader *Copy() const;

	/* Change the actual sample rate of a sound. */
	void SetSampleRate(int hz);

	int GetSampleRate() const { return samplerate; }
};

#endif
/*
 * Copyright (c) 2003 by the person(s) listed below.  All rights reserved.
 *	Glenn Maynard
 */
