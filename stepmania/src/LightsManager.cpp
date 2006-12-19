#include "global.h"
#include "LightsManager.h"
#include "GameState.h"
#include "RageTimer.h"
#include "arch/Lights/LightsDriver.h"
#include "RageUtil.h"
#include "GameInput.h"	// for GameController
#include "InputMapper.h"
#include "Game.h"
#include "PrefsManager.h"
#include "Actor.h"
#include "Preference.h"
#include "Foreach.h"
#include "GameManager.h"
#include "CommonMetrics.h"
#include "Style.h"

const RString DEFAULT_LIGHTS_DRIVER = "SystemMessage,Export";
static Preference<RString> g_sLightsDriver( "LightsDriver", "" ); // "" == DEFAULT_LIGHTS_DRIVER
Preference<float>	g_fLightsFalloffSeconds( "LightsFalloffSeconds", 0.1f );
Preference<float>	g_fLightsAheadSeconds( "LightsAheadSeconds", 0.05f );
static Preference<bool>	g_bBlinkGameplayButtonLightsOnNote( "BlinkGameplayButtonLightsOnNote", false );

static const char *CabinetLightNames[] = {
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
StringToX( CabinetLight );

static const char *LightsModeNames[] = {
	"Attract",
	"Joining",
	"Menu",
	"Demonstration",
	"Gameplay",
	"Stage",
	"Cleared",
	"TestAutoCycle",
	"TestManualCycle",
};
XToString( LightsMode );

static void GetUsedGameInputs( vector<GameInput> &vGameInputsOut )
{
	vGameInputsOut.clear();

	set<GameInput> vGIs;
	vector<const Style*> vStyles;
	GAMEMAN->GetStylesForGame( GAMESTATE->m_pCurGame, vStyles );
	FOREACH( const Style*, vStyles, style )
	{
		bool bFound = find( CommonMetrics::STEPS_TYPES_TO_SHOW.GetValue().begin(), CommonMetrics::STEPS_TYPES_TO_SHOW.GetValue().end(), (*style)->m_StepsType ) != CommonMetrics::STEPS_TYPES_TO_SHOW.GetValue().end();
		if( !bFound )
			continue;
		FOREACH_PlayerNumber( pn )
		{
			for( int iCol=0; iCol<(*style)->m_iColsPerPlayer; ++iCol )
			{
				GameInput gi = (*style)->StyleInputToGameInput( iCol, pn );
				if( gi.IsValid() )
				{
					vGIs.insert( gi );
				}
			}
		}
	}
	FOREACHS_CONST( GameInput, vGIs, gi )
		vGameInputsOut.push_back( *gi );
}

LightsManager*	LIGHTSMAN = NULL;	// global and accessable from anywhere in our program

LightsManager::LightsManager()
{
	ZERO( m_fSecsLeftInCabinetLightBlink );
	ZERO( m_fSecsLeftInGameButtonBlink );
	ZERO( m_fActorLights );
	ZERO( m_fSecsLeftInActorLightBlink );
	m_iQueuedCoinCounterPulses = 0;
	m_CoinCounterTimer.SetZero();

	m_LightsMode = LIGHTSMODE_JOINING;
	RString sDriver = g_sLightsDriver.Get();
	if( sDriver.empty() )
		sDriver = DEFAULT_LIGHTS_DRIVER;
	LightsDriver::Create( sDriver, m_vpDrivers );
	m_fTestAutoCycleCurrentIndex = 0;
	m_clTestManualCycleCurrent = CabinetLight_Invalid;
	m_iControllerTestManualCycleCurrent = -1;
}

LightsManager::~LightsManager()
{
	FOREACH( LightsDriver*, m_vpDrivers, iter )
		SAFE_DELETE( *iter );
	m_vpDrivers.clear();
}

// XXX: make themable
static const float g_fLightEffectRiseSeconds = 0.075f;
static const float g_fLightEffectFalloffSeconds = 0.35f;
static const float g_fCoinPulseTime = 0.100f;

void LightsManager::BlinkActorLight( CabinetLight cl )
{
	m_fSecsLeftInActorLightBlink[cl] = g_fLightEffectRiseSeconds;
}

float LightsManager::GetActorLightLatencySeconds() const
{
	return g_fLightEffectRiseSeconds;
}

void LightsManager::Update( float fDeltaTime )
{
	//
	// Update actor effect lights.
	//
	FOREACH_CabinetLight( cl )
	{
		float fTime = fDeltaTime;
		float &fDuration = m_fSecsLeftInActorLightBlink[cl];
		if( fDuration > 0 )
		{
			/* The light has power left.  Brighten it. */
			float fSeconds = min( fDuration, fTime );
			fDuration -= fSeconds;
			fTime -= fSeconds;
			fapproach( m_fActorLights[cl], 1, fSeconds / g_fLightEffectRiseSeconds );
		}

		if( fTime > 0 )
		{
			/* The light is out of power.  Dim it. */
			fapproach( m_fActorLights[cl], 0, fTime / g_fLightEffectFalloffSeconds );
		}

		Actor::SetBGMLight( cl, m_fActorLights[cl] );
	}

	if( !IsEnabled() )
		return;

	// update lights falloff
	{
		FOREACH_CabinetLight( cl )
			fapproach( m_fSecsLeftInCabinetLightBlink[cl], 0, fDeltaTime );
		FOREACH_GameController( gc )
			FOREACH_GameButton( gb )
				fapproach( m_fSecsLeftInGameButtonBlink[gc][gb], 0, fDeltaTime );
	}

	//
	// Set new lights state cabinet lights
	//
	{
		ZERO( m_LightsState.m_bCabinetLights );
		ZERO( m_LightsState.m_bGameButtonLights );
	}

	{
		m_LightsState.m_bCoinCounter = false;
		if( !m_CoinCounterTimer.IsZero() )
		{
			float fAgo = m_CoinCounterTimer.Ago();
			if( fAgo < g_fCoinPulseTime )
				m_LightsState.m_bCoinCounter = true;
			else if( fAgo >= g_fCoinPulseTime * 2 )
				m_CoinCounterTimer.SetZero();
		}
		else if( m_iQueuedCoinCounterPulses )
		{
			m_CoinCounterTimer.Touch();
			--m_iQueuedCoinCounterPulses;
		}
	}

	if( m_LightsMode == LIGHTSMODE_TEST_AUTO_CYCLE )
	{
		m_fTestAutoCycleCurrentIndex += fDeltaTime;
		m_fTestAutoCycleCurrentIndex = fmodf( m_fTestAutoCycleCurrentIndex, NUM_CabinetLight*100 );
	}

	switch( m_LightsMode )
	{
	case LIGHTSMODE_JOINING:
		{
//			int iBeat = (int)(GAMESTATE->m_fLightSongBeat);
//			bool bBlinkOn = (iBeat%2)==0;

			FOREACH_PlayerNumber( pn )
			{
				if( GAMESTATE->m_bSideIsJoined[pn] )
				{
					m_LightsState.m_bCabinetLights[LIGHT_MARQUEE_UP_LEFT+pn] = true;
					m_LightsState.m_bCabinetLights[LIGHT_MARQUEE_LR_LEFT+pn] = true;
					m_LightsState.m_bCabinetLights[LIGHT_BUTTONS_LEFT+pn] = true;
					m_LightsState.m_bCabinetLights[LIGHT_BASS_LEFT+pn] = true;
				}
			}
		}
		break;
	case LIGHTSMODE_ATTRACT:
		{
			int iSec = (int)RageTimer::GetTimeSinceStartFast();
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

			int iBeat = (int)(GAMESTATE->m_fLightSongBeat);
			int iTopIndex = iBeat;
			wrap( iTopIndex, 4 );
			switch( iTopIndex )
			{
			case 0:	m_LightsState.m_bCabinetLights[LIGHT_MARQUEE_UP_LEFT]  = true;	break;
			case 1:	m_LightsState.m_bCabinetLights[LIGHT_MARQUEE_LR_RIGHT] = true;	break;
			case 2:	m_LightsState.m_bCabinetLights[LIGHT_MARQUEE_UP_RIGHT] = true;	break;
			case 3:	m_LightsState.m_bCabinetLights[LIGHT_MARQUEE_LR_LEFT]  = true;	break;
			default:	ASSERT(0);
			}

			/* Flash the button lights for active players. */
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
		FOREACH_CabinetLight( cl )
			m_LightsState.m_bCabinetLights[cl] = m_fSecsLeftInCabinetLightBlink[cl] > 0;
		break;
	case LIGHTSMODE_STAGE:
	case LIGHTSMODE_ALL_CLEARED:
		{
			FOREACH_CabinetLight( cl )
			{
				bool b = true;
				switch( cl )
				{
				case LIGHT_BUTTONS_LEFT:
				case LIGHT_BUTTONS_RIGHT:
					b = false;
					break;
				default:
					b = true;
					break;
				}
				m_LightsState.m_bCabinetLights[cl] = b;
			}
		}
		break;
	case LIGHTSMODE_TEST_AUTO_CYCLE:
		{
			int iSec = GetTestAutoCycleCurrentIndex();
			FOREACH_CabinetLight( cl )
			{
				bool bOn = (iSec%NUM_CabinetLight) == cl;
				m_LightsState.m_bCabinetLights[cl] = bOn;
			}
		}
		break;
	case LIGHTSMODE_TEST_MANUAL_CYCLE:
		{
			FOREACH_CabinetLight( cl )
			{
				bool bOn = cl == m_clTestManualCycleCurrent;
				m_LightsState.m_bCabinetLights[cl] = bOn;
			}
		}
		break;
	default:
		ASSERT(0);
	}


	// If not joined, has enough credits, and not too late to join, then 
	// blink the menu buttons rapidly so they'll press Start
	{
		int iBeat = (int)(GAMESTATE->m_fLightSongBeat*4);
		bool bBlinkOn = (iBeat%2)==0;
		FOREACH_PlayerNumber( pn )
		{
			if( !GAMESTATE->m_bSideIsJoined[pn] && GAMESTATE->PlayersCanJoin() && GAMESTATE->EnoughCreditsToJoin() )
			{
				m_LightsState.m_bCabinetLights[LIGHT_BUTTONS_LEFT+pn] = bBlinkOn;
			}
		}
	}


	//
	// Update game controller lights
	//
	// FIXME:  Works only with Game==dance
	// FIXME:  lights pads for players who aren't playing
	switch( m_LightsMode )
	{
	case LIGHTSMODE_ATTRACT:
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
	case LIGHTSMODE_MENU:
	case LIGHTSMODE_GAMEPLAY:
		{
			if( m_LightsMode == LIGHTSMODE_GAMEPLAY  &&  g_bBlinkGameplayButtonLightsOnNote )
			{
				//
				// Blink on notes.
				//
				FOREACH_GameController( gc )
				{
					FOREACH_GameButton( gb )
					{
						m_LightsState.m_bGameButtonLights[gc][gb] = m_fSecsLeftInGameButtonBlink[gc][gb] > 0 ;
					}
				}
			}
			else
			{
				//
				// Blink on button pressess.
				//
				FOREACH_GameController( gc )
				{
					// don't blink unjoined sides
					if( !GAMESTATE->m_bSideIsJoined[gc] )
						continue;

					FOREACH_GameButton( gb )
					{
						bool bOn = INPUTMAPPER->IsBeingPressed( GameInput(gc,gb) );
						m_LightsState.m_bGameButtonLights[gc][gb] = bOn;
					}
				}
			}
		}
		break;
	case LIGHTSMODE_TEST_AUTO_CYCLE:
		{
			int index = GetTestAutoCycleCurrentIndex();

			vector<GameInput> vGI;
			GetUsedGameInputs( vGI );

			wrap( index, vGI.size() );

			ZERO( m_LightsState.m_bGameButtonLights );

			GameController gc = vGI[index].controller;
			GameButton gb = vGI[index].button;
			m_LightsState.m_bGameButtonLights[gc][gb] = true;
		}
		break;
	case LIGHTSMODE_TEST_MANUAL_CYCLE:
		{
			ZERO( m_LightsState.m_bGameButtonLights );

			vector<GameInput> vGI;
			GetUsedGameInputs( vGI );
			if( m_iControllerTestManualCycleCurrent != -1 )
			{
				GameController gc = vGI[m_iControllerTestManualCycleCurrent].controller;
				GameButton gb = vGI[m_iControllerTestManualCycleCurrent].button;
				m_LightsState.m_bGameButtonLights[gc][gb] = true;
			}
		}
		break;
	default:
		ASSERT(0);
	}

	// apply new light values we set above
	FOREACH( LightsDriver*, m_vpDrivers, iter )
		(*iter)->Set( &m_LightsState );
}

void LightsManager::BlinkCabinetLight( CabinetLight cl )
{
	m_fSecsLeftInCabinetLightBlink[cl] = g_fLightsFalloffSeconds;
}

void LightsManager::BlinkGameButton( GameInput gi )
{
	m_fSecsLeftInGameButtonBlink[gi.controller][gi.button] = g_fLightsFalloffSeconds;
}

void LightsManager::SetLightsMode( LightsMode lm )
{
	m_LightsMode = lm;
}

LightsMode LightsManager::GetLightsMode()
{
	return m_LightsMode;
}

void LightsManager::ChangeTestCabinetLight( int iDir )
{
	m_iControllerTestManualCycleCurrent = -1;

	m_clTestManualCycleCurrent = (CabinetLight)(m_clTestManualCycleCurrent+iDir);
	wrap( *ConvertValue<int>(&m_clTestManualCycleCurrent), NUM_CabinetLight );
}

void LightsManager::ChangeTestGameButtonLight( int iDir )
{
	m_clTestManualCycleCurrent = CabinetLight_Invalid;
	
	vector<GameInput> vGI;
	GetUsedGameInputs( vGI );

	m_iControllerTestManualCycleCurrent += iDir;
	wrap( m_iControllerTestManualCycleCurrent, vGI.size() );
}

CabinetLight LightsManager::GetFirstLitCabinetLight()
{
	FOREACH_CabinetLight( cl )
	{
		if( m_LightsState.m_bCabinetLights[cl] )
			return cl;
	}
	return CabinetLight_Invalid;
}

void LightsManager::GetFirstLitGameButtonLight( GameController &gcOut, GameButton &gbOut )
{
	FOREACH_GameController( gc )
	{
		FOREACH_GameButton( gb )
		{
			if( m_LightsState.m_bGameButtonLights[gc][gb] )
			{
				gcOut = gc;
				gbOut = gb;
				return;
			}
		}
	}
	gcOut = GameController_Invalid;
	gbOut = GameButton_Invalid;
}

bool LightsManager::IsEnabled() const
{
	return m_vpDrivers.size() > 1 || PREFSMAN->m_bDebugLights;
}


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
