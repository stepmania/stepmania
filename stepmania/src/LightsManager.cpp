#include "global.h"
/*
-----------------------------------------------------------------------------
 Class: LightsManager

 Desc: See header.

 Copyright (c) 2001-2002 by the person(s) listed below.  All rights reserved.
	Chris Danford
-----------------------------------------------------------------------------
*/

#include "LightsManager.h"
#include "GameState.h"
#include "RageTimer.h"
#include "arch/Lights/LightsDriver.h"
#include "arch/arch.h"
#include "RageUtil.h"
#include "GameInput.h"	// for GameController
#include "InputMapper.h"


static const CString CabinetLightNames[NUM_CABINET_LIGHTS] = {
	"MarqueeUpLeft",
	"MarqueeUpRight",
	"MarqueeLrLeft",
	"MarqueeLrRight",
	"ButtonsLeft",
	"ButtonsRight",
	"BassLeft",
	"BassRight",
};
XToString( CabinetLight );

static const CString LightsModeNames[NUM_LIGHTS_MODES] = {
	"Attract",
	"Joining",
	"Menu",
	"Demonstration",
	"Gameplay",
	"Stage",
	"Cleared",
	"Test",
};
XToString( LightsMode );


LightsManager*	LIGHTSMAN = NULL;	// global and accessable from anywhere in our program

LightsManager::LightsManager(CString sDriver)
{
	m_LightsMode = LIGHTSMODE_JOINING;

	m_pDriver = MakeLightsDriver(sDriver);

	m_fSecsLeftInMarqueeBlink = 0;
	m_fSecsLeftInBassBlink = 0;
}

LightsManager::~LightsManager()
{
	SAFE_DELETE( m_pDriver );
}

