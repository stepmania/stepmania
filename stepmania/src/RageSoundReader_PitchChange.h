/* RageSoundReader_PitchChange - change the pitch and speed of an audio stream independently. */

#ifndef RAGE_SOUND_READER_PITCH_CHANGE_H
#define RAGE_SOUND_READER_PITCH_CHANGE_H

#include "RageSoundReader_Filter.h"
class RageSoundReader_SpeedChange;
class RageSoundReader_Resample_Good;

class RageSoundReader_PitchChange: public RageSoundReader_Filter
{
public:
	RageSoundReader_PitchChange( RageSoundReader *pSource );
	RageSoundReader_PitchChange( const RageSoundReader_PitchChange &cpy );

	virtual int Read( char *pBuf, int iFrames );
	virtual bool SetProperty( const RString &sProperty, float fValue );

	void SetSpeedRatio( float fRatio ) { m_fSpeedRatio = fRatio; }
	void SetPitchRatio( float fRatio ) { m_fPitchRatio = fRatio; }

	virtual RageSoundReader_PitchChange *Copy() const { return new RageSoundReader_PitchChange(*this); }

private:
	RageSoundReader_SpeedChange *m_pSpeedChange; // freed by RageSoundReader_Filter
	RageSoundReader_Resample_Good *m_pResample; // freed by RageSoundReader_Filter

	float m_fSpeedRatio;
	float m_fPitchRatio;
};

#endif

/*
 * Copyright (c) 2006 Glenn Maynard
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
