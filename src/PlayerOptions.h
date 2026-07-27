#ifndef PLAYER_OPTIONS_H
#define PLAYER_OPTIONS_H

class Course;
class Song;
class Steps;
class Trail;
struct lua_State;

#define ONE( arr ) { for( unsigned Z = 0; Z < ARRAYLEN(arr); ++Z ) arr[Z]=1.0f; }

#include "GameConstantsAndTypes.h"
#include "PlayerNumber.h"
#include "PrefsManager.h"

#include <bitset>
#include <vector>


enum LifeType
{
	LifeType_Bar,
	LifeType_Battery,
	LifeType_Time,
	NUM_LifeType,
	LifeType_Invalid
};
const RString& LifeTypeToString( LifeType cat );
const RString& LifeTypeToLocalizedString( LifeType cat );
LuaDeclareType( LifeType );

enum DrainType
{
	DrainType_Normal,
	DrainType_NoRecover,
	DrainType_SuddenDeath,
	NUM_DrainType,
	DrainType_Invalid
};
const RString& DrainTypeToString( DrainType cat );
const RString& DrainTypeToLocalizedString( DrainType cat );
LuaDeclareType( DrainType );

enum ModTimerType
{
	ModTimerType_Game,
	ModTimerType_Beat,
	ModTimerType_Song,
	ModTimerType_Default,
	NUM_ModTimerType,
	ModTimerType_Invalid
};
const RString& ModTimerTypeToString( ModTimerType cat );
const RString& ModTimerTypeToLocalizedString( ModTimerType cat );
LuaDeclareType( ModTimerType );

/** @brief Per-player options that are not saved between sessions. */
class PlayerOptions
{
public:
	/**
	 * @brief Set up the PlayerOptions with some reasonable defaults.
	 *
	 * This code was taken from Init() to use proper initialization. */
	PlayerOptions(): m_LifeType(LifeType_Bar), m_DrainType(DrainType_Normal),
		m_ModTimerType(ModTimerType_Default),
		m_BatteryLives(4),
		m_bSetScrollSpeed(false),
		m_fTimeSpacing(0), m_SpeedfTimeSpacing(1.0f),
		m_fMaxScrollBPM(0), m_SpeedfMaxScrollBPM(1.0f),
		m_fScrollSpeed(1.0f), m_SpeedfScrollSpeed(1.0f),
		m_fScrollBPM(200), m_SpeedfScrollBPM(1.0f),
		m_fDark(0), m_SpeedfDark(1.0f),
		m_fBlind(0), m_SpeedfBlind(1.0f),
		m_fCover(0), m_SpeedfCover(1.0f),
		m_fRandAttack(0), m_SpeedfRandAttack(1.0f),
		m_fNoAttack(0), m_SpeedfNoAttack(1.0f),
		m_fPlayerAutoPlay(0), m_SpeedfPlayerAutoPlay(1.0f),
		m_fPerspectiveTilt(0), m_SpeedfPerspectiveTilt(1.0f),
		m_fSkew(0), m_SpeedfSkew(1.0f),
		m_fPassmark(0), m_SpeedfPassmark(1.0f),
		m_fRandomSpeed(0), m_SpeedfRandomSpeed(1.0f),
		m_fModTimerMult(0), m_SpeedfModTimerMult(1.0f),
		m_fModTimerOffset(0), m_SpeedfModTimerOffset(1.0f),
		m_fDrawSize(0), m_SpeedfDrawSize(1.0f),
		m_fDrawSizeBack(0), m_SpeedfDrawSizeBack(1.0f),
		m_bMuteOnError(false),
		m_bStealthType(false), m_bStealthPastReceptors(false),
		m_bDizzyHolds(false), m_bZBuffer(false),
		m_bCosecant(false),
		m_FailType(FailType_Immediate),
		m_MinTNSToHideNotes(PREFSMAN->m_MinTNSToHideNotes)
	{
		m_sNoteSkin = "";
		m_fVisualDelay = 0.0f;
		ZERO( m_fAccels );	ONE( m_SpeedfAccels );
		ZERO( m_fEffects );	ONE( m_SpeedfEffects );
		ZERO( m_fAppearances );	ONE( m_SpeedfAppearances );
		ZERO( m_fScrolls );	ONE( m_SpeedfScrolls );
		ZERO( m_bTurns );	ZERO( m_bTransforms );
		ZERO( m_fMovesX );	ONE( m_SpeedfMovesX );
		ZERO( m_fMovesY );	ONE( m_SpeedfMovesY );
		ZERO( m_fMovesZ );	ONE( m_SpeedfMovesZ );
		ZERO( m_fConfusionX );	ONE( m_SpeedfConfusionX );
		ZERO( m_fConfusionY );	ONE( m_SpeedfConfusionY );
		ZERO( m_fConfusionZ );	ONE( m_SpeedfConfusionZ );
		ZERO( m_fDarks );	ONE( m_SpeedfDarks );
		ZERO( m_fStealth );	ONE( m_SpeedfStealth );
		ZERO( m_fTiny );	ONE( m_SpeedfTiny );
		ZERO( m_fBumpy );	ONE( m_SpeedfBumpy );
		ZERO( m_fReverse );	ONE( m_SpeedfReverse );
	};
	void Init();
	void Approach( const PlayerOptions& other, float fDeltaSeconds );
	RString GetString( bool bForceNoteSkin = false ) const;
	RString GetSavedPrefsString() const;	// only the basic options that players would want for every song
	enum ResetPrefsType
	{
		saved_prefs,
		saved_prefs_invalid_for_course
	};
	void ResetPrefs( ResetPrefsType type );
	void ResetSavedPrefs() { ResetPrefs(saved_prefs); };
	void ResetSavedPrefsInvalidForCourse() { ResetPrefs(saved_prefs_invalid_for_course); }
	void GetMods( std::vector<RString> &AddTo, bool bForceNoteSkin = false ) const;
	void GetLocalizedMods( std::vector<RString> &AddTo ) const;
	void FromString( const RString &sMultipleMods );
	bool FromOneModString( const RString &sOneMod, RString &sErrorDetailOut );	// On error, return false and optionally set sErrorDetailOut
	void ChooseRandomModifiers();
	bool ContainsTransformOrTurn() const;

