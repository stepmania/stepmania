#include "global.h"
#include "RageSoundDriver_Null.h"
#include "RageLog.h"
#include "RageUtil.h"

const int channels = 2;
const int samplerate = 44100;

void RageSound_Null::Update( float fDeltaTime )
{
	/* "Play" frames. */
	while( last_cursor_pos < GetPosition(NULL)+1024*4 )
	{
		int16_t buf[256*channels];
		this->Mix( buf, 256, last_cursor_pos, GetPosition(NULL) );
		last_cursor_pos += 256;
	}

	RageSound_Generic_Software::Update( fDeltaTime );
}

int64_t RageSound_Null::GetPosition( const RageSoundBase *snd ) const
{
	return int64_t( RageTimer::GetTimeSinceStart() * samplerate );
}

RageSound_Null::RageSound_Null()
{
	last_cursor_pos = GetPosition( NULL );

	StartDecodeThread();
}

float RageSound_Null::GetPlayLatency() const
{
	return 0;  /* silence is fast! */
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
