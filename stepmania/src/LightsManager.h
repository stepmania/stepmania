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


enum Light
{
	LIGHT_MARUQEE_UP_LEFT,
	LIGHT_MARUQEE_UP_RIGHT,
	LIGHT_MARUQEE_LR_LEFT,
	LIGHT_MARUQEE_LR_RIGHT,
	LIGHT_MENU_LEFT,
	LIGHT_MENU_RIGHT,
	LIGHT_BASS_LEFT,
	LIGHT_BASS_RIGHT,
	LIGHT_GAME_BUTTON1,	// 
	LIGHT_GAME_BUTTON2,
	LIGHT_GAME_BUTTON3,
	LIGHT_GAME_BUTTON4,
	LIGHT_GAME_BUTTON5,
	LIGHT_GAME_BUTTON6,
	LIGHT_GAME_BUTTON7,
	LIGHT_GAME_BUTTON8,
	NUM_LIGHTS
};

const Light LIGHT_LAST_GAME_BUTTON = LIGHT_GAME_BUTTON8;

enum LightMode
{
	LIGHTMODE_ATTRACT,
	LIGHTMODE_JOINING,
	LIGHTMODE_MENU,
	LIGHTMODE_DEMONSTRATION,
	LIGHTMODE_GAMEPLAY,
	LIGHTMODE_STAGE,
	LIGHTMODE_ALL_CLEARED,
};

class LightsDriver;

class LightsManager
{
public:
	LightsManager(CString sDriver);
	~LightsManager();
	
	void Update( float fDeltaTime );

	void SetLightMode( LightMode lm );
	void SetAllUpperLights( bool bOn );

private:
	void SetLight( Light light, bool bOn );

	LightsDriver* m_pDriver;
	LightMode m_LightMode;
};


extern LightsManager*	LIGHTSMAN;	// global and accessable from anywhere in our program

#endif