	// Lua
	void PushSelf( lua_State *L );

	bool operator==( const PlayerOptions &other ) const;
	bool operator!=( const PlayerOptions &other ) const { return !operator==(other); }
	PlayerOptions& operator=(PlayerOptions const& other);
	PlayerOptions(const PlayerOptions& other) { operator=(other); }

	/** @brief The various acceleration mods. */
	enum Accel {
		ACCEL_BOOST, /**< The arrows start slow, then zoom towards the targets. */
		ACCEL_BRAKE, /**< The arrows start fast, then slow down as they approach the targets. */
		ACCEL_WAVE,
		ACCEL_WAVE_PERIOD,
		ACCEL_EXPAND,
		ACCEL_EXPAND_PERIOD,
		ACCEL_TAN_EXPAND,
		ACCEL_TAN_EXPAND_PERIOD,
		ACCEL_BOOMERANG, /**< The arrows start from above the targets, go down, then come back up. */
		NUM_ACCELS
	};
	enum Effect	{
		EFFECT_DRUNK,
		EFFECT_DRUNK_SPEED,
		EFFECT_DRUNK_OFFSET,
		EFFECT_DRUNK_PERIOD,
		EFFECT_TAN_DRUNK,
		EFFECT_TAN_DRUNK_SPEED,
		EFFECT_TAN_DRUNK_OFFSET,
		EFFECT_TAN_DRUNK_PERIOD,
		EFFECT_DRUNK_Z,
		EFFECT_DRUNK_Z_SPEED,
		EFFECT_DRUNK_Z_OFFSET,
		EFFECT_DRUNK_Z_PERIOD,
		EFFECT_TAN_DRUNK_Z,
		EFFECT_TAN_DRUNK_Z_SPEED,
		EFFECT_TAN_DRUNK_Z_OFFSET,
		EFFECT_TAN_DRUNK_Z_PERIOD,
		EFFECT_DIZZY,
		EFFECT_ATTENUATE_X,
		EFFECT_ATTENUATE_Y,
		EFFECT_ATTENUATE_Z,
		EFFECT_SHRINK_TO_MULT,
		EFFECT_SHRINK_TO_LINEAR,
		EFFECT_PULSE_INNER,
		EFFECT_PULSE_OUTER,
		EFFECT_PULSE_OFFSET,
		EFFECT_PULSE_PERIOD,
		EFFECT_CONFUSION,
		EFFECT_CONFUSION_OFFSET,
		EFFECT_CONFUSION_X,
		EFFECT_CONFUSION_X_OFFSET,
		EFFECT_CONFUSION_Y,
		EFFECT_CONFUSION_Y_OFFSET,
		EFFECT_BOUNCE,
		EFFECT_BOUNCE_PERIOD,
		EFFECT_BOUNCE_OFFSET,
		EFFECT_BOUNCE_Z,
		EFFECT_BOUNCE_Z_PERIOD,
		EFFECT_BOUNCE_Z_OFFSET,
		EFFECT_MINI,
		EFFECT_TINY,
		EFFECT_FLIP,
		EFFECT_INVERT,
		EFFECT_TORNADO,
		EFFECT_TORNADO_PERIOD,
		EFFECT_TORNADO_OFFSET,
		EFFECT_TAN_TORNADO,
		EFFECT_TAN_TORNADO_PERIOD,
		EFFECT_TAN_TORNADO_OFFSET,
		EFFECT_TORNADO_Z,
		EFFECT_TORNADO_Z_PERIOD,
		EFFECT_TORNADO_Z_OFFSET,
		EFFECT_TAN_TORNADO_Z,
		EFFECT_TAN_TORNADO_Z_PERIOD,
		EFFECT_TAN_TORNADO_Z_OFFSET,
		EFFECT_TIPSY,
		EFFECT_TIPSY_SPEED,
		EFFECT_TIPSY_OFFSET,
		EFFECT_TAN_TIPSY,
		EFFECT_TAN_TIPSY_SPEED,
		EFFECT_TAN_TIPSY_OFFSET,
		EFFECT_BUMPY,
		EFFECT_BUMPY_OFFSET,
		EFFECT_BUMPY_PERIOD,
		EFFECT_TAN_BUMPY,
		EFFECT_TAN_BUMPY_OFFSET,
		EFFECT_TAN_BUMPY_PERIOD,
		EFFECT_BUMPY_X,
		EFFECT_BUMPY_X_OFFSET,
		EFFECT_BUMPY_X_PERIOD,
		EFFECT_TAN_BUMPY_X,
		EFFECT_TAN_BUMPY_X_OFFSET,
		EFFECT_TAN_BUMPY_X_PERIOD,
		EFFECT_BEAT,
		EFFECT_BEAT_OFFSET,
		EFFECT_BEAT_PERIOD,
		EFFECT_BEAT_MULT,
		EFFECT_BEAT_Y,
		EFFECT_BEAT_Y_OFFSET,
		EFFECT_BEAT_Y_PERIOD,
		EFFECT_BEAT_Y_MULT,
		EFFECT_BEAT_Z,
		EFFECT_BEAT_Z_OFFSET,
		EFFECT_BEAT_Z_PERIOD,
		EFFECT_BEAT_Z_MULT,
		EFFECT_DIGITAL,
		EFFECT_DIGITAL_STEPS,
		EFFECT_DIGITAL_PERIOD,
		EFFECT_DIGITAL_OFFSET,
		EFFECT_TAN_DIGITAL,
		EFFECT_TAN_DIGITAL_STEPS,
		EFFECT_TAN_DIGITAL_PERIOD,
		EFFECT_TAN_DIGITAL_OFFSET,
		EFFECT_DIGITAL_Z,
		EFFECT_DIGITAL_Z_STEPS,
		EFFECT_DIGITAL_Z_PERIOD,
		EFFECT_DIGITAL_Z_OFFSET,
		EFFECT_TAN_DIGITAL_Z,
		EFFECT_TAN_DIGITAL_Z_STEPS,
		EFFECT_TAN_DIGITAL_Z_PERIOD,
		EFFECT_TAN_DIGITAL_Z_OFFSET,
		EFFECT_ZIGZAG,
		EFFECT_ZIGZAG_PERIOD,
		EFFECT_ZIGZAG_OFFSET,
		EFFECT_ZIGZAG_Z,
		EFFECT_ZIGZAG_Z_PERIOD,
		EFFECT_ZIGZAG_Z_OFFSET,
		EFFECT_SAWTOOTH,
		EFFECT_SAWTOOTH_PERIOD,
		EFFECT_SAWTOOTH_Z,
		EFFECT_SAWTOOTH_Z_PERIOD,
		EFFECT_SQUARE,
		EFFECT_SQUARE_PERIOD,
		EFFECT_SQUARE_OFFSET,
		EFFECT_SQUARE_Z,
		EFFECT_SQUARE_Z_PERIOD,
		EFFECT_SQUARE_Z_OFFSET,
		EFFECT_PARABOLA_X,
		EFFECT_PARABOLA_Y,
		EFFECT_PARABOLA_Z,
		EFFECT_XMODE,
		EFFECT_TWIRL,
		EFFECT_ROLL,
		NUM_EFFECTS
	};
	/** @brief The various appearance mods. */
	enum Appearance {
		APPEARANCE_HIDDEN, /**< The arrows disappear partway up. */
		APPEARANCE_HIDDEN_OFFSET, /**< This determines when the arrows disappear. */
		APPEARANCE_SUDDEN, /**< The arrows appear partway up. */
		APPEARANCE_SUDDEN_OFFSET, /**< This determines when the arrows appear. */
		APPEARANCE_STEALTH, /**< The arrows are not shown at all. */
		APPEARANCE_BLINK, /**< The arrows blink constantly. */
		APPEARANCE_RANDOMVANISH, /**< The arrows disappear, and then reappear in a different column. */
		NUM_APPEARANCES
	};
	/** @brief The various turn mods. */
	enum Turn {
		TURN_NONE=0, /**< No turning of the arrows is performed. */
		TURN_MIRROR, /**< The arrows are mirrored from their normal position. */
		TURN_LRMIRROR, /**< The left and right arrows are mirrored from their normal position. */
		TURN_UDMIRROR, /**< The up and down arrows are mirrored from their normal position. */
		TURN_BACKWARDS, /**< The arrows are turned 180 degrees. This does NOT always equal mirror. */
		TURN_LEFT, /**< The arrows are turned 90 degrees to the left. */
		TURN_RIGHT, /**< The arrows are turned 90 degress to the right. */
		TURN_SHUFFLE, /**< Some of the arrow columns are changed throughout the whole song. */
		TURN_SOFT_SHUFFLE, /**< Only shuffle arrow columns on an axis of symmetry. */
		TURN_SUPER_SHUFFLE, /**< Every arrow is placed on a random column. */
		TURN_HYPER_SHUFFLE, /**< Every arrow is placed on a random column, but more fairly than SuperShuffle. */
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
		TRANSFORM_ATTACKMINES,
		TRANSFORM_ECHO,
		TRANSFORM_STOMP,
		TRANSFORM_PLANTED,
		TRANSFORM_FLOORED,
		TRANSFORM_TWISTER,
		TRANSFORM_HOLDROLLS,
		TRANSFORM_NOJUMPS,
		TRANSFORM_NOHANDS,
		TRANSFORM_NOLIFTS,
		TRANSFORM_NOFAKES,
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

