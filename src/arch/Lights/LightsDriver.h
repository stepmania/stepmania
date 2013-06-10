/* LightsDriver - Controls lights */

#ifndef LightsDriver_H
#define LightsDriver_H

#include "LightsManager.h"
#include "arch/RageDriver.h"

struct LightsState;

class LightsDriver: public RageDriver
{
public:
	static void Create( const RString &sDriver, vector<LightsDriver *> &apAdd );
	static DriverList m_pDriverList;

	LightsDriver() {};
	virtual ~LightsDriver() {};
	
	virtual void Set( const LightsState *ls ) = 0;
};

#define REGISTER_SOUND_DRIVER_CLASS2( name, x ) \
	static RegisterRageDriver register_##x( &LightsDriver::m_pDriverList, #name, CreateClass<LightsDriver_##x, RageDriver> )
#define REGISTER_SOUND_DRIVER_CLASS( name ) REGISTER_SOUND_DRIVER_CLASS2( name, name )

#endif

/*
 * (c) 2003-2004 Chris Danford
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
