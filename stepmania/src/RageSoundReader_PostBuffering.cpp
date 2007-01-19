#include "global.h"
#include "RageSoundReader_PostBuffering.h"
#include "RageSoundUtil.h"

/*
 * This filter is normally inserted after extended buffering, implementing
 * properties that do not seek the sound, allowing these properties to be
 * changed with low latency.
 */

RageSoundReader_PostBuffering::RageSoundReader_PostBuffering( RageSoundReader *pSource ):
	RageSoundReader_Filter( pSource )
{
	m_fVolume = 1.0f;
}


int RageSoundReader_PostBuffering::Read( char *pBuf, int iFrames )
{
	iFrames = m_pSource->Read( pBuf, iFrames );
	if( iFrames < 0 )
		return iFrames;

	if( m_fVolume != 1.0f )
		RageSoundUtil::Attenuate( (int16_t *) pBuf, iFrames * this->GetNumChannels(), m_fVolume );

	return iFrames;
}

bool RageSoundReader_PostBuffering::SetProperty( const RString &sProperty, float fValue )
{
	if( sProperty == "Volume" )
	{
		m_fVolume = fValue;
		return true;
	}

	return RageSoundReader_Filter::SetProperty( sProperty, fValue );
}

/*
 * Copyright (c) 2007 Glenn Maynard
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
