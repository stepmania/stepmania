#include "global.h"
#include "LightsDriver.h"
#include "RageLog.h"
#include "Foreach.h"
#include "arch/arch_default.h"

DriverList LightsDriver::m_pDriverList;

void LightsDriver::Create( const RString &sDrivers, vector<LightsDriver *> &Add )
{
	LOG->Trace( "Initializing lights drivers: %s", sDrivers.c_str() );

	vector<RString> asDriversToTry;
	split( sDrivers, ",", asDriversToTry, true );
	
	FOREACH_CONST( RString, asDriversToTry, Driver )
	{
		RageDriver *pRet = m_pDriverList.Create( *Driver );
		if( pRet == NULL )
		{
			LOG->Trace( "Unknown lights driver: %s", Driver->c_str() );
			continue;
		}

		LightsDriver *pDriver = dynamic_cast<LightsDriver *>( pRet );
		ASSERT( pDriver != NULL );

		LOG->Info( "Lights driver: %s", Driver->c_str() );
		Add.push_back( pDriver );
	}
}

/*
 * (c) 2002-2005 Glenn Maynard
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
