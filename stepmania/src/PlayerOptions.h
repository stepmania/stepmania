#pragma once
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
	float m_fArrowScrollSpeed;
	enum EffectType	{ EFFECT_NONE=0, EFFECT_BOOST, EFFECT_WAVE, EFFECT_DRUNK, EFFECT_DIZZY, EFFECT_SPACE, EFFECT_MINI };
	EffectType m_EffectType;
	enum AppearanceType	{ APPEARANCE_VISIBLE=0, APPEARANCE_HIDDEN, APPEARANCE_SUDDEN, APPEARANCE_STEALTH, APPEARANCE_BLINK };
	AppearanceType m_AppearanceType;
	enum TurnType { TURN_NONE=0, TURN_MIRROR, TURN_LEFT, TURN_RIGHT, TURN_SHUFFLE };
	TurnType m_TurnType;
	bool m_bLittle;
	bool m_bReverseScroll;
	enum ColorType { COLOR_ARCADE=0, COLOR_NOTE, COLOR_FLAT, COLOR_PLAIN };
	ColorType m_ColorType;
	bool m_bHoldNotes;
	bool m_bDark;

	PlayerOptions() 
	{
		m_fArrowScrollSpeed = 1.0f;
		m_EffectType = EFFECT_NONE;
		m_AppearanceType = APPEARANCE_VISIBLE;
		m_TurnType = TURN_NONE;
		m_bLittle = false;
		m_bReverseScroll = false;
		m_ColorType = COLOR_ARCADE;
		m_bHoldNotes = true;
		m_bDark = false;
	};
	CString GetString();
	void FromString( CString sOptions );
};
