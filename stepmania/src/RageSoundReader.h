/* RageSoundReader - Data source for a RageSound. */

#ifndef RAGE_SOUND_READER_H
#define RAGE_SOUND_READER_H

class RageSoundReader
{
public:
	virtual int GetLength() const = 0; /* ms */
	virtual int GetLength_Fast() const { return GetLength(); } /* ms */
	virtual int SetPosition( int iSample ) = 0;
	virtual int Read( char *pBuf, int iFrames ) = 0;
	virtual ~RageSoundReader() { }
	virtual RageSoundReader *Copy() const = 0;
	virtual int GetSampleRate() const = 0;
	virtual unsigned GetNumChannels() const { return 2; } /* 1 or 2 */
	virtual bool IsStreamingFromDisk() const = 0;
	virtual bool SetProperty( const RString &sProperty, float fValue ) { return false; }

	/* Return values for Read(). */
	enum {
		ERROR = -1,
		END_OF_FILE = -2
	};

	/* GetNextSourceFrame() provides the source frame associated with the next frame
	 * that will be read via Read().  GetStreamToSourceRatio() returns the ratio
	 * for extrapolating the source frames of the remainder of the block.  These
	 * values are valid so long as no parameters are changed before the next Read(). */
	virtual int GetNextSourceFrame() const = 0;
	virtual float GetStreamToSourceRatio() const = 0;

	RString GetError() const { return m_sError; }

protected:
	void SetError( RString sError ) const { m_sError = sError; }

private:
	mutable RString m_sError;
};

#endif

/*
 * Copyright (c) 2002-2003 Glenn Maynard
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
