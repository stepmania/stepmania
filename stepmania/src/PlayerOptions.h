/* PlayerOptions - Per-player options that are not saved between sessions. */

#ifndef PLAYER_OPTIONS_H
#define PLAYER_OPTIONS_H

class Song;
class Steps;
class Course;
class Trail;

#include "GameConstantsAndTypes.h"

struct PlayerOptions
{
	PlayerOptions() { Init(); };
	void Init();
	void Approach( const PlayerOptions& other, float fDeltaSeconds );
	CString GetString() const;
	CString GetSavedPrefsString() const;	// only the basic options that players would want for every song
	void ResetSavedPrefs();
	void GetMods( vector<CString> &AddTo ) const;
	void GetThemedMods( vector<CString> &AddTo ) const;
	void FromString( CString sOptions, bool bWarnOnInvalid = false );
	void ChooseRandomMofifiers();
	bool ContainsTransformOrTurn() const;

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
		APPEARANCE_HIDDEN_OFFSET,
		APPEARANCE_SUDDEN,
		APPEARANCE_SUDDEN_OFFSET,
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
		TRANSFORM_NOROLLS,
		TRANSFORM_NOMINES,
		TRANSFORM_LITTLE,
		TRANSFORM_WIDE,
		TRANSFORM_BIG,
		TRANSFORM_QUICK,
		TRANSFORM_BMRIZE,
		TRANSFORM_SKIPPY,
		TRANSFORM_MINES,
		TRANSFORM_ECHO,
		TRANSFORM_STOMP,
		TRANSFORM_PLANTED,
		TRANSFORM_FLOORED,
		TRANSFORM_TWISTER,
		TRANSFORM_NOJUMPS,
		TRANSFORM_NOHANDS,
		TRANSFORM_NOQUADS,
		TRANSFORM_NOSTRETCH,
		NUM_TRANSFORMS
	};
	enum Scroll {
		SCROLL_REVERSE=0,
		SCROLL_SPLIT,
		SCROLL_ALTERNATE,
		SCROLL_CROSS,
		SCROLL_CENTERED,
		NUM_SCROLLS
	};
	enum ScoreDisplay {
		SCORING_ADD=0,
		SCORING_SUBTRACT,
		SCORING_AVERAGE,
		NUM_SCOREDISPLAYS
	};

	float GetReversePercentForColumn( int iCol ) const; // accounts for all Directions

	/* All floats have a corresponding speed setting, which determines how fast
	 * PlayerOptions::Approach approaches. */
	bool	m_bSetScrollSpeed;				// true if the scroll speed was set by FromString
	float	m_fTimeSpacing,			m_SpeedfTimeSpacing;	// instead of Beat spacing
	float	m_fScrollSpeed,			m_SpeedfScrollSpeed;	// used if !m_bTimeSpacing
	float	m_fScrollBPM,			m_SpeedfScrollBPM;		// used if m_bTimeSpacing
	float	m_fAccels[NUM_ACCELS],	m_SpeedfAccels[NUM_ACCELS];
	float	m_fEffects[NUM_EFFECTS],m_SpeedfEffects[NUM_EFFECTS];
	float	m_fAppearances[NUM_APPEARANCES],m_SpeedfAppearances[NUM_APPEARANCES];
	float	m_fScrolls[NUM_SCROLLS],m_SpeedfScrolls[NUM_SCROLLS];
	float	m_fDark,				m_SpeedfDark;
	float	m_fBlind,				m_SpeedfBlind;
	float	m_fCover,				m_SpeedfCover;	// hide the background per-player--can't think of a good name
	bool	m_bSetTiltOrSkew;				// true if the tilt or skew was set by FromString
	float	m_fPerspectiveTilt,		m_SpeedfPerspectiveTilt;		// -1 = near, 0 = overhead, +1 = space
	float	m_fSkew,				m_SpeedfSkew;		// 0 = vanish point is in center of player, 1 = vanish point is in center of screen

	float	m_fRandomSpeed,			m_SpeedfRandomSpeed;

	/* If this is > 0, then the player must have life above this value at the end of
	 * the song to pass.  This is independent of SongOptions::m_FailType. */
	float		m_fPassmark,			m_SpeedfPassmark;
	
	bool		m_bTurns[NUM_TURNS];
	bool		m_bTransforms[NUM_TRANSFORMS];
	ScoreDisplay m_ScoreDisplay;

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
	void ToggleOneTurn( Turn t );


	// return true if any mods being used will make the song(s) easier
	bool IsEasierForSongAndSteps( Song* pSong, Steps* pSteps );
	bool IsEasierForCourseAndTrail( Course* pCourse, Trail* pTrail );
};

#endif

/*
 * (c) 2001-2004 Chris Danford, Glenn Maynard
 * All rights reserved.
 * 
 * Permission is hereby granted, free of charge, to any person obtaining a
 * copy of this software and associated documentation files (the
 * "Software"), to deal in the Software without restriction, including
 * without limitation the rights to use, copy, modify, merge, publish,
 * distribute, and/or sell copies of the Software, and to permit persons to
 * whom the Software is furnished to do so, provided that the above
 * copyright notice(s) and this permission notice appear in all copies of
 * the Software and that both the above copyright notice(s) and this
 * permission notice appear in supporting documentation.
 * 
 * THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS
 * OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF
 * MERCHANTABILITY, FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT OF
 * THIRD PARTY RIGHTS. IN NO EVENT SHALL THE COPYRIGHT HOLDER OR HOLDERS
 * INCLUDED IN THIS NOTICE BE LIABLE FOR ANY CLAIM, OR ANY SPECIAL INDIRECT
 * OR CONSEQUENTIAL DAMAGES, OR ANY DAMAGES WHATSOEVER RESULTING FROM LOSS
 * OF USE, DATA OR PROFITS, WHETHER IN AN ACTION OF CONTRACT, NEGLIGENCE OR
 * OTHER TORTIOUS ACTION, ARISING OUT OF OR IN CONNECTION WITH THE USE OR
 * PERFORMANCE OF THIS SOFTWARE.
 */
