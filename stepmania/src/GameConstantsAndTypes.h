#pragma once
/*
-----------------------------------------------------------------------------
 File: GameConstantsAndTypes.h

 Desc: These things don't change very often.

 Copyright (c) 2001-2002 by the person(s) listed below.  All rights reserved.
	Chris Danford
-----------------------------------------------------------------------------
*/


#define		SCREEN_WIDTH	(640)
#define		SCREEN_HEIGHT	(480)

#define		SCREEN_LEFT		(0)
#define		SCREEN_RIGHT	(SCREEN_WIDTH)
#define		SCREEN_TOP		(0)
#define		SCREEN_BOTTOM	(SCREEN_HEIGHT)

#define		CENTER_X		(SCREEN_LEFT + (SCREEN_RIGHT - SCREEN_LEFT)/2.0f)
#define		CENTER_Y		(SCREEN_TOP + (SCREEN_BOTTOM - SCREEN_TOP)/2.0f)


const int MAX_NOTE_TRACKS	=	16;


const int MAX_MEASURES		= 200;	// this should be long enough to hold 10:00 minute songs (
const int BEATS_PER_MEASURE = 4;
const int MAX_BEATS			= MAX_MEASURES * BEATS_PER_MEASURE;

const int ELEMENTS_PER_BEAT	= 12;	// It is important that this number is evenly divisible by 2, 3, 4, and 8
const int ELEMENTS_PER_MEASURE = ELEMENTS_PER_BEAT * BEATS_PER_MEASURE;
const int MAX_TAP_NOTE_ROWS = MAX_BEATS*ELEMENTS_PER_BEAT;

const int MAX_HOLD_NOTE_ELEMENTS = 200;

enum RadarCatrgory	// starting from 12-o'clock rotating clockwise
{
	RADAR_STREAM = 0,
	RADAR_VOLTAGE,
	RADAR_AIR,
	RADAR_CHAOS,
	RADAR_FREEZE,
	NUM_RADAR_VALUES	// leave this at the end
};

enum DifficultyClass 
{ 
	CLASS_EASY,		// corresponds to Basic
	CLASS_MEDIUM,	// corresponds to Trick, Another, Standard
	CLASS_HARD,		// corresponds to Maniac, SSR, Heavy
	NUM_DIFFICULTY_CLASSES
};

enum GameMode
{
	GAME_MODE_ARCADE,
	GAME_MODE_FREE_PLAY,
	GAME_MODE_NONSTOP,
	NUM_GAME_MODES
};

enum GameButtonGraphic { 
	GRAPHIC_NOTE_COLOR_PART,
	GRAPHIC_NOTE_GRAY_PART,
	GRAPHIC_RECEPTOR,
	GRAPHIC_TAP_EXPLOSION_BRIGHT,
	GRAPHIC_TAP_EXPLOSION_DIM,
	GRAPHIC_HOLD_EXPLOSION,
	NUM_GAME_BUTTON_GRAPHICS	// leave this at the end
};


enum PlayerNumber {
	PLAYER_1 = 0,
	PLAYER_2,
	NUM_PLAYERS,	// leave this at the end
	PLAYER_NONE
};

inline D3DXCOLOR PlayerToColor( PlayerNumber p ) 
{
	switch( p )
	{
		case PLAYER_1:	return D3DXCOLOR(0.4f,1.0f,0.8f,1);	// sea green
		case PLAYER_2:	return D3DXCOLOR(1.0f,0.5f,0.2f,1);	// orange
		default:	ASSERT( false ); return D3DXCOLOR(1,1,1,1);
	}
};
inline D3DXCOLOR PlayerToColor( int p ) { return PlayerToColor( (PlayerNumber)p ); }

enum NoteType 
{ 
	NOTE_4TH,	// quarter notes
	NOTE_8TH,	// eighth notes
	NOTE_12TH,	// triplets
	NOTE_16TH,	// sixteenth notes
	NUM_NOTE_TYPES
};

inline D3DXCOLOR NoteTypeToColor( NoteType nt )
{
	switch( nt )
	{
	case NOTE_4TH:	return D3DXCOLOR(1,0,0,1);	// red
	case NOTE_8TH:	return D3DXCOLOR(0,0,1,1);	// blue
	case NOTE_12TH:	return D3DXCOLOR(1,0,1,1);	// purple
	case NOTE_16TH:	return D3DXCOLOR(1,1,0,1);	// yellow
	default:		ASSERT( false );	return D3DXCOLOR(1,1,1,1);
	}		
};

inline float NoteTypeToBeat( NoteType nt )
{
	switch( nt )
	{
	case NOTE_4TH:	return 1.0f;	// quarter notes
	case NOTE_8TH:	return 1.0f/2;	// eighth notes
	case NOTE_12TH:	return 1.0f/3;	// triplets
	case NOTE_16TH:	return 1.0f/4;	// sixteenth notes
	default:	ASSERT( false );	return 0;
	}
};

inline bool IsNoteOfType( int iNoteIndex, NoteType t )
{ 
	switch( t )
	{
	case NOTE_4TH:	return iNoteIndex % (ELEMENTS_PER_MEASURE/4) == 0;
	case NOTE_8TH:	return iNoteIndex % (ELEMENTS_PER_MEASURE/8) == 0;
	case NOTE_12TH:	return iNoteIndex % (ELEMENTS_PER_MEASURE/12) == 0;
	case NOTE_16TH:	return iNoteIndex % (ELEMENTS_PER_MEASURE/16) == 0;
	default:	ASSERT( false );	return false;
	}
};

