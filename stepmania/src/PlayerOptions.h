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
	CString GetString() const;
	void FromString( CString sOptions );
	void ChooseRandomMofifiers();

	bool operator==( const PlayerOptions &other ) const;
	bool operator!=( const PlayerOptions &other ) const { return !operator==(other); }


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
		TRANSFORM_NOHOLDS,
		TRANSFORM_NOMINES,
		TRANSFORM_LITTLE,
		TRANSFORM_WIDE,
		TRANSFORM_BIG,
		TRANSFORM_QUICK,
		TRANSFORM_BMRIZE,
		TRANSFORM_SKIPPY,
		TRANSFORM_MINES,
		TRANSFORM_ECHO,
		TRANSFORM_PLANTED,
		TRANSFORM_STOMP,
		TRANSFORM_TWISTER,
		TRANSFORM_NOJUMPS,
		TRANSFORM_NOHANDS,
		TRANSFORM_NOQUADS,
		NUM_TRANSFORMS
	};
	enum Scroll {
		SCROLL_REVERSE=0,
		SCROLL_SPLIT,
		SCROLL_ALTERNATE,
		SCROLL_CROSS,
		NUM_SCROLLS
	};
	float GetReversePercentForColumn( int iCol ); // accounts for all Directions

	
	bool		m_bTimeSpacing;	// instead of Beat spacing

	/* All floats have a corresponding speed setting, which determines how fast
	 * PlayerOptions::Approach approaches. */
	float		m_fScrollSpeed,			m_SpeedfScrollSpeed;		// used if !m_bTimeSpacing
	float		m_fScrollBPM,			m_SpeedfScrollBPM;		// used if m_bTimeSpacing
	float		m_fAccels[NUM_ACCELS],	m_SpeedfAccels[NUM_ACCELS];
	float		m_fEffects[NUM_EFFECTS],m_SpeedfEffects[NUM_EFFECTS];
	float		m_fAppearances[NUM_APPEARANCES],m_SpeedfAppearances[NUM_APPEARANCES];
	float		m_fScrolls[NUM_SCROLLS],m_SpeedfScrolls[NUM_SCROLLS];
	float		m_fDark,				m_SpeedfDark;
	float		m_fBlind,				m_SpeedfBlind;
	float		m_fPerspectiveTilt,		m_SpeedfPerspectiveTilt;		// -1 = near, 0 = overhead, +1 = space
	float		m_fSkew,				m_SpeedfSkew;		// 0 = vanish point is in center of player, 1 = vanish point is in center of screen

	/* If this is > 0, then the player must have life above this value at the end of
	 * the song to pass.  This is independent of SongOptions::m_FailType. */
	float		m_fPassmark,			m_SpeedfPassmark;

	Turn		m_Turn;
	bool		m_bTransforms[NUM_TRANSFORMS];
	bool		m_bTimingAssist;
	bool		m_bProTiming;
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
