#ifndef RAGE_SOUND_READER_PRELOAD
#define RAGE_SOUND_READER_PRELOAD

#include "RageSoundReader.h"

/* This reader simply precaches all of the data from another reader. This
 * reduces CPU usage for sounds that are played several times at once. */

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
};

class SoundReader_Preload: public SoundReader {
	rc_string buf;

	/* Bytes: */
	int position;

	int total_samples() const;

public:
	/* This will throw a nonfatal exception if this sound isn't
	 * suitable for preloading. */
	bool Open(SoundReader *source);
	int GetLength() const;
	int GetLength_Fast() const;
	int SetPosition_Accurate(int ms);
	int SetPosition_Fast(int ms);
	int Read(char *buf, unsigned len);
	SoundReader *Copy() const;
	~SoundReader_Preload() { }
};

#endif
/*
-----------------------------------------------------------------------------
 Copyright (c) 2003 by the person(s) listed below.  All rights reserved.
	Glenn Maynard
-----------------------------------------------------------------------------
*/
