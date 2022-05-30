#include "global.h"
#include "LightsDriver.h"
#include "RageLog.h"

#include "arch/arch_default.h"

//We explicitly load these drivers as they should always be available.
#include "arch/Lights/LightsDriver_SystemMessage.h"
#include "arch/Lights/LightsDriver_Export.h"

DriverList LightsDriver::m_pDriverList;

void LightsDriver::Create( const RString &sDrivers, vector<LightsDriver *> &Add )
{
	LOG->Trace( "Initializing lights drivers: %s", sDrivers.c_str() );

	vector<RString> asDriversToTry;
	split( sDrivers, ",", asDriversToTry, true );
	
	for (RString const &Driver : asDriversToTry)
	{
		RageDriver *pRet = m_pDriverList.Create( Driver );
		if( pRet == nullptr )
		{
			LOG->Trace( "Unknown lights driver: %s", Driver.c_str() );
			continue;
		}

		LightsDriver *pDriver = dynamic_cast<LightsDriver *>( pRet );
		ASSERT( pDriver != nullptr );

		LOG->Info( "Lights driver: %s", Driver.c_str() );
		Add.push_back( pDriver );
	}

	//ensure these are always available to use for debugging
	//or if InputHandlers that want lighting state.
	Add.push_back(new LightsDriver_SystemMessage);
	Add.push_back(new LightsDriver_Export);
}

void LightsDriver::Reset()
{
	LightsState state;
	ZERO( state.m_bCabinetLights );
	ZERO( state.m_bGameButtonLights );
	ZERO( state.m_bCoinCounter );
	Set( &state );
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