inline D3DXCOLOR GetNoteColorFromIndex( int iStepIndex )
{ 
	for( int t=0; t<NUM_NOTE_TYPES; t++ )
	{
		if( IsNoteOfType( iStepIndex, (NoteType)t ) )
			return NoteTypeToColor( (NoteType)t );
	}
	return D3DXCOLOR(0.5f,0.5f,0.5f,1);
};

typedef unsigned char TapNote;
typedef unsigned char TrackNumber;
typedef unsigned short NoteIndex;
typedef TrackNumber ColumnNumber;

enum SongSortOrder { 
	SORT_GROUP, 
	SORT_TITLE, 
	SORT_BPM, 
	SORT_MOST_PLAYED, 
	NUM_SORT_ORDERS 
};



struct GameOptions
{
	GameOptions() 
	{
		m_bIgnoreJoyAxes = false;
		m_bShowFPS = true;
		m_bUseRandomVis = false;
		m_bAnnouncer = true;
		m_bShowCaution = true;
		m_bShowSelectDifficulty = true;
		m_bShowSelectGroup = true;
		m_iNumArcadeStages = 3;
		m_JudgementDifficulty = JUDGE_NORMAL;
	};
	bool m_bIgnoreJoyAxes;
	bool m_bShowFPS;
	bool m_bUseRandomVis;
	bool m_bAnnouncer;
	bool m_bShowCaution;
	bool m_bShowSelectDifficulty;
	bool m_bShowSelectGroup;
	int	m_iNumArcadeStages;
	enum JudgementDifficulty { JUDGE_EASY=0, JUDGE_NORMAL, JUDGE_HARD };
	JudgementDifficulty m_JudgementDifficulty;
};

enum GraphicProfile
{
	PROFILE_SUPER_LOW = 0,
	PROFILE_LOW,
	PROFILE_MEDIUM,
	PROFILE_HIGH,
	PROFILE_CUSTOM,
	NUM_GRAPHIC_PROFILES
};

struct GraphicProfileOptions
{
	char m_szProfileName[30];
	DWORD m_dwWidth;
	DWORD m_dwHeight;
	DWORD m_dwMaxTextureSize;
	DWORD m_dwDisplayColor;
	DWORD m_dwTextureColor;
	bool m_bBackgrounds;
};

inline DWORD GetHeightFromWidth( DWORD dwWidth )
{
	switch( dwWidth )
	{
	case 320:	return 240;
	case 400:	return 300;
	case 512:	return 384;
	case 640:	return 480;
	case 800:	return 600;
	case 1024:	return 768;
	case 1280:	return 1024;
	default:	return 480;
	}
}


struct PlayerOptions
{
	PlayerOptions() {
		m_fArrowScrollSpeed = 1.0f;
		m_EffectType = EFFECT_NONE;
		m_AppearanceType = APPEARANCE_VISIBLE;
		m_TurnType = TURN_NONE;
		m_bLittle = false;
		m_bReverseScroll = false;
		m_ColorType = COLOR_ARCADE;
		m_bAllowFreezeArrows = true;
		m_DrainType = DRAIN_NORMAL;

		m_fInitialLifePercentage = 0.5f;
		m_fLifeAdjustments[LIFE_PERFECT] =	 0.010f;
		m_fLifeAdjustments[LIFE_GREAT]	=	 0.005f;
		m_fLifeAdjustments[LIFE_GOOD]	=	 0.000f;
		m_fLifeAdjustments[LIFE_BOO]		=	-0.015f;
		m_fLifeAdjustments[LIFE_MISS]	=	-0.030f;
	};

	float m_fArrowScrollSpeed;
	enum EffectType	{ EFFECT_NONE=0, EFFECT_BOOST, EFFECT_WAVE, EFFECT_DRUNK, EFFECT_DIZZY, EFFECT_SPACE };
	EffectType m_EffectType;
	enum AppearanceType	{ APPEARANCE_VISIBLE=0, APPEARANCE_HIDDEN, APPEARANCE_SUDDEN, APPEARANCE_STEALTH };
	AppearanceType m_AppearanceType;
	enum TurnType { TURN_NONE=0, TURN_MIRROR, TURN_LEFT, TURN_RIGHT, TURN_SHUFFLE };
	TurnType m_TurnType;
	bool m_bLittle;
	bool m_bReverseScroll;
	enum ColorType { COLOR_ARCADE=0, COLOR_NOTE, COLOR_FLAT, COLOR_PLAIN };
	ColorType m_ColorType;
	bool m_bAllowFreezeArrows;
	enum DrainType { DRAIN_NORMAL=0, DRAIN_NO_RECOVER, DRAIN_SUDDEN_DEATH };
	DrainType m_DrainType;

	float m_fInitialLifePercentage;
	enum LifeAdjustmentType { LIFE_PERFECT=0, LIFE_GREAT, LIFE_GOOD, LIFE_BOO, LIFE_MISS, NUM_LIFE_ADJUSTMENTS };
	float m_fLifeAdjustments[NUM_LIFE_ADJUSTMENTS];
};

struct SongOptions
{
	SongOptions() {
		m_FailType = FAIL_ARCADE;
		m_AssistType = ASSIST_NONE;
		m_fMusicRate = 1.0f;
		m_fMusicPitch = 0.0f;
		m_bShowMeasureBars = false;
	};

	enum FailType { FAIL_ARCADE=0, FAIL_END_OF_SONG, FAIL_OFF };
	FailType m_FailType;
	enum AssistType { ASSIST_NONE=0, ASSIST_TICK };
	AssistType m_AssistType;
	float m_fMusicRate;
	float m_fMusicPitch;
	bool m_bShowMeasureBars;
};


struct ScoreSummary {
	ScoreSummary() { perfect=great=good=boo=miss=ok=ng=max_combo=0; score=0; };
	int perfect, great, good, boo, miss, ok, ng, max_combo;
	float score;
};
