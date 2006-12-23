/* SoundReader_FileReader - base class for SoundReaders that read from files. */

#ifndef RAGE_SOUND_READER_FILE_READER_H
#define RAGE_SOUND_READER_FILE_READER_H

#include "RageSoundReader.h"

#define SoundReader_FileReader RageSoundReader_FileReader
class RageSoundReader_FileReader: public RageSoundReader
{
public:
	/*
	 * Return OPEN_OK if the file is open and ready to go.  Return OPEN_UNKNOWN_FILE_FORMAT
	 * if the file appears to be of a different type.  Return OPEN_FATAL_ERROR if
	 * the file appears to be the correct type, but there was an error initializing
	 * the file.
	 *
	 * If the file can not be opened at all, or contains no data, return OPEN_MATCH_BUT_FAIL.
	 */
	enum OpenResult
	{
		OPEN_OK,
		OPEN_UNKNOWN_FILE_FORMAT=1,
		OPEN_FATAL_ERROR=2,
	};
	virtual OpenResult Open(RString filename) = 0;
	virtual bool IsStreamingFromDisk() const { return true; }
	virtual float GetStreamToSourceRatio() const { return 1.0f; }
	virtual RString GetError() const { return m_sError; }

	static RageSoundReader *OpenFile( RString filename, RString &error );

protected:
	void SetError( RString sError ) const { m_sError = sError; }

private:
	static RageSoundReader_FileReader *TryOpenFile( RString filename, RString &error, RString format, bool &bKeepTrying );
	mutable RString m_sError;
};

#endif

/*
 * Copyright (c) 2003-2004 Glenn Maynard
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

