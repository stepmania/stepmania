#ifndef RAGE_SOUND_READER_FILE_READER
#define RAGE_SOUND_READER_FILE_READER

/* Simple abstract class for SoundReaders that read from files. */
#include "RageSoundReader.h"

class SoundReader_FileReader: public SoundReader {
public:
	virtual bool Open(CString filename) = 0;
};

#endif
/*
 * Copyright (c) 2003 by the person(s) listed below.  All rights reserved.
 *	Glenn Maynard
 */