	float GetReversePercentForColumn( int iCol ) const; // accounts for all Directions

	PlayerNumber m_pn; // Needed for fetching the style.

	LifeType m_LifeType;
	DrainType m_DrainType;	// only used with LifeBar
	ModTimerType m_ModTimerType;
	int m_BatteryLives;
	/* All floats have a corresponding speed setting, which determines how fast
	 * PlayerOptions::Approach approaches. */
	bool	m_bSetScrollSpeed;				// true if the scroll speed was set by FromString
	float	m_fTimeSpacing,			m_SpeedfTimeSpacing;	// instead of Beat spacing (CMods, mMods)
	float	m_fMaxScrollBPM,		m_SpeedfMaxScrollBPM;
	float	m_fScrollSpeed,			m_SpeedfScrollSpeed;	// used if !m_bTimeSpacing (xMods)
	float	m_fScrollBPM,			m_SpeedfScrollBPM;		// used if m_bTimeSpacing (CMod)
	float	m_fAccels[NUM_ACCELS],		m_SpeedfAccels[NUM_ACCELS];
	float	m_fEffects[NUM_EFFECTS],	m_SpeedfEffects[NUM_EFFECTS];
	float	m_fAppearances[NUM_APPEARANCES],m_SpeedfAppearances[NUM_APPEARANCES];
	float	m_fScrolls[NUM_SCROLLS],	m_SpeedfScrolls[NUM_SCROLLS];
	float	m_fDark,			m_SpeedfDark;
	float	m_fBlind,			m_SpeedfBlind;
	float	m_fCover,			m_SpeedfCover;	// hide the background per-player--can't think of a good name
	float	m_fRandAttack,			m_SpeedfRandAttack;
	float	m_fNoAttack,			m_SpeedfNoAttack;
	float	m_fPlayerAutoPlay,		m_SpeedfPlayerAutoPlay;
	float	m_fPerspectiveTilt,		m_SpeedfPerspectiveTilt;		// -1 = near, 0 = overhead, +1 = space
	float	m_fSkew,			m_SpeedfSkew;		// 0 = vanish point is in center of player, 1 = vanish point is in center of screen

