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
#include "GameManager.h"
#include "CommonMetrics.h"
#include "Style.h"

const RString DEFAULT_LIGHTS_DRIVER = "SystemMessage,Export";
static Preference<RString> g_sLightsDriver( "LightsDriver", "" ); // "" == DEFAULT_LIGHTS_DRIVER
Preference<float>	g_fLightsFalloffSeconds( "LightsFalloffSeconds", 0.1f );
Preference<float>	g_fLightsAheadSeconds( "LightsAheadSeconds", 0.05f );
static Preference<bool>	g_bBlinkGameplayButtonLightsOnNote( "BlinkGameplayButtonLightsOnNote", false );

static ThemeMetric<RString> GAME_BUTTONS_TO_SHOW( "LightsManager", "GameButtonsToShow" );

static const char *CabinetLightNames[] = {
	"MarqueeUpLeft",
	"MarqueeUpRight",
	"MarqueeLrLeft",
	"MarqueeLrRight",
	"BassLeft",
	"BassRight",
};
XToString( CabinetLight );
StringToX( CabinetLight );

static const char *LightsModeNames[] = {
	"Attract",
	"Joining",
	"MenuStartOnly",
	"MenuStartAndDirections",
	"Demonstration",
	"Gameplay",
	"Stage",
	"Cleared",
	"TestAutoCycle",
	"TestManualCycle",
};
XToString( LightsMode );
LuaXType( LightsMode );

static void GetUsedGameInputs( vector<GameInput> &vGameInputsOut )
{
	vGameInputsOut.clear();

	vector<RString> asGameButtons;
	split( GAME_BUTTONS_TO_SHOW.GetValue(), ",", asGameButtons );
	FOREACH_ENUM( GameController,  gc )
	{
		for (RString const &button : asGameButtons)
		{
			GameButton gb = StringToGameButton( INPUTMAPPER->GetInputScheme(), button );
			if( gb != GameButton_Invalid )
			{
				GameInput gi = GameInput( gc, gb );
				vGameInputsOut.push_back( gi );
			}
		}
	}

	set<GameInput> vGIs;
	vector<const Style*> vStyles;
	GAMEMAN->GetStylesForGame( GAMESTATE->m_pCurGame, vStyles );
	auto const &value = CommonMetrics::STEPS_TYPES_TO_SHOW.GetValue();
	for (Style const *style : vStyles)
	{
		bool bFound = find( value.begin(), value.end(), style->m_StepsType ) != value.end();
		if( !bFound )
			continue;
		FOREACH_PlayerNumber( pn )
		{
			for( int iCol=0; iCol < style->m_iColsPerPlayer; ++iCol )
			{
				vector<GameInput> gi;
				style->StyleInputToGameInput( iCol, pn, gi );
				for(size_t i= 0; i < gi.size(); ++i)
				{
					if(gi[i].IsValid())
					{
						vGIs.insert(gi[i]);
					}
				}
			}
		}
	}

	for (GameInput const &input : vGIs)
		vGameInputsOut.push_back( input );
}

LightsManager*	LIGHTSMAN = nullptr;	// global and accessible from anywhere in our program

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

	SetLightsMode( LIGHTSMODE_ATTRACT );
}

LightsManager::~LightsManager()
{
	for (LightsDriver *iter : m_vpDrivers)
	{
		SAFE_DELETE( iter );
	}
	m_vpDrivers.clear();
}

