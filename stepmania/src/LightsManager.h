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
	LIGHT_MARUQEE_MENU_LEFT,
	LIGHT_MARUQEE_MENU_RIGHT,
	LIGHT_MARUQEE_BASS_LEFT,
	LIGHT_MARUQEE_BASS_RIGHT,
	NUM_LIGHTS
};

enum LightMode
{
	LIGHTMODE_ATTRACT,
	LIGHTMODE_JOINING,
	LIGHTMODE_MENU,
	LIGHTMODE_DEMONSTRATION,
	LIGHTMODE_GAMEPLAY,
	LIGHTMODE_ALL_ON,
	LIGHTMODE_ALL_OFF,
};

class LightsDriver;

class LightsManager
{
public:
	LightsManager(CString sDriver);
	~LightsManager();
	
	void Update( float fDeltaTime );

	void SetLightMode( LightMode lm );

private:
	void SetLight( Light light, bool bOn );

	LightsDriver* m_pDriver;
	LightMode m_LightMode;
};


extern LightsManager*	LIGHTSMAN;	// global and accessable from anywhere in our program

#endif
