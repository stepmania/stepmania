/*
 * Implements properties: 
 *   "Speed" - cause the sound to play faster or slower
 *   "Pitch" - raise or lower the pitch of the sound
 *
 * This class just forwards calls to the RubberBand library.
 */

#include "global.h"
#include "RageSoundReader_RubberBand.h"
#include "RageLog.h"

int OPTIONS =
	RubberBand::RubberBandStretcher::OptionProcessRealTime;

RageSoundReader_RubberBand::RageSoundReader_RubberBand( RageSoundReader *pSource ):
	RageSoundReader_Filter( NULL ),
	m_RubberBand(
			pSource->GetSampleRate(),
			pSource->GetNumChannels(),
			OPTIONS)
{
	m_pSource = pSource;
}

RageSoundReader_RubberBand::RageSoundReader_RubberBand( const RageSoundReader_RubberBand &cpy ):
	RageSoundReader_Filter( cpy ),
	m_RubberBand(
			cpy.m_pSource->GetSampleRate(),
			cpy.m_pSource->GetNumChannels(),
			OPTIONS)
{
	m_pSource = cpy.m_pSource;
}

int RageSoundReader_RubberBand::Read( float *pBuf, int iFrames )
{
	int iStride = m_pSource->GetNumChannels();
	float ** pTmpBufs = (float **)alloca(iStride * sizeof(float *));
	while ( m_RubberBand.available() < iFrames )
	{
		size_t uSamplesNeeded = m_RubberBand.getSamplesRequired();
		float * pTmpBufInterleaved = (float *)alloca(uSamplesNeeded * sizeof(float) * iStride);
		for (int i = 0; i < iStride; ++i) {
			pTmpBufs[i] = (float *)alloca(uSamplesNeeded * sizeof(float));
		}
		int iFramesSoFar = 0;
		while (uSamplesNeeded != 0) {
			int iFramesRead = m_pSource->Read(pTmpBufInterleaved, uSamplesNeeded);
			if (iFramesRead < 0) {
				return iFramesRead;
			}
			if (iFramesRead == 0) {
				printf("0 frames read\n");
			}
			// We need to deinterleave the data
			for (int i = 0; i < iFramesRead; ++i) {
				for (int j = 0; j < iStride; ++j) {
					pTmpBufs[j][i + iFramesSoFar] = pTmpBufInterleaved[i*iStride + j];
				}
			}
			if ((size_t)iFramesRead > uSamplesNeeded) {
				uSamplesNeeded = 0;
			} else {
				uSamplesNeeded -= iFramesRead;
				iFramesSoFar += iFramesRead;
			}
		}
		m_RubberBand.process(pTmpBufs, (size_t)iFramesSoFar, false);
	}

	// Read data out and interleave
	for (int i = 0; i < iStride; ++i) {
		pTmpBufs[i] = (float *)alloca(iFrames * sizeof(float));
	}
	int iFramesRead = (int)m_RubberBand.retrieve(pTmpBufs, iFrames);
	for (int i = 0; i < iFramesRead; ++i) {
		for (int j = 0; j < iStride; ++j) {
			pBuf[i * iStride + j] = pTmpBufs[j][i];
		}
	}
	return iFramesRead;
}

bool RageSoundReader_RubberBand::SetProperty( const RString &sProperty, float fValue )
{
	if( sProperty == "Rate" )
	{
		/* Don't propagate this.  m_pResample will take it, but it's under
		 * our control. */
		return false;
	}
	if( sProperty == "Speed" )
	{
		SetSpeedRatio( fValue );
		return true;
	}

	if( sProperty == "Pitch" )
	{
		SetPitchRatio( fValue );
		return true;
	}

	return RageSoundReader_Filter::SetProperty( sProperty, fValue );
}

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
