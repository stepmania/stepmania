#ifndef RAGE_SOUND_READER_FILE_READER
#define RAGE_SOUND_READER_FILE_READER

/* Simple abstract class for SoundReaders that read from files. */
#include "RageSoundReader.h"

class SoundReader_FileReader: public SoundReader {
public:
	/* Return OPEN_OK if the file is open and ready to go.  Return OPEN_NO_MATCH
	 * if the file appears to be of a different type.  Return OPEN_MATCH_BUT_FAIL if
	 * the file appears to be the correct type, but there was an error initializing
	 * the file.
	 *
	 * If the file can not be opened at all, or contains no data, return OPEN_MATCH_BUT_FAIL. */
	enum OpenResult { OPEN_OK, OPEN_NO_MATCH, OPEN_MATCH_BUT_FAIL };
	virtual OpenResult Open(CString filename) = 0;

	static SoundReader *OpenFile( CString filename, CString &error );
private:
	static SoundReader_FileReader *TryOpenFile( CString filename, CString &error, CString format );
};

#endif
/*
 * Copyright (c) 2003 by the person(s) listed below.  All rights reserved.
 *	Glenn Maynard
 */
