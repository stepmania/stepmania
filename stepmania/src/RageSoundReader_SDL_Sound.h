#ifndef RAGE_SOUND_READER_SDL_SOUND
#define RAGE_SOUND_READER_SDL_SOUND

#include "RageSoundReader.h"
#include "SDL_sound-1.0.0/SDL_sound.h"

class SoundReader_SDL_Sound: public SoundReader {
	Sound_Sample *Sample;
	const char *inbuf;
	unsigned avail;
	int SetPosition(int ms, bool accurate);
	CString filename;

public:
	bool Open(CString filename);
	int GetLength() const;
	int GetLength_Fast() const;
	int SetPosition_Accurate(int ms)  { return SetPosition(ms, true); }
	int SetPosition_Fast(int ms) { return SetPosition(ms, false); }
	int Read(char *buf, unsigned len);
	~SoundReader_SDL_Sound();
	SoundReader *Copy() const;
};

#endif
/*
-----------------------------------------------------------------------------
 Copyright (c) 2002-2003 by the person(s) listed below.  All rights reserved.
	Glenn Maynard
-----------------------------------------------------------------------------
*/
