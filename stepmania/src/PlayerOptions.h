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

/* nonstandard extension used : nameless struct/union
 * It is, in fact, nonstandard.  G++ 3.x can handle it. 2.95.x can not. XXX */
#if defined(_MSC_VER)
#pragma warning (push)
#pragma warning (disable : 4201)
#endif

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
		ACCEL_LAND,
		ACCEL_WAVE,
		ACCEL_EXPAND,
		ACCEL_BOOMERANG,
		NUM_ACCELS
	};
	enum Effect	{
		EFFECT_DRUNK,
		EFFECT_DIZZY,
		EFFECT_SPACE,
		EFFECT_MINI,
		EFFECT_FLIP,
		EFFECT_TORNADO,
		NUM_EFFECTS
	};
	enum Appearance {
		APPEARANCE_HIDDEN,
		APPEARANCE_SUDDEN,
		APPEARANCE_STEALTH,
		APPEARANCE_BLINK,
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
		NUM_TRANSFORMS
	};

	
	float		m_fScrollSpeed;
	float		m_fAccels[NUM_ACCELS];
	float		m_fEffects[NUM_EFFECTS];
	float		m_fAppearances[NUM_APPEARANCES];
	float		m_fReverseScroll;
	float		m_fDark;
	Turn		m_Turn;
	Transform	m_Transform;
	bool		m_bHoldNotes;


	void NextAccel();
	void NextEffect();
	void NextAppearance();
	void NextTurn();
	void NextTransform();

	Accel GetFirstAccel();
	Effect GetFirstEffect();
	Appearance GetFirstAppearance();

	void SetOneAccel( Accel a );
	void SetOneEffect( Effect e );
	void SetOneAppearance( Appearance a );
};

#endif