void LightsManager::Update( float fDeltaTime )
{
	m_fSecsLeftInMarqueeBlink = max( 0, m_fSecsLeftInMarqueeBlink - fDeltaTime );
	m_fSecsLeftInBassBlink =	max( 0, m_fSecsLeftInBassBlink - fDeltaTime );

	//
	// Update cabinet lights
	//
	FOREACH_CabinetLight( cl )
		m_LightsState.m_bCabinetLights[cl] = false;

	switch( m_LightsMode )
	{
	case LIGHTSMODE_JOINING:
		{
			int iSec = (int)(RageTimer::GetTimeSinceStart()*4);
			bool bBlinkOn = (iSec%2)==0; 

			FOREACH_PlayerNumber( pn )
			{
				if( GAMESTATE->m_bSideIsJoined[pn] )
				{
					m_LightsState.m_bCabinetLights[LIGHT_MARQUEE_UP_LEFT+pn] = true;
					m_LightsState.m_bCabinetLights[LIGHT_MARQUEE_LR_LEFT+pn] = true;
					m_LightsState.m_bCabinetLights[LIGHT_BUTTONS_LEFT+pn] = true;
					m_LightsState.m_bCabinetLights[LIGHT_BASS_LEFT+pn] = true;
				}
				else
				{
					m_LightsState.m_bCabinetLights[LIGHT_BUTTONS_LEFT+pn] = bBlinkOn;
				}
			}
		}
		break;
	case LIGHTSMODE_ATTRACT:
		{
			int iSec = (int)RageTimer::GetTimeSinceStart();
			int iTopIndex = iSec % 4;
			switch( iTopIndex )
			{
			case 0:	m_LightsState.m_bCabinetLights[LIGHT_MARQUEE_UP_LEFT]  = true;	break;
			case 1:	m_LightsState.m_bCabinetLights[LIGHT_MARQUEE_LR_RIGHT] = true;	break;
			case 2:	m_LightsState.m_bCabinetLights[LIGHT_MARQUEE_UP_RIGHT] = true;	break;
			case 3:	m_LightsState.m_bCabinetLights[LIGHT_MARQUEE_LR_LEFT]  = true;	break;
			default:	ASSERT(0);
			}

			bool bOn = (iSec%4)==0;
			m_LightsState.m_bCabinetLights[LIGHT_BASS_LEFT]			= bOn;
			m_LightsState.m_bCabinetLights[LIGHT_BASS_RIGHT]		= bOn;
		}
		break;
	case LIGHTSMODE_MENU:
		{
			FOREACH_CabinetLight( cl )
				m_LightsState.m_bCabinetLights[cl] = false;

			int iBeat = (int)(GAMESTATE->m_fSongBeat);
			int iTopIndex = iBeat % 4;
			switch( iTopIndex )
			{
			case 0:	m_LightsState.m_bCabinetLights[LIGHT_MARQUEE_UP_LEFT]  = true;	break;
			case 1:	m_LightsState.m_bCabinetLights[LIGHT_MARQUEE_LR_RIGHT] = true;	break;
			case 2:	m_LightsState.m_bCabinetLights[LIGHT_MARQUEE_UP_RIGHT] = true;	break;
			case 3:	m_LightsState.m_bCabinetLights[LIGHT_MARQUEE_LR_LEFT]  = true;	break;
			default:	ASSERT(0);
			}

			bool bBlinkOn = (iBeat%2)==0;

			FOREACH_PlayerNumber( pn )
			{
				if( GAMESTATE->m_bSideIsJoined[pn] )
				{
					m_LightsState.m_bCabinetLights[LIGHT_BUTTONS_LEFT+pn] = bBlinkOn;
				}
			}
		}
		break;
	case LIGHTSMODE_DEMONSTRATION:
	case LIGHTSMODE_GAMEPLAY:
		{
			FOREACH_CabinetLight( cl )
				m_LightsState.m_bCabinetLights[cl] = false;

			bool bAllMarqueeLightsOn = !GAMESTATE->m_bPastHereWeGo;

			bool bOn;
			
			bOn = bAllMarqueeLightsOn || (m_fSecsLeftInMarqueeBlink > 0);
			m_LightsState.m_bCabinetLights[LIGHT_MARQUEE_UP_LEFT]	= bOn;
			m_LightsState.m_bCabinetLights[LIGHT_MARQUEE_UP_RIGHT]	= bOn;
			m_LightsState.m_bCabinetLights[LIGHT_MARQUEE_LR_LEFT]	= bOn;
			m_LightsState.m_bCabinetLights[LIGHT_MARQUEE_LR_RIGHT]	= bOn;

			bOn = m_fSecsLeftInBassBlink > 0;
			m_LightsState.m_bCabinetLights[LIGHT_BASS_LEFT]			= bOn;
			m_LightsState.m_bCabinetLights[LIGHT_BASS_RIGHT]		= bOn;
		}
		break;
	case LIGHTSMODE_STAGE:
	case LIGHTSMODE_ALL_CLEARED:
		{
			FOREACH_CabinetLight( cl )
				m_LightsState.m_bCabinetLights[cl] = true;
		}
		break;
	case LIGHTSMODE_TEST:
		{
			int iSec = (int)RageTimer::GetTimeSinceStart();
			FOREACH_CabinetLight( cl )
			{
				bool bOn = (iSec%NUM_CABINET_LIGHTS) == cl;
				m_LightsState.m_bCabinetLights[cl] = bOn;
			}
		}
		break;
	default:
		ASSERT(0);
	}

	//
	// Update game controller lights
	//
	// FIXME:  Works only with Game==dance
	// FIXME:  lights pads for players who aren't playing
	switch( m_LightsMode )
	{
	case LIGHTSMODE_ATTRACT:
	case LIGHTSMODE_MENU:
	case LIGHTSMODE_DEMONSTRATION:
		{
			ZERO( m_LightsState.m_bGameButtonLights );
		}
		break;
	case LIGHTSMODE_ALL_CLEARED:
	case LIGHTSMODE_STAGE:
	case LIGHTSMODE_JOINING:
		{
			FOREACH_GameController( gc )
			{
				bool bOn = GAMESTATE->m_bSideIsJoined[gc];

				FOREACH_GameButton( gb )
					m_LightsState.m_bGameButtonLights[gc][gb] = bOn;
			}
		}
		break;
	case LIGHTSMODE_GAMEPLAY:
		{
			FOREACH_GameController( gc )
			{
				FOREACH_GameButton( gb )
				{
					bool bOn = INPUTMAPPER->IsButtonDown( GameInput(gc,gb) );
					m_LightsState.m_bGameButtonLights[gc][gb] = bOn;
				}
			}
		}
		break;
	case LIGHTSMODE_TEST:
		{
			int iSec = (int)RageTimer::GetTimeSinceStart();

			FOREACH_GameController( gc )
			{
				FOREACH_GameButton( gb )
				{
					bool bOn = (iSec%MAX_GAME_BUTTONS)==gb;
					m_LightsState.m_bGameButtonLights[gc][gb] = bOn;
				}
			}
		}
		break;
	default:
		ASSERT(0);
	}

	// apply new light values we set above
	m_pDriver->Set( &m_LightsState );
}

void LightsManager::GameplayBlinkMarqueeLights()
{
	m_fSecsLeftInMarqueeBlink = 0.1f;
}

void LightsManager::GameplayBlinkBassLights()
{
	m_fSecsLeftInBassBlink = 0.1f;
}

void LightsManager::SetLightsMode( LightsMode lm )
{
	m_LightsMode = lm;
}

LightsMode LightsManager::GetLightsMode()
{
	return m_LightsMode;
}
