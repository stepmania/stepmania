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


LightsManager*	LIGHTSMAN = NULL;	// global and accessable from anywhere in our program

LightsManager::LightsManager(CString sDriver)
{
	m_LightMode = LIGHTMODE_JOINING;

	m_pDriver = MakeLightsDriver(sDriver);
}

LightsManager::~LightsManager()
{
	SAFE_DELETE( m_pDriver );
}

void LightsManager::Update( float fDeltaTime )
{
	//
	// Update cabinet lights
	//
	switch( m_LightMode )
	{
	case LIGHTMODE_JOINING:
		{
			switch( GAMESTATE->GetNumSidesJoined() )
			{
			case 0:
				{
					int iSec = (int)(RageTimer::GetTimeSinceStart()*2);
					float fBeatPercentage = GAMESTATE->m_fSongBeat - (int)GAMESTATE->m_fSongBeat;
					bool bOn = (iSec%2)==0; 
					for( int i=0; i<8; i++ )
						m_pDriver->SetLight( (Light)i, bOn );
				}
				break;
			default:
				for( int i=0; i<8; i++ )
				{
					bool bOn = GAMESTATE->m_bSideIsJoined[i%2];
					m_pDriver->SetLight( (Light)i, bOn );
				}
			}
		}
		break;
	case LIGHTMODE_ATTRACT:
		{
			int iSec = (int)RageTimer::GetTimeSinceStart();
			int i;
			for( i=0; i<4; i++ )
			{
				bool bOn = (iSec%4)==i;
				m_pDriver->SetLight( (Light)i, bOn );
			}
			for( i=4; i<6; i++ )
			{
				bool bOn = false;
				m_pDriver->SetLight( (Light)i, bOn );
			}
			for( i=6; i<8; i++ )
			{
				bool bOn = (iSec%3)==0;
				m_pDriver->SetLight( (Light)i, bOn );
			}
		}
		break;
	case LIGHTMODE_MENU:
		{
			int iSec = (int)RageTimer::GetTimeSinceStart();
			int i;
			for( i=0; i<2; i++ )
			{
				bool bOn = (iSec%2)==0;
				m_pDriver->SetLight( (Light)i, bOn );
			}
			for( i=2; i<4; i++ )
			{
				bool bOn = (iSec%2)==1;
				m_pDriver->SetLight( (Light)i, bOn );
			}
			for( i=4; i<6; i++ )
			{
				bool bOn = (iSec%2)==0;
				m_pDriver->SetLight( (Light)i, bOn );
			}
			for( i=6; i<8; i++ )
			{
				bool bOn = (iSec%2)==1;
				m_pDriver->SetLight( (Light)i, bOn );
			}
		}
		break;
	case LIGHTMODE_DEMONSTRATION:
	case LIGHTMODE_GAMEPLAY:
		{
			int i;

			// top lights are controlled my ScreenGameplay
			//for( int i=0; i<4; i++ )
			//	m_pDriver->SetLight( (Light)i, bOn );

			// menu lights are always off
			for( i=4; i<6; i++ )
				m_pDriver->SetLight( (Light)i, false );

			// bass lights
			float fBeatPercentage = GAMESTATE->m_fSongBeat - (int)GAMESTATE->m_fSongBeat;
			bool bOn = fBeatPercentage < 0.1 || fBeatPercentage > 0.9; 
			for( i=7; i<8; i++ )
				m_pDriver->SetLight( (Light)i, bOn );
		}
		break;
	case LIGHTMODE_STAGE:
		{
			for( int i=0; i<8; i++ )
				m_pDriver->SetLight( (Light)i, true );
		}
		break;
	case LIGHTMODE_ALL_CLEARED:
		{
			for( int i=0; i<8; i++ )
				m_pDriver->SetLight( (Light)i, false );
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
	switch( m_LightMode )
	{
	case LIGHTMODE_ATTRACT:
	case LIGHTMODE_MENU:
	case LIGHTMODE_DEMONSTRATION:
		{
			for( int i=LIGHT_GAME_BUTTON1; i<=LIGHT_LAST_GAME_BUTTON; i++ )
				m_pDriver->SetLight( (Light)i, false );
		}
		break;
	case LIGHTMODE_ALL_CLEARED:
	case LIGHTMODE_STAGE:
	case LIGHTMODE_JOINING:
		{
			for( int c=0; c<2; c++ )
			{
				GameController gc = (GameController)c;
				bool bOn = GAMESTATE->m_bSideIsJoined[c];

				for( int i=0; i<4; i++ )
				{
					GameButton gb = (GameButton)i;
					Light light = (Light)(LIGHT_GAME_BUTTON1 + c*4+i);
					m_pDriver->SetLight( light, bOn );
				}
			}
		}
		break;
	case LIGHTMODE_GAMEPLAY:
		{
			for( int c=0; c<2; c++ )
			{
				GameController gc = (GameController)c;

				for( int i=0; i<4; i++ )
				{
					GameButton gb = (GameButton)i;

					Light light = (Light)(LIGHT_GAME_BUTTON1 + c*4+i);

					bool bOn = INPUTMAPPER->IsButtonDown( GameInput(gc,gb) );
					m_pDriver->SetLight( light, bOn );
				}
			}
		}
		break;
	default:
		ASSERT(0);
	}

	// apply new light values we set above
	m_pDriver->Flush();
}

void LightsManager::SetLightMode( LightMode lm )
{
	m_LightMode = lm;
}

void LightsManager::SetAllUpperLights( bool bOn )
{
	for( int i=0; i<4; i++ )
	{
		m_pDriver->SetLight( (Light)i, bOn );
	}
}
