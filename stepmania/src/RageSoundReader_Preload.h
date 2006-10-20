/* RageSoundReader_Preload - Preload sounds from another reader into memory. */

#ifndef RAGE_SOUND_READER_PRELOAD
#define RAGE_SOUND_READER_PRELOAD

#include "RageSoundReader.h"

/* Trivial wrapper to refcount strings, since std::string is not always
 * refcounted.  Without this, Copy() is very slow. */
class rc_string
{
	mutable string *buf;
	mutable int *cnt;

public:
	rc_string();
	rc_string(const rc_string &rhs);
	~rc_string();
	string &get_owned();
	const string &get() const;
	int GetReferenceCount() const { return *cnt; }
};

class RageSoundReader_Preload: public SoundReader
{
	rc_string buf;

	/* Bytes: */
	int position;

	int total_samples() const;

	int samplerate;
	unsigned channels;
	float OffsetFix;

public:
	/* Return true if the sound has been preloaded, in which case source will
	 * be deleted.  Otherwise, return false. */
	bool Open(SoundReader *source);
	int GetLength() const;
	int GetLength_Fast() const;
	int SetPosition_Accurate(int ms);
	int SetPosition_Fast(int ms);
	int Read(char *buf, unsigned len);
	int GetSampleRate() const { return samplerate; }
	unsigned GetNumChannels() const { return channels; }
	bool IsStreamingFromDisk() const { return false; }

	/* Return the total number of copies of this sound.  (If 1 is returned,
	 * this is the last copy.) */
	int GetReferenceCount() const;

	RageSoundReader_Preload *Copy() const;
	~RageSoundReader_Preload() { }

	/* Attempt to preload a sound.  pSound must be rewound. */
	static bool PreloadSound( SoundReader *&pSound );
};

#endif

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
