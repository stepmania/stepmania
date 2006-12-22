/* RageSoundReader_Extend - Add looping, delay and truncation and fading. */

#ifndef RAGE_SOUND_READER_EXTEND
#define RAGE_SOUND_READER_EXTEND

#include "RageSoundReader_Filter.h"

class RageSoundReader_Extend: public RageSoundReader_Filter
{
public:
	RageSoundReader_Extend( RageSoundReader *pSource );
	virtual int SetPosition( int iFrame );
	virtual int Read( char *pBuffer, int iFrames );
	virtual int GetNextSourceFrame() const;
	virtual bool SetProperty( const RString &sProperty, float fValue );

	RageSoundReader_Extend *Copy() const { return new RageSoundReader_Extend(*this); }
	~RageSoundReader_Extend() { }

private:
	int GetEndFrame() const;
	int GetData( char *pBuffer, int iFrames );

	int m_iPositionFrames;
	
	enum StopMode { M_LOOP, M_STOP, M_CONTINUE };
	StopMode m_StopMode;
	int m_iStartFrames;
	int m_iLengthFrames;
	int m_iFadeFrames;
};

#endif

/*
 * Copyright (c) 2003-2006 Glenn Maynard
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
