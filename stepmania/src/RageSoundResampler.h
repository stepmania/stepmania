#ifndef RAGE_SOUND_RESAMPLER_H
#define RAGE_SOUND_RESAMPLER_H

#include "SDL_utils.h"

enum { MAX_CHANNELS = 15 };

class RageSoundResampler
{
	int InputRate, OutputRate;
	int InputChannels, OutputChannels;

	Sint16 prev[MAX_CHANNELS];

	vector<Sint16> outbuf;
	bool at_eof;
	int ipos;

	float t[MAX_CHANNELS];

public:
	RageSoundResampler();

	/* Configuration: */
	void SetInputSampleRate(int hz) { InputRate = hz; }
	void SetOutputSampleRate(int hz) { OutputRate = hz; }
	void SetInputChannels(int ch) { InputChannels = ch; }
	void SetOutputChannels(int ch) { OutputChannels = ch; }

	/* Write data to be converted. */
	void write(const void *data, int bytes);

	/* Indicate that there is no more data. */
	void eof();

	void reset();

	/* Return the number of bytes currently readable. */
	unsigned avail() const { return outbuf.size() / 2; }

	/* Read converted data.  Returns the number of bytes filled.
	 * If eof() has been called and the output is completely
	 * flushed, returns -1. */
	int read(void *data, unsigned bytes);
};

#endif
