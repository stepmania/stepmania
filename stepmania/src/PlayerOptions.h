#pragma once
/*
-----------------------------------------------------------------------------
 Class: PlayerOptions

 Desc: Per-player options that are not saved between sessions.

 Copyright (c) 2001-2002 by the person(s) listed below.  All rights reserved.
	Chris Danford
-----------------------------------------------------------------------------
*/

const int NUM_EFFECT_TYPES = 9;

struct PlayerOptions
{
	float m_fArrowScrollSpeed;
	bool m_bBoost;
	enum EffectType	{
		EFFECT_WAVE,
		EFFECT_DRUNK,
		EFFECT_DIZZY,
		EFFECT_SPACE,
		EFFECT_MINI,
		EFFECT_FLIP,
		EFFECT_TORNADO,
		NUM_EFFECT_TYPES
	};
	bool m_bEffects[NUM_EFFECT_TYPES];
	void NextEffect();
	enum AppearanceType	{ APPEARANCE_VISIBLE=0, APPEARANCE_HIDDEN, APPEARANCE_SUDDEN, APPEARANCE_STEALTH, APPEARANCE_BLINK, NUM_APPEARANCE_TYPES };
	AppearanceType m_AppearanceType;
	enum TurnType { TURN_NONE=0, TURN_MIRROR, TURN_LEFT, TURN_RIGHT, TURN_SHUFFLE, TURN_SUPER_SHUFFLE, NUM_TURN_TYPES };
	TurnType m_TurnType;
	bool m_bLittle;
	bool m_bReverseScroll;
	enum ColorType { COLOR_VIVID=0, COLOR_NOTE, COLOR_FLAT, COLOR_PLAIN, NUM_COLOR_TYPES };
	ColorType m_ColorType;
	bool m_bHoldNotes;
	bool m_bDark;

	PlayerOptions() { Init(); };
	void Init();
	CString GetString();
	void FromString( CString sOptions );
};
