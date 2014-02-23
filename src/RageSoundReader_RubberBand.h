/* RageSoundReader_RubberBand - change the pitch and speed of an audio stream independently using RubberBand Library. */

#ifndef RAGE_SOUND_READER_RUBBER_BAND_H
#define RAGE_SOUND_READER_RUBBER_BAND_H

#ifndef HAVE_RUBBERBAND
#error "HAVE_RUBBERBAND is not defined"
#endif

#include <rubberband/RubberBandStretcher.h>
#include "RageSoundReader_Filter.h"

class RageSoundReader_RubberBand: public RageSoundReader_Filter
{
public:
	RageSoundReader_RubberBand( RageSoundReader *pSource );
	RageSoundReader_RubberBand( const RageSoundReader_RubberBand &cpy );

	virtual int Read( float *pBuf, int iFrames );
	virtual bool SetProperty( const RString &sProperty, float fValue );

	void SetSpeedRatio( float fRatio ) { m_RubberBand.setTimeRatio(1/fRatio); }
	void SetPitchRatio( float fRatio ) { m_RubberBand.setPitchScale(1/fRatio); }

	virtual RageSoundReader_RubberBand *Copy() const { return new RageSoundReader_RubberBand(*this); }

	virtual float GetStreamToSourceRatio() const {
		return (1/m_RubberBand.getTimeRatio()) * m_pSource->GetStreamToSourceRatio();
	}

private:
	RubberBand::RubberBandStretcher m_RubberBand;

	// Swallow up warnings. If they must be used, define them.
	RageSoundReader_RubberBand& operator=(const RageSoundReader_RubberBand& rhs);
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
