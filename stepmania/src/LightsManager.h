#ifndef LightsManager_H
#define LightsManager_H
/*
-----------------------------------------------------------------------------
 Class: LightsManager

 Desc: Control lights.

 Copyright (c) 2001-2002 by the person(s) listed below.  All rights reserved.
	Chris Danford
-----------------------------------------------------------------------------
*/

#include "PlayerNumber.h"
#include "GameInput.h"
#include "EnumHelper.h"

const float LIGHTS_FALLOFF_SECONDS = 0.1f;

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
	NUM_CABINET_LIGHTS
};
#define FOREACH_CabinetLight( i ) FOREACH_ENUM( CabinetLight, NUM_CABINET_LIGHTS, i )
const CString& CabinetLightToString( CabinetLight cl );

enum LightsMode
{
	LIGHTSMODE_ATTRACT,
	LIGHTSMODE_JOINING,
	LIGHTSMODE_MENU,
	LIGHTSMODE_DEMONSTRATION,
	LIGHTSMODE_GAMEPLAY,
	LIGHTSMODE_STAGE,
	LIGHTSMODE_ALL_CLEARED,
	LIGHTSMODE_TEST,
	NUM_LIGHTS_MODES
};
const CString& LightsModeToString( LightsMode lm );

struct LightsState
{
	bool m_bCabinetLights[NUM_CABINET_LIGHTS];
	bool m_bGameButtonLights[MAX_GAME_CONTROLLERS][MAX_GAME_BUTTONS];
};

class LightsDriver;

class LightsManager
{
public:
	LightsManager(CString sDriver);
	~LightsManager();
	
	void Update( float fDeltaTime );

	void GameplayBlinkLight( CabinetLight cl );

	void SetLightsMode( LightsMode lm );
	LightsMode GetLightsMode();

private:
	float m_fSecsLeftInBlink[NUM_CABINET_LIGHTS];

	LightsDriver* m_pDriver;
	LightsMode m_LightsMode;
	LightsState m_LightsState;
};


extern LightsManager*	LIGHTSMAN;	// global and accessable from anywhere in our program

#endif
