#ifndef RAGE_SOUND_NULL
#define RAGE_SOUND_NULL

#include "RageSoundDriver_Generic_Software.h"

class RageSound_Null: public RageSound_Generic_Software
{
private:
	int64_t last_cursor_pos;

protected:
	int64_t GetPosition( const RageSoundBase *snd ) const;
	float GetPlayLatency() const;
	void Update( float fDeltaTime );

public:
    RageSound_Null();
};

#endif

/*
 * Copyright (c) 2003 by the person(s) listed below.  All rights reserved.
 *
 * Glenn Maynard
 * 
 * 2003-02   Modified to fake playing sound   Aaron VonderHaar
 * 
 */
