#include "global.h"
#include "RageSoundDriver_Null.h"
#include "RageLog.h"
#include "RageUtil.h"

const int channels = 2;
const int samplerate = 44100;

void RageSound_Null::Update( float fDeltaTime )
{
	/* "Play" frames. */
	while( last_cursor_pos < GetPosition(NULL)+1024 )
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
 * Copyright (c) 2002-2004 by the person(s) listed below.  All rights reserved.
 *
 * Glenn Maynard
 * Aaron VonderHaar
 */
