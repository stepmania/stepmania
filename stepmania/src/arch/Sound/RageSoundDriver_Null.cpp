#include "global.h"
#include "RageSoundDriver_Null.h"
#include "RageLog.h"
#include "RageUtil.h"
#include "PrefsManager.h"

REGISTER_SOUND_DRIVER_CLASS( Null );

const int channels = 2;

void RageSoundDriver_Null::Update()
{
	/* "Play" frames. */
	while( m_iLastCursorPos < GetPosition(NULL)+1024*4 )
	{
		int16_t buf[256*channels];
		this->Mix( buf, 256, m_iLastCursorPos, GetPosition(NULL) );
		m_iLastCursorPos += 256;
	}

	RageSound_Generic_Software::Update();
}

int64_t RageSoundDriver_Null::GetPosition( const RageSoundBase *snd ) const
{
	return int64_t( RageTimer::GetTimeSinceStart() * m_iSampleRate );
}

RageSoundDriver_Null::RageSoundDriver_Null()
{
	m_iSampleRate = PREFSMAN->m_iSoundPreferredSampleRate;
	if( m_iSampleRate == 0 )
		m_iSampleRate = 44100;
	m_iLastCursorPos = GetPosition( NULL );
	StartDecodeThread();
}

int RageSoundDriver_Null::GetSampleRate( int iRate ) const
{
	return m_iSampleRate;
}

/*
 * (c) 2002-2004 Glenn Maynard, Aaron VonderHaar
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