	/* If this is > 0, then the player must have life above this value at the end of
	 * the song to pass.  This is independent of SongOptions::m_FailType. */
	float		m_fPassmark,			m_SpeedfPassmark;

	float	m_fRandomSpeed,			m_SpeedfRandomSpeed;
	float	m_fModTimerMult,		m_SpeedfModTimerMult;
	float	m_fModTimerOffset,		m_SpeedfModTimerOffset;
	float	m_fDrawSize,			m_SpeedfDrawSize;
	float	m_fDrawSizeBack,		m_SpeedfDrawSizeBack;
	/* The maximum column number is 16.*/
	float	m_fMovesX[16],			m_SpeedfMovesX[16];
	float	m_fMovesY[16],			m_SpeedfMovesY[16];
	float	m_fMovesZ[16],			m_SpeedfMovesZ[16];
	float	m_fConfusionX[16],		m_SpeedfConfusionX[16];
	float	m_fConfusionY[16],		m_SpeedfConfusionY[16];
	float	m_fConfusionZ[16],		m_SpeedfConfusionZ[16];
	float	m_fDarks[16],			m_SpeedfDarks[16];
	float	m_fStealth[16],			m_SpeedfStealth[16];
	float	m_fTiny[16],			m_SpeedfTiny[16];
	float	m_fBumpy[16],			m_SpeedfBumpy[16];
	float	m_fReverse[16],			m_SpeedfReverse[16];

