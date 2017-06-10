// Cross-platform, libusb-based driver for outputting lights
// via a PacDrive (http://www.ultimarc.com/pacdrive.html)

#include "global.h"
#include "RageLog.h"
#include "LightsMapper.h"
#include "io/PacDrive.h"
#include "LightsDriver_PacDrive.h"

REGISTER_LIGHTS_DRIVER( PacDrive );

LightsDriver_PacDrive::LightsDriver_PacDrive()
{
	m_bHasDevice = Board.Open();

	if( m_bHasDevice == false )
	{
		LOG->Warn( "Could not establish a connection with PacDrive." );
		return;
	}

	// load any alternate lights mappings
	SetLightsMappings();

	// clear all lights
	Board.Write( 0 );
}

void LightsDriver_PacDrive::SetLightsMappings()
{
	uint32_t iCabinetLights[NUM_CABINET_LIGHTS] =
	{
		// up-left, up-right, down-left, down-right marquees
		(1 << 0), (1 << 1), (1 << 2), (1 << 3),

		// left buttons, right buttons, left bass, right bass
		(1 << 4), (1 << 5), (1 << 6), (1 << 7)
	};

	uint32_t iGameLights[MAX_GAME_CONTROLLERS][MAX_GAME_BUTTONS] =
	{
		// left, right, up, down
		{ ( 1 << 8), (1 << 9), (1 << 10), (1 << 11) },		// player 1
		{ ( 1 << 12), (1 << 13), (1 << 14), (1 << 15) },	// player 2
	};

	m_LightsMappings.SetCabinetLights( iCabinetLights );
	m_LightsMappings.SetCustomGameLights( iGameLights );

	LightsMapper::LoadMappings( "PacDrive", m_LightsMappings );
}

LightsDriver_PacDrive::~LightsDriver_PacDrive()
{
	if( !m_bHasDevice )
		return;

	// clear all lights and close the connection
	Board.Write( 0 );
	Board.Close();
}

void LightsDriver_PacDrive::Set( const LightsState *ls )
{
	if( !m_bHasDevice )
		return;

	uint16_t iWriteData = 0;

	// Lights 1 - 8 are used for the cabinet lights
	FOREACH_CabinetLight( cl )
		if( ls->m_bCabinetLights[cl] )
			iWriteData |= m_LightsMappings.m_iCabinetLights[cl];

	// Lights 9-12 for P1 pad, 13-16 for P2 pad
	// FIXME: make this work for all game-types?
	FOREACH_GameController( gc )
		FOREACH_GameButton( gb )
			if( ls->m_bGameButtonLights[gc][gb] )
				iWriteData |= m_LightsMappings.m_iGameLights[gc][gb];


	// write the data - if it fails, stop updating
	if( !Board.Write(iWriteData) )
	{
		LOG->Warn( "Lost connection with PacDrive." );
		m_bHasDevice = false;
	}
}

/*
 * Copyright (c) 2008 BoXoRRoXoRs
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
