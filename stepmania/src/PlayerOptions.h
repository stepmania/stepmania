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
	PlayerOptions() { Init(); };
	void Init();
	void Approach( const PlayerOptions& other, float fDeltaSeconds );
	CString GetString();
	void FromString( CString sOptions );
	void ChooseRandomMofifiers();


	enum Accel {
		ACCEL_BOOST,
		ACCEL_BRAKE,
		ACCEL_WAVE,
		ACCEL_EXPAND,
		ACCEL_BOOMERANG,
		NUM_ACCELS
	};
	enum Effect	{
		EFFECT_DRUNK,
		EFFECT_DIZZY,
		EFFECT_MINI,
		EFFECT_FLIP,
		EFFECT_TORNADO,
		EFFECT_TIPSY,
		EFFECT_BUMPY,
		EFFECT_BEAT,
		NUM_EFFECTS
	};
	enum Appearance {
		APPEARANCE_HIDDEN,
		APPEARANCE_SUDDEN,
		APPEARANCE_STEALTH,
		APPEARANCE_BLINK,
		APPEARANCE_RANDOMVANISH,
		NUM_APPEARANCES
	};
	enum Turn {
		TURN_NONE=0,
		TURN_MIRROR,
		TURN_LEFT,
		TURN_RIGHT,
		TURN_SHUFFLE,
		TURN_SUPER_SHUFFLE,
		NUM_TURNS 
	};
	enum Transform {
		TRANSFORM_NONE=0,
		TRANSFORM_LITTLE,
		TRANSFORM_WIDE,
		TRANSFORM_BIG,
		TRANSFORM_QUICK,
		TRANSFORM_SKIPPY,
		TRANSFORM_MINES,
		NUM_TRANSFORMS
	};
	enum Scroll {
		SCROLL_REVERSE=0,
		SCROLL_SPLIT,
		SCROLL_ALTERNATE,
		NUM_SCROLLS
	};
	float GetReversePercentForColumn( int iCol ); // accounts for all Directions

	
	bool		m_bTimeSpacing;	// instead of Beat spacing
	float		m_fScrollSpeed;		// used if !m_bTimeSpacing
	float		m_fScrollBPM;		// used if m_bTimeSpacing
	float		m_fAccels[NUM_ACCELS];
	float		m_fEffects[NUM_EFFECTS];
	float		m_fAppearances[NUM_APPEARANCES];
	float		m_fScrolls[NUM_SCROLLS];
	float		m_fDark;
	float		m_fBlind;
	Turn		m_Turn;
	Transform	m_Transform;
	bool		m_bHoldNotes;
	bool		m_bTimingAssist;
	bool		m_bProTiming;
	float		m_fPerspectiveTilt;		// -1 = near, 0 = overhead, +1 = space
	CString		m_sPositioning;	/* The current positioning mode, or empty to use the normal positions. */
	CString		m_sNoteSkin;

	void NextAccel();
	void NextEffect();
	void NextAppearance();
	void NextTurn();
	void NextTransform();
	void NextPerspective();
	void NextScroll();

	Accel GetFirstAccel();
	Effect GetFirstEffect();
	Appearance GetFirstAppearance();
	Scroll GetFirstScroll();

	void SetOneAccel( Accel a );
	void SetOneEffect( Effect e );
	void SetOneAppearance( Appearance a );
	void SetOneScroll( Scroll s );
};

#endif