	bool		m_bTurns[NUM_TURNS];
	bool		m_bTransforms[NUM_TRANSFORMS];
	bool		m_bMuteOnError;
	bool		m_bStealthType;
	bool		m_bStealthPastReceptors;
	bool		m_bDizzyHolds;
	bool		m_bZBuffer;
	bool		m_bCosecant;
	/** @brief The method for which a player can fail a song. */
	FailType m_FailType;
	TapNoteScore m_MinTNSToHideNotes;

	/**
	 * @brief The Noteskin to use.
	 *
	 * If an empty string, it means to not change from the default. */
	RString		m_sNoteSkin;

	/** @brief The Visual Delay additionally applied on a per-player basis in ms. */
	float	m_fVisualDelay;

	/** @brief The TimingWindow that can be disabled.
	 *  Valid values are only W1-W5 which map to indices 0-5 respectively.
     *  Other values are ignored. */
	std::bitset<5> m_twDisabledWindows;

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
	bool IsEasierForSongAndSteps( Song* pSong, Steps* pSteps, PlayerNumber pn ) const;
	bool IsEasierForCourseAndTrail( Course* pCourse, Trail* pTrail ) const;
};

#define ADD_MULTICOL_METHOD( method_name) \
	ADD_METHOD( method_name##1 ); \
	ADD_METHOD( method_name##2 ); \
	ADD_METHOD( method_name##3 ); \
	ADD_METHOD( method_name##4 ); \
	ADD_METHOD( method_name##5 ); \
	ADD_METHOD( method_name##6 ); \
	ADD_METHOD( method_name##7 ); \
	ADD_METHOD( method_name##8 ); \
	ADD_METHOD( method_name##9 ); \
	ADD_METHOD( method_name##10 ); \
	ADD_METHOD( method_name##11 ); \
	ADD_METHOD( method_name##12 ); \
	ADD_METHOD( method_name##13 ); \
	ADD_METHOD( method_name##14 ); \
	ADD_METHOD( method_name##15 ); \
	ADD_METHOD( method_name##16 );
#define MULTICOL_FLOAT_INTERFACE(func_name, member, valid) \
	FLOAT_INTERFACE(func_name##1, member[0], valid); \
	FLOAT_INTERFACE(func_name##2, member[1], valid); \
	FLOAT_INTERFACE(func_name##3, member[2], valid); \
	FLOAT_INTERFACE(func_name##4, member[3], valid); \
	FLOAT_INTERFACE(func_name##5, member[4], valid); \
	FLOAT_INTERFACE(func_name##6, member[5], valid); \
	FLOAT_INTERFACE(func_name##7, member[6], valid); \
	FLOAT_INTERFACE(func_name##8, member[7], valid); \
	FLOAT_INTERFACE(func_name##9, member[8], valid); \
	FLOAT_INTERFACE(func_name##10, member[9], valid); \
	FLOAT_INTERFACE(func_name##11, member[10], valid); \
	FLOAT_INTERFACE(func_name##12, member[11], valid); \
	FLOAT_INTERFACE(func_name##13, member[12], valid); \
	FLOAT_INTERFACE(func_name##14, member[13], valid); \
	FLOAT_INTERFACE(func_name##15, member[14], valid); \
	FLOAT_INTERFACE(func_name##16, member[15], valid);

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
