/* LightsManager - Control lights. */

#ifndef LightsManager_H
#define LightsManager_H

#include "PlayerNumber.h"
#include "GameInput.h"
#include "EnumHelper.h"
#include "Preference.h"

extern Preference<float>	g_fLightsFalloffSeconds;
extern Preference<float>	g_fLightsAheadSeconds;

enum CabinetLight
{
	LIGHT_MARQUEE_UP_LEFT,
	LIGHT_MARQUEE_UP_RIGHT,
	LIGHT_MARQUEE_LR_LEFT,
	LIGHT_MARQUEE_LR_RIGHT,
	LIGHT_BUTTONS_LEFT,
	LIGHT_BUTTONS_RIGHT,
	LIGHT_BASS_LEFT,
	LIGHT_BASS_RIGHT,
	NUM_CABINET_LIGHTS,
	LIGHT_INVALID
};
#define FOREACH_CabinetLight( i ) FOREACH_ENUM( CabinetLight, NUM_CABINET_LIGHTS, i )
const RString& CabinetLightToString( CabinetLight cl );
CabinetLight StringToCabinetLight( const RString& s);

enum LightsMode
{
	LIGHTSMODE_ATTRACT,
	LIGHTSMODE_JOINING,
	LIGHTSMODE_MENU,
	LIGHTSMODE_DEMONSTRATION,
	LIGHTSMODE_GAMEPLAY,
	LIGHTSMODE_STAGE,
	LIGHTSMODE_ALL_CLEARED,
	LIGHTSMODE_TEST_AUTO_CYCLE,
	LIGHTSMODE_TEST_MANUAL_CYCLE,
	NUM_LIGHTS_MODES
};
const RString& LightsModeToString( LightsMode lm );

struct LightsState
{
	bool m_bCabinetLights[NUM_CABINET_LIGHTS];
	bool m_bGameButtonLights[MAX_GAME_CONTROLLERS][MAX_GAME_BUTTONS];
};

class LightsDriver;

class LightsManager
{
public:
	LightsManager();
	~LightsManager();
	
	void Update( float fDeltaTime );
	bool IsEnabled() const;

	void BlinkCabinetLight( CabinetLight cl );
	void BlinkGameButton( GameInput gi );
	void BlinkActorLight( CabinetLight cl );
	float GetActorLightLatencySeconds() const;

	void SetLightsMode( LightsMode lm );
	LightsMode GetLightsMode();

	void PrevTestCabinetLight()		{ ChangeTestCabinetLight(-1); }
	void NextTestCabinetLight()		{ ChangeTestCabinetLight(+1); }
	void PrevTestGameButtonLight()	{ ChangeTestGameButtonLight(-1); }
	void NextTestGameButtonLight()	{ ChangeTestGameButtonLight(+1); }

	CabinetLight	GetFirstLitCabinetLight();
	void			GetFirstLitGameButtonLight( GameController &gcOut, GameButton &gbOut );

private:
	void ChangeTestCabinetLight( int iDir );
	void ChangeTestGameButtonLight( int iDir );

	float m_fSecsLeftInCabinetLightBlink[NUM_CABINET_LIGHTS];
	float m_fSecsLeftInGameButtonBlink[MAX_GAME_CONTROLLERS][MAX_GAME_BUTTONS];
	float m_fActorLights[NUM_CABINET_LIGHTS];	// current "power" of each actor light
	float m_fSecsLeftInActorLightBlink[NUM_CABINET_LIGHTS];	// duration to "power" an actor light

	vector<LightsDriver*> m_vpDrivers;
	LightsMode m_LightsMode;
	LightsState m_LightsState;

	int GetTestAutoCycleCurrentIndex() { return (int)m_fTestAutoCycleCurrentIndex; }

	float			m_fTestAutoCycleCurrentIndex;
	CabinetLight	m_clTestManualCycleCurrent;
	int				m_iControllerTestManualCycleCurrent;
};


extern LightsManager*	LIGHTSMAN;	// global and accessable from anywhere in our program

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
