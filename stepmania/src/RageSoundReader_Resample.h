#ifndef RAGE_SOUND_READER_RESAMPLE
#define RAGE_SOUND_READER_RESAMPLE

#include "RageSoundReader.h"
#include "RageSoundResampler.h"

/* This class changes the sampling rate of a sound. */
class RageSoundReader_Resample: public SoundReader
{
	int samplerate;

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
	RageSoundReader_Resample();
	virtual ~RageSoundReader_Resample();
	SoundReader *Copy() const;

	/* Change the actual sample rate of a sound. */
	void SetSampleRate(int hz);

	int GetSampleRate() const { return samplerate; }
};

#endif
