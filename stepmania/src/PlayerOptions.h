#ifndef PLAYEROPTIONS_H
#define PLAYEROPTIONS_H
/*
-----------------------------------------------------------------------------
 Class: PlayerOptions

 Desc: Per-player options that are not saved between sessions.

 Copyright (c) 2001-2002 by the person(s) listed below.  All rights reserved.
	Chris Danford
-----------------------------------------------------------------------------
*/

struct PlayerOptions
{
	float m_fScrollSpeed;
	enum AccelType {
		ACCEL_OFF,
		ACCEL_BOOST,
		ACCEL_LAND,
		ACCEL_WAVE,
		ACCEL_EXPAND,
		ACCEL_BOOMERANG,
		NUM_ACCEL_TYPES
	} m_AccelType;
	void NextAccel();
	enum EffectType	{
		EFFECT_DRUNK,
		EFFECT_DIZZY,
		EFFECT_SPACE,
		EFFECT_MINI,
		EFFECT_FLIP,
		EFFECT_TORNADO,
		NUM_EFFECT_TYPES
	};
	bool m_bEffects[NUM_EFFECT_TYPES];	// no effects are mutually exclusive
	void NextEffect();
	enum AppearanceType	{
		APPEARANCE_VISIBLE=0,
		APPEARANCE_HIDDEN,
		APPEARANCE_SUDDEN,
		APPEARANCE_STEALTH,
		APPEARANCE_BLINK,
		NUM_APPEARANCE_TYPES
	} m_AppearanceType;
	void NextAppearance();
	enum TurnType {
		TURN_NONE=0,
		TURN_MIRROR,
		TURN_LEFT,
		TURN_RIGHT,
		TURN_SHUFFLE,
		TURN_SUPER_SHUFFLE,
		NUM_TURN_TYPES 
	} m_TurnType;
	void NextTurn();
	bool m_bLittle;
	bool m_bReverseScroll;
	enum ColorType {
		COLOR_VIVID=0,
		COLOR_NOTE,
		COLOR_FLAT,
		NUM_COLOR_TYPES
	} m_ColorType;
	void NextColor();
	bool m_bHoldNotes;
	bool m_bDark;

	PlayerOptions() { Init(); };
	void Init();
	CString GetString();
	void FromString( CString sOptions );
};

#endif
