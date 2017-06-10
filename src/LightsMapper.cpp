/* Please note:
 * Cabinet mappings (lights + counter) are global ([PIUIO]).
 * Game buttons are divided by game and player ([PIUIO-dance-P1]).
 * It is possible to overlap buttons, and it is
 * possible to specify numbers that are never used.
 * (I may implement something to limit this later.)
 */

#include "global.h"
#include "RageLog.h"
#include "GameState.h"
#include "Game.h"
#include "IniFile.h"
#include "LightsMapper.h"

#define LIGHTS_INI_PATH "Data/LightsMaps.ini"

uint32_t LightsMapping::GetLightsField( const LightsState *ls ) const
{
	uint32_t ret = 0;

	if( !ls )
		return ret;

	FOREACH_CabinetLight( cl )
		if( ls->m_bCabinetLights[cl] )
			ret |= m_iCabinetLights[cl];

	FOREACH_GameController( gc )
		FOREACH_GameButton( gb )
			if( ls->m_bGameButtonLights[gc][gb] )
				ret |= m_iGameLights[gc][gb];

	ret |= m_iCoinCounter[ls->m_bCoinCounter ? 1 : 0];

	return ret;
}

/* It is important to note that ToMapping writes to the
 * input; FromMapping does not. This is because we never
 * have a reason to convert a LightsMapping value to a
 * proper number from a bit mapping. */
static void ToMapping( const CString &sBits, uint32_t &iByte )
{
	// reset
	iByte = 0;
	CStringArray sArray;
	split( sBits, ",", sArray );

	if( sArray.empty() )
		return;

	for( unsigned char i = 0; i < sArray.size(); i++ )
	{
		unsigned shift = atoi( sArray[i].c_str() );

		if( shift > 0 || shift <= 32 )
			iByte |= (1 << (32-shift));
		else
			LOG->Warn( "ToMapping(): invalid value \"%u\" (1 << %u)", shift, (1 << (31-shift)) );
	}
}

static CString FromMapping( uint32_t iMap )
{
	CStringArray sArray;

	// offset: our test starts at 0, ini values start at 1	
	for( int i = 0; i < 32; i++ )
		if( iMap & (1 << (31-i)) )
			sArray.push_back( ssprintf("%u", i+1) );

	return join( ",", sArray );
}

void LightsMapper::LoadMappings( const CString &sDeviceName, LightsMapping &mapping )
{
	IniFile ini;

	const Game* pGame = GAMESTATE->GetCurrentGame();

	// check for a pre-existing key set
	if( !ini.ReadFile( LIGHTS_INI_PATH ) || ini.GetChild( sDeviceName ) == NULL )
	{
		LOG->Warn( "Mappings not set for device \"%s\", game \"%s\". Writing default mappings.",
			sDeviceName.c_str(), pGame->m_szName );

		LightsMapper::WriteMappings( sDeviceName, mapping );
		return;
	}

	// load the INI values into this, then convert them as needed.
	CString sBuffer;

	// load cabinet-light data and convert the string to a uint32_t
	FOREACH_CabinetLight( cl )
	{
		ini.GetValue( sDeviceName, CabinetLightToString(cl), sBuffer );
		ToMapping( sBuffer, mapping.m_iCabinetLights[cl] );
	}

	ini.GetValue( sDeviceName, "CoinCounterOn", sBuffer );
	ToMapping( sBuffer, mapping.m_iCoinCounter[1] );
	ini.GetValue( sDeviceName, "CoinCounterOff", sBuffer );
	ToMapping( sBuffer, mapping.m_iCoinCounter[0] );

	// "PIUIO-dance-", "PIUIO-pump-", etc.
	CString sBaseKey = sDeviceName + "-" + pGame->m_szName + "-";

	FOREACH_GameController( gc )
	{
		// [PIUIO-dance-P1], [PIUIO-dance-P2]
		CString sPlayerKey = sBaseKey + PlayerNumberToString((PlayerNumber)gc);

		// only read up to the last-set game button, which is the operator button
		FOREACH_ENUM( GameButton, pGame->ButtonNameToIndex("Operator"), gb )
		{
			ini.GetValue( sPlayerKey, GameButtonToString(pGame, gb), sBuffer );
			ToMapping( sBuffer, mapping.m_iGameLights[gc][gb] );
		}
	}

	// re-write, to update all the values
	LightsMapper::WriteMappings( sDeviceName, mapping );
}

void LightsMapper::WriteMappings( const CString &sDeviceName, LightsMapping &mapping )
{
	IniFile ini;
	ini.ReadFile( LIGHTS_INI_PATH );

	const Game* pGame = GAMESTATE->GetCurrentGame();

	// set cabinet and counter data
	ini.SetValue( sDeviceName, "CoinCounterOn", FromMapping(mapping.m_iCoinCounter[1]) );
	ini.SetValue( sDeviceName, "CoinCounterOff", FromMapping(mapping.m_iCoinCounter[0]) );

	FOREACH_CabinetLight( cl )
		ini.SetValue( sDeviceName, CabinetLightToString(cl), FromMapping(mapping.m_iCabinetLights[cl]) );

	// "PIUIO-dance-", "PIUIO-pump-", etc.
	CString sBaseKey = sDeviceName + "-" + pGame->m_szName + "-";

	FOREACH_GameController( gc )
	{
		// [PIUIO-dance-P1], [PIUIO-dance-P2]
		CString sPlayerKey = sBaseKey + PlayerNumberToString((PlayerNumber)gc);

		// only read up to the last-set game button, which is the operator button
		FOREACH_ENUM( GameButton, pGame->ButtonNameToIndex("Operator"), gb )
			ini.SetValue( sPlayerKey, GameButtonToString(pGame, gb),
				FromMapping(mapping.m_iGameLights[gc][gb]) );
	}

	ini.WriteFile( LIGHTS_INI_PATH );
}

/*
 * (c) 2008 BoXoRRoXoRs
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
