#ifndef RAGE_SOUND_READER_RESAMPLE_GOOD
#define RAGE_SOUND_READER_RESAMPLE_GOOD

#include "RageSoundReader.h"
#include "RageSoundReader_Resample.h"

/* This class changes the sampling rate of a sound. */
class RageSoundReader_Resample_Good: public RageSoundReader_Resample
{
	enum { BUFSIZE = 4096 };

	SoundReader *source;
	bool HighQuality;
	int samplerate;

	void *resamp[2];
	void *empty_resamp[2];
	float inbuf[2][BUFSIZE];
	int BufSamples;
	bool eof;

	void Reset();
	void ReopenResampler();
	float GetFactor() const;
	bool FillBuf();

public:
	/* We own source. */
	void Open(SoundReader *source);
	int GetLength() const;
	int GetLength_Fast() const;
	int SetPosition_Accurate(int ms);
	int SetPosition_Fast(int ms);
	int Read(char *buf, unsigned len);
	RageSoundReader_Resample_Good();
	virtual ~RageSoundReader_Resample_Good();
	SoundReader *Copy() const;
	float GetOffsetFix() const { return source->GetOffsetFix(); }

	/* Change the actual sample rate of a sound. */
	void SetSampleRate( int hz );
	void SetHighQuality( bool hq ) { HighQuality = hq; }

	int GetSampleRate() const { return samplerate; }
};

#endif
/*
 * Copyright (c) 2003 by the person(s) listed below.  All rights reserved.
 *	Glenn Maynard
 */
