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
	enum EffectType	{
		EFFECT_NONE		=0, 
		EFFECT_BOOST	=1<<0,
		EFFECT_WAVE		=1<<1,
		EFFECT_DRUNK	=1<<2,
		EFFECT_DIZZY	=1<<3,
		EFFECT_SPACE	=1<<4,
		EFFECT_MINI		=1<<5,
		NUM_EFFECT_TYPES };
	int m_EffectType;
	enum AppearanceType	{ APPEARANCE_VISIBLE=0, APPEARANCE_HIDDEN, APPEARANCE_SUDDEN, APPEARANCE_STEALTH, APPEARANCE_BLINK, NUM_APPEARANCE_TYPES };
	AppearanceType m_AppearanceType;
	enum TurnType { TURN_NONE=0, TURN_MIRROR, TURN_LEFT, TURN_RIGHT, TURN_SHUFFLE };
	TurnType m_TurnType;
	bool m_bLittle;
	bool m_bReverseScroll;
	enum ColorType { COLOR_VIVID=0, COLOR_NOTE, COLOR_FLAT, COLOR_PLAIN, NUM_COLOR_TYPES };
	ColorType m_ColorType;
	bool m_bHoldNotes;
	bool m_bDark;

	PlayerOptions() { Init(); };
	void Init()
	{
		m_fArrowScrollSpeed = 1.0f;
		m_EffectType = EFFECT_NONE;
		m_AppearanceType = APPEARANCE_VISIBLE;
		m_TurnType = TURN_NONE;
		m_bLittle = false;
		m_bReverseScroll = false;
		m_ColorType = COLOR_VIVID;
		m_bHoldNotes = true;
		m_bDark = false;
	};
	CString GetString();
	void FromString( CString sOptions );
};