// XXX: Allow themer to change these. (rewritten; who wrote original? -aj)
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
	// Update actor effect lights.
	FOREACH_CabinetLight( cl )
	{
		float fTime = fDeltaTime;
		float &fDuration = m_fSecsLeftInActorLightBlink[cl];
		if( fDuration > 0 )
		{
			// The light has power left.  Brighten it.
			float fSeconds = min( fDuration, fTime );
			fDuration -= fSeconds;
			fTime -= fSeconds;
			fapproach( m_fActorLights[cl], 1, fSeconds / g_fLightEffectRiseSeconds );
		}

		if( fTime > 0 )
		{
			// The light is out of power.  Dim it.
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
		FOREACH_ENUM( GameController,  gc )
			FOREACH_ENUM( GameButton,  gb )
				fapproach( m_fSecsLeftInGameButtonBlink[gc][gb], 0, fDeltaTime );
	}

	// Set new lights state cabinet lights
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
		DEFAULT_FAIL( m_LightsMode );

		case LIGHTSMODE_ATTRACT:
		{
			int iSec = (int)RageTimer::GetTimeSinceStartFast();
			int iTopIndex = iSec % 4;

			// Aldo: Disabled this line, apparently it was a forgotten initialization
			//CabinetLight cl = CabinetLight_Invalid;

			switch( iTopIndex )
			{
				DEFAULT_FAIL( iTopIndex );
				case 0:	m_LightsState.m_bCabinetLights[LIGHT_MARQUEE_UP_LEFT]  = true;	break;
				case 1:	m_LightsState.m_bCabinetLights[LIGHT_MARQUEE_LR_RIGHT] = true;	break;
				case 2:	m_LightsState.m_bCabinetLights[LIGHT_MARQUEE_UP_RIGHT] = true;	break;
				case 3:	m_LightsState.m_bCabinetLights[LIGHT_MARQUEE_LR_LEFT]  = true;	break;
			}

			if( iTopIndex == 0 )
			{
				m_LightsState.m_bCabinetLights[LIGHT_BASS_LEFT] = true;
				m_LightsState.m_bCabinetLights[LIGHT_BASS_RIGHT] = true;
			}

			break;
		}
		case LIGHTSMODE_MENU_START_ONLY:
		case LIGHTSMODE_MENU_START_AND_DIRECTIONS:
		case LIGHTSMODE_JOINING:
		{
			static int iLight;

			// if we've crossed a beat boundary, advance the light index
			{
				static float fLastBeat;
				float fLightSongBeat = GAMESTATE->m_Position.m_fLightSongBeat;

				if( fracf(fLightSongBeat) < fracf(fLastBeat) )
				{
					++iLight;
					wrap( iLight, 4 );
				}

				fLastBeat = fLightSongBeat;
			}

			CabinetLight cl = CabinetLight_Invalid;

			switch( iLight )
			{
				DEFAULT_FAIL( iLight );
				case 0:	cl = LIGHT_MARQUEE_UP_LEFT;	break;
				case 1:	cl = LIGHT_MARQUEE_LR_RIGHT;	break;
				case 2:	cl = LIGHT_MARQUEE_UP_RIGHT;	break;
				case 3:	cl = LIGHT_MARQUEE_LR_LEFT;	break;
			}

			m_LightsState.m_bCabinetLights[cl] = true;

			break;
		}

		case LIGHTSMODE_DEMONSTRATION:
		case LIGHTSMODE_GAMEPLAY:
		{
			FOREACH_CabinetLight( cl )
				m_LightsState.m_bCabinetLights[cl] = m_fSecsLeftInCabinetLightBlink[cl] > 0;

			break;
		}

		case LIGHTSMODE_STAGE:
		case LIGHTSMODE_ALL_CLEARED:
		{
			FOREACH_CabinetLight( cl )
				m_LightsState.m_bCabinetLights[cl] = true;

			break;
		}

		case LIGHTSMODE_TEST_AUTO_CYCLE:
		{
			int iSec = GetTestAutoCycleCurrentIndex();

			CabinetLight cl = CabinetLight(iSec % NUM_CabinetLight);
			m_LightsState.m_bCabinetLights[cl] = true;

			break;
		}

		case LIGHTSMODE_TEST_MANUAL_CYCLE:
		{
			CabinetLight cl = m_clTestManualCycleCurrent;
			m_LightsState.m_bCabinetLights[cl] = true;

			break;
		}
	}


	// Update game controller lights
	switch( m_LightsMode )
	{
		DEFAULT_FAIL( m_LightsMode );

		case LIGHTSMODE_ALL_CLEARED:
		case LIGHTSMODE_STAGE:
		case LIGHTSMODE_JOINING:
		{
			FOREACH_ENUM( GameController, gc )
			{
				if( GAMESTATE->m_bSideIsJoined[gc] )
				{
					FOREACH_ENUM( GameButton, gb )
						m_LightsState.m_bGameButtonLights[gc][gb] = true;
				}
			}

			break;
		}

		case LIGHTSMODE_MENU_START_ONLY:
		case LIGHTSMODE_MENU_START_AND_DIRECTIONS:
		{
			float fLightSongBeat = GAMESTATE->m_Position.m_fLightSongBeat;

			/* Blink menu lights on the first half of the beat */
			if( fracf(fLightSongBeat) <= 0.5f )
			{
				FOREACH_PlayerNumber( pn )
				{
					if( !GAMESTATE->m_bSideIsJoined[pn] )
						continue;

					m_LightsState.m_bGameButtonLights[pn][GAME_BUTTON_START] = true;

					if( m_LightsMode == LIGHTSMODE_MENU_START_AND_DIRECTIONS )
					{
						m_LightsState.m_bGameButtonLights[pn][GAME_BUTTON_MENULEFT] = true;
						m_LightsState.m_bGameButtonLights[pn][GAME_BUTTON_MENURIGHT] = true;
						m_LightsState.m_bGameButtonLights[pn][GAME_BUTTON_MENUUP] = true;
						m_LightsState.m_bGameButtonLights[pn][GAME_BUTTON_MENUDOWN] = true;
					}
					else
					{
						//flash select during evaluation screen to indicate
						//that the button can be used for screenshots etc.
						m_LightsState.m_bGameButtonLights[pn][GAME_BUTTON_SELECT] = true;
					}
				}
			}

			// fall through to blink on button presses
		}

		case LIGHTSMODE_DEMONSTRATION:
		case LIGHTSMODE_GAMEPLAY:
		{
			bool bGameplay = (m_LightsMode == LIGHTSMODE_DEMONSTRATION) || (m_LightsMode == LIGHTSMODE_GAMEPLAY);

			// Blink on notes during gameplay.
			if( bGameplay && g_bBlinkGameplayButtonLightsOnNote )
			{
				FOREACH_ENUM( GameController,  gc )
				{
					FOREACH_ENUM( GameButton,  gb )
					{
						m_LightsState.m_bGameButtonLights[gc][gb] = m_fSecsLeftInGameButtonBlink[gc][gb] > 0 ;
					}
				}
				break;
			}

			// fall through to blink on button presses
		}

		case LIGHTSMODE_ATTRACT:
		{
			// Blink on button presses.
			FOREACH_ENUM( GameController,  gc )
			{
				FOREACH_GameButton_Custom( gb )
				{
					bool bOn = INPUTMAPPER->IsBeingPressed( GameInput(gc,gb) );
					m_LightsState.m_bGameButtonLights[gc][gb] = bOn;
				}
			}

			break;
		}

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

			break;
		}

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

			break;
		}
	}

	// If not joined, has enough credits, and not too late to join, then
	// blink the menu buttons rapidly so they'll press Start
	{
		int iBeat = (int)(GAMESTATE->m_Position.m_fLightSongBeat*4);
		bool bBlinkOn = (iBeat%2)==0;
		FOREACH_PlayerNumber( pn )
		{
			if( !GAMESTATE->m_bSideIsJoined[pn] && GAMESTATE->PlayersCanJoin() && GAMESTATE->EnoughCreditsToJoin() )
				m_LightsState.m_bGameButtonLights[pn][GAME_BUTTON_START] = bBlinkOn;
		}
	}

	// apply new light values we set above
	for (LightsDriver *iter : m_vpDrivers)
		iter->Set( &m_LightsState );
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
	m_fTestAutoCycleCurrentIndex = 0;
	m_clTestManualCycleCurrent = CabinetLight_Invalid;
	m_iControllerTestManualCycleCurrent = -1;
}

LightsMode LightsManager::GetLightsMode()
{
	return m_LightsMode;
}

void LightsManager::ChangeTestCabinetLight( int iDir )
{
	m_iControllerTestManualCycleCurrent = -1;

	enum_add( m_clTestManualCycleCurrent, iDir );
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

GameInput LightsManager::GetFirstLitGameButtonLight()
{
	FOREACH_ENUM( GameController, gc )
	{
		FOREACH_ENUM( GameButton, gb )
		{
			if( m_LightsState.m_bGameButtonLights[gc][gb] )
				return GameInput( gc, gb );
		}
	}
	return GameInput();
}

bool LightsManager::IsEnabled() const
{
	return m_vpDrivers.size() >= 1 || PREFSMAN->m_bDebugLights;
}

void LightsManager::TurnOffAllLights()
{
	for(LightsDriver *iter : m_vpDrivers)
		iter->Reset();
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
