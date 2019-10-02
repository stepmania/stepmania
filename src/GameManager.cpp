#include "global.h"
#include "StepMania.h"
#include "arch/Dialog/Dialog.h"
#include "GameManager.h"
#include "GameConstantsAndTypes.h"
#include "GameInput.h"	// for GameButton constants
#include "GameLoop.h"  // for ChangeGame
#include "RageLog.h"
#include "RageUtil.h"
#include "NoteSkinManager.h"
#include "RageInputDevice.h"
#include "ThemeManager.h"
#include "LightsManager.h"	// for NUM_CabinetLight
#include "Game.h"
#include "Style.h"

GameManager*	GAMEMAN = nullptr;	// global and accessable from anywhere in our program

enum 
{
	TRACK_1 = 0,
	TRACK_2,
	TRACK_3,
	TRACK_4,
	TRACK_5,
	TRACK_6,
	TRACK_7,
	TRACK_8,
	TRACK_9,
	TRACK_10,
	TRACK_11,
	TRACK_12,
	TRACK_13,
	TRACK_14,
	TRACK_15,
	TRACK_16,
	// 16 tracks needed for beat-double7 and techno-double8
};

RString StepsTypeInfo::GetLocalizedString() const
{
	if( THEME->HasString( "StepsType", szName ) )
		return THEME->GetString( "StepsType", szName );
	return szName;
}

static const StepsTypeInfo g_StepsTypeInfos[] = {
	// dance
	{ "dance-single",	4,	true,	StepsTypeCategory_Single },
	{ "dance-double",	8,	true,	StepsTypeCategory_Double },
	{ "dance-couple",	8,	true,	StepsTypeCategory_Couple },
	{ "dance-solo",		6,	true,	StepsTypeCategory_Single },
	{ "dance-threepanel",	3,	true,	StepsTypeCategory_Single }, // thanks to kurisu
	{ "dance-routine",	8,	false,	StepsTypeCategory_Routine },
	// pump
	{ "pump-single",	5,	true,	StepsTypeCategory_Single },
	{ "pump-halfdouble",	6,	true,	StepsTypeCategory_Double },
	{ "pump-double",	10,	true,	StepsTypeCategory_Double },
	{ "pump-couple",	10,	true,	StepsTypeCategory_Couple },
	// uh, dance-routine has that one bool as false... wtf? -aj
	{ "pump-routine",	10,	true,	StepsTypeCategory_Routine },
	// kb7
	{ "kb7-single",		7,	true,	StepsTypeCategory_Single },
	// { "kb7-small",		7,	true,	StepsTypeCategory_Single },
	// ez2dancer
	{ "ez2-single",		5,	true,	StepsTypeCategory_Single },	// Single: TL,LHH,D,RHH,TR
	{ "ez2-double",		10,	true,	StepsTypeCategory_Double },	// Double: Single x2
	{ "ez2-real",		7,	true,	StepsTypeCategory_Single },	// Real: TL,LHH,LHL,D,RHL,RHH,TR
	// parapara paradise
	{ "para-single",	5,	true,	StepsTypeCategory_Single },
	// ds3ddx
	{ "ds3ddx-single",	8,	true,	StepsTypeCategory_Single },
	// beatmania
	{ "bm-single5",		6,	true,	StepsTypeCategory_Single },	// called "bm" for backward compat
	{ "bm-versus5",		6,	true,	StepsTypeCategory_Single },	// called "bm" for backward compat
	{ "bm-double5",		12,	true,	StepsTypeCategory_Double },	// called "bm" for backward compat
	{ "bm-single7",		8,	true,	StepsTypeCategory_Single },	// called "bm" for backward compat
	{ "bm-versus7",		8,	true,	StepsTypeCategory_Single },	// called "bm" for backward compat
	{ "bm-double7",		16,	true,	StepsTypeCategory_Double },	// called "bm" for backward compat
	// dance maniax
	{ "maniax-single",	4,	true,	StepsTypeCategory_Single },
	{ "maniax-double",	8,	true,	StepsTypeCategory_Double },
	// technomotion
	{ "techno-single4",	4,	true,	StepsTypeCategory_Single },
	{ "techno-single5",	5,	true,	StepsTypeCategory_Single },
	{ "techno-single8",	8,	true,	StepsTypeCategory_Single },
	{ "techno-double4",	8,	true,	StepsTypeCategory_Double },
	{ "techno-double5",	10,	true,	StepsTypeCategory_Double },
	{ "techno-double8",	16,	true,	StepsTypeCategory_Double },
	// pop'n music
	{ "pnm-five",		5,	true,	StepsTypeCategory_Single },	// called "pnm" for backward compat
	{ "pnm-nine",		9,	true,	StepsTypeCategory_Single },	// called "pnm" for backward compat
	// cabinet lights and other fine StepsTypes that don't exist lol
	{ "lights-cabinet",	NUM_CabinetLight,	false,	StepsTypeCategory_Single }, // XXX disable lights autogen for now
	// kickbox mania
	{ "kickbox-human", 4, true, StepsTypeCategory_Single },
	{ "kickbox-quadarm", 4, true, StepsTypeCategory_Single },
	{ "kickbox-insect", 6, true, StepsTypeCategory_Single },
	{ "kickbox-arachnid", 8, true, StepsTypeCategory_Single },
};


// Important:  Every game must define the buttons: "Start", "Back", "MenuLeft", "Operator" and "MenuRight"
static const AutoMappings g_AutoKeyMappings_Dance = AutoMappings (
	"",
	"",
	"",
	AutoMappingEntry( 0, KEY_DEL,		GAME_BUTTON_MENULEFT,		false ),
	AutoMappingEntry( 0, KEY_PGDN,		GAME_BUTTON_MENURIGHT,		false ),
	AutoMappingEntry( 0, KEY_HOME,		GAME_BUTTON_MENUUP,		false ),
	AutoMappingEntry( 0, KEY_END,		GAME_BUTTON_MENUDOWN,		false ),
	AutoMappingEntry( 0, KEY_LEFT,		DANCE_BUTTON_LEFT,		false ),
	AutoMappingEntry( 0, KEY_RIGHT,		DANCE_BUTTON_RIGHT,		false ),
	AutoMappingEntry( 0, KEY_UP,		DANCE_BUTTON_UP,		false ),
	AutoMappingEntry( 0, KEY_DOWN,		DANCE_BUTTON_DOWN,		false ),
	AutoMappingEntry( 0, KEY_KP_SLASH,		GAME_BUTTON_MENULEFT,		true ),
	AutoMappingEntry( 0, KEY_KP_ASTERISK,	GAME_BUTTON_MENURIGHT,		true ),
	AutoMappingEntry( 0, KEY_KP_HYPHEN,		GAME_BUTTON_MENUUP,		true ),
	AutoMappingEntry( 0, KEY_KP_PLUS,		GAME_BUTTON_MENUDOWN,		true ),
	AutoMappingEntry( 0, KEY_KP_C4,		DANCE_BUTTON_LEFT,		true ),
	AutoMappingEntry( 0, KEY_KP_C6,		DANCE_BUTTON_RIGHT,		true ),
	AutoMappingEntry( 0, KEY_KP_C8,		DANCE_BUTTON_UP,		true ),
	AutoMappingEntry( 0, KEY_KP_C2,		DANCE_BUTTON_DOWN,		true ),
	AutoMappingEntry( 0, KEY_KP_C7,		DANCE_BUTTON_UPLEFT,		true ),
	AutoMappingEntry( 0, KEY_KP_C9,		DANCE_BUTTON_UPRIGHT,		true )
);

// xxx: get this from the theme? (see others)
// the problem with getting it from the noteskin is that this is meant to be
// static const; if we switch to anything we likely won't get const anymore
// but i may be talking out of my ass -aj
static const int DANCE_COL_SPACING = 64;

//static ThemeMetric<int>	DANCE_COL_SPACING("ColumnSpacing","Dance");
// named after a similar metric in Aldo_MX's build for compatibility/familiarity:
//float	DANCE_COL_SPACING	NOTESKIN->GetMetricF("NoteDisplay","ArrowColSpacing");
/* looking for ARROW_SIZE should be enough (ArrowSize)
 * just arroweffects.cpp for rowspacing (ArrowRowSpacing)
 */

static const Style g_Style_Dance_Single =
{	// STYLE_DANCE_SINGLE
	true,				// m_bUsedForGameplay
	true,				// m_bUsedForEdit
	true,				// m_bUsedForDemonstration
	true,				// m_bUsedForHowToPlay
	"single",			// m_szName
	StepsType_dance_single,	// m_StepsType
	StyleType_OnePlayerOneSide,		// m_StyleType
	4,				// m_iColsPerPlayer
	{	// m_ColumnInfo[NUM_PLAYERS][MAX_COLS_PER_PLAYER];
		{	// PLAYER_1
			{ TRACK_1,	-DANCE_COL_SPACING*1.5f, nullptr },
			{ TRACK_2,	-DANCE_COL_SPACING*0.5f, nullptr },
			{ TRACK_3,	+DANCE_COL_SPACING*0.5f, nullptr },
			{ TRACK_4,	+DANCE_COL_SPACING*1.5f, nullptr },
		},
		{	// PLAYER_2
			{ TRACK_1,	-DANCE_COL_SPACING*1.5f, nullptr },
			{ TRACK_2,	-DANCE_COL_SPACING*0.5f, nullptr },
			{ TRACK_3,	+DANCE_COL_SPACING*0.5f, nullptr },
			{ TRACK_4,	+DANCE_COL_SPACING*1.5f, nullptr },
		},
	},
	{	// m_iInputColumn[NUM_GameController][NUM_GameButton]
		{ 0, 3, 2, 1, Style::END_MAPPING },
		{ 0, 3, 2, 1, Style::END_MAPPING }
	},
	{	// m_iColumnDrawOrder[MAX_COLS_PER_PLAYER];
		0, 1, 2, 3
	},
	true, // m_bCanUseBeginnerHelper
	false, // m_bLockDifficulties
};

static const Style g_Style_Dance_Versus =
{	// STYLE_DANCE_VERSUS
	true,				// m_bUsedForGameplay
	false,				// m_bUsedForEdit
	true,				// m_bUsedForDemonstration
	false,				// m_bUsedForHowToPlay
	"versus",			// m_szName
	StepsType_dance_single,	// m_StepsType
	StyleType_TwoPlayersTwoSides,		// m_StyleType
	4,				// m_iColsPerPlayer
	{	// m_ColumnInfo[NUM_PLAYERS][MAX_COLS_PER_PLAYER];
		{	// PLAYER_1
			{ TRACK_1,	-DANCE_COL_SPACING*1.5f, nullptr },
			{ TRACK_2,	-DANCE_COL_SPACING*0.5f, nullptr },
			{ TRACK_3,	+DANCE_COL_SPACING*0.5f, nullptr },
			{ TRACK_4,	+DANCE_COL_SPACING*1.5f, nullptr },
		},
		{	// PLAYER_2
			{ TRACK_1,	-DANCE_COL_SPACING*1.5f, nullptr },
			{ TRACK_2,	-DANCE_COL_SPACING*0.5f, nullptr },
			{ TRACK_3,	+DANCE_COL_SPACING*0.5f, nullptr },
			{ TRACK_4,	+DANCE_COL_SPACING*1.5f, nullptr },
		},
	},
	{
		{ 0, 3, 2, 1, Style::END_MAPPING },
		{ 0, 3, 2, 1, Style::END_MAPPING }
	},
	{	// m_iColumnDrawOrder[MAX_COLS_PER_PLAYER];
		0, 1, 2, 3
	},
	true, // m_bCanUseBeginnerHelper
	false, // m_bLockDifficulties
};

static const Style g_Style_Dance_Double =
{	// STYLE_DANCE_DOUBLE
	true,				// m_bUsedForGameplay
	true,				// m_bUsedForEdit
	true,				// m_bUsedForDemonstration
	false,				// m_bUsedForHowToPlay
	"double",			// m_szName
	StepsType_dance_double,	// m_StepsType
	StyleType_OnePlayerTwoSides,		// m_StyleType
	8,				// m_iColsPerPlayer
	{	// m_ColumnInfo[NUM_PLAYERS][MAX_COLS_PER_PLAYER];
		{	// PLAYER_1
			{ TRACK_1,	-DANCE_COL_SPACING*3.5f, nullptr },
			{ TRACK_2,	-DANCE_COL_SPACING*2.5f, nullptr },
			{ TRACK_3,	-DANCE_COL_SPACING*1.5f, nullptr },
			{ TRACK_4,	-DANCE_COL_SPACING*0.5f, nullptr },
			{ TRACK_5,	+DANCE_COL_SPACING*0.5f, nullptr },
			{ TRACK_6,	+DANCE_COL_SPACING*1.5f, nullptr },
			{ TRACK_7,	+DANCE_COL_SPACING*2.5f, nullptr },
			{ TRACK_8,	+DANCE_COL_SPACING*3.5f, nullptr },
		},
		{	// PLAYER_2
			{ TRACK_1,	-DANCE_COL_SPACING*3.5f, nullptr },
			{ TRACK_2,	-DANCE_COL_SPACING*2.5f, nullptr },
			{ TRACK_3,	-DANCE_COL_SPACING*1.5f, nullptr },
			{ TRACK_4,	-DANCE_COL_SPACING*0.5f, nullptr },
			{ TRACK_5,	+DANCE_COL_SPACING*0.5f, nullptr },
			{ TRACK_6,	+DANCE_COL_SPACING*1.5f, nullptr },
			{ TRACK_7,	+DANCE_COL_SPACING*2.5f, nullptr },
			{ TRACK_8,	+DANCE_COL_SPACING*3.5f, nullptr },
		},
	},
	{	// m_iInputColumn[NUM_GameController][NUM_GameButton]
		{ 0, 3, 2, 1, Style::END_MAPPING },
		{ 4, 7, 6, 5, Style::END_MAPPING }
	},
	{	// m_iColumnDrawOrder[MAX_COLS_PER_PLAYER];
		0,1,2,3,4,5,6,7
	},
	false, // m_bCanUseBeginnerHelper
	false, // m_bLockDifficulties
};

static const Style g_Style_Dance_Couple =
{	// STYLE_DANCE_COUPLE
	true,				// m_bUsedForGameplay
	false,				// m_bUsedForEdit
	false,				// m_bUsedForDemonstration
	false,				// m_bUsedForHowToPlay
	"couple",			// m_szName
	StepsType_dance_couple,	// m_StepsType
	StyleType_TwoPlayersTwoSides,		// m_StyleType
	4,				// m_iColsPerPlayer
	{	// m_ColumnInfo[NUM_PLAYERS][MAX_COLS_PER_PLAYER];
		{	// PLAYER_1
			{ TRACK_1,	-DANCE_COL_SPACING*1.5f, nullptr },
			{ TRACK_2,	-DANCE_COL_SPACING*0.5f, nullptr },
			{ TRACK_3,	+DANCE_COL_SPACING*0.5f, nullptr },
			{ TRACK_4,	+DANCE_COL_SPACING*1.5f, nullptr },
		},
		{	// PLAYER_2
			{ TRACK_5,	-DANCE_COL_SPACING*1.5f, nullptr },
			{ TRACK_6,	-DANCE_COL_SPACING*0.5f, nullptr },
			{ TRACK_7,	+DANCE_COL_SPACING*0.5f, nullptr },
			{ TRACK_8,	+DANCE_COL_SPACING*1.5f, nullptr },
		},
	},
	{	// m_iInputColumn[NUM_GameController][NUM_GameButton]
		{ 0, 3, 2, 1, Style::END_MAPPING },
		{ 0, 3, 2, 1, Style::END_MAPPING }
	},
	{	// m_iColumnDrawOrder[MAX_COLS_PER_PLAYER];
		0,1,2,3
	},
	true, // m_bCanUseBeginnerHelper
	true, // m_bLockDifficulties
};

static const Style g_Style_Dance_Solo =
{	// STYLE_DANCE_SOLO
	true,				// m_bUsedForGameplay
	true,				// m_bUsedForEdit
	false,				// m_bUsedForDemonstration
	false,				// m_bUsedForHowToPlay
	"solo",				// m_szName
	StepsType_dance_solo,		// m_StepsType
	StyleType_OnePlayerOneSide,		// m_StyleType
	6,				// m_iColsPerPlayer
	{	// m_ColumnInfo[NUM_PLAYERS][MAX_COLS_PER_PLAYER];
		{	// PLAYER_1
			{ TRACK_1,	-DANCE_COL_SPACING*2.5f, nullptr },
			{ TRACK_2,	-DANCE_COL_SPACING*1.5f, nullptr },
			{ TRACK_3,	-DANCE_COL_SPACING*0.5f, nullptr },
			{ TRACK_4,	+DANCE_COL_SPACING*0.5f, nullptr },
			{ TRACK_5,	+DANCE_COL_SPACING*1.5f, nullptr },
			{ TRACK_6,	+DANCE_COL_SPACING*2.5f, nullptr },
		},
		{	// PLAYER_2
			{ TRACK_1,	-DANCE_COL_SPACING*2.5f, nullptr },
			{ TRACK_2,	-DANCE_COL_SPACING*1.5f, nullptr },
			{ TRACK_3,	-DANCE_COL_SPACING*0.5f, nullptr },
			{ TRACK_4,	+DANCE_COL_SPACING*0.5f, nullptr },
			{ TRACK_5,	+DANCE_COL_SPACING*1.5f, nullptr },
			{ TRACK_6,	+DANCE_COL_SPACING*2.5f, nullptr },
		},
	},
	{	// m_iInputColumn[NUM_GameController][NUM_GameButton]
		{ 0, 5, 3, 2, 1, 4, Style::END_MAPPING },
		{ 0, 5, 3, 2, 1, 4, Style::END_MAPPING }
	},
	{	// m_iColumnDrawOrder[MAX_COLS_PER_PLAYER];
		0,1,2,3,4,5
	},
	false, // m_bCanUseBeginnerHelper
	false, // m_bLockDifficulties
};

static const Style g_Style_Dance_Couple_Edit =
{	// STYLE_DANCE_COUPLE
	false,				// m_bUsedForGameplay
	true,				// m_bUsedForEdit
	false,				// m_bUsedForDemonstration
	false,				// m_bUsedForHowToPlay
	"couple-edit",			// m_szName
	StepsType_dance_couple,	// m_StepsType
	StyleType_OnePlayerTwoSides,		// m_StyleType
	8,				// m_iColsPerPlayer
	{	// m_ColumnInfo[NUM_PLAYERS][MAX_COLS_PER_PLAYER];
		{	// PLAYER_1
			{ TRACK_1,	-DANCE_COL_SPACING*4.f, nullptr },
			{ TRACK_2,	-DANCE_COL_SPACING*3.f, nullptr },
			{ TRACK_3,	-DANCE_COL_SPACING*2.f, nullptr },
			{ TRACK_4,	-DANCE_COL_SPACING*1.f, nullptr },
			{ TRACK_5,	+DANCE_COL_SPACING*1.f, nullptr },
			{ TRACK_6,	+DANCE_COL_SPACING*2.f, nullptr },
			{ TRACK_7,	+DANCE_COL_SPACING*3.f, nullptr },
			{ TRACK_8,	+DANCE_COL_SPACING*4.f, nullptr },
		},
		{	// PLAYER_2
			{ TRACK_1,	-DANCE_COL_SPACING*4.f, nullptr },
			{ TRACK_2,	-DANCE_COL_SPACING*3.f, nullptr },
			{ TRACK_3,	-DANCE_COL_SPACING*2.f, nullptr },
			{ TRACK_4,	-DANCE_COL_SPACING*1.f, nullptr },
			{ TRACK_5,	+DANCE_COL_SPACING*1.f, nullptr },
			{ TRACK_6,	+DANCE_COL_SPACING*2.f, nullptr },
			{ TRACK_7,	+DANCE_COL_SPACING*3.f, nullptr },
			{ TRACK_8,	+DANCE_COL_SPACING*4.f, nullptr },
		},
	},
	{	// m_iInputColumn[NUM_GameController][NUM_GameButton]
		{ 0, 3, 2, 1, Style::END_MAPPING },
		{ 4, 7, 6, 5, Style::END_MAPPING },
	},
	{	// m_iColumnDrawOrder[MAX_COLS_PER_PLAYER];
		0,1,2,3,4,5,6,7
	},
	false, // m_bCanUseBeginnerHelper
	false, // m_bLockDifficulties
};

static const Style g_Style_Dance_ThreePanel =
{	// STYLE_DANCE_THREEPANEL
	true,				// m_bUsedForGameplay
	true,				// m_bUsedForEdit
	false,				// m_bUsedForDemonstration
	false,				// m_bUsedForHowToPlay
	"threepanel",			// m_szName
	StepsType_dance_threepanel,		// m_StepsType
	StyleType_OnePlayerOneSide,		// m_StyleType
	3,				// m_iColsPerPlayer
	{	// m_ColumnInfo[NUM_PLAYERS][MAX_COLS_PER_PLAYER];
		{	// PLAYER_1
			{ TRACK_1,	-DANCE_COL_SPACING*1.0f, nullptr },
			{ TRACK_2,	+DANCE_COL_SPACING*0.0f, nullptr },
			{ TRACK_3,	+DANCE_COL_SPACING*1.0f, nullptr },
		},
		{	// PLAYER_2
			{ TRACK_1,	-DANCE_COL_SPACING*1.0f, nullptr },
			{ TRACK_2,	+DANCE_COL_SPACING*0.0f, nullptr },
			{ TRACK_3,	+DANCE_COL_SPACING*1.0f, nullptr },
		},
	},
	{	// m_iInputColumn[NUM_GameController][NUM_GameButton]
		// 4 3 5
		{ 0, 2, Style::NO_MAPPING, 1, 0, 2, Style::END_MAPPING },
		{ 0, 2, Style::NO_MAPPING, 1, 0, 2, Style::END_MAPPING }
	},
	{	// m_iColumnDrawOrder[MAX_COLS_PER_PLAYER];
		0,1,2
	},
	false, // m_bCanUseBeginnerHelper
	false, // m_bLockDifficulties
};

// todo: re-enable? (lol)
/*
static const Style g_Style_Dance_Solo_Versus =
{	// STYLE_DANCE_SOLO_VERSUS 
	"dance-solo-versus",		// m_szName
	StepsType_dance_solo,		// m_StepsType
	ONE_PLAYER_ONE_CREDIT,		// m_StyleType
	6,				// m_iColsPerPlayer
	{	// m_ColumnInfo[NUM_PLAYERS][MAX_COLS_PER_PLAYER];
		{	// PLAYER_1
			{ TRACK_1,	-DANCE_6PANEL_VERSUS_COL_SPACING*2.5f },
			{ TRACK_2,	-DANCE_6PANEL_VERSUS_COL_SPACING*1.5f },
			{ TRACK_3,	-DANCE_6PANEL_VERSUS_COL_SPACING*0.5f },
			{ TRACK_4,	+DANCE_6PANEL_VERSUS_COL_SPACING*0.5f },
			{ TRACK_5,	+DANCE_6PANEL_VERSUS_COL_SPACING*1.5f },
			{ TRACK_6,	+DANCE_6PANEL_VERSUS_COL_SPACING*2.5f },
		},
		{	// PLAYER_2
			{ TRACK_1,	-DANCE_6PANEL_VERSUS_COL_SPACING*2.5f },
			{ TRACK_2,	-DANCE_6PANEL_VERSUS_COL_SPACING*1.5f },
			{ TRACK_3,	-DANCE_6PANEL_VERSUS_COL_SPACING*0.5f },
			{ TRACK_4,	+DANCE_6PANEL_VERSUS_COL_SPACING*0.5f },
			{ TRACK_5,	+DANCE_6PANEL_VERSUS_COL_SPACING*1.5f },
			{ TRACK_6,	+DANCE_6PANEL_VERSUS_COL_SPACING*2.5f },
		},
	},
	{	// m_iInputColumn[NUM_GameController][NUM_GameButton]
		{ 0, 5, 3, 2, 1, 4, Style::END_MAPPING },
		{ 0, 5, 3, 2, 1, 4, Style::END_MAPPING }
	},
	{	// m_iColumnDrawOrder[MAX_COLS_PER_PLAYER];
		0,5,1,4,2,3		// outside in
	},
	false, // m_bCanUseBeginnerHelper
	false, // m_bLockDifficulties
};	*/

static const Style g_Style_Dance_Routine =
{	// STYLE_DANCE_ROUTINE
	true,				// m_bUsedForGameplay
	true,				// m_bUsedForEdit
	false,				// m_bUsedForDemonstration
	false,				// m_bUsedForHowToPlay
	"routine",			// m_szName
	StepsType_dance_routine,	// m_StepsType
	StyleType_TwoPlayersSharedSides,	// m_StyleType
	8,				// m_iColsPerPlayer
	{	// m_ColumnInfo[NUM_PLAYERS][MAX_COLS_PER_PLAYER];
		{	// PLAYER_1
			{ TRACK_1,	-DANCE_COL_SPACING*3.5f, nullptr },
			{ TRACK_2,	-DANCE_COL_SPACING*2.5f, nullptr },
			{ TRACK_3,	-DANCE_COL_SPACING*1.5f, nullptr },
			{ TRACK_4,	-DANCE_COL_SPACING*0.5f, nullptr },
			{ TRACK_5,	+DANCE_COL_SPACING*0.5f, nullptr },
			{ TRACK_6,	+DANCE_COL_SPACING*1.5f, nullptr },
			{ TRACK_7,	+DANCE_COL_SPACING*2.5f, nullptr },
			{ TRACK_8,	+DANCE_COL_SPACING*3.5f, nullptr },
		},
		{	// PLAYER_2
			{ TRACK_1,	-DANCE_COL_SPACING*3.5f, nullptr },
			{ TRACK_2,	-DANCE_COL_SPACING*2.5f, nullptr },
			{ TRACK_3,	-DANCE_COL_SPACING*1.5f, nullptr },
			{ TRACK_4,	-DANCE_COL_SPACING*0.5f, nullptr },
			{ TRACK_5,	+DANCE_COL_SPACING*0.5f, nullptr },
			{ TRACK_6,	+DANCE_COL_SPACING*1.5f, nullptr },
			{ TRACK_7,	+DANCE_COL_SPACING*2.5f, nullptr },
			{ TRACK_8,	+DANCE_COL_SPACING*3.5f, nullptr },
		},
	},
	{	// m_iInputColumn[NUM_GameController][NUM_GameButton]
		{ 0, 3, 2, 1, Style::END_MAPPING },
		{ 4, 7, 6, 5, Style::END_MAPPING }
	},
	{	// m_iColumnDrawOrder[MAX_COLS_PER_PLAYER];
		0,1,2,3,4,5,6,7
	},
	false, // m_bCanUseBeginnerHelper
	true, // m_bLockDifficulties
};

static const Style *g_apGame_Dance_Styles[] =
{
	&g_Style_Dance_Single,
	&g_Style_Dance_Versus,
	&g_Style_Dance_Double,
	&g_Style_Dance_Couple,
	&g_Style_Dance_Solo,
	&g_Style_Dance_Couple_Edit,
	&g_Style_Dance_Routine,
	&g_Style_Dance_ThreePanel,
	nullptr
};

static const Game g_Game_Dance = 
{
	"dance",					// m_szName
	g_apGame_Dance_Styles,				// m_apStyles
	false,						// m_bCountNotesSeparately
	false, // m_bTickHolds
	false, // m_PlayersHaveSeparateStyles
	{						// m_InputScheme
		"dance",				// m_szName
		NUM_DANCE_BUTTONS,			// m_iButtonsPerController
		{	// m_szButtonNames
			{ "Left",		GAME_BUTTON_LEFT },
			{ "Right",		GAME_BUTTON_RIGHT },
			{ "Up",			GAME_BUTTON_UP },
			{ "Down",		GAME_BUTTON_DOWN },
			{ "UpLeft",		GameButton_Invalid },
			{ "UpRight",		GameButton_Invalid },
		},
		&g_AutoKeyMappings_Dance
	},
	{
		{ GameButtonType_Step },
		{ GameButtonType_Step },
		{ GameButtonType_Step },
		{ GameButtonType_Step },
		{ GameButtonType_Step },
		{ GameButtonType_Step },
	},
	TNS_W1,	// m_mapW1To
	TNS_W2,	// m_mapW2To
	TNS_W3,	// m_mapW3To
	TNS_W4,	// m_mapW4To
	TNS_W5,	// m_mapW5To
};

static const AutoMappings g_AutoKeyMappings_Pump = AutoMappings (
	"",
	"",
	"",
	AutoMappingEntry( 0, KEY_Cq,		PUMP_BUTTON_UPLEFT,		false ),
	AutoMappingEntry( 0, KEY_Ce,		PUMP_BUTTON_UPRIGHT,		false ),
	AutoMappingEntry( 0, KEY_Cs,		PUMP_BUTTON_CENTER,		false ),
	AutoMappingEntry( 0, KEY_Cz,		PUMP_BUTTON_DOWNLEFT,		false ),
	AutoMappingEntry( 0, KEY_Cc,		PUMP_BUTTON_DOWNRIGHT,		false ),
	AutoMappingEntry( 0, KEY_KP_C7,		PUMP_BUTTON_UPLEFT,		true ),
	AutoMappingEntry( 0, KEY_KP_C9,		PUMP_BUTTON_UPRIGHT,		true ),
	AutoMappingEntry( 0, KEY_KP_C5,		PUMP_BUTTON_CENTER,		true ),
	AutoMappingEntry( 0, KEY_KP_C1,		PUMP_BUTTON_DOWNLEFT,		true ),
	AutoMappingEntry( 0, KEY_KP_C3,		PUMP_BUTTON_DOWNRIGHT,		true ),

	// unmap confusing default MenuButtons
	AutoMappingEntry( 0, KEY_KP_C8,		GameButton_Invalid,		false ),
	AutoMappingEntry( 0, KEY_KP_C2,		GameButton_Invalid,		false ),
	AutoMappingEntry( 0, KEY_KP_C4,		GameButton_Invalid,		false ),
	AutoMappingEntry( 0, KEY_KP_C6,		GameButton_Invalid,		false )
);

// PIU Defaults: RowSpacing = 60; ColSpacing = 52; ArrowSize = 54;
// apparently column spacing is 48px
//static ThemeMetric<int>	PUMP_COL_SPACING	("ColumnSpacing","Pump");
static const int PUMP_COL_SPACING = 48;

static const Style g_Style_Pump_Single =
{	// STYLE_PUMP_SINGLE
	true,				// m_bUsedForGameplay
	true,				// m_bUsedForEdit
	false,				// m_bUsedForDemonstration
	true,				// m_bUsedForHowToPlay
	"single",			// m_szName
	StepsType_pump_single,		// m_StepsType
	StyleType_OnePlayerOneSide,		// m_StyleType
	5,				// m_iColsPerPlayer
	{	// m_ColumnInfo[NUM_PLAYERS][MAX_COLS_PER_PLAYER];
		{	// PLAYER_1
			{ TRACK_1,	-PUMP_COL_SPACING*2.0f, nullptr },
			{ TRACK_2,	-PUMP_COL_SPACING*1.0f, nullptr },
			{ TRACK_3,	+PUMP_COL_SPACING*0.0f, nullptr },
			{ TRACK_4,	+PUMP_COL_SPACING*1.0f, nullptr },
			{ TRACK_5,	+PUMP_COL_SPACING*2.0f, nullptr },
		},
		{	// PLAYER_2
			{ TRACK_1,	-PUMP_COL_SPACING*2.0f, nullptr },
			{ TRACK_2,	-PUMP_COL_SPACING*1.0f, nullptr },
			{ TRACK_3,	+PUMP_COL_SPACING*0.0f, nullptr },
			{ TRACK_4,	+PUMP_COL_SPACING*1.0f, nullptr },
			{ TRACK_5,	+PUMP_COL_SPACING*2.0f, nullptr },
		},
	},
	{	// m_iInputColumn[NUM_GameController][NUM_GameButton]
		{ 1, 3, 2, 0, 4, Style::END_MAPPING },
		{ 1, 3, 2, 0, 4, Style::END_MAPPING },
	},
	{	// m_iColumnDrawOrder[MAX_COLS_PER_PLAYER];
		2,1,3,0,4
	},
	false, // m_bCanUseBeginnerHelper
	false, // m_bLockDifficulties
};

static const Style g_Style_Pump_Versus =
{	// STYLE_PUMP_VERSUS
	true,				// m_bUsedForGameplay
	false,				// m_bUsedForEdit
	true,				// m_bUsedForDemonstration
	false,				// m_bUsedForHowToPlay
	"versus",			// m_szName
	StepsType_pump_single,		// m_StepsType
	StyleType_TwoPlayersTwoSides,		// m_StyleType
	5,				// m_iColsPerPlayer
	{	// m_ColumnInfo[NUM_PLAYERS][MAX_COLS_PER_PLAYER];
		{	// PLAYER_1
			{ TRACK_1,	-PUMP_COL_SPACING*2.0f, nullptr },
			{ TRACK_2,	-PUMP_COL_SPACING*1.0f, nullptr },
			{ TRACK_3,	+PUMP_COL_SPACING*0.0f, nullptr },
			{ TRACK_4,	+PUMP_COL_SPACING*1.0f, nullptr },
			{ TRACK_5,	+PUMP_COL_SPACING*2.0f, nullptr },
		},
		{	// PLAYER_2
			{ TRACK_1,	-PUMP_COL_SPACING*2.0f, nullptr },
			{ TRACK_2,	-PUMP_COL_SPACING*1.0f, nullptr },
			{ TRACK_3,	+PUMP_COL_SPACING*0.0f, nullptr },
			{ TRACK_4,	+PUMP_COL_SPACING*1.0f, nullptr },
			{ TRACK_5,	+PUMP_COL_SPACING*2.0f, nullptr },
		},
	},
	{	// m_iInputColumn[NUM_GameController][NUM_GameButton]
		{ 1, 3, 2, 0, 4, Style::END_MAPPING },
		{ 1, 3, 2, 0, 4, Style::END_MAPPING },
	},
	{	// m_iColumnDrawOrder[MAX_COLS_PER_PLAYER];
		2,1,3,0,4
	},
	false, // m_bCanUseBeginnerHelper
	false, // m_bLockDifficulties
};

static const Style g_Style_Pump_HalfDouble =
{	// STYLE_PUMP_HALFDOUBLE
	true,				// m_bUsedForGameplay
	true,				// m_bUsedForEdit
	false,				// m_bUsedForDemonstration
	false,				// m_bUsedForHowToPlay
	"halfdouble",			// m_szName
	StepsType_pump_halfdouble,	// m_StepsType
	StyleType_OnePlayerTwoSides,		// m_StyleType
	6,				// m_iColsPerPlayer
	{	// m_ColumnInfo[NUM_PLAYERS][MAX_COLS_PER_PLAYER];
		{	// PLAYER_1
			{ TRACK_1,	-PUMP_COL_SPACING*2.5f-4, nullptr },
			{ TRACK_2,	-PUMP_COL_SPACING*1.5f-4, nullptr },
			{ TRACK_3,	-PUMP_COL_SPACING*0.5f-4, nullptr },
			{ TRACK_4,	+PUMP_COL_SPACING*0.5f+4, nullptr },
			{ TRACK_5,	+PUMP_COL_SPACING*1.5f+4, nullptr },
			{ TRACK_6,	+PUMP_COL_SPACING*2.5f+4, nullptr },
		},
		{	// PLAYER_2
			{ TRACK_1,	-PUMP_COL_SPACING*2.5f-4, nullptr },
			{ TRACK_2,	-PUMP_COL_SPACING*1.5f-4, nullptr },
			{ TRACK_3,	-PUMP_COL_SPACING*0.5f-4, nullptr },
			{ TRACK_4,	+PUMP_COL_SPACING*0.5f+4, nullptr },
			{ TRACK_5,	+PUMP_COL_SPACING*1.5f+4, nullptr },
			{ TRACK_6,	+PUMP_COL_SPACING*2.5f+4, nullptr },
		},
	},
	{	// m_iInputColumn[NUM_GameController][NUM_GameButton]
		{ Style::NO_MAPPING, 1, 0, Style::NO_MAPPING, 2, Style::END_MAPPING },
		{ 4, Style::NO_MAPPING, 5, 3, Style::NO_MAPPING, Style::END_MAPPING }
	},
	{	// m_iColumnDrawOrder[MAX_COLS_PER_PLAYER];
		2,3,1,4,0,5
	},
	false, // m_bCanUseBeginnerHelper
	false, // m_bLockDifficulties
};

static const Style g_Style_Pump_Double =
{	// STYLE_PUMP_DOUBLE
	true,				// m_bUsedForGameplay
	true,				// m_bUsedForEdit
	false,				// m_bUsedForDemonstration
	false,				// m_bUsedForHowToPlay
	"double",			// m_szName
	StepsType_pump_double,		// m_StepsType
	StyleType_OnePlayerTwoSides,		// m_StyleType
	10,				// m_iColsPerPlayer
	{	// m_ColumnInfo[NUM_PLAYERS][MAX_COLS_PER_PLAYER];
		{	// PLAYER_1
			{ TRACK_1,	-PUMP_COL_SPACING*4.5f-4, nullptr },
			{ TRACK_2,	-PUMP_COL_SPACING*3.5f-4, nullptr },
			{ TRACK_3,	-PUMP_COL_SPACING*2.5f-4, nullptr },
			{ TRACK_4,	-PUMP_COL_SPACING*1.5f-4, nullptr },
			{ TRACK_5,	-PUMP_COL_SPACING*0.5f-4, nullptr },
			{ TRACK_6,	+PUMP_COL_SPACING*0.5f+4, nullptr },
			{ TRACK_7,	+PUMP_COL_SPACING*1.5f+4, nullptr },
			{ TRACK_8,	+PUMP_COL_SPACING*2.5f+4, nullptr },
			{ TRACK_9,	+PUMP_COL_SPACING*3.5f+4, nullptr },
			{ TRACK_10,	+PUMP_COL_SPACING*4.5f+4, nullptr },
		},
		{	// PLAYER_2
			{ TRACK_1,	-PUMP_COL_SPACING*4.5f-4, nullptr },
			{ TRACK_2,	-PUMP_COL_SPACING*3.5f-4, nullptr },
			{ TRACK_3,	-PUMP_COL_SPACING*2.5f-4, nullptr },
			{ TRACK_4,	-PUMP_COL_SPACING*1.5f-4, nullptr },
			{ TRACK_5,	-PUMP_COL_SPACING*0.5f-4, nullptr },
			{ TRACK_6,	+PUMP_COL_SPACING*0.5f+4, nullptr },
			{ TRACK_7,	+PUMP_COL_SPACING*1.5f+4, nullptr },
			{ TRACK_8,	+PUMP_COL_SPACING*2.5f+4, nullptr },
			{ TRACK_9,	+PUMP_COL_SPACING*3.5f+4, nullptr },
			{ TRACK_10,	+PUMP_COL_SPACING*4.5f+4, nullptr },
		},
	},
	{	// m_iInputColumn[NUM_GameController][NUM_GameButton]
		{ 1, 3, 2, 0, 4, Style::END_MAPPING },
		{ 6, 8, 7, 5, 9, Style::END_MAPPING },
	},
	{	// m_iColumnDrawOrder[MAX_COLS_PER_PLAYER];
		2,1,3,0,4, 2+5,1+5,3+5,0+5,4+5
	},
	false, // m_bCanUseBeginnerHelper
	false, // m_bLockDifficulties
};

static const Style g_Style_Pump_Couple =
{	// STYLE_PUMP_COUPLE
	true,				// m_bUsedForGameplay
	false,				// m_bUsedForEdit
	false,				// m_bUsedForDemonstration
	false,				// m_bUsedForHowToPlay
	"couple",			// m_szName
	StepsType_pump_couple,		// m_StepsType
	StyleType_TwoPlayersTwoSides,		// m_StyleType
	5,				// m_iColsPerPlayer
	{	// m_ColumnInfo[NUM_PLAYERS][MAX_COLS_PER_PLAYER];
		{	// PLAYER_1
			{ TRACK_1,	-PUMP_COL_SPACING*2.0f, nullptr },
			{ TRACK_2,	-PUMP_COL_SPACING*1.0f, nullptr },
			{ TRACK_3,	+PUMP_COL_SPACING*0.0f, nullptr },
			{ TRACK_4,	+PUMP_COL_SPACING*1.0f, nullptr },
			{ TRACK_5,	+PUMP_COL_SPACING*2.0f, nullptr },
		},
		{	// PLAYER_2
			{ TRACK_6,	-PUMP_COL_SPACING*2.0f, nullptr },
			{ TRACK_7,	-PUMP_COL_SPACING*1.0f, nullptr },
			{ TRACK_8,	+PUMP_COL_SPACING*0.0f, nullptr },
			{ TRACK_9,	+PUMP_COL_SPACING*1.0f, nullptr },
			{ TRACK_10,	+PUMP_COL_SPACING*2.0f, nullptr },
		},
	},
	{	// m_iInputColumn[NUM_GameController][NUM_GameButton]
		{ 1, 3, 2, 0, 4, Style::END_MAPPING },
		{ 1, 3, 2, 0, 4, Style::END_MAPPING },
	},
	{	// m_iColumnDrawOrder[MAX_COLS_PER_PLAYER];
		2,1,3,0,4
	},
	false, // m_bCanUseBeginnerHelper
	true, // m_bLockDifficulties
};

static const Style g_Style_Pump_Couple_Edit =
{	// STYLE_PUMP_EDIT_COUPLE
	false,				// m_bUsedForGameplay
	true,				// m_bUsedForEdit
	false,				// m_bUsedForDemonstration
	false,				// m_bUsedForHowToPlay
	"couple-edit",			// m_szName
	StepsType_pump_couple,		// m_StepsType
	StyleType_OnePlayerTwoSides,		// m_StyleType
	10,				// m_iColsPerPlayer
	{	// m_ColumnInfo[NUM_PLAYERS][MAX_COLS_PER_PLAYER];
		{	// PLAYER_1
			{ TRACK_1,	-PUMP_COL_SPACING*5.0f-4, nullptr },
			{ TRACK_2,	-PUMP_COL_SPACING*4.0f-4, nullptr },
			{ TRACK_3,	-PUMP_COL_SPACING*3.0f-4, nullptr },
			{ TRACK_4,	-PUMP_COL_SPACING*2.0f-4, nullptr },
			{ TRACK_5,	-PUMP_COL_SPACING*1.0f-4, nullptr },
			{ TRACK_6,	+PUMP_COL_SPACING*1.0f+4, nullptr },
			{ TRACK_7,	+PUMP_COL_SPACING*2.0f+4, nullptr },
			{ TRACK_8,	+PUMP_COL_SPACING*3.0f+4, nullptr },
			{ TRACK_9,	+PUMP_COL_SPACING*4.0f+4, nullptr },
			{ TRACK_10,	+PUMP_COL_SPACING*5.0f+4, nullptr },
		},
		{	// PLAYER_2
			{ TRACK_1,	-PUMP_COL_SPACING*5.0f-4, nullptr },
			{ TRACK_2,	-PUMP_COL_SPACING*4.0f-4, nullptr },
			{ TRACK_3,	-PUMP_COL_SPACING*3.0f-4, nullptr },
			{ TRACK_4,	-PUMP_COL_SPACING*2.0f-4, nullptr },
			{ TRACK_5,	-PUMP_COL_SPACING*1.0f-4, nullptr },
			{ TRACK_6,	+PUMP_COL_SPACING*1.0f+4, nullptr },
			{ TRACK_7,	+PUMP_COL_SPACING*2.0f+4, nullptr },
			{ TRACK_8,	+PUMP_COL_SPACING*3.0f+4, nullptr },
			{ TRACK_9,	+PUMP_COL_SPACING*4.0f+4, nullptr },
			{ TRACK_10,	+PUMP_COL_SPACING*5.0f+4, nullptr },
		},
	},
	{	// m_iInputColumn[NUM_GameController][NUM_GameButton]
		{ 1, 3, 2, 0, 4, Style::END_MAPPING },
		{ 6, 8, 7, 5, 9, Style::END_MAPPING },
	},
	{	// m_iColumnDrawOrder[MAX_COLS_PER_PLAYER];
		2,1,3,0,4, 2+5,1+5,3+5,0+5,4+5
	},
	false, // m_bCanUseBeginnerHelper
	false, // m_bLockDifficulties
};

static const Style g_Style_Pump_Routine =
{	// STYLE_PUMP_ROUTINE
	true,				// m_bUsedForGameplay
	true,				// m_bUsedForEdit
	false,				// m_bUsedForDemonstration
	false,				// m_bUsedForHowToPlay
	"routine",			// m_szName
	StepsType_pump_routine,		// m_StepsType
	StyleType_TwoPlayersSharedSides,	// m_StyleType
	10,				// m_iColsPerPlayer
	{	// m_ColumnInfo[NUM_PLAYERS][MAX_COLS_PER_PLAYER];
		{	// PLAYER_1
			{ TRACK_1,	-PUMP_COL_SPACING*4.5f-4, nullptr },
			{ TRACK_2,	-PUMP_COL_SPACING*3.5f-4, nullptr },
			{ TRACK_3,	-PUMP_COL_SPACING*2.5f-4, nullptr },
			{ TRACK_4,	-PUMP_COL_SPACING*1.5f-4, nullptr },
			{ TRACK_5,	-PUMP_COL_SPACING*0.5f-4, nullptr },
			{ TRACK_6,	+PUMP_COL_SPACING*0.5f+4, nullptr },
			{ TRACK_7,	+PUMP_COL_SPACING*1.5f+4, nullptr },
			{ TRACK_8,	+PUMP_COL_SPACING*2.5f+4, nullptr },
			{ TRACK_9,	+PUMP_COL_SPACING*3.5f+4, nullptr },
			{ TRACK_10,	+PUMP_COL_SPACING*4.5f+4, nullptr },
		},
		{	// PLAYER_2
			{ TRACK_1,	-PUMP_COL_SPACING*4.5f-4, nullptr },
			{ TRACK_2,	-PUMP_COL_SPACING*3.5f-4, nullptr },
			{ TRACK_3,	-PUMP_COL_SPACING*2.5f-4, nullptr },
			{ TRACK_4,	-PUMP_COL_SPACING*1.5f-4, nullptr },
			{ TRACK_5,	-PUMP_COL_SPACING*0.5f-4, nullptr },
			{ TRACK_6,	+PUMP_COL_SPACING*0.5f+4, nullptr },
			{ TRACK_7,	+PUMP_COL_SPACING*1.5f+4, nullptr },
			{ TRACK_8,	+PUMP_COL_SPACING*2.5f+4, nullptr },
			{ TRACK_9,	+PUMP_COL_SPACING*3.5f+4, nullptr },
			{ TRACK_10,	+PUMP_COL_SPACING*4.5f+4, nullptr },
		},
	},
	{	// m_iInputColumn[NUM_GameController][NUM_GameButton]
		{ 1, 3, 2, 0, 4, Style::END_MAPPING },
		{ 6, 8, 7, 5, 9, Style::END_MAPPING },
	},
	{	// m_iColumnDrawOrder[MAX_COLS_PER_PLAYER];
		2,1,3,0,4,7,6,8,5,9
	},
	false, // m_bCanUseBeginnerHelper
	true, // m_bLockDifficulties
};

static const Style *g_apGame_Pump_Styles[] =
{
	&g_Style_Pump_Single,
	&g_Style_Pump_Versus,
	&g_Style_Pump_HalfDouble,
	&g_Style_Pump_Double,
	&g_Style_Pump_Couple,
	&g_Style_Pump_Couple_Edit,
	&g_Style_Pump_Routine,
	nullptr
};

static const Game g_Game_Pump = 
{
	"pump",						// m_szName
	g_apGame_Pump_Styles,				// m_apStyles
	false,						// m_bCountNotesSeparately
	true, // m_bTickHolds
	false, // m_PlayersHaveSeparateStyles
	{						// m_InputScheme
		"pump",					// m_szName
		NUM_PUMP_BUTTONS,			// m_iButtonsPerController
		{	// m_szButtonNames
			{ "UpLeft",		GAME_BUTTON_UP },
			{ "UpRight",		GAME_BUTTON_DOWN },
			{ "Center",		GAME_BUTTON_START },
			{ "DownLeft",		GAME_BUTTON_LEFT },
			{ "DownRight",		GAME_BUTTON_RIGHT },
		},
		&g_AutoKeyMappings_Pump
	},
	{
		{ GameButtonType_Step },
		{ GameButtonType_Step },
		{ GameButtonType_Step },
		{ GameButtonType_Step },
		{ GameButtonType_Step },
	},
	TNS_W1,	// m_mapW1To
	TNS_W2,	// m_mapW2To
	TNS_W3,	// m_mapW3To
	TNS_W4,	// m_mapW4To
	TNS_W5,	// m_mapW5To
};

static const AutoMappings g_AutoKeyMappings_KB7 = AutoMappings (
	"",
	"",
	"",
	AutoMappingEntry( 0, KEY_Cs,		KB7_BUTTON_KEY1,		false ),
	AutoMappingEntry( 0, KEY_Cd,		KB7_BUTTON_KEY2,		false ),
	AutoMappingEntry( 0, KEY_Cf,		KB7_BUTTON_KEY3,		false ),
	AutoMappingEntry( 0, KEY_SPACE,		KB7_BUTTON_KEY4,		false ),
	AutoMappingEntry( 0, KEY_Cj,		KB7_BUTTON_KEY5,		false ),
	AutoMappingEntry( 0, KEY_Ck,		KB7_BUTTON_KEY6,		false ),
	AutoMappingEntry( 0, KEY_Cl,		KB7_BUTTON_KEY7,		false )
);

//ThemeMetric<int>	KB7_COL_SPACING	("ColumnSpacing","KB7");
static const int KB7_COL_SPACING = 64;
static const Style g_Style_KB7_Single =
{	// STYLE_KB7_SINGLE
	true,				// m_bUsedForGameplay
	true,				// m_bUsedForEdit
	false,				// m_bUsedForDemonstration
	true,				// m_bUsedForHowToPlay
	"single",			// m_szName
	StepsType_kb7_single,		// m_StepsType
	StyleType_OnePlayerOneSide,		// m_StyleType
	7,				// m_iColsPerPlayer
	{	// m_ColumnInfo[NUM_PLAYERS][MAX_COLS_PER_PLAYER];
		{	// PLAYER_1
			{ TRACK_1,	-KB7_COL_SPACING*3.25f, nullptr },
			{ TRACK_2,	-KB7_COL_SPACING*2.25f, nullptr },
			{ TRACK_3,	-KB7_COL_SPACING*1.25f, nullptr },
			{ TRACK_4,	+KB7_COL_SPACING*0.0f, nullptr },
			{ TRACK_5,	+KB7_COL_SPACING*1.25f, nullptr },
			{ TRACK_6,	+KB7_COL_SPACING*2.25f, nullptr },
			{ TRACK_7,	+KB7_COL_SPACING*3.25f, nullptr },
		},
		{	// PLAYER_2
			{ TRACK_1,	-KB7_COL_SPACING*3.25f, nullptr },
			{ TRACK_2,	-KB7_COL_SPACING*2.25f, nullptr },
			{ TRACK_3,	-KB7_COL_SPACING*1.25f, nullptr },
			{ TRACK_4,	+KB7_COL_SPACING*0.0f, nullptr },
			{ TRACK_5,	+KB7_COL_SPACING*1.25f, nullptr },
			{ TRACK_6,	+KB7_COL_SPACING*2.25f, nullptr },
			{ TRACK_7,	+KB7_COL_SPACING*3.25f, nullptr },
		},
	},
	{	// m_iInputColumn[NUM_GameController][NUM_GameButton]
		{ 0, 1, 2, 3, 4, 5, 6, Style::END_MAPPING },
		{ 0, 1, 2, 3, 4, 5, 6, Style::END_MAPPING },
	},
	{	// m_iColumnDrawOrder[MAX_COLS_PER_PLAYER];
		0,1,2,3,4,5,6 // doesn't work?
	},
	false, // m_bCanUseBeginnerHelper
	false, // m_bLockDifficulties
};

// ...
/* static const int KB7_COL_SPACING_SMALL = 10;
static const Style g_Style_KB7_Small =
{	// STYLE_KB7_SMALL
	true,				// m_bUsedForGameplay
	true,				// m_bUsedForEdit
	false,				// m_bUsedForDemonstration
	true,				// m_bUsedForHowToPlay
	"small",			// m_szName
	StepsType_kb7_single,		// m_StepsType
	StyleType_OnePlayerOneSide,		// m_StyleType
	7,				// m_iColsPerPlayer
	{	// m_ColumnInfo[NUM_PLAYERS][MAX_COLS_PER_PLAYER];
		{	// PLAYER_1
			{ TRACK_1,	-34, nullptr },
			{ TRACK_2,	-24, nullptr },
			{ TRACK_3,	-15, nullptr },
			{ TRACK_4,	0, nullptr },
			{ TRACK_5,	+15, nullptr },
			{ TRACK_6,	+24, nullptr },
			{ TRACK_7,	+34, nullptr },
		},
		{	// PLAYER_2
			{ TRACK_1,	-34, nullptr },
			{ TRACK_2,	-24, nullptr },
			{ TRACK_3,	-15, nullptr },
			{ TRACK_4,	0, nullptr },
			{ TRACK_5,	+15, nullptr },
			{ TRACK_6,	+24 nullptr },
			{ TRACK_7,	+34, nullptr },
		},
	},
	{	// m_iInputColumn[NUM_GameController][NUM_GameButton]
		{ 0, 1, 2, 3, 4, 5, 6, Style::END_MAPPING },
		{ 0, 1, 2, 3, 4, 5, 6, Style::END_MAPPING },
	},
	{	// m_iColumnDrawOrder[MAX_COLS_PER_PLAYER];
		0,1,2,3,4,5,6 // doesn't work?
	},
	false, // m_bCanUseBeginnerHelper
	false, // m_bLockDifficulties
}; */

static const Style g_Style_KB7_Versus =
{	// STYLE_KB7_VERSUS
	true,				// m_bUsedForGameplay
	false,				// m_bUsedForEdit
	true,				// m_bUsedForDemonstration
	false,				// m_bUsedForHowToPlay
	"versus",			// m_szName
	StepsType_kb7_single,		// m_StepsType
	StyleType_TwoPlayersTwoSides,		// m_StyleType
	7,				// m_iColsPerPlayer
	{	// m_ColumnInfo[NUM_PLAYERS][MAX_COLS_PER_PLAYER];
		{	// PLAYER_1
			{ TRACK_1,	-KB7_COL_SPACING*3.0f, nullptr },
			{ TRACK_2,	-KB7_COL_SPACING*2.0f, nullptr },
			{ TRACK_3,	-KB7_COL_SPACING*1.0f, nullptr },
			{ TRACK_4,	+KB7_COL_SPACING*0.0f, nullptr },
			{ TRACK_5,	+KB7_COL_SPACING*1.0f, nullptr },
			{ TRACK_6,	+KB7_COL_SPACING*2.0f, nullptr },
			{ TRACK_7,	+KB7_COL_SPACING*3.0f, nullptr },
		},
		{	// PLAYER_2
			{ TRACK_1,	-KB7_COL_SPACING*3.0f, nullptr },
			{ TRACK_2,	-KB7_COL_SPACING*2.0f, nullptr },
			{ TRACK_3,	-KB7_COL_SPACING*1.0f, nullptr },
			{ TRACK_4,	+KB7_COL_SPACING*0.0f, nullptr },
			{ TRACK_5,	+KB7_COL_SPACING*1.0f, nullptr },
			{ TRACK_6,	+KB7_COL_SPACING*2.0f, nullptr },
			{ TRACK_7,	+KB7_COL_SPACING*3.0f, nullptr },
		},
	},
	{	// m_iInputColumn[NUM_GameController][NUM_GameButton]
		{ 0, 1, 2, 3, 4, 5, 6, Style::END_MAPPING },
		{ 0, 1, 2, 3, 4, 5, 6, Style::END_MAPPING },
	},
	{	// m_iColumnDrawOrder[MAX_COLS_PER_PLAYER];
		0,1,2,3,4,5,6
	},
	false, // m_bCanUseBeginnerHelper
	false, // m_bLockDifficulties
};

static const Style *g_apGame_KB7_Styles[] =
{
	&g_Style_KB7_Single,
	// &g_Style_KB7_Small,
	&g_Style_KB7_Versus,
	nullptr
};

static const Game g_Game_KB7 = 
{
	"kb7",						// m_szName
	g_apGame_KB7_Styles,				// m_apStyles
	true,						// m_bCountNotesSeparately
	false, // m_bTickHolds
	false, // m_PlayersHaveSeparateStyles
	{						// m_InputScheme
		"kb7",					// m_szName
		NUM_KB7_BUTTONS,			// m_iButtonsPerController
		{	// m_szButtonNames
			{ "Key1",		GameButton_Invalid },
			{ "Key2",		GAME_BUTTON_LEFT },
			{ "Key3",		GAME_BUTTON_DOWN },
			{ "Key4",		GameButton_Invalid },
			{ "Key5",		GAME_BUTTON_UP },
			{ "Key6",		GAME_BUTTON_RIGHT },
			{ "Key7",		GameButton_Invalid },
		},
		&g_AutoKeyMappings_KB7
	},
	{
		{ GameButtonType_Step },
		{ GameButtonType_Step },
		{ GameButtonType_Step },
		{ GameButtonType_Step },
		{ GameButtonType_Step },
		{ GameButtonType_Step },
		{ GameButtonType_Step },
	},
	TNS_W1,	// m_mapW1To
	TNS_W2,	// m_mapW2To
	TNS_W3,	// m_mapW3To
	TNS_W4,	// m_mapW4To
	TNS_W5,	// m_mapW5To
};

//ThemeMetric<int>	EZ2_COL_SPACING	("ColumnSpacing","EZ2");
static const int EZ2_COL_SPACING = 46;
// do you even need this if they're the same now -aj
//ThemeMetric<int>	EZ2__REAL_COL_SPACING	("ColumnSpacing","EZ2Real");
static const int EZ2_REAL_COL_SPACING = 46;
static const Style g_Style_Ez2_Single =
{	// STYLE_EZ2_SINGLE
	true,				// m_bUsedForGameplay
	true,				// m_bUsedForEdit
	false,				// m_bUsedForDemonstration
	true,				// m_bUsedForHowToPlay
	"single",			// m_szName
	StepsType_ez2_single,		// m_StepsType
	StyleType_OnePlayerOneSide,		// m_StyleType
	5,				// m_iColsPerPlayer
	{	// m_ColumnInfo[NUM_PLAYERS][MAX_COLS_PER_PLAYER];
		{	// PLAYER_1
			{ TRACK_1,	-EZ2_COL_SPACING*2.0f, nullptr },
			{ TRACK_2,	-EZ2_COL_SPACING*1.0f, nullptr },
			{ TRACK_3,	+EZ2_COL_SPACING*0.0f, nullptr },
			{ TRACK_4,	+EZ2_COL_SPACING*1.0f, nullptr },
			{ TRACK_5,	+EZ2_COL_SPACING*2.0f, nullptr },
		},
		{	// PLAYER_2
			{ TRACK_1,	-EZ2_COL_SPACING*2.0f, nullptr },
			{ TRACK_2,	-EZ2_COL_SPACING*1.0f, nullptr },
			{ TRACK_3,	+EZ2_COL_SPACING*0.0f, nullptr },
			{ TRACK_4,	+EZ2_COL_SPACING*1.0f, nullptr },
			{ TRACK_5,	+EZ2_COL_SPACING*2.0f, nullptr },
		},
	},
	{	// m_iInputColumn[NUM_GameController][NUM_GameButton]
		{ 0, 4, 2, 1, 3, Style::END_MAPPING },
		{ 0, 4, 2, 1, 3, Style::END_MAPPING },
	},
	{	// m_iColumnDrawOrder[MAX_COLS_PER_PLAYER];
		2,0,4,1,3 // This should be from back to front: Down, UpLeft, UpRight, Upper Left Hand, Upper Right Hand 
	},
	false, // m_bCanUseBeginnerHelper
	false, // m_bLockDifficulties
};

static const Style g_Style_Ez2_Real =
{	// STYLE_EZ2_REAL
	true,				// m_bUsedForGameplay
	true,				// m_bUsedForEdit
	false,				// m_bUsedForDemonstration
	false,				// m_bUsedForHowToPlay
	"real",				// m_szName
	StepsType_ez2_real,		// m_StepsType
	StyleType_OnePlayerOneSide,		// m_StyleType
	7,				// m_iColsPerPlayer
	{	// m_ColumnInfo[NUM_PLAYERS][MAX_COLS_PER_PLAYER];
		{	// PLAYER_1
			{ TRACK_1,	-EZ2_REAL_COL_SPACING*2.3f, nullptr },
			{ TRACK_2,	-EZ2_REAL_COL_SPACING*1.6f, nullptr },
			{ TRACK_3,	-EZ2_REAL_COL_SPACING*0.9f, nullptr }, 
			{ TRACK_4,	+EZ2_REAL_COL_SPACING*0.0f, nullptr },
			{ TRACK_5,	+EZ2_REAL_COL_SPACING*0.9f, nullptr }, 
			{ TRACK_6,	+EZ2_REAL_COL_SPACING*1.6f, nullptr },
			{ TRACK_7,	+EZ2_REAL_COL_SPACING*2.3f, nullptr },
		},
		{	// PLAYER_2
			{ TRACK_1,	-EZ2_REAL_COL_SPACING*2.3f, nullptr },
			{ TRACK_2,	-EZ2_REAL_COL_SPACING*1.6f, nullptr },
			{ TRACK_3,	-EZ2_REAL_COL_SPACING*0.9f, nullptr }, 
			{ TRACK_4,	+EZ2_REAL_COL_SPACING*0.0f, nullptr },
			{ TRACK_5,	+EZ2_REAL_COL_SPACING*0.9f, nullptr }, 
			{ TRACK_6,	+EZ2_REAL_COL_SPACING*1.6f, nullptr },
			{ TRACK_7,	+EZ2_REAL_COL_SPACING*2.3f, nullptr },
		},
	},
	{	// m_iInputColumn[NUM_GameController][NUM_GameButton]
		{ 0, 6, 3, 2, 4, 1, 5, Style::END_MAPPING },
		{ 0, 6, 3, 2, 4, 1, 5, Style::END_MAPPING },
	},
	{	// m_iColumnDrawOrder[MAX_COLS_PER_PLAYER];
		3,0,6,1,5,2,4 // This should be from back to front: Down, UpLeft, UpRight, Lower Left Hand, Lower Right Hand, Upper Left Hand, Upper Right Hand
	},
	false, // m_bCanUseBeginnerHelper
	false, // m_bLockDifficulties
};

static const Style g_Style_Ez2_Single_Versus =
{	// STYLE_EZ2_SINGLE_VERSUS
	true,				// m_bUsedForGameplay
	true,				// m_bUsedForEdit
	true,				// m_bUsedForDemonstration
	false,				// m_bUsedForHowToPlay
	"versus",			// m_szName
	StepsType_ez2_single,		// m_StepsType
	StyleType_TwoPlayersTwoSides,		// m_StyleType
	5,				// m_iColsPerPlayer
	{	// m_ColumnInfo[NUM_PLAYERS][MAX_COLS_PER_PLAYER];
		{	// PLAYER_1
			{ TRACK_1,	-EZ2_COL_SPACING*2.0f, nullptr },
			{ TRACK_2,	-EZ2_COL_SPACING*1.0f, nullptr },
			{ TRACK_3,	+EZ2_COL_SPACING*0.0f, nullptr },
			{ TRACK_4,	+EZ2_COL_SPACING*1.0f, nullptr },
			{ TRACK_5,	+EZ2_COL_SPACING*2.0f, nullptr },
		},
		{	// PLAYER_2
			{ TRACK_1,	-EZ2_COL_SPACING*2.0f, nullptr },
			{ TRACK_2,	-EZ2_COL_SPACING*1.0f, nullptr },
			{ TRACK_3,	+EZ2_COL_SPACING*0.0f, nullptr },
			{ TRACK_4,	+EZ2_COL_SPACING*1.0f, nullptr },
			{ TRACK_5,	+EZ2_COL_SPACING*2.0f, nullptr },
		},
	},
	{	// m_iInputColumn[NUM_GameController][NUM_GameButton]
		{ 0, 4, 2, 1, 3, Style::END_MAPPING },
		{ 0, 4, 2, 1, 3, Style::END_MAPPING },
	},
	{	// m_iColumnDrawOrder[MAX_COLS_PER_PLAYER];
		2,0,4,1,3 // This should be from back to front: Down, UpLeft, UpRight, Upper Left Hand, Upper Right Hand 
	},
	false, // m_bCanUseBeginnerHelper
	false, // m_bLockDifficulties
};

static const Style g_Style_Ez2_Real_Versus =
{	// STYLE_EZ2_REAL_VERSUS
	true,				// m_bUsedForGameplay
	true,				// m_bUsedForEdit
	false,				// m_bUsedForDemonstration
	false,				// m_bUsedForHowToPlay
	"versusReal",			// m_szName
	StepsType_ez2_real,		// m_StepsType
	StyleType_TwoPlayersTwoSides,		// m_StyleType
	7,				// m_iColsPerPlayer
	{	// m_ColumnInfo[NUM_PLAYERS][MAX_COLS_PER_PLAYER];
		{	// PLAYER_1
			{ TRACK_1,	-EZ2_REAL_COL_SPACING*2.3f, nullptr },
			{ TRACK_2,	-EZ2_REAL_COL_SPACING*1.6f, nullptr },
			{ TRACK_3,	-EZ2_REAL_COL_SPACING*0.9f, nullptr }, 
			{ TRACK_4,	+EZ2_REAL_COL_SPACING*0.0f, nullptr },
			{ TRACK_5,	+EZ2_REAL_COL_SPACING*0.9f, nullptr }, 
			{ TRACK_6,	+EZ2_REAL_COL_SPACING*1.6f, nullptr },
			{ TRACK_7,	+EZ2_REAL_COL_SPACING*2.3f, nullptr },
		},
		{	// PLAYER_2
			{ TRACK_1,	-EZ2_REAL_COL_SPACING*2.3f, nullptr },
			{ TRACK_2,	-EZ2_REAL_COL_SPACING*1.6f, nullptr },
			{ TRACK_3,	-EZ2_REAL_COL_SPACING*0.9f, nullptr }, 
			{ TRACK_4,	+EZ2_REAL_COL_SPACING*0.0f, nullptr },
			{ TRACK_5,	+EZ2_REAL_COL_SPACING*0.9f, nullptr }, 
			{ TRACK_6,	+EZ2_REAL_COL_SPACING*1.6f, nullptr },
			{ TRACK_7,	+EZ2_REAL_COL_SPACING*2.3f, nullptr },
		},
	},
	{	// m_iInputColumn[NUM_GameController][NUM_GameButton]
		{ 0, 6, 3, 2, 4, 1, 5, Style::END_MAPPING },
		{ 0, 6, 3, 2, 4, 1, 5, Style::END_MAPPING },
	},
	{	// m_iColumnDrawOrder[MAX_COLS_PER_PLAYER];
		3,0,6,2,4,1,5 // This should be from back to front: Down, UpLeft, UpRight, Lower Left Hand, Lower Right Hand, Upper Left Hand, Upper Right Hand 
	},
	false, // m_bCanUseBeginnerHelper
	false, // m_bLockDifficulties
};

static const Style g_Style_Ez2_Double =
{	// STYLE_EZ2_DOUBLE
	true,				// m_bUsedForGameplay
	true,				// m_bUsedForEdit
	false,				// m_bUsedForDemonstration
	false,				// m_bUsedForHowToPlay
	"double",			// m_szName
	StepsType_ez2_double,		// m_StepsType
	StyleType_OnePlayerTwoSides,		// m_StyleType
	10,				// m_iColsPerPlayer
	{	// m_ColumnInfo[NUM_PLAYERS][MAX_COLS_PER_PLAYER];
		{	// PLAYER_1
			{ TRACK_1,	-EZ2_COL_SPACING*4.5f, nullptr },
			{ TRACK_2,	-EZ2_COL_SPACING*3.5f, nullptr },
			{ TRACK_3,	-EZ2_COL_SPACING*2.5f, nullptr },
			{ TRACK_4,	-EZ2_COL_SPACING*1.5f, nullptr },
			{ TRACK_5,	-EZ2_COL_SPACING*0.5f, nullptr },
			{ TRACK_6,	+EZ2_COL_SPACING*0.5f, nullptr },
			{ TRACK_7,	+EZ2_COL_SPACING*1.5f, nullptr },
			{ TRACK_8,	+EZ2_COL_SPACING*2.5f, nullptr },
			{ TRACK_9,	+EZ2_COL_SPACING*3.5f, nullptr },
			{ TRACK_10,	+EZ2_COL_SPACING*4.5f, nullptr },
		},
		{	// PLAYER_2
			{ TRACK_1,	-EZ2_COL_SPACING*4.5f, nullptr },
			{ TRACK_2,	-EZ2_COL_SPACING*3.5f, nullptr },
			{ TRACK_3,	-EZ2_COL_SPACING*2.5f, nullptr },
			{ TRACK_4,	-EZ2_COL_SPACING*1.5f, nullptr },
			{ TRACK_5,	-EZ2_COL_SPACING*0.5f, nullptr },
			{ TRACK_6,	+EZ2_COL_SPACING*0.5f, nullptr },
			{ TRACK_7,	+EZ2_COL_SPACING*1.5f, nullptr },
			{ TRACK_8,	+EZ2_COL_SPACING*2.5f, nullptr },
			{ TRACK_9,	+EZ2_COL_SPACING*3.5f, nullptr },
			{ TRACK_10,	+EZ2_COL_SPACING*4.5f, nullptr },
		},
	},
	{	// m_iInputColumn[NUM_GameController][NUM_GameButton]
		{ 0, 4, 2, 1, 3, Style::END_MAPPING },
		{ 5, 9, 7, 6, 8, Style::END_MAPPING },
	},
	{	// m_iColumnDrawOrder[MAX_COLS_PER_PLAYER];
		2,0,4,1,3,7,5,9,6,8 // This should be from back to front: Down, UpLeft, UpRight, Upper Left Hand, Upper Right Hand 
	},
	false, // m_bCanUseBeginnerHelper
	false, // m_bLockDifficulties
};

static const Style *g_apGame_Ez2_Styles[] =
{
	&g_Style_Ez2_Single,
	&g_Style_Ez2_Real,
	&g_Style_Ez2_Single_Versus,
	&g_Style_Ez2_Real_Versus,
	&g_Style_Ez2_Double,
	nullptr
};

static const AutoMappings g_AutoKeyMappings_Ez2 = AutoMappings (
	"",
	"",
	"",
	AutoMappingEntry( 0, KEY_Cz,		EZ2_BUTTON_FOOTUPLEFT,		false ),
	AutoMappingEntry( 0, KEY_Cb,		EZ2_BUTTON_FOOTUPRIGHT,		false ),
	AutoMappingEntry( 0, KEY_Cc,		EZ2_BUTTON_FOOTDOWN,		false ),
	AutoMappingEntry( 0, KEY_Cx,		EZ2_BUTTON_HANDUPLEFT,		false ),
	AutoMappingEntry( 0, KEY_Cv,		EZ2_BUTTON_HANDUPRIGHT,		false ),
	AutoMappingEntry( 0, KEY_Cs,		EZ2_BUTTON_HANDLRLEFT,		false ),
	AutoMappingEntry( 0, KEY_Cf,		EZ2_BUTTON_HANDLRRIGHT,		false )
);

static const Game g_Game_Ez2 = 
{
	"ez2",						// m_szName
	g_apGame_Ez2_Styles,				// m_apStyles
	true,						// m_bCountNotesSeparately
	false, // m_bTickHolds
	false, // m_PlayersHaveSeparateStyles
	{						// m_InputScheme
		"ez2",					// m_szName
		NUM_EZ2_BUTTONS,			// m_iButtonsPerController
		{	// m_szButtonNames
			{ "FootUpLeft",		GAME_BUTTON_UP },
			{ "FootUpRight",	GAME_BUTTON_DOWN },
			{ "FootDown",		GAME_BUTTON_START },
			{ "HandUpLeft",		GAME_BUTTON_LEFT },
			{ "HandUpRight",	GAME_BUTTON_RIGHT },
			{ "HandLrLeft",		GameButton_Invalid },
			{ "HandLrRight",	GameButton_Invalid },
		},
		&g_AutoKeyMappings_Ez2
	},
	{
		{ GameButtonType_Step },
		{ GameButtonType_Step },
		{ GameButtonType_Step },
		{ GameButtonType_Step },
		{ GameButtonType_Step },
		{ GameButtonType_Step },
		{ GameButtonType_Step },
	},
	TNS_W2,		// m_mapW1To
	TNS_W2,		// m_mapW2To
	TNS_W2,		// m_mapW3To
	TNS_W4,		// m_mapW4To
	TNS_Miss,	// m_mapW5To
};

//ThemeMetric<int>	PARA_COL_SPACING	("ColumnSpacing","Para");
static const int PARA_COL_SPACING = 54;

static const Style g_Style_Para_Single =
{	// STYLE_PARA_SINGLE
	true,				// m_bUsedForGameplay
	true,				// m_bUsedForEdit
	true,				// m_bUsedForDemonstration
	true,				// m_bUsedForHowToPlay
	"single",			// m_szName
	StepsType_para_single,		// m_StepsType
	StyleType_OnePlayerOneSide,		// m_StyleType
	5,				// m_iColsPerPlayer
	{	// m_ColumnInfo[NUM_PLAYERS][MAX_COLS_PER_PLAYER];
		{	// PLAYER_1
			{ TRACK_1,	-PARA_COL_SPACING*2.0f, nullptr },
			{ TRACK_2,	-PARA_COL_SPACING*1.0f, nullptr },
			{ TRACK_3,	+PARA_COL_SPACING*0.0f, nullptr },
			{ TRACK_4,	+PARA_COL_SPACING*1.0f, nullptr },
			{ TRACK_5,	+PARA_COL_SPACING*2.0f, nullptr },
		},
		{	// PLAYER_2
			{ TRACK_1,	-PARA_COL_SPACING*2.0f, nullptr },
			{ TRACK_2,	-PARA_COL_SPACING*1.0f, nullptr },
			{ TRACK_3,	+PARA_COL_SPACING*0.0f, nullptr },
			{ TRACK_4,	+PARA_COL_SPACING*1.0f, nullptr },
			{ TRACK_5,	+PARA_COL_SPACING*2.0f, nullptr },
		},
	},
	{	// m_iInputColumn[NUM_GameController][NUM_GameButton]
		{ 0, 1, 2, 3, 4, Style::END_MAPPING },
		{ 0, 1, 2, 3, 4, Style::END_MAPPING },
	},
	{	// m_iColumnDrawOrder[MAX_COLS_PER_PLAYER];
		2,0,4,1,3 
	},
	false, // m_bCanUseBeginnerHelper
	false, // m_bLockDifficulties
};

static const Style g_Style_Para_Versus =
{	// STYLE_PARA_VERSUS
	true,				// m_bUsedForGameplay
	true,				// m_bUsedForEdit
	true,				// m_bUsedForDemonstration
	true,				// m_bUsedForHowToPlay
	"versus",			// m_szName
	StepsType_para_single,		// m_StepsType
	StyleType_TwoPlayersTwoSides,		// m_StyleType
	5,				// m_iColsPerPlayer
	{	// m_ColumnInfo[NUM_PLAYERS][MAX_COLS_PER_PLAYER];
		{	// PLAYER_1
			{ TRACK_1,	-PARA_COL_SPACING*2.0f, nullptr },
			{ TRACK_2,	-PARA_COL_SPACING*1.0f, nullptr },
			{ TRACK_3,	+PARA_COL_SPACING*0.0f, nullptr },
			{ TRACK_4,	+PARA_COL_SPACING*1.0f, nullptr },
			{ TRACK_5,	+PARA_COL_SPACING*2.0f, nullptr },
		},
		{	// PLAYER_2
			{ TRACK_1,	-PARA_COL_SPACING*2.0f, nullptr },
			{ TRACK_2,	-PARA_COL_SPACING*1.0f, nullptr },
			{ TRACK_3,	+PARA_COL_SPACING*0.0f, nullptr },
			{ TRACK_4,	+PARA_COL_SPACING*1.0f, nullptr },
			{ TRACK_5,	+PARA_COL_SPACING*2.0f, nullptr },
		},
	},
	{	// m_iInputColumn[NUM_GameController][NUM_GameButton]
		{ 0, 1, 2, 3, 4, Style::END_MAPPING },
		{ 0, 1, 2, 3, 4, Style::END_MAPPING },
	},
	{	// m_iColumnDrawOrder[MAX_COLS_PER_PLAYER];
		2,0,4,1,3 
	},
	false, // m_bCanUseBeginnerHelper
	false, // m_bLockDifficulties
};

static const Style *g_apGame_Para_Styles[] =
{
	&g_Style_Para_Single,
	&g_Style_Para_Versus,
	nullptr
};

static const AutoMappings g_AutoKeyMappings_Para = AutoMappings (
	"",
	"",
	"",
	AutoMappingEntry( 0, KEY_Cz,		PARA_BUTTON_LEFT,		false ),
	AutoMappingEntry( 0, KEY_Cx,		PARA_BUTTON_UPLEFT,		false ),
	AutoMappingEntry( 0, KEY_Cc,		PARA_BUTTON_UP,			false ),
	AutoMappingEntry( 0, KEY_Cv,		PARA_BUTTON_UPRIGHT,		false ),
	AutoMappingEntry( 0, KEY_Cb,		PARA_BUTTON_RIGHT,		false )
);

static const Game g_Game_Para = 
{
	"para",						// m_szName
	g_apGame_Para_Styles,				// m_apStyles
	false,						// m_bCountNotesSeparately
	false, // m_bTickHolds
	false, // m_PlayersHaveSeparateStyles
	{						// m_InputScheme
		"para",					// m_szName
		NUM_PARA_BUTTONS,			// m_iButtonsPerController
		{	// m_szButtonNames
			{ "Left",		GAME_BUTTON_LEFT },
			{ "UpLeft",		GAME_BUTTON_DOWN },
			{ "Up",			GameButton_Invalid },
			{ "UpRight",		GAME_BUTTON_UP },
			{ "Right",		GAME_BUTTON_RIGHT },
		},
		&g_AutoKeyMappings_Para
	},
	{
		{ GameButtonType_Step },
		{ GameButtonType_Step },
		{ GameButtonType_Step },
		{ GameButtonType_Step },
		{ GameButtonType_Step },
	},
	TNS_W1,	// m_mapW1To
	TNS_W2,	// m_mapW2To
	TNS_W3,	// m_mapW3To
	TNS_W4,	// m_mapW4To
	TNS_W5,	// m_mapW5To
};

//ThemeMetric<int>	DS3DDX_COL_SPACING	("ColumnSpacing","DS3DDX");
static const int DS3DDX_COL_SPACING = 46;
static const Style g_Style_DS3DDX_Single =
{	// STYLE_DS3DDX_SINGLE
	true,				// m_bUsedForGameplay
	true,				// m_bUsedForEdit
	true,				// m_bUsedForDemonstration
	true,				// m_bUsedForHowToPlay
	"single",			// m_szName
	StepsType_ds3ddx_single,	// m_StepsType
	StyleType_OnePlayerOneSide,		// m_StyleType
	8,				// m_iColsPerPlayer
	{	// m_ColumnInfo[NUM_PLAYERS][MAX_COLS_PER_PLAYER];
		{	// PLAYER_1
			{ TRACK_1,	-DS3DDX_COL_SPACING*3.0f, nullptr },
			{ TRACK_2,	-DS3DDX_COL_SPACING*2.0f, nullptr },
			{ TRACK_3,	-DS3DDX_COL_SPACING*1.0f, nullptr },
			{ TRACK_4,	-DS3DDX_COL_SPACING*0.0f, nullptr },
			{ TRACK_5,	+DS3DDX_COL_SPACING*0.0f, nullptr },
			{ TRACK_6,	+DS3DDX_COL_SPACING*1.0f, nullptr },
			{ TRACK_7,	+DS3DDX_COL_SPACING*2.0f, nullptr },
			{ TRACK_8,	+DS3DDX_COL_SPACING*3.0f , nullptr},
		},
		{	// PLAYER_2
			{ TRACK_1,	-DS3DDX_COL_SPACING*3.0f, nullptr },
			{ TRACK_2,	-DS3DDX_COL_SPACING*2.0f, nullptr },
			{ TRACK_3,	-DS3DDX_COL_SPACING*1.0f, nullptr },
			{ TRACK_4,	-DS3DDX_COL_SPACING*0.0f, nullptr },
			{ TRACK_5,	+DS3DDX_COL_SPACING*0.0f, nullptr },
			{ TRACK_6,	+DS3DDX_COL_SPACING*1.0f, nullptr },
			{ TRACK_7,	+DS3DDX_COL_SPACING*2.0f, nullptr },
			{ TRACK_8,	+DS3DDX_COL_SPACING*3.0f, nullptr },
		},
	},
	{	// m_iInputColumn[NUM_GameController][NUM_GameButton]
		{ 0, 1, 2, 3, 4, 5, 6, 7, Style::END_MAPPING },
		{ 0, 1, 2, 3, 4, 5, 6, 7, Style::END_MAPPING },
	},
	{	// m_iColumnDrawOrder[MAX_COLS_PER_PLAYER];
		0,1,2,3,4,5,6,7
	},
	false, // m_bCanUseBeginnerHelper
	false, // m_bLockDifficulties
};

static const Style *g_apGame_DS3DDX_Styles[] =
{
	&g_Style_DS3DDX_Single,
	nullptr
};

static const AutoMappings g_AutoKeyMappings_DS3DDX = AutoMappings (
	"",
	"",
	"",
	AutoMappingEntry( 0, KEY_Ca,		DS3DDX_BUTTON_HANDLEFT,		false ),
	AutoMappingEntry( 0, KEY_Cz,		DS3DDX_BUTTON_FOOTDOWNLEFT,	false ),
	AutoMappingEntry( 0, KEY_Cq,		DS3DDX_BUTTON_FOOTUPLEFT,	false ),
	AutoMappingEntry( 0, KEY_Cw,		DS3DDX_BUTTON_HANDUP,		false ),
	AutoMappingEntry( 0, KEY_Cx,		DS3DDX_BUTTON_HANDDOWN,		false ),
	AutoMappingEntry( 0, KEY_Ce,		DS3DDX_BUTTON_FOOTUPRIGHT,	false ),
	AutoMappingEntry( 0, KEY_Cc,		DS3DDX_BUTTON_FOOTDOWNRIGHT,	false ),
	AutoMappingEntry( 0, KEY_Cd,		DS3DDX_BUTTON_HANDRIGHT,	false )
);

static const Game g_Game_DS3DDX = 
{
	"ds3ddx",					// m_szName
	g_apGame_DS3DDX_Styles,				// m_apStyles
	false,						// m_bCountNotesSeparately
	false, // m_bTickHolds
	false, // m_PlayersHaveSeparateStyles
	{						// m_InputScheme
		"ds3ddx",				// m_szName
		NUM_DS3DDX_BUTTONS,			// m_iButtonsPerController
		{	// m_szButtonNames
			{ "HandLeft",		GAME_BUTTON_LEFT },
			{ "FootDownLeft",	GameButton_Invalid },
			{ "FootUpLeft",		GameButton_Invalid },
			{ "HandUp",		GAME_BUTTON_UP },
			{ "HandDown",		GAME_BUTTON_DOWN },
			{ "FootUpRight",	GameButton_Invalid },
			{ "FootDownRight",	GameButton_Invalid },
			{ "HandRight",		GAME_BUTTON_RIGHT },
		},
		&g_AutoKeyMappings_DS3DDX
	},
	{
		{ GameButtonType_Step },
		{ GameButtonType_Step },
		{ GameButtonType_Step },
		{ GameButtonType_Step },
		{ GameButtonType_Step },
		{ GameButtonType_Step },
		{ GameButtonType_Step },
		{ GameButtonType_Step },
	},
	TNS_W1,	// m_mapW1To
	TNS_W2,	// m_mapW2To
	TNS_W3,	// m_mapW3To
	TNS_W4,	// m_mapW4To
	TNS_W5,	// m_mapW5To
};

//ThemeMetric<int>	BEAT_COL_SPACING	("ColumnSpacing","Beat");
static const int BEAT_COL_SPACING = 34;
static const Style g_Style_Beat_Single5 =
{	// STYLE_BEAT_SINGLE5
	true,				// m_bUsedForGameplay
	true,				// m_bUsedForEdit
	true,				// m_bUsedForDemonstration
	true,				// m_bUsedForHowToPlay
	"single5",			// m_szName
	StepsType_beat_single5,	// m_StepsType
	StyleType_OnePlayerOneSide,		// m_StyleType
	6,				// m_iColsPerPlayer
	{	// m_ColumnInfo[NUM_PLAYERS][MAX_COLS_PER_PLAYER];
		{	// PLAYER_1
			{ TRACK_1,	-BEAT_COL_SPACING*2.5f, nullptr },
			{ TRACK_2,	-BEAT_COL_SPACING*1.5f, nullptr },
			{ TRACK_3,	-BEAT_COL_SPACING*0.5f, nullptr },
			{ TRACK_4,	+BEAT_COL_SPACING*0.5f, nullptr },
			{ TRACK_5,	+BEAT_COL_SPACING*1.5f, nullptr },
			{ TRACK_6,	+BEAT_COL_SPACING*3.0f, "scratch" },
		},
		{	// PLAYER_2
			{ TRACK_1,	-BEAT_COL_SPACING*2.5f, nullptr },
			{ TRACK_2,	-BEAT_COL_SPACING*1.5f, nullptr },
			{ TRACK_3,	-BEAT_COL_SPACING*0.5f, nullptr },
			{ TRACK_4,	+BEAT_COL_SPACING*0.5f, nullptr },
			{ TRACK_5,	+BEAT_COL_SPACING*1.5f, nullptr },
			{ TRACK_6,	+BEAT_COL_SPACING*3.0f, "scratch" },
		},
	},
	{	// m_iInputColumn[NUM_GameController][NUM_GameButton]
		{ 0, 1, 2, 3, 4, Style::NO_MAPPING, Style::NO_MAPPING, 5, 5, Style::END_MAPPING },
		{ 0, 1, 2, 3, 4, Style::NO_MAPPING, Style::NO_MAPPING, 5, 5, Style::END_MAPPING }
	},
	{	// m_iColumnDrawOrder[MAX_COLS_PER_PLAYER];
		0,1,2,3,4,5
	},
	false, // m_bCanUseBeginnerHelper
	false, // m_bLockDifficulties
};

static const Style g_Style_Beat_Versus5 =
{	// STYLE_BEAT_VERSUS
	true,				// m_bUsedForGameplay
	false,				// m_bUsedForEdit
	true,				// m_bUsedForDemonstration
	false,				// m_bUsedForHowToPlay
	"versus5",			// m_szName
	StepsType_beat_single5,	// m_StepsType
	StyleType_TwoPlayersTwoSides,		// m_StyleType
	6,				// m_iColsPerPlayer
	{	// m_ColumnInfo[NUM_PLAYERS][MAX_COLS_PER_PLAYER];
		{	// PLAYER_1
			{ TRACK_1,	-BEAT_COL_SPACING*2.5f, nullptr },
			{ TRACK_2,	-BEAT_COL_SPACING*1.5f, nullptr },
			{ TRACK_3,	-BEAT_COL_SPACING*0.5f, nullptr },
			{ TRACK_4,	+BEAT_COL_SPACING*0.5f, nullptr },
			{ TRACK_5,	+BEAT_COL_SPACING*1.5f, nullptr },
			{ TRACK_6,	+BEAT_COL_SPACING*3.0f, "scratch" },
		},
		{	// PLAYER_2
			{ TRACK_1,	-BEAT_COL_SPACING*2.5f, nullptr },
			{ TRACK_2,	-BEAT_COL_SPACING*1.5f, nullptr },
			{ TRACK_3,	-BEAT_COL_SPACING*0.5f, nullptr },
			{ TRACK_4,	+BEAT_COL_SPACING*0.5f, nullptr },
			{ TRACK_5,	+BEAT_COL_SPACING*1.5f, nullptr },
			{ TRACK_6,	+BEAT_COL_SPACING*3.0f, "scratch" },
		},
	},
	{	// m_iInputColumn[NUM_GameController][NUM_GameButton]
		{ 0, 1, 2, 3, 4, Style::NO_MAPPING, Style::NO_MAPPING, 5, 5, Style::END_MAPPING },
		{ 0, 1, 2, 3, 4, Style::NO_MAPPING, Style::NO_MAPPING, 5, 5, Style::END_MAPPING }
	},
	{	// m_iColumnDrawOrder[MAX_COLS_PER_PLAYER];
		0,1,2,3,4,5
	},
	false, // m_bCanUseBeginnerHelper
	true, // m_bLockDifficulties
};

static const Style g_Style_Beat_Double5 =
{	// STYLE_BEAT_DOUBLE
	true,				// m_bUsedForGameplay
	true,				// m_bUsedForEdit
	false,				// m_bUsedForDemonstration
	false,				// m_bUsedForHowToPlay
	"double5",			// m_szName
	StepsType_beat_double5,	// m_StepsType
	StyleType_OnePlayerTwoSides,		// m_StyleType
	12,				// m_iColsPerPlayer
	{	// m_ColumnInfo[NUM_PLAYERS][MAX_COLS_PER_PLAYER];
		{	// PLAYER_1
			{ TRACK_1,	-BEAT_COL_SPACING*6.0f, nullptr },
			{ TRACK_2,	-BEAT_COL_SPACING*5.0f, nullptr },
			{ TRACK_3,	-BEAT_COL_SPACING*4.0f, nullptr },
			{ TRACK_4,	-BEAT_COL_SPACING*3.0f, nullptr },
			{ TRACK_5,	-BEAT_COL_SPACING*2.0f, nullptr },
			{ TRACK_6,	-BEAT_COL_SPACING*1.5f, "scratch" },
			{ TRACK_7,	+BEAT_COL_SPACING*0.5f, nullptr },
			{ TRACK_8,	+BEAT_COL_SPACING*1.5f, nullptr },
			{ TRACK_9,	+BEAT_COL_SPACING*2.5f, nullptr },
			{ TRACK_10,	+BEAT_COL_SPACING*3.5f, nullptr },
			{ TRACK_11,	+BEAT_COL_SPACING*4.5f, nullptr },
			{ TRACK_12,	+BEAT_COL_SPACING*6.0f, "scratch" },
		},
		{	// PLAYER_2
			{ TRACK_1,	-BEAT_COL_SPACING*6.0f, nullptr },
			{ TRACK_2,	-BEAT_COL_SPACING*5.0f, nullptr },
			{ TRACK_3,	-BEAT_COL_SPACING*4.0f, nullptr },
			{ TRACK_4,	-BEAT_COL_SPACING*3.0f, nullptr },
			{ TRACK_5,	-BEAT_COL_SPACING*2.0f, nullptr },
			{ TRACK_6,	-BEAT_COL_SPACING*1.5f, "scratch" },
			{ TRACK_7,	+BEAT_COL_SPACING*0.5f, nullptr },
			{ TRACK_8,	+BEAT_COL_SPACING*1.5f, nullptr },
			{ TRACK_9,	+BEAT_COL_SPACING*2.5f, nullptr },
			{ TRACK_10,	+BEAT_COL_SPACING*3.5f, nullptr },
			{ TRACK_11,	+BEAT_COL_SPACING*4.5f, nullptr },
			{ TRACK_12,	+BEAT_COL_SPACING*6.0f, "scratch" },
		},
	},
	{	// m_iInputColumn[NUM_GameController][NUM_GameButton]
		{ 0, 1, 2, 3, 4, Style::NO_MAPPING, Style::NO_MAPPING, 5, 5, Style::END_MAPPING },
		{ 6, 7, 8, 9, 10, Style::NO_MAPPING, Style::NO_MAPPING, 11, 11, Style::END_MAPPING }
	},
	{	// m_iColumnDrawOrder[MAX_COLS_PER_PLAYER];
		0,1,2,3,4,5,6,7,8,9,10,11
	},
	false, // m_bCanUseBeginnerHelper
	false, // m_bLockDifficulties
};

static const Style g_Style_Beat_Single7 =
{	// STYLE_BEAT_SINGLE7
	true,				// m_bUsedForGameplay
	true,				// m_bUsedForEdit
	true,				// m_bUsedForDemonstration
	true,				// m_bUsedForHowToPlay
	"single7",			// m_szName
	StepsType_beat_single7,	// m_StepsType
	StyleType_OnePlayerOneSide,		// m_StyleType
	8,				// m_iColsPerPlayer
	{	// m_ColumnInfo[NUM_PLAYERS][MAX_COLS_PER_PLAYER];
		{	// PLAYER_1
			{ TRACK_8,	-BEAT_COL_SPACING*3.5f, "scratch" },
			{ TRACK_1,	-BEAT_COL_SPACING*2.0f, nullptr },
			{ TRACK_2,	-BEAT_COL_SPACING*1.0f, nullptr },
			{ TRACK_3,	-BEAT_COL_SPACING*0.0f, nullptr },
			{ TRACK_4,	+BEAT_COL_SPACING*1.0f, nullptr },
			{ TRACK_5,	+BEAT_COL_SPACING*2.0f, nullptr },
			{ TRACK_6,	+BEAT_COL_SPACING*3.0f, nullptr },
			{ TRACK_7,	+BEAT_COL_SPACING*4.0f, nullptr },
		},
		{	// PLAYER_2
			{ TRACK_8,	+BEAT_COL_SPACING*4.0f, "scratch" },
			{ TRACK_1,	-BEAT_COL_SPACING*3.5f, nullptr },
			{ TRACK_2,	-BEAT_COL_SPACING*2.5f, nullptr },
			{ TRACK_3,	-BEAT_COL_SPACING*1.5f, nullptr },
			{ TRACK_4,	-BEAT_COL_SPACING*0.5f, nullptr },
			{ TRACK_5,	+BEAT_COL_SPACING*0.5f, nullptr },
			{ TRACK_6,	+BEAT_COL_SPACING*1.5f, nullptr },
			{ TRACK_7,	+BEAT_COL_SPACING*2.5f, nullptr },
		},
	},
	{	// m_iInputColumn[NUM_GameController][NUM_GameButton]
		{ 1, 2, 3, 4, 5, 6, 7, 0, 0, Style::END_MAPPING },
		{ 1, 2, 3, 4, 5, 6, 7, 0, 0, Style::END_MAPPING },
	},
	{	// m_iColumnDrawOrder[MAX_COLS_PER_PLAYER];
		0,1,2,3,4,5,6,7
	},
	false, // m_bCanUseBeginnerHelper
	false, // m_bLockDifficulties
};

static const Style g_Style_Beat_Versus7 =
{	// STYLE_BEAT_VERSUS7
	true,				// m_bUsedForGameplay
	true,				// m_bUsedForEdit
	true,				// m_bUsedForDemonstration
	false,				// m_bUsedForHowToPlay
	"versus7",			// m_szName
	StepsType_beat_single7,	// m_StepsType
	StyleType_TwoPlayersTwoSides,		// m_StyleType
	8,				// m_iColsPerPlayer
	{	// m_ColumnInfo[NUM_PLAYERS][MAX_COLS_PER_PLAYER];
		{	// PLAYER_1
			{ TRACK_8,	-BEAT_COL_SPACING*3.5f, "scratch" },
			{ TRACK_1,	-BEAT_COL_SPACING*2.0f, nullptr },
			{ TRACK_2,	-BEAT_COL_SPACING*1.0f, nullptr },
			{ TRACK_3,	-BEAT_COL_SPACING*0.0f, nullptr },
			{ TRACK_4,	+BEAT_COL_SPACING*1.0f, nullptr },
			{ TRACK_5,	+BEAT_COL_SPACING*2.0f, nullptr },
			{ TRACK_6,	+BEAT_COL_SPACING*3.0f, nullptr },
			{ TRACK_7,	+BEAT_COL_SPACING*4.0f, nullptr },
		},
		{	// PLAYER_2
			{ TRACK_8,	+BEAT_COL_SPACING*4.0f, "scratch" },
			{ TRACK_1,	-BEAT_COL_SPACING*3.5f, nullptr },
			{ TRACK_2,	-BEAT_COL_SPACING*2.5f, nullptr },
			{ TRACK_3,	-BEAT_COL_SPACING*1.5f, nullptr },
			{ TRACK_4,	-BEAT_COL_SPACING*0.5f, nullptr },
			{ TRACK_5,	+BEAT_COL_SPACING*0.5f, nullptr },
			{ TRACK_6,	+BEAT_COL_SPACING*1.5f, nullptr },
			{ TRACK_7,	+BEAT_COL_SPACING*2.5f, nullptr },
		},
	},
	{	// m_iInputColumn[NUM_GameController][NUM_GameButton]
		{ 1, 2, 3, 4, 5, 6, 7, 0, 0, Style::END_MAPPING },
		{ 1, 2, 3, 4, 5, 6, 7, 0, 0, Style::END_MAPPING },
	},
	{	// m_iColumnDrawOrder[MAX_COLS_PER_PLAYER];
		0,1,2,3,4,5,6,7
	},
	false, // m_bCanUseBeginnerHelper
	true, // m_bLockDifficulties
};


static const Style g_Style_Beat_Double7 =
{	// STYLE_BEAT_DOUBLE7
	true,				// m_bUsedForGameplay
	true,				// m_bUsedForEdit
	false,				// m_bUsedForDemonstration
	false,				// m_bUsedForHowToPlay
	"double7",			// m_szName
	StepsType_beat_double7,	// m_StepsType
	StyleType_OnePlayerTwoSides,		// m_StyleType
	16,				// m_iColsPerPlayer
	{	// m_ColumnInfo[NUM_PLAYERS][MAX_COLS_PER_PLAYER];
		{	// PLAYER_1
			{ TRACK_8,	-BEAT_COL_SPACING*8.0f, "scratch" },
			{ TRACK_1,	-BEAT_COL_SPACING*6.5f, nullptr },
			{ TRACK_2,	-BEAT_COL_SPACING*5.5f, nullptr },
			{ TRACK_3,	-BEAT_COL_SPACING*4.5f, nullptr },
			{ TRACK_4,	-BEAT_COL_SPACING*3.5f, nullptr },
			{ TRACK_5,	-BEAT_COL_SPACING*2.5f, nullptr },
			{ TRACK_6,	-BEAT_COL_SPACING*1.5f, nullptr },
			{ TRACK_7,	-BEAT_COL_SPACING*0.5f, nullptr },
			{ TRACK_9,	+BEAT_COL_SPACING*0.5f, nullptr },
			{ TRACK_10,	+BEAT_COL_SPACING*1.5f, nullptr },
			{ TRACK_11,	+BEAT_COL_SPACING*2.5f, nullptr },
			{ TRACK_12,	+BEAT_COL_SPACING*3.5f, nullptr },
			{ TRACK_13,	+BEAT_COL_SPACING*4.5f, nullptr },
			{ TRACK_14,	+BEAT_COL_SPACING*5.5f, nullptr },
			{ TRACK_15,	+BEAT_COL_SPACING*6.5f, nullptr },
			{ TRACK_16,	+BEAT_COL_SPACING*8.0f, "scratch" },
		},
		{	// PLAYER_2
			{ TRACK_8,	-BEAT_COL_SPACING*8.0f, "scratch" },
			{ TRACK_1,	-BEAT_COL_SPACING*6.5f, nullptr },
			{ TRACK_2,	-BEAT_COL_SPACING*5.5f, nullptr },
			{ TRACK_3,	-BEAT_COL_SPACING*4.5f, nullptr },
			{ TRACK_4,	-BEAT_COL_SPACING*3.5f, nullptr },
			{ TRACK_5,	-BEAT_COL_SPACING*2.5f, nullptr },
			{ TRACK_6,	-BEAT_COL_SPACING*1.5f, nullptr },
			{ TRACK_7,	-BEAT_COL_SPACING*0.5f, nullptr },
			{ TRACK_9,	+BEAT_COL_SPACING*0.5f, nullptr },
			{ TRACK_10,	+BEAT_COL_SPACING*1.5f, nullptr },
			{ TRACK_11,	+BEAT_COL_SPACING*2.5f, nullptr },
			{ TRACK_12,	+BEAT_COL_SPACING*3.5f, nullptr },
			{ TRACK_13,	+BEAT_COL_SPACING*4.5f, nullptr },
			{ TRACK_14,	+BEAT_COL_SPACING*5.5f, nullptr },
			{ TRACK_15,	+BEAT_COL_SPACING*6.5f, nullptr },
			{ TRACK_16,	+BEAT_COL_SPACING*8.0f, "scratch" },
		},
	},
	{	// m_iInputColumn[NUM_GameController][NUM_GameButton]
		{ 1, 2, 3, 4, 5, 6, 7, 0, 0, Style::END_MAPPING },
		{ 8, 9, 10, 11, 12, 13, 14, 15, 15, Style::END_MAPPING },
	},
	{	// m_iColumnDrawOrder[MAX_COLS_PER_PLAYER];
		0,1,2,3,4,5,6,7,8,9,10,11,12,13,14,15
	},
	false, // m_bCanUseBeginnerHelper
	false, // m_bLockDifficulties
};

static const Style *g_apGame_Beat_Styles[] =
{
	&g_Style_Beat_Single5,
	&g_Style_Beat_Versus5,
	&g_Style_Beat_Double5,
	&g_Style_Beat_Single7,
	&g_Style_Beat_Versus7,
	&g_Style_Beat_Double7,
	nullptr
};

static const AutoMappings g_AutoKeyMappings_Beat = AutoMappings (
	"",
	"",
	"",
	AutoMappingEntry( 0, KEY_Cm,	BEAT_BUTTON_KEY1,		false ),
	AutoMappingEntry( 0, KEY_Ck,	BEAT_BUTTON_KEY2,		false ),
	AutoMappingEntry( 0, KEY_COMMA,	BEAT_BUTTON_KEY3,		false ),
	AutoMappingEntry( 0, KEY_Cl,	BEAT_BUTTON_KEY4,		false ),
	AutoMappingEntry( 0, KEY_PERIOD,	BEAT_BUTTON_KEY5,		false ),
	AutoMappingEntry( 0, KEY_SEMICOLON,	BEAT_BUTTON_KEY6,		false ),
	AutoMappingEntry( 0, KEY_SLASH,	BEAT_BUTTON_KEY7,		false ),
	AutoMappingEntry( 0, KEY_SPACE,	BEAT_BUTTON_SCRATCHUP,		false )
);

static const Game g_Game_Beat = 
{
	"beat",						// m_szName
	g_apGame_Beat_Styles,				// m_apStyles
	true,						// m_bCountNotesSeparately
	false, // m_bTickHolds
	false, // m_PlayersHaveSeparateStyles
	{						// m_InputScheme
		"beat",					// m_szName
		NUM_BEAT_BUTTONS,			// m_iButtonsPerController
		{	// m_szButtonNames
			{ "Key1",		GAME_BUTTON_LEFT },
			{ "Key2",		GameButton_Invalid },
			{ "Key3",		GameButton_Invalid },
			{ "Key4",		GameButton_Invalid },
			{ "Key5",		GameButton_Invalid },
			{ "Key6",		GameButton_Invalid },
			{ "Key7",		GAME_BUTTON_RIGHT },
			{ "Scratch up",		GAME_BUTTON_UP },
			{ "Scratch down",	GAME_BUTTON_DOWN },
		},
		&g_AutoKeyMappings_Beat
	},
	{
		{ GameButtonType_Step },
		{ GameButtonType_Step },
		{ GameButtonType_Step },
		{ GameButtonType_Step },
		{ GameButtonType_Step },
		{ GameButtonType_Step },
		{ GameButtonType_Step },
		{ GameButtonType_Step },
		{ GameButtonType_Step },
	},
	TNS_W1,	// m_mapW1To
	TNS_W2,	// m_mapW2To
	TNS_W3,	// m_mapW3To
	TNS_W4,	// m_mapW4To
	TNS_W5,	// m_mapW5To
};

//ThemeMetric<int>	MANIAX_COL_SPACING	("ColumnSpacing","Maniax");
static const int MANIAX_COL_SPACING = 36;
static const Style g_Style_Maniax_Single =
{	// STYLE_MANIAX_SINGLE
	true,				// m_bUsedForGameplay
	true,				// m_bUsedForEdit
	true,				// m_bUsedForDemonstration
	true,				// m_bUsedForHowToPlay
	"single",			// m_szName
	StepsType_maniax_single,	// m_StepsType
	StyleType_OnePlayerOneSide,		// m_StyleType
	4,				// m_iColsPerPlayer
	{	// m_ColumnInfo[NUM_PLAYERS][MAX_COLS_PER_PLAYER];
		{	// PLAYER_1
			{ TRACK_1,	-MANIAX_COL_SPACING*1.5f, nullptr },
			{ TRACK_2,	-MANIAX_COL_SPACING*0.5f, nullptr },
			{ TRACK_3,	+MANIAX_COL_SPACING*0.5f, nullptr },
			{ TRACK_4,	+MANIAX_COL_SPACING*1.5f, nullptr },
		},
		{	// PLAYER_2
			{ TRACK_1,	-MANIAX_COL_SPACING*1.5f, nullptr },
			{ TRACK_2,	-MANIAX_COL_SPACING*0.5f, nullptr },
			{ TRACK_3,	+MANIAX_COL_SPACING*0.5f, nullptr },
			{ TRACK_4,	+MANIAX_COL_SPACING*1.5f, nullptr },
		},
	},
	{	// m_iInputColumn[NUM_GameController][NUM_GameButton]
		{ 1, 2, 0, 3, Style::END_MAPPING },
		{ 1, 2, 0, 3, Style::END_MAPPING },
	},
	{	// m_iColumnDrawOrder[MAX_COLS_PER_PLAYER];
		0, 1, 2, 3
	},
	false, // m_bCanUseBeginnerHelper
	false, // m_bLockDifficulties
};

static const Style g_Style_Maniax_Versus =
{	// STYLE_MANIAX_VERSUS
	true,				// m_bUsedForGameplay
	false,				// m_bUsedForEdit
	false,				// m_bUsedForDemonstration
	false,				// m_bUsedForHowToPlay
	"versus",			// m_szName
	StepsType_maniax_single,	// m_StepsType
	StyleType_TwoPlayersTwoSides,		// m_StyleType
	4,				// m_iColsPerPlayer
	{	// m_ColumnInfo[NUM_PLAYERS][MAX_COLS_PER_PLAYER];
		{	// PLAYER_1
			{ TRACK_1,	-MANIAX_COL_SPACING*1.5f, nullptr },
			{ TRACK_2,	-MANIAX_COL_SPACING*0.5f, nullptr },
			{ TRACK_3,	+MANIAX_COL_SPACING*0.5f, nullptr },
			{ TRACK_4,	+MANIAX_COL_SPACING*1.5f, nullptr },
		},
		{	// PLAYER_2
			{ TRACK_1,	-MANIAX_COL_SPACING*1.5f, nullptr },
			{ TRACK_2,	-MANIAX_COL_SPACING*0.5f, nullptr },
			{ TRACK_3,	+MANIAX_COL_SPACING*0.5f, nullptr },
			{ TRACK_4,	+MANIAX_COL_SPACING*1.5f, nullptr },
		},
	},
	{	// m_iInputColumn[NUM_GameController][NUM_GameButton]
		{ 1, 2, 0, 3, Style::END_MAPPING },
		{ 1, 2, 0, 3, Style::END_MAPPING },
	},
	{	// m_iColumnDrawOrder[MAX_COLS_PER_PLAYER];
		0, 1, 2, 3
	},
	false, // m_bCanUseBeginnerHelper
	false, // m_bLockDifficulties
};

static const Style g_Style_Maniax_Double =
{	// STYLE_MANIAX_DOUBLE
	true,				// m_bUsedForGameplay
	true,				// m_bUsedForEdit
	false,				// m_bUsedForDemonstration
	false,				// m_bUsedForHowToPlay
	"double",			// m_szName
	StepsType_maniax_double,	// m_StepsType
	StyleType_OnePlayerTwoSides,		// m_StyleType
	8,				// m_iColsPerPlayer
	{	// m_ColumnInfo[NUM_PLAYERS][MAX_COLS_PER_PLAYER];
		{	// PLAYER_1
			{ TRACK_1,	-MANIAX_COL_SPACING*3.5f, nullptr },
			{ TRACK_2,	-MANIAX_COL_SPACING*2.5f, nullptr },
			{ TRACK_3,	-MANIAX_COL_SPACING*1.5f, nullptr },
			{ TRACK_4,	-MANIAX_COL_SPACING*0.5f, nullptr },
			{ TRACK_5,	+MANIAX_COL_SPACING*0.5f, nullptr },
			{ TRACK_6,	+MANIAX_COL_SPACING*1.5f, nullptr },
			{ TRACK_7,	+MANIAX_COL_SPACING*2.5f, nullptr },
			{ TRACK_8,	+MANIAX_COL_SPACING*3.5f, nullptr },
		},
		{	// PLAYER_2
			{ TRACK_1,	-MANIAX_COL_SPACING*3.5f, nullptr },
			{ TRACK_2,	-MANIAX_COL_SPACING*2.5f, nullptr },
			{ TRACK_3,	-MANIAX_COL_SPACING*1.5f, nullptr },
			{ TRACK_4,	-MANIAX_COL_SPACING*0.5f, nullptr },
			{ TRACK_5,	+MANIAX_COL_SPACING*0.5f, nullptr },
			{ TRACK_6,	+MANIAX_COL_SPACING*1.5f, nullptr },
			{ TRACK_7,	+MANIAX_COL_SPACING*2.5f, nullptr },
			{ TRACK_8,	+MANIAX_COL_SPACING*3.5f, nullptr },
		},
	},
	{	// m_iInputColumn[NUM_GameController][NUM_GameButton]
		{ 1, 2, 0, 3, Style::END_MAPPING },
		{ 5, 6, 4, 7, Style::END_MAPPING },
	},
	{	// m_iColumnDrawOrder[MAX_COLS_PER_PLAYER];
		0,1,2,3,4,5,6,7
	},
	false, // m_bCanUseBeginnerHelper
	false, // m_bLockDifficulties
};

static const Style *g_apGame_Maniax_Styles[] =
{
	&g_Style_Maniax_Single,
	&g_Style_Maniax_Versus,
	&g_Style_Maniax_Double,
	nullptr
};

static const AutoMappings g_AutoKeyMappings_Maniax = AutoMappings (
	"",
	"",
	"",
	AutoMappingEntry( 0, KEY_Ca,		MANIAX_BUTTON_HANDUPLEFT,	false ),
	AutoMappingEntry( 0, KEY_Cs,		MANIAX_BUTTON_HANDUPRIGHT,	false ),
	AutoMappingEntry( 0, KEY_Cz,		MANIAX_BUTTON_HANDLRLEFT,	false ),
	AutoMappingEntry( 0, KEY_Cx,		MANIAX_BUTTON_HANDLRRIGHT,	false ),
	AutoMappingEntry( 0, KEY_KP_C4,		MANIAX_BUTTON_HANDUPLEFT,	true ),
	AutoMappingEntry( 0, KEY_KP_C5,		MANIAX_BUTTON_HANDUPRIGHT,	true ),
	AutoMappingEntry( 0, KEY_KP_C1,		MANIAX_BUTTON_HANDLRLEFT,	true ),
	AutoMappingEntry( 0, KEY_KP_C2,		MANIAX_BUTTON_HANDLRRIGHT,	true )
);

static const Game g_Game_Maniax = 
{
	"maniax",					// m_szName
	g_apGame_Maniax_Styles,				// m_apStyles
	false,						// m_bCountNotesSeparately
	false, // m_bTickHolds
	false, // m_PlayersHaveSeparateStyles
	{						// m_InputScheme
		"maniax",				// m_szName
		NUM_MANIAX_BUTTONS,			// m_iButtonsPerController
		{	// m_szButtonNames
			{ "HandUpLeft",		GAME_BUTTON_LEFT },
			{ "HandUpRight",	GAME_BUTTON_RIGHT },
			{ "HandLrLeft",		GAME_BUTTON_DOWN },
			{ "HandLrRight",	GAME_BUTTON_UP },
		},
		&g_AutoKeyMappings_Maniax
	},
	{
		{ GameButtonType_Step },
		{ GameButtonType_Step },
		{ GameButtonType_Step },
		{ GameButtonType_Step },
	},
	TNS_W1,	// m_mapW1To
	TNS_W2,	// m_mapW2To
	TNS_W3,	// m_mapW3To
	TNS_W4,	// m_mapW4To
	TNS_W5,	// m_mapW5To
};

/** Techno *******************************************************************/

//ThemeMetric<int>	TECHNO_COL_SPACING	("ColumnSpacing","Techno");
static const int TECHNO_COL_SPACING = 56;
//ThemeMetric<int>	TECHNO_VERSUS_COL_SPACING	("ColumnSpacing","TechnoVersus");
static const Style g_Style_Techno_Single4 =
{	// STYLE_TECHNO_SINGLE4
	true,				// m_bUsedForGameplay
	true,				// m_bUsedForEdit
	false,				// m_bUsedForDemonstration
	false,				// m_bUsedForHowToPlay
	"single4",			// m_szName
	StepsType_techno_single4,	// m_StepsType
	StyleType_OnePlayerOneSide,		// m_StyleType
	4,				// m_iColsPerPlayer
	{	// m_ColumnInfo[NUM_PLAYERS][MAX_COLS_PER_PLAYER];
		{	// PLAYER_1
			{ TRACK_1,	-TECHNO_COL_SPACING*1.5f, nullptr },
			{ TRACK_2,	-TECHNO_COL_SPACING*0.5f, nullptr },
			{ TRACK_3,	+TECHNO_COL_SPACING*0.5f, nullptr },
			{ TRACK_4,	+TECHNO_COL_SPACING*1.5f, nullptr },
		},
		{	// PLAYER_2
			{ TRACK_1,	-TECHNO_COL_SPACING*1.5f, nullptr },
			{ TRACK_2,	-TECHNO_COL_SPACING*0.5f, nullptr },
			{ TRACK_3,	+TECHNO_COL_SPACING*0.5f, nullptr },
			{ TRACK_4,	+TECHNO_COL_SPACING*1.5f, nullptr },
		},
	},
	{	// m_iInputColumn[NUM_GameController][NUM_GameButton]
		{ 0, 3, 2, 1, Style::END_MAPPING },
		{ 0, 3, 2, 1, Style::END_MAPPING },
	},
	{	// m_iColumnDrawOrder[MAX_COLS_PER_PLAYER];
		0,1,2,3
	},
	false, // m_bCanUseBeginnerHelper
	false, // m_bLockDifficulties
};

static const Style g_Style_Techno_Single5 =
{	// STYLE_TECHNO_SINGLE5
	true,				// m_bUsedForGameplay
	true,				// m_bUsedForEdit
	false,				// m_bUsedForDemonstration
	false,				// m_bUsedForHowToPlay
	"single5",			// m_szName
	StepsType_techno_single5,	// m_StepsType
	StyleType_OnePlayerOneSide,		// m_StyleType
	5,				// m_iColsPerPlayer
	{	// m_ColumnInfo[NUM_PLAYERS][MAX_COLS_PER_PLAYER];
		{	// PLAYER_1
			{ TRACK_1,	-TECHNO_COL_SPACING*2.0f, nullptr },
			{ TRACK_2,	-TECHNO_COL_SPACING*1.0f, nullptr },
			{ TRACK_3,	+TECHNO_COL_SPACING*0.0f, nullptr },
			{ TRACK_4,	+TECHNO_COL_SPACING*1.0f, nullptr },
			{ TRACK_5,	+TECHNO_COL_SPACING*2.0f, nullptr },
		},
		{	// PLAYER_2
			{ TRACK_1,	-TECHNO_COL_SPACING*2.0f, nullptr },
			{ TRACK_2,	-TECHNO_COL_SPACING*1.0f, nullptr },
			{ TRACK_3,	+TECHNO_COL_SPACING*0.0f, nullptr },
			{ TRACK_4,	+TECHNO_COL_SPACING*1.0f, nullptr },
			{ TRACK_5,	+TECHNO_COL_SPACING*2.0f, nullptr },
		},
	},
	{	// m_iInputColumn[NUM_GameController][NUM_GameButton]
		{ Style::NO_MAPPING, Style::NO_MAPPING, Style::NO_MAPPING, Style::NO_MAPPING,
			1, 3, 2, 0, 4, Style::END_MAPPING },
		{ Style::NO_MAPPING, Style::NO_MAPPING, Style::NO_MAPPING, Style::NO_MAPPING,
			1, 3, 2, 0, 4, Style::END_MAPPING },
	},
	{	// m_iColumnDrawOrder[MAX_COLS_PER_PLAYER];
		0,1,2,3,4
	},
	false, // m_bCanUseBeginnerHelper
	false, // m_bLockDifficulties
};

static const Style g_Style_Techno_Single8 =
{	// STYLE_TECHNO_SINGLE8
	true,				// m_bUsedForGameplay
	true,				// m_bUsedForEdit
	false,				// m_bUsedForDemonstration
	true,				// m_bUsedForHowToPlay
	"single8",			// m_szName
	StepsType_techno_single8,	// m_StepsType
	StyleType_OnePlayerOneSide,		// m_StyleType
	8,				// m_iColsPerPlayer
	{	// m_ColumnInfo[NUM_PLAYERS][MAX_COLS_PER_PLAYER];
		{	// PLAYER_1
			{ TRACK_1,	-TECHNO_COL_SPACING*3.5f, nullptr },
			{ TRACK_2,	-TECHNO_COL_SPACING*2.5f, nullptr },
			{ TRACK_3,	-TECHNO_COL_SPACING*1.5f, nullptr },
			{ TRACK_4,	-TECHNO_COL_SPACING*0.5f, nullptr },
			{ TRACK_5,	+TECHNO_COL_SPACING*0.5f, nullptr },
			{ TRACK_6,	+TECHNO_COL_SPACING*1.5f, nullptr },
			{ TRACK_7,	+TECHNO_COL_SPACING*2.5f, nullptr },
			{ TRACK_8,	+TECHNO_COL_SPACING*3.5f, nullptr },
		},
		{	// PLAYER_2
			{ TRACK_1,	-TECHNO_COL_SPACING*3.5f, nullptr },
			{ TRACK_2,	-TECHNO_COL_SPACING*2.5f, nullptr },
			{ TRACK_3,	-TECHNO_COL_SPACING*1.5f, nullptr },
			{ TRACK_4,	-TECHNO_COL_SPACING*0.5f, nullptr },
			{ TRACK_5,	+TECHNO_COL_SPACING*0.5f, nullptr },
			{ TRACK_6,	+TECHNO_COL_SPACING*1.5f, nullptr },
			{ TRACK_7,	+TECHNO_COL_SPACING*2.5f, nullptr },
			{ TRACK_8,	+TECHNO_COL_SPACING*3.5f, nullptr },
		},
	},
	{	// m_iInputColumn[NUM_GameController][NUM_GameButton]
		{ 1, 6, 4, 3, 2, 5, Style::NO_MAPPING, 0, 7, Style::END_MAPPING },
		{ 1, 6, 4, 3, 2, 5, Style::NO_MAPPING, 0, 7, Style::END_MAPPING },
	},
	{	// m_iColumnDrawOrder[MAX_COLS_PER_PLAYER];
		0,1,2,3,4,5,6,7
	},
	false, // m_bCanUseBeginnerHelper
	false, // m_bLockDifficulties
};

static const Style g_Style_Techno_Versus4 =
{	// STYLE_TECHNO_VERSUS4
	true,				// m_bUsedForGameplay
	false,				// m_bUsedForEdit
	false,				// m_bUsedForDemonstration
	false,				// m_bUsedForHowToPlay
	"versus4",			// m_szName
	StepsType_techno_single4,	// m_StepsType
	StyleType_TwoPlayersTwoSides,		// m_StyleType
	4,				// m_iColsPerPlayer
	{	// m_ColumnInfo[NUM_PLAYERS][MAX_COLS_PER_PLAYER];
		{	// PLAYER_1
			{ TRACK_1,	-TECHNO_COL_SPACING*1.5f, nullptr },
			{ TRACK_2,	-TECHNO_COL_SPACING*0.5f, nullptr },
			{ TRACK_3,	+TECHNO_COL_SPACING*0.5f, nullptr },
			{ TRACK_4,	+TECHNO_COL_SPACING*1.5f, nullptr },
		},
		{	// PLAYER_2
			{ TRACK_1,	-TECHNO_COL_SPACING*1.5f, nullptr },
			{ TRACK_2,	-TECHNO_COL_SPACING*0.5f, nullptr },
			{ TRACK_3,	+TECHNO_COL_SPACING*0.5f, nullptr },
			{ TRACK_4,	+TECHNO_COL_SPACING*1.5f, nullptr },
		},
	},
	{	// m_iInputColumn[NUM_GameController][NUM_GameButton]
		{ 0, 3, 2, 1, Style::END_MAPPING },
		{ 0, 3, 2, 1, Style::END_MAPPING },
	},
	{	// m_iColumnDrawOrder[MAX_COLS_PER_PLAYER];
		0,1,2,3
	},
	false, // m_bCanUseBeginnerHelper
	false, // m_bLockDifficulties
};

static const Style g_Style_Techno_Versus5 =
{	// STYLE_TECHNO_VERSUS5
	true,				// m_bUsedForGameplay
	false,				// m_bUsedForEdit
	false,				// m_bUsedForDemonstration
	false,				// m_bUsedForHowToPlay
	"versus5",			// m_szName
	StepsType_techno_single5,	// m_StepsType
	StyleType_TwoPlayersTwoSides,		// m_StyleType
	5,				// m_iColsPerPlayer
	{	// m_ColumnInfo[NUM_PLAYERS][MAX_COLS_PER_PLAYER];
		{	// PLAYER_1
			{ TRACK_1,	-TECHNO_COL_SPACING*2.0f, nullptr },
			{ TRACK_2,	-TECHNO_COL_SPACING*1.0f, nullptr },
			{ TRACK_3,	+TECHNO_COL_SPACING*0.0f, nullptr },
			{ TRACK_4,	+TECHNO_COL_SPACING*1.0f, nullptr },
			{ TRACK_5,	+TECHNO_COL_SPACING*2.0f, nullptr },
		},
		{	// PLAYER_2
			{ TRACK_1,	-TECHNO_COL_SPACING*2.0f, nullptr },
			{ TRACK_2,	-TECHNO_COL_SPACING*1.0f, nullptr },
			{ TRACK_3,	+TECHNO_COL_SPACING*0.0f, nullptr },
			{ TRACK_4,	+TECHNO_COL_SPACING*1.0f, nullptr },
			{ TRACK_5,	+TECHNO_COL_SPACING*2.0f, nullptr },
		},
	},
	{	// m_iInputColumn[NUM_GameController][NUM_GameButton]
		{ Style::NO_MAPPING, Style::NO_MAPPING, Style::NO_MAPPING, Style::NO_MAPPING,
			1, 3, 2, 0, 4, Style::END_MAPPING },
		{ Style::NO_MAPPING, Style::NO_MAPPING, Style::NO_MAPPING, Style::NO_MAPPING,
			1, 3, 2, 0, 4, Style::END_MAPPING },
	},
	{	// m_iColumnDrawOrder[MAX_COLS_PER_PLAYER];
		0,1,2,3,4
	},
	false, // m_bCanUseBeginnerHelper
	false, // m_bLockDifficulties
};

static const Style g_Style_Techno_Versus8 =
{	// STYLE_TECHNO_VERSUS8
	true,				// m_bUsedForGameplay
	false,				// m_bUsedForEdit
	true,				// m_bUsedForDemonstration
	false,				// m_bUsedForHowToPlay
	"versus8",			// m_szName
	StepsType_techno_single8,	// m_StepsType
	StyleType_TwoPlayersTwoSides,		// m_StyleType
	8,				// m_iColsPerPlayer
	{	// m_ColumnInfo[NUM_PLAYERS][MAX_COLS_PER_PLAYER];
		{	// PLAYER_1
			{ TRACK_1,	-TECHNO_COL_SPACING*3.5f, nullptr },
			{ TRACK_2,	-TECHNO_COL_SPACING*2.5f, nullptr },
			{ TRACK_3,	-TECHNO_COL_SPACING*1.5f, nullptr },
			{ TRACK_4,	-TECHNO_COL_SPACING*0.5f, nullptr },
			{ TRACK_5,	+TECHNO_COL_SPACING*0.5f, nullptr },
			{ TRACK_6,	+TECHNO_COL_SPACING*1.5f, nullptr },
			{ TRACK_7,	+TECHNO_COL_SPACING*2.5f, nullptr },
			{ TRACK_8,	+TECHNO_COL_SPACING*3.5f, nullptr },
		},
		{	// PLAYER_2
			{ TRACK_1,	-TECHNO_COL_SPACING*3.5f, nullptr },
			{ TRACK_2,	-TECHNO_COL_SPACING*2.5f, nullptr },
			{ TRACK_3,	-TECHNO_COL_SPACING*1.5f, nullptr },
			{ TRACK_4,	-TECHNO_COL_SPACING*0.5f, nullptr },
			{ TRACK_5,	+TECHNO_COL_SPACING*0.5f, nullptr },
			{ TRACK_6,	+TECHNO_COL_SPACING*1.5f, nullptr },
			{ TRACK_7,	+TECHNO_COL_SPACING*2.5f, nullptr },
			{ TRACK_8,	+TECHNO_COL_SPACING*3.5f, nullptr },
		},
	},
	{	// m_iInputColumn[NUM_GameController][NUM_GameButton]
		{ 1, 6, 4, 3, 2, 5, Style::NO_MAPPING, 0, 7, Style::END_MAPPING },
		{ 1, 6, 4, 3, 2, 5, Style::NO_MAPPING, 0, 7, Style::END_MAPPING },
//			{ 1, 6, 4, Style::NO_MAPPING, 2, 5, Style::NO_MAPPING, 0, 7, Style::END_MAPPING },
//			{ 1, 6, 4, Style::NO_MAPPING, 2, 5, Style::NO_MAPPING, 0, 7, Style::END_MAPPING },
	},
	{	// m_iColumnDrawOrder[MAX_COLS_PER_PLAYER];
		0,1,2,3,4,5,6,7
	},
	false, // m_bCanUseBeginnerHelper
	false, // m_bLockDifficulties
};

static const Style g_Style_Techno_Double4 =
{	// STYLE_TECHNO_DOUBLE4
	true,				// m_bUsedForGameplay
	true,				// m_bUsedForEdit
	false,				// m_bUsedForDemonstration
	false,				// m_bUsedForHowToPlay
	"double4",			// m_szName
	StepsType_techno_double4,	// m_StepsType
	StyleType_OnePlayerTwoSides,		// m_StyleType
	8,				// m_iColsPerPlayer
	{	// m_ColumnInfo[NUM_PLAYERS][MAX_COLS_PER_PLAYER];
		{	// PLAYER_1
			{ TRACK_1,	-TECHNO_COL_SPACING*3.5f, nullptr },
			{ TRACK_2,	-TECHNO_COL_SPACING*2.5f, nullptr },
			{ TRACK_3,	-TECHNO_COL_SPACING*1.5f, nullptr },
			{ TRACK_4,	-TECHNO_COL_SPACING*0.5f, nullptr },
			{ TRACK_5,	+TECHNO_COL_SPACING*0.5f, nullptr },
			{ TRACK_6,	+TECHNO_COL_SPACING*1.5f, nullptr },
			{ TRACK_7,	+TECHNO_COL_SPACING*2.5f, nullptr },
			{ TRACK_8,	+TECHNO_COL_SPACING*3.5f, nullptr },
		},
		{	// PLAYER_2
			{ TRACK_1,	-TECHNO_COL_SPACING*3.5f, nullptr },
			{ TRACK_2,	-TECHNO_COL_SPACING*2.5f, nullptr },
			{ TRACK_3,	-TECHNO_COL_SPACING*1.5f, nullptr },
			{ TRACK_4,	-TECHNO_COL_SPACING*0.5f, nullptr },
			{ TRACK_5,	+TECHNO_COL_SPACING*0.5f, nullptr },
			{ TRACK_6,	+TECHNO_COL_SPACING*1.5f, nullptr },
			{ TRACK_7,	+TECHNO_COL_SPACING*2.5f, nullptr },
			{ TRACK_8,	+TECHNO_COL_SPACING*3.5f, nullptr },
		},
	},
	{	// m_iInputColumn[NUM_GameController][NUM_GameButton]
		{ 0, 3, 2, 1, Style::END_MAPPING },
		{ 4, 7, 6, 5, Style::END_MAPPING },
	},
	{	// m_iColumnDrawOrder[MAX_COLS_PER_PLAYER];
		0,1,2,3,4,5,6,7
	},
	false, // m_bCanUseBeginnerHelper
	false, // m_bLockDifficulties
};

static const Style g_Style_Techno_Double5 =
{	// STYLE_TECHNO_DOUBLE5
	true,				// m_bUsedForGameplay
	true,				// m_bUsedForEdit
	false,				// m_bUsedForDemonstration
	false,				// m_bUsedForHowToPlay
	"double5",			// m_szName
	StepsType_techno_double5,	// m_StepsType
	StyleType_OnePlayerTwoSides,		// m_StyleType
	10,				// m_iColsPerPlayer
	{	// m_ColumnInfo[NUM_PLAYERS][MAX_COLS_PER_PLAYER];
		{	// PLAYER_1
			{ TRACK_1,	-TECHNO_COL_SPACING*4.5f, nullptr },
			{ TRACK_2,	-TECHNO_COL_SPACING*3.5f, nullptr },
			{ TRACK_3,	-TECHNO_COL_SPACING*2.5f, nullptr },
			{ TRACK_4,	-TECHNO_COL_SPACING*1.5f, nullptr },
			{ TRACK_5,	-TECHNO_COL_SPACING*0.5f, nullptr },
			{ TRACK_6,	+TECHNO_COL_SPACING*0.5f, nullptr },
			{ TRACK_7,	+TECHNO_COL_SPACING*1.5f, nullptr },
			{ TRACK_8,	+TECHNO_COL_SPACING*2.5f, nullptr },
			{ TRACK_9,	+TECHNO_COL_SPACING*3.5f, nullptr },
			{ TRACK_10,	+TECHNO_COL_SPACING*4.5f, nullptr },
		},
		{	// PLAYER_2
			{ TRACK_1,	-TECHNO_COL_SPACING*4.5f, nullptr },
			{ TRACK_2,	-TECHNO_COL_SPACING*3.5f, nullptr },
			{ TRACK_3,	-TECHNO_COL_SPACING*2.5f, nullptr },
			{ TRACK_4,	-TECHNO_COL_SPACING*1.5f, nullptr },
			{ TRACK_5,	-TECHNO_COL_SPACING*0.5f, nullptr },
			{ TRACK_6,	+TECHNO_COL_SPACING*0.5f, nullptr },
			{ TRACK_7,	+TECHNO_COL_SPACING*1.5f, nullptr },
			{ TRACK_8,	+TECHNO_COL_SPACING*2.5f, nullptr },
			{ TRACK_9,	+TECHNO_COL_SPACING*3.5f, nullptr },
			{ TRACK_10,	+TECHNO_COL_SPACING*4.5f, nullptr },
		},
	},
	{	// m_iInputColumn[NUM_GameController][NUM_GameButton]
		{
			Style::NO_MAPPING,
			Style::NO_MAPPING,
			Style::NO_MAPPING,
			Style::NO_MAPPING,
			1, 3, 2, 0, 4, Style::END_MAPPING
		},
		{
			Style::NO_MAPPING,
			Style::NO_MAPPING,
			Style::NO_MAPPING,
			Style::NO_MAPPING,
			6, 8, 7, 5, 9, Style::END_MAPPING
		},
	},
	{	// m_iColumnDrawOrder[MAX_COLS_PER_PLAYER];
		0,1,2,3,4,5,6,7,8,9
	},
	false, // m_bCanUseBeginnerHelper
	false, // m_bLockDifficulties
};

static const Style g_Style_Techno_Double8 =
{	// STYLE_TECHNO_DOUBLE8
	true,				// m_bUsedForGameplay
	true,				// m_bUsedForEdit
	false,				// m_bUsedForDemonstration
	false,				// m_bUsedForHowToPlay
	"double8",			// m_szName
	StepsType_techno_double8,	// m_StepsType
	StyleType_OnePlayerTwoSides,		// m_StyleType
	16,				// m_iColsPerPlayer
	{	// m_ColumnInfo[NUM_PLAYERS][MAX_COLS_PER_PLAYER];
		{	// PLAYER_1
			{ TRACK_1,	-TECHNO_COL_SPACING*7.5f, nullptr },
			{ TRACK_2,	-TECHNO_COL_SPACING*6.5f, nullptr },
			{ TRACK_3,	-TECHNO_COL_SPACING*5.5f, nullptr },
			{ TRACK_4,	-TECHNO_COL_SPACING*4.5f, nullptr },
			{ TRACK_5,	-TECHNO_COL_SPACING*3.5f, nullptr },
			{ TRACK_6,	-TECHNO_COL_SPACING*2.5f, nullptr },
			{ TRACK_7,	-TECHNO_COL_SPACING*1.5f, nullptr },
			{ TRACK_8,	-TECHNO_COL_SPACING*0.5f, nullptr },
			{ TRACK_9,	+TECHNO_COL_SPACING*0.5f, nullptr },
			{ TRACK_10,	+TECHNO_COL_SPACING*1.5f, nullptr },
			{ TRACK_11,	+TECHNO_COL_SPACING*2.5f, nullptr },
			{ TRACK_12,	+TECHNO_COL_SPACING*3.5f, nullptr },
			{ TRACK_13,	+TECHNO_COL_SPACING*4.5f, nullptr },
			{ TRACK_14,	+TECHNO_COL_SPACING*5.5f, nullptr },
			{ TRACK_15,	+TECHNO_COL_SPACING*6.5f, nullptr },
			{ TRACK_16,	+TECHNO_COL_SPACING*7.5f, nullptr },
		},
		{	// PLAYER_2
			{ TRACK_1,	-TECHNO_COL_SPACING*7.5f, nullptr },
			{ TRACK_2,	-TECHNO_COL_SPACING*6.5f, nullptr },
			{ TRACK_3,	-TECHNO_COL_SPACING*5.5f, nullptr },
			{ TRACK_4,	-TECHNO_COL_SPACING*4.5f, nullptr },
			{ TRACK_5,	-TECHNO_COL_SPACING*3.5f, nullptr },
			{ TRACK_6,	-TECHNO_COL_SPACING*2.5f, nullptr },
			{ TRACK_7,	-TECHNO_COL_SPACING*1.5f, nullptr },
			{ TRACK_8,	-TECHNO_COL_SPACING*0.5f, nullptr },
			{ TRACK_9,	+TECHNO_COL_SPACING*0.5f, nullptr },
			{ TRACK_10,	+TECHNO_COL_SPACING*1.5f, nullptr },
			{ TRACK_11,	+TECHNO_COL_SPACING*2.5f, nullptr },
			{ TRACK_12,	+TECHNO_COL_SPACING*3.5f, nullptr },
			{ TRACK_13,	+TECHNO_COL_SPACING*4.5f, nullptr },
			{ TRACK_14,	+TECHNO_COL_SPACING*5.5f, nullptr },
			{ TRACK_15,	+TECHNO_COL_SPACING*6.5f, nullptr },
			{ TRACK_16,	+TECHNO_COL_SPACING*7.5f, nullptr },
		},
	},
	{	// m_iInputColumn[NUM_GameController][NUM_GameButton]
		{ 1, 6, 4, 3, 2, 5, Style::NO_MAPPING, 0, 7, Style::END_MAPPING },
		{ 9, 14, 12, 11, 10, 13, Style::NO_MAPPING, 8, 15, Style::END_MAPPING },
	},
	{	// m_iColumnDrawOrder[MAX_COLS_PER_PLAYER];
		0,1,2,3,4,5,6,7,8,9,10,11,12,13,14,15
	},
	false, // m_bCanUseBeginnerHelper
	false, // m_bLockDifficulties
};

static const Style *g_apGame_Techno_Styles[] =
{
	&g_Style_Techno_Single4,
	&g_Style_Techno_Single5,
	&g_Style_Techno_Single8,
	&g_Style_Techno_Versus4,
	&g_Style_Techno_Versus5,
	&g_Style_Techno_Versus8,
	&g_Style_Techno_Double4,
	&g_Style_Techno_Double5,
	&g_Style_Techno_Double8,
	nullptr
};

static const AutoMappings g_AutoKeyMappings_Techno = AutoMappings (
	"",
	"",
	"",
	AutoMappingEntry( 0, KEY_Ca,		TECHNO_BUTTON_LEFT,	false ),
	AutoMappingEntry( 0, KEY_Cd,		TECHNO_BUTTON_RIGHT,	false ),
	AutoMappingEntry( 0, KEY_Cw,		TECHNO_BUTTON_UP,	false ),
	AutoMappingEntry( 0, KEY_Cx,		TECHNO_BUTTON_DOWN,	false ),
	AutoMappingEntry( 0, KEY_Cq,		TECHNO_BUTTON_UPLEFT,	false ),
	AutoMappingEntry( 0, KEY_Ce,		TECHNO_BUTTON_UPRIGHT,	false ),
	AutoMappingEntry( 0, KEY_Cs,		TECHNO_BUTTON_CENTER,	false ),
	AutoMappingEntry( 0, KEY_Cz,		TECHNO_BUTTON_DOWNLEFT,	false ),
	AutoMappingEntry( 0, KEY_Cc,		TECHNO_BUTTON_DOWNRIGHT,false ),
	AutoMappingEntry( 0, KEY_KP_C4,		TECHNO_BUTTON_LEFT,	true ),
	AutoMappingEntry( 0, KEY_KP_C6,		TECHNO_BUTTON_RIGHT,	true ),
	AutoMappingEntry( 0, KEY_KP_C8,		TECHNO_BUTTON_UP,	true ),
	AutoMappingEntry( 0, KEY_KP_C2,		TECHNO_BUTTON_DOWN,	true ),
	AutoMappingEntry( 0, KEY_KP_C7,		TECHNO_BUTTON_UPLEFT,	true ),
	AutoMappingEntry( 0, KEY_KP_C9,		TECHNO_BUTTON_UPRIGHT,	true ),
	AutoMappingEntry( 0, KEY_KP_C5,		TECHNO_BUTTON_CENTER,	true ),
	AutoMappingEntry( 0, KEY_KP_C1,		TECHNO_BUTTON_DOWNLEFT,	true ),
	AutoMappingEntry( 0, KEY_KP_C3,		TECHNO_BUTTON_DOWNRIGHT,true )
);

static const Game g_Game_Techno = 
{
	"techno",					// m_szName
	g_apGame_Techno_Styles,				// m_apStyles
	false,						// m_bCountNotesSeparately
	false, // m_bTickHolds
	false, // m_PlayersHaveSeparateStyles
	{						// m_InputScheme
		"techno",				// m_szName
		NUM_TECHNO_BUTTONS,			// m_iButtonsPerController
		{	// m_szButtonNames
			{ "Left",		GAME_BUTTON_LEFT },
			{ "Right",		GAME_BUTTON_RIGHT },
			{ "Up",			GAME_BUTTON_UP },
			{ "Down",		GAME_BUTTON_DOWN },
			{ "UpLeft",		GameButton_Invalid },
			{ "UpRight",		GameButton_Invalid },
			{ "Center",		GameButton_Invalid },
			{ "DownLeft",		GameButton_Invalid },
			{ "DownRight",		GameButton_Invalid },
		},
		&g_AutoKeyMappings_Techno
	},
	{
		{ GameButtonType_Step },
		{ GameButtonType_Step },
		{ GameButtonType_Step },
		{ GameButtonType_Step },
		{ GameButtonType_Step },
		{ GameButtonType_Step },
		{ GameButtonType_Step },
		{ GameButtonType_Step },
		{ GameButtonType_Step },
	},
	TNS_W1,	// m_mapW1To
	TNS_W2,	// m_mapW2To
	TNS_W3,	// m_mapW3To
	TNS_W4,	// m_mapW4To
	TNS_W5,	// m_mapW5To
};

/** popn *********************************************************************/
//ThemeMetric<int>	POPN5_COL_SPACING	("ColumnSpacing","Popn5");
static const int POPN5_COL_SPACING = 32; 
//ThemeMetric<int>	POPN9_COL_SPACING	("ColumnSpacing","Popn9");
static const int POPN9_COL_SPACING = 32; 
static const Style g_Style_Popn_Five =
{	// STYLE_POPN_FIVE
	true,				// m_bUsedForGameplay
	true,				// m_bUsedForEdit
	false,				// m_bUsedForDemonstration
	false,				// m_bUsedForHowToPlay
	"popn-five",			// m_szName
	StepsType_popn_five,		// m_StepsType
	StyleType_OnePlayerOneSide,		// m_StyleType
	5,				// m_iColsPerPlayer
	{	// m_ColumnInfo[NUM_PLAYERS][MAX_COLS_PER_PLAYER];
		{	// PLAYER_1
			{ TRACK_1,	-POPN5_COL_SPACING*2.0f, nullptr },
			{ TRACK_2,	-POPN5_COL_SPACING*1.0f, nullptr },
			{ TRACK_3,	+POPN5_COL_SPACING*0.0f, nullptr },
			{ TRACK_4,	+POPN5_COL_SPACING*1.0f, nullptr },
			{ TRACK_5,	+POPN5_COL_SPACING*2.0f, nullptr },
		},
		{	// PLAYER_2
			{ TRACK_1,	-POPN5_COL_SPACING*2.0f, nullptr },
			{ TRACK_2,	-POPN5_COL_SPACING*1.0f, nullptr },
			{ TRACK_3,	+POPN5_COL_SPACING*0.0f, nullptr },
			{ TRACK_4,	+POPN5_COL_SPACING*1.0f, nullptr },
			{ TRACK_5,	+POPN5_COL_SPACING*2.0f, nullptr },
		},
	},
	{	// m_iInputColumn[NUM_GameController][NUM_GameButton]
		{ Style::NO_MAPPING, Style::NO_MAPPING, 0, 1, 2, 3, 4, Style::END_MAPPING },
		{ Style::NO_MAPPING, Style::NO_MAPPING, 0, 1, 2, 3, 4, Style::END_MAPPING },
	},
	{	// m_iColumnDrawOrder[MAX_COLS_PER_PLAYER];
		0,1,2,3,4
	},
	false, // m_bCanUseBeginnerHelper
	false, // m_bLockDifficulties
};

static const Style g_Style_Popn_Nine =
{	// STYLE_POPN_NINE
	true,				// m_bUsedForGameplay
	true,				// m_bUsedForEdit
	true,				// m_bUsedForDemonstration
	true,				// m_bUsedForHowToPlay
	"popn-nine",			// m_szName
	StepsType_popn_nine,		// m_StepsType
	StyleType_OnePlayerOneSide,		// m_StyleType
	9,				// m_iColsPerPlayer
	{	// m_ColumnInfo[NUM_PLAYERS][MAX_COLS_PER_PLAYER];
		{	// PLAYER_1
			{ TRACK_1,	-POPN9_COL_SPACING*4.0f, nullptr },
			{ TRACK_2,	-POPN9_COL_SPACING*3.0f, nullptr },
			{ TRACK_3,	-POPN9_COL_SPACING*2.0f, nullptr },
			{ TRACK_4,	-POPN9_COL_SPACING*1.0f, nullptr },
			{ TRACK_5,	+POPN9_COL_SPACING*0.0f, nullptr },
			{ TRACK_6,	+POPN9_COL_SPACING*1.0f, nullptr },
			{ TRACK_7,	+POPN9_COL_SPACING*2.0f, nullptr },
			{ TRACK_8,	+POPN9_COL_SPACING*3.0f, nullptr },
			{ TRACK_9,	+POPN9_COL_SPACING*4.0f, nullptr },
		},
		{	// PLAYER_2
			{ TRACK_1,	-POPN9_COL_SPACING*4.0f, nullptr },
			{ TRACK_2,	-POPN9_COL_SPACING*3.0f, nullptr },
			{ TRACK_3,	-POPN9_COL_SPACING*2.0f, nullptr },
			{ TRACK_4,	-POPN9_COL_SPACING*1.0f, nullptr },
			{ TRACK_5,	+POPN9_COL_SPACING*0.0f, nullptr },
			{ TRACK_6,	+POPN9_COL_SPACING*1.0f, nullptr },
			{ TRACK_7,	+POPN9_COL_SPACING*2.0f, nullptr },
			{ TRACK_8,	+POPN9_COL_SPACING*3.0f, nullptr },
			{ TRACK_9,	+POPN9_COL_SPACING*4.0f, nullptr },
		},
	},
	{	// m_iInputColumn[NUM_GameController][NUM_GameButton]
		{ 0, 1, 2, 3, 4, 5, 6, 7, 8, 9, Style::END_MAPPING },
		{ 0, 1, 2, 3, 4, 5, 6, 7, 8, 9, Style::END_MAPPING },
	},
	{	// m_iColumnDrawOrder[MAX_COLS_PER_PLAYER];
		0,1,2,3,4,5,6,7,8
	},
	false, // m_bCanUseBeginnerHelper
	false, // m_bLockDifficulties
};

static const Style *g_apGame_Popn_Styles[] =
{
	&g_Style_Popn_Five,
	&g_Style_Popn_Nine,
	nullptr
};

static const AutoMappings g_AutoKeyMappings_Popn = AutoMappings (
	"",
	"",
	"",
	AutoMappingEntry( 0, KEY_Cz,		POPN_BUTTON_LEFT_WHITE,	false ),
	AutoMappingEntry( 0, KEY_Cs,		POPN_BUTTON_LEFT_YELLOW,false ),
	AutoMappingEntry( 0, KEY_Cx,		POPN_BUTTON_LEFT_GREEN,	false ),
	AutoMappingEntry( 0, KEY_Cd,		POPN_BUTTON_LEFT_BLUE,	false ),
	AutoMappingEntry( 0, KEY_Cc,		POPN_BUTTON_RED,	false ),
	AutoMappingEntry( 0, KEY_Cf,		POPN_BUTTON_RIGHT_BLUE,	false ),
	AutoMappingEntry( 0, KEY_Cv,		POPN_BUTTON_RIGHT_GREEN,false ),
	AutoMappingEntry( 0, KEY_Cg,		POPN_BUTTON_RIGHT_YELLOW,false ),
	AutoMappingEntry( 0, KEY_Cb,		POPN_BUTTON_RIGHT_WHITE,false )
);

static const Game g_Game_Popn = 
{
	"popn",						// m_szName
	g_apGame_Popn_Styles,				// m_apStyles
	true,						// m_bCountNotesSeparately
	false, // m_bTickHolds
	false, // m_PlayersHaveSeparateStyles
	{						// m_InputScheme
		"popn",					// m_szName
		NUM_POPN_BUTTONS,			// m_iButtonsPerController
		{	// m_szButtonNames
			{ "Left White",		GameButton_Invalid },
			{ "Left Yellow",	GAME_BUTTON_UP },
			{ "Left Green",		GameButton_Invalid },
			{ "Left Blue",		GAME_BUTTON_LEFT },
			{ "Red",		GAME_BUTTON_START },
			{ "Right Blue",		GAME_BUTTON_RIGHT },
			{ "Right Green",	GameButton_Invalid },
			{ "Right Yellow",	GAME_BUTTON_DOWN },
			{ "Right White",	GameButton_Invalid },
		},
		&g_AutoKeyMappings_Popn
	},
	{
		{ GameButtonType_Step },
		{ GameButtonType_Step },
		{ GameButtonType_Step },
		{ GameButtonType_Step },
		{ GameButtonType_Step },
		{ GameButtonType_Step },
		{ GameButtonType_Step },
		{ GameButtonType_Step },
		{ GameButtonType_Step },
	},
	TNS_W1,	// m_mapW1To
	TNS_W2,	// m_mapW2To
	TNS_W3,	// m_mapW3To
	TNS_W4,	// m_mapW4To
	TNS_W5,	// m_mapW5To
};

/** Lights *******************************************************************/

static const AutoMappings g_AutoKeyMappings_Lights = AutoMappings (
	"",
	"",
	"",
	AutoMappingEntry( 0, KEY_Cq,		LIGHTS_BUTTON_MARQUEE_UP_LEFT,	false ),
	AutoMappingEntry( 0, KEY_Cw,		LIGHTS_BUTTON_MARQUEE_UP_RIGHT,	false ),
	AutoMappingEntry( 0, KEY_Ce,		LIGHTS_BUTTON_MARQUEE_LR_LEFT,	false ),
	AutoMappingEntry( 0, KEY_Cr,		LIGHTS_BUTTON_MARQUEE_LR_RIGHT,	false ),
	AutoMappingEntry( 0, KEY_Ct,		LIGHTS_BUTTON_BUTTONS_LEFT,	false ),
	AutoMappingEntry( 0, KEY_Cy,		LIGHTS_BUTTON_BUTTONS_RIGHT,	false ),
	AutoMappingEntry( 0, KEY_Cu,		LIGHTS_BUTTON_BASS_LEFT,	false ),
	AutoMappingEntry( 0, KEY_Ci,		LIGHTS_BUTTON_BASS_RIGHT,	false )
);

static const Style g_Style_Lights_Cabinet =
{	// STYLE_LIGHTS_CABINET
	true,				// m_bUsedForGameplay
	true,				// m_bUsedForEdit
	false,				// m_bUsedForDemonstration
	false,				// m_bUsedForHowToPlay
	"cabinet",			// m_szName
	StepsType_lights_cabinet,	// m_StepsType
	StyleType_OnePlayerOneSide,		// m_StyleType
	NUM_CabinetLight,		// m_iColsPerPlayer
	{	// m_ColumnInfo[NUM_PLAYERS][MAX_COLS_PER_PLAYER];
		{	// PLAYER_1
			{ TRACK_1,	-DANCE_COL_SPACING*3.5f, nullptr },
			{ TRACK_2,	-DANCE_COL_SPACING*2.5f, nullptr },
			{ TRACK_3,	-DANCE_COL_SPACING*1.5f, nullptr },
			{ TRACK_4,	-DANCE_COL_SPACING*0.5f, nullptr },
			{ TRACK_5,	+DANCE_COL_SPACING*0.5f, nullptr },
			{ TRACK_6,	+DANCE_COL_SPACING*1.5f, nullptr },
			{ TRACK_7,	+DANCE_COL_SPACING*2.5f, nullptr },
			{ TRACK_8,	+DANCE_COL_SPACING*3.5f, nullptr },
		},
		{	// PLAYER_2
			{ TRACK_1,	-DANCE_COL_SPACING*3.5f, nullptr },
			{ TRACK_2,	-DANCE_COL_SPACING*2.5f, nullptr },
			{ TRACK_3,	-DANCE_COL_SPACING*1.5f, nullptr },
			{ TRACK_4,	-DANCE_COL_SPACING*0.5f, nullptr },
			{ TRACK_5,	+DANCE_COL_SPACING*0.5f, nullptr },
			{ TRACK_6,	+DANCE_COL_SPACING*1.5f, nullptr },
			{ TRACK_7,	+DANCE_COL_SPACING*2.5f, nullptr },
			{ TRACK_8,	+DANCE_COL_SPACING*3.5f, nullptr },
		},
	},
	{	// m_iInputColumn[NUM_GameController][NUM_GameButton]
		{ 0, 1, 2, 3, 4, 5, 6, 7, 8, Style::END_MAPPING },
		{ 0, 1, 2, 3, 4, 5, 6, 7, 8, Style::END_MAPPING },
	},
	{	// m_iColumnDrawOrder[MAX_COLS_PER_PLAYER];
		0,1,2,3,4,5,6,7
	},
	false, // m_bCanUseBeginnerHelper
	false, // m_bLockDifficulties
};

static const Style *g_apGame_Lights_Styles[] =
{
	&g_Style_Lights_Cabinet,
	nullptr
};

static const Game g_Game_Lights = 
{
	"lights",					// m_szName
	g_apGame_Lights_Styles,				// m_apStyles
	false,						// m_bCountNotesSeparately
	false, // m_bTickHolds
	false, // m_PlayersHaveSeparateStyles
	{						// m_InputScheme
		"lights",				// m_szName
		NUM_LIGHTS_BUTTONS,			// m_iButtonsPerController
		{	// m_szButtonNames
			{ "MarqueeUpLeft",	GAME_BUTTON_LEFT },
			{ "MarqueeUpRight",	GAME_BUTTON_RIGHT },
			{ "MarqueeLrLeft",	GAME_BUTTON_UP },
			{ "MarqueeLrRight",	GAME_BUTTON_DOWN },
			{ "ButtonsLeft",	GameButton_Invalid },
			{ "ButtonsRight",	GameButton_Invalid },
			{ "BassLeft",		GameButton_Invalid },
			{ "BassRight",		GameButton_Invalid },
		},
		&g_AutoKeyMappings_Lights
	},
	{
		{ GameButtonType_Step },
		{ GameButtonType_Step },
		{ GameButtonType_Step },
		{ GameButtonType_Step },
		{ GameButtonType_Step },
		{ GameButtonType_Step },
		{ GameButtonType_Step },
		{ GameButtonType_Step },
	},
	TNS_W1,	// m_mapW1To
	TNS_W2,	// m_mapW2To
	TNS_W3,	// m_mapW3To
	TNS_W4,	// m_mapW4To
	TNS_W5,	// m_mapW5To
};

/** Kickbox mania **********************************************************/
static const AutoMappings g_AutoKeyMappings_Kickbox = AutoMappings (
	"", "", "",
	AutoMappingEntry(0, KEY_Cq, GAME_BUTTON_MENULEFT, false),
	AutoMappingEntry(0, KEY_Cr, GAME_BUTTON_MENURIGHT, false),
	AutoMappingEntry(0, KEY_Ce, GAME_BUTTON_MENUUP, false),
	AutoMappingEntry(0, KEY_Cw, GAME_BUTTON_MENUDOWN, false),
	AutoMappingEntry(0, KEY_Cz, KICKBOX_BUTTON_DOWN_LEFT_FOOT, false),
	AutoMappingEntry(0, KEY_Ca, KICKBOX_BUTTON_UP_LEFT_FOOT, false),
	AutoMappingEntry(0, KEY_Cs, KICKBOX_BUTTON_UP_LEFT_FIST, false),
	AutoMappingEntry(0, KEY_Cx, KICKBOX_BUTTON_DOWN_LEFT_FIST, false),
	AutoMappingEntry(0, KEY_Cc, KICKBOX_BUTTON_DOWN_RIGHT_FIST, false),
	AutoMappingEntry(0, KEY_Cd, KICKBOX_BUTTON_UP_RIGHT_FIST, false),
	AutoMappingEntry(0, KEY_Cf, KICKBOX_BUTTON_UP_RIGHT_FOOT, false),
	AutoMappingEntry(0, KEY_Cv, KICKBOX_BUTTON_DOWN_RIGHT_FOOT, false),
	AutoMappingEntry(0, KEY_Cu, GAME_BUTTON_MENULEFT, true),
	AutoMappingEntry(0, KEY_Cp, GAME_BUTTON_MENURIGHT, true),
	AutoMappingEntry(0, KEY_Ci, GAME_BUTTON_MENUUP, true),
	AutoMappingEntry(0, KEY_Co, GAME_BUTTON_MENUDOWN, true),
	AutoMappingEntry(0, KEY_Cm, KICKBOX_BUTTON_DOWN_LEFT_FOOT, true),
	AutoMappingEntry(0, KEY_Cj, KICKBOX_BUTTON_UP_LEFT_FOOT, true),
	AutoMappingEntry(0, KEY_Ck, KICKBOX_BUTTON_UP_LEFT_FIST, true),
	AutoMappingEntry(0, KEY_COMMA, KICKBOX_BUTTON_DOWN_LEFT_FIST, true),
	AutoMappingEntry(0, KEY_PERIOD, KICKBOX_BUTTON_DOWN_RIGHT_FIST, true),
	AutoMappingEntry(0, KEY_Cl, KICKBOX_BUTTON_UP_RIGHT_FIST, true),
	AutoMappingEntry(0, KEY_SEMICOLON, KICKBOX_BUTTON_UP_RIGHT_FOOT, true),
	AutoMappingEntry(0, KEY_SLASH, KICKBOX_BUTTON_DOWN_RIGHT_FOOT, true)
);

static const int KICKBOX_COL_SPACING= 64;

static const Style g_Style_Kickbox_Human=
{
	true, // m_bUsedForGameplay
	true, // m_bUsedForEdit
	true, // m_bUsedForDemonstration
	true, // m_bUsedForHowToPlay
	"human", // m_szName
	StepsType_kickbox_human, // m_StepsType
	StyleType_OnePlayerOneSide, // m_StyleType
	4, // m_iColsPerPlayer
	{ // m_ColumnInfo[NUM_PLAYERS][MAX_COLS_PER_PLAYER];
		{	// PLAYER_1
			{ TRACK_1,	-KICKBOX_COL_SPACING*1.5f, nullptr },
			{ TRACK_2,	-KICKBOX_COL_SPACING*0.5f, nullptr },
			{ TRACK_3,	+KICKBOX_COL_SPACING*0.5f, nullptr },
			{ TRACK_4,	+KICKBOX_COL_SPACING*1.5f, nullptr },
		},
		{	// PLAYER_2
			{ TRACK_1,	-KICKBOX_COL_SPACING*1.5f, nullptr },
			{ TRACK_2,	-KICKBOX_COL_SPACING*0.5f, nullptr },
			{ TRACK_3,	+KICKBOX_COL_SPACING*0.5f, nullptr },
			{ TRACK_4,	+KICKBOX_COL_SPACING*1.5f, nullptr },
		},
	},
	{	// m_iInputColumn[NUM_GameController][NUM_GameButton]
		{ 0, 0, 1, 1, 2, 2, 3, 3, Style::END_MAPPING },
		{ 0, 0, 1, 1, 2, 2, 3, 3, Style::END_MAPPING }
	},
	{	// m_iColumnDrawOrder[MAX_COLS_PER_PLAYER];
		0, 1, 2, 3
	},
	false, // m_bCanUseBeginnerHelper
	false, // m_bLockDifficulties
};

static const Style g_Style_Kickbox_Human_Versus=
{
	true, // m_bUsedForGameplay
	false, // m_bUsedForEdit
	true, // m_bUsedForDemonstration
	false, // m_bUsedForHowToPlay
	"hversus", // m_szName
	StepsType_kickbox_human, // m_StepsType
	StyleType_TwoPlayersTwoSides, // m_StyleType
	4, // m_iColsPerPlayer
	{ // m_ColumnInfo[NUM_PLAYERS][MAX_COLS_PER_PLAYER];
		{	// PLAYER_1
			{ TRACK_1,	-KICKBOX_COL_SPACING*1.5f, nullptr },
			{ TRACK_2,	-KICKBOX_COL_SPACING*0.5f, nullptr },
			{ TRACK_3,	+KICKBOX_COL_SPACING*0.5f, nullptr },
			{ TRACK_4,	+KICKBOX_COL_SPACING*1.5f, nullptr },
		},
		{	// PLAYER_2
			{ TRACK_1,	-KICKBOX_COL_SPACING*1.5f, nullptr },
			{ TRACK_2,	-KICKBOX_COL_SPACING*0.5f, nullptr },
			{ TRACK_3,	+KICKBOX_COL_SPACING*0.5f, nullptr },
			{ TRACK_4,	+KICKBOX_COL_SPACING*1.5f, nullptr },
		},
	},
	{	// m_iInputColumn[NUM_GameController][NUM_GameButton]
		{ 0, 0, 1, 1, 2, 2, 3, 3, Style::END_MAPPING },
		{ 0, 0, 1, 1, 2, 2, 3, 3, Style::END_MAPPING }
	},
	{	// m_iColumnDrawOrder[MAX_COLS_PER_PLAYER];
		0, 1, 2, 3
	},
	false, // m_bCanUseBeginnerHelper
	false, // m_bLockDifficulties
};

static const Style g_Style_Kickbox_Quadarm=
{
	true, // m_bUsedForGameplay
	true, // m_bUsedForEdit
	true, // m_bUsedForDemonstration
	true, // m_bUsedForHowToPlay
	"quadarm", // m_szName
	StepsType_kickbox_quadarm, // m_StepsType
	StyleType_OnePlayerOneSide, // m_StyleType
	4, // m_iColsPerPlayer
	{ // m_ColumnInfo[NUM_PLAYERS][MAX_COLS_PER_PLAYER];
		{	// PLAYER_1
			{ TRACK_1,	-KICKBOX_COL_SPACING*1.5f, nullptr },
			{ TRACK_2,	-KICKBOX_COL_SPACING*0.5f, nullptr },
			{ TRACK_3,	+KICKBOX_COL_SPACING*0.5f, nullptr },
			{ TRACK_4,	+KICKBOX_COL_SPACING*1.5f, nullptr },
		},
		{	// PLAYER_2
			{ TRACK_1,	-KICKBOX_COL_SPACING*1.5f, nullptr },
			{ TRACK_2,	-KICKBOX_COL_SPACING*0.5f, nullptr },
			{ TRACK_3,	+KICKBOX_COL_SPACING*0.5f, nullptr },
			{ TRACK_4,	+KICKBOX_COL_SPACING*1.5f, nullptr },
		},
	},
	{	// m_iInputColumn[NUM_GameController][NUM_GameButton]
		{ Style::NO_MAPPING, Style::NO_MAPPING, 0, 1, 2, 3, Style::NO_MAPPING, Style::NO_MAPPING, Style::END_MAPPING },
		{ Style::NO_MAPPING, Style::NO_MAPPING, 0, 1, 2, 3, Style::NO_MAPPING, Style::NO_MAPPING, Style::END_MAPPING }
	},
	{	// m_iColumnDrawOrder[MAX_COLS_PER_PLAYER];
		0, 1, 2, 3
	},
	false, // m_bCanUseBeginnerHelper
	false, // m_bLockDifficulties
};

static const Style g_Style_Kickbox_Quadarm_Versus=
{
	true, // m_bUsedForGameplay
	false, // m_bUsedForEdit
	true, // m_bUsedForDemonstration
	false, // m_bUsedForHowToPlay
	"qversus", // m_szName
	StepsType_kickbox_quadarm, // m_StepsType
	StyleType_TwoPlayersTwoSides, // m_StyleType
	4, // m_iColsPerPlayer
	{ // m_ColumnInfo[NUM_PLAYERS][MAX_COLS_PER_PLAYER];
		{	// PLAYER_1
			{ TRACK_1,	-KICKBOX_COL_SPACING*1.5f, nullptr },
			{ TRACK_2,	-KICKBOX_COL_SPACING*0.5f, nullptr },
			{ TRACK_3,	+KICKBOX_COL_SPACING*0.5f, nullptr },
			{ TRACK_4,	+KICKBOX_COL_SPACING*1.5f, nullptr },
		},
		{	// PLAYER_2
			{ TRACK_1,	-KICKBOX_COL_SPACING*1.5f, nullptr },
			{ TRACK_2,	-KICKBOX_COL_SPACING*0.5f, nullptr },
			{ TRACK_3,	+KICKBOX_COL_SPACING*0.5f, nullptr },
			{ TRACK_4,	+KICKBOX_COL_SPACING*1.5f, nullptr },
		},
	},
	{	// m_iInputColumn[NUM_GameController][NUM_GameButton]
		{ Style::NO_MAPPING, Style::NO_MAPPING, 0, 1, 2, 3, Style::NO_MAPPING, Style::NO_MAPPING, Style::END_MAPPING },
		{ Style::NO_MAPPING, Style::NO_MAPPING, 0, 1, 2, 3, Style::NO_MAPPING, Style::NO_MAPPING, Style::END_MAPPING }
	},
	{	// m_iColumnDrawOrder[MAX_COLS_PER_PLAYER];
		0, 1, 2, 3
	},
	false, // m_bCanUseBeginnerHelper
	false, // m_bLockDifficulties
};

static const Style g_Style_Kickbox_Insect=
{
	true, // m_bUsedForGameplay
	true, // m_bUsedForEdit
	true, // m_bUsedForDemonstration
	true, // m_bUsedForHowToPlay
	"insect", // m_szName
	StepsType_kickbox_insect, // m_StepsType
	StyleType_OnePlayerOneSide, // m_StyleType
	6, // m_iColsPerPlayer
	{ // m_ColumnInfo[NUM_PLAYERS][MAX_COLS_PER_PLAYER];
		{	// PLAYER_1
			{ TRACK_1,	-KICKBOX_COL_SPACING*2.5f, nullptr },
			{ TRACK_2,	-KICKBOX_COL_SPACING*1.5f, nullptr },
			{ TRACK_3,	-KICKBOX_COL_SPACING*0.5f, nullptr },
			{ TRACK_4,	+KICKBOX_COL_SPACING*0.5f, nullptr },
			{ TRACK_5,	+KICKBOX_COL_SPACING*1.5f, nullptr },
			{ TRACK_6,	+KICKBOX_COL_SPACING*2.5f, nullptr },
		},
		{	// PLAYER_2
			{ TRACK_1,	-KICKBOX_COL_SPACING*2.5f, nullptr },
			{ TRACK_2,	-KICKBOX_COL_SPACING*1.5f, nullptr },
			{ TRACK_3,	-KICKBOX_COL_SPACING*0.5f, nullptr },
			{ TRACK_4,	+KICKBOX_COL_SPACING*0.5f, nullptr },
			{ TRACK_5,	+KICKBOX_COL_SPACING*1.5f, nullptr },
			{ TRACK_6,	+KICKBOX_COL_SPACING*2.5f, nullptr },
		},
	},
	{	// m_iInputColumn[NUM_GameController][NUM_GameButton]
		{ 0, 0, 1, 2, 3, 4, 5, 5, Style::END_MAPPING },
		{ 0, 0, 1, 2, 3, 4, 5, 5, Style::END_MAPPING }
	},
	{	// m_iColumnDrawOrder[MAX_COLS_PER_PLAYER];
		0, 1, 2, 3, 4, 5
	},
	false, // m_bCanUseBeginnerHelper
	false, // m_bLockDifficulties
};

static const Style g_Style_Kickbox_Insect_Versus=
{
	true, // m_bUsedForGameplay
	false, // m_bUsedForEdit
	true, // m_bUsedForDemonstration
	false, // m_bUsedForHowToPlay
	"iversus", // m_szName
	StepsType_kickbox_insect, // m_StepsType
	StyleType_TwoPlayersTwoSides, // m_StyleType
	6, // m_iColsPerPlayer
	{ // m_ColumnInfo[NUM_PLAYERS][MAX_COLS_PER_PLAYER];
		{	// PLAYER_1
			{ TRACK_1,	-KICKBOX_COL_SPACING*2.5f, nullptr },
			{ TRACK_2,	-KICKBOX_COL_SPACING*1.5f, nullptr },
			{ TRACK_3,	-KICKBOX_COL_SPACING*0.5f, nullptr },
			{ TRACK_4,	+KICKBOX_COL_SPACING*0.5f, nullptr },
			{ TRACK_5,	+KICKBOX_COL_SPACING*1.5f, nullptr },
			{ TRACK_6,	+KICKBOX_COL_SPACING*2.5f, nullptr },
		},
		{	// PLAYER_2
			{ TRACK_1,	-KICKBOX_COL_SPACING*2.5f, nullptr },
			{ TRACK_2,	-KICKBOX_COL_SPACING*1.5f, nullptr },
			{ TRACK_3,	-KICKBOX_COL_SPACING*0.5f, nullptr },
			{ TRACK_4,	+KICKBOX_COL_SPACING*0.5f, nullptr },
			{ TRACK_5,	+KICKBOX_COL_SPACING*1.5f, nullptr },
			{ TRACK_6,	+KICKBOX_COL_SPACING*2.5f, nullptr },
		},
	},
	{	// m_iInputColumn[NUM_GameController][NUM_GameButton]
		{ 0, 0, 1, 2, 3, 4, 5, 5, Style::END_MAPPING },
		{ 0, 0, 1, 2, 3, 4, 5, 5, Style::END_MAPPING }
	},
	{	// m_iColumnDrawOrder[MAX_COLS_PER_PLAYER];
		0, 1, 2, 3, 4, 5
	},
	false, // m_bCanUseBeginnerHelper
	false, // m_bLockDifficulties
};

static const Style g_Style_Kickbox_Arachnid=
{
	true, // m_bUsedForGameplay
	true, // m_bUsedForEdit
	true, // m_bUsedForDemonstration
	true, // m_bUsedForHowToPlay
	"arachnid", // m_szName
	StepsType_kickbox_arachnid, // m_StepsType
	StyleType_OnePlayerOneSide, // m_StyleType
	8, // m_iColsPerPlayer
	{ // m_ColumnInfo[NUM_PLAYERS][MAX_COLS_PER_PLAYER];
		{	// PLAYER_1
			{ TRACK_1,	-KICKBOX_COL_SPACING*3.5f, nullptr },
			{ TRACK_2,	-KICKBOX_COL_SPACING*2.5f, nullptr },
			{ TRACK_3,	-KICKBOX_COL_SPACING*1.5f, nullptr },
			{ TRACK_4,	-KICKBOX_COL_SPACING*0.5f, nullptr },
			{ TRACK_5,	+KICKBOX_COL_SPACING*0.5f, nullptr },
			{ TRACK_6,	+KICKBOX_COL_SPACING*1.5f, nullptr },
			{ TRACK_7,	+KICKBOX_COL_SPACING*2.5f, nullptr },
			{ TRACK_8,	+KICKBOX_COL_SPACING*3.5f, nullptr },
		},
		{	// PLAYER_2
			{ TRACK_1,	-KICKBOX_COL_SPACING*3.5f, nullptr },
			{ TRACK_2,	-KICKBOX_COL_SPACING*2.5f, nullptr },
			{ TRACK_3,	-KICKBOX_COL_SPACING*1.5f, nullptr },
			{ TRACK_4,	-KICKBOX_COL_SPACING*0.5f, nullptr },
			{ TRACK_5,	+KICKBOX_COL_SPACING*0.5f, nullptr },
			{ TRACK_6,	+KICKBOX_COL_SPACING*1.5f, nullptr },
			{ TRACK_7,	+KICKBOX_COL_SPACING*2.5f, nullptr },
			{ TRACK_8,	+KICKBOX_COL_SPACING*3.5f, nullptr },
		},
	},
	{	// m_iInputColumn[NUM_GameController][NUM_GameButton]
		{ 0, 1, 2, 3, 4, 5, 6, 7, Style::END_MAPPING },
		{ 0, 1, 2, 3, 4, 5, 6, 7, Style::END_MAPPING }
	},
	{	// m_iColumnDrawOrder[MAX_COLS_PER_PLAYER];
		0, 1, 2, 3, 4, 5, 6, 7
	},
	false, // m_bCanUseBeginnerHelper
	false, // m_bLockDifficulties
};

static const Style g_Style_Kickbox_Arachnid_Versus=
{
	true, // m_bUsedForGameplay
	false, // m_bUsedForEdit
	true, // m_bUsedForDemonstration
	false, // m_bUsedForHowToPlay
	"aversus", // m_szName
	StepsType_kickbox_arachnid, // m_StepsType
	StyleType_TwoPlayersTwoSides, // m_StyleType
	8, // m_iColsPerPlayer
	{ // m_ColumnInfo[NUM_PLAYERS][MAX_COLS_PER_PLAYER];
		{	// PLAYER_1
			{ TRACK_1,	-KICKBOX_COL_SPACING*3.5f, nullptr },
			{ TRACK_2,	-KICKBOX_COL_SPACING*2.5f, nullptr },
			{ TRACK_3,	-KICKBOX_COL_SPACING*1.5f, nullptr },
			{ TRACK_4,	-KICKBOX_COL_SPACING*0.5f, nullptr },
			{ TRACK_5,	+KICKBOX_COL_SPACING*0.5f, nullptr },
			{ TRACK_6,	+KICKBOX_COL_SPACING*1.5f, nullptr },
			{ TRACK_7,	+KICKBOX_COL_SPACING*2.5f, nullptr },
			{ TRACK_8,	+KICKBOX_COL_SPACING*3.5f, nullptr },
		},
		{	// PLAYER_2
			{ TRACK_1,	-KICKBOX_COL_SPACING*3.5f, nullptr },
			{ TRACK_2,	-KICKBOX_COL_SPACING*2.5f, nullptr },
			{ TRACK_3,	-KICKBOX_COL_SPACING*1.5f, nullptr },
			{ TRACK_4,	-KICKBOX_COL_SPACING*0.5f, nullptr },
			{ TRACK_5,	+KICKBOX_COL_SPACING*0.5f, nullptr },
			{ TRACK_6,	+KICKBOX_COL_SPACING*1.5f, nullptr },
			{ TRACK_7,	+KICKBOX_COL_SPACING*2.5f, nullptr },
			{ TRACK_8,	+KICKBOX_COL_SPACING*3.5f, nullptr },
		},
	},
	{	// m_iInputColumn[NUM_GameController][NUM_GameButton]
		{ 0, 1, 2, 3, 4, 5, 6, 7, Style::END_MAPPING },
		{ 0, 1, 2, 3, 4, 5, 6, 7, Style::END_MAPPING }
	},
	{	// m_iColumnDrawOrder[MAX_COLS_PER_PLAYER];
		0, 1, 2, 3, 4, 5, 6, 7
	},
	false, // m_bCanUseBeginnerHelper
	false, // m_bLockDifficulties
};

static const Style* g_apGame_Kickbox_Styles[] =
{
	&g_Style_Kickbox_Human,
	&g_Style_Kickbox_Quadarm,
	&g_Style_Kickbox_Insect,
	&g_Style_Kickbox_Arachnid,
	&g_Style_Kickbox_Human_Versus,
	&g_Style_Kickbox_Quadarm_Versus,
	&g_Style_Kickbox_Insect_Versus,
	&g_Style_Kickbox_Arachnid_Versus,
	nullptr
};

static const Game g_Game_Kickbox =
{
	"kickbox", // m_szName
	g_apGame_Kickbox_Styles, // m_apStyles
	true, // m_bCountNotesSeparately
	false, // m_bTickHolds
	true, // m_PlayersHaveSeparateStyles
	{ // m_InputScheme
		"kickbox", // m_szName
		NUM_KICKBOX_BUTTONS, // m_iButtonsPerController
		{ // m_szButtonNames
			{ "DownLeftFoot", GameButton_Invalid },
			{ "UpLeftFoot", GameButton_Invalid },
			{ "UpLeftFist", GAME_BUTTON_LEFT },
			{ "DownLeftFist", GAME_BUTTON_UP },
			{ "DownRightFist", GAME_BUTTON_DOWN },
			{ "UpRightFist", GAME_BUTTON_RIGHT },
			{ "UpRightFoot", GameButton_Invalid },
			{ "DownRightFoot", GameButton_Invalid },
		},
		&g_AutoKeyMappings_Kickbox
	},
	{
		{ GameButtonType_Step },
		{ GameButtonType_Step },
		{ GameButtonType_Step },
		{ GameButtonType_Step },
		{ GameButtonType_Step },
		{ GameButtonType_Step },
		{ GameButtonType_Step },
		{ GameButtonType_Step },
	},
	TNS_W1,	// m_mapW1To
	TNS_W2,	// m_mapW2To
	TNS_W3,	// m_mapW3To
	TNS_W4,	// m_mapW4To
	TNS_W5,	// m_mapW5To
};

static const Game *g_Games[] = 
{
	&g_Game_Dance,
	&g_Game_Pump,
	&g_Game_KB7,
	&g_Game_Ez2,
	&g_Game_Para,
	&g_Game_DS3DDX,
	&g_Game_Beat,
	&g_Game_Maniax,
	&g_Game_Techno,
	&g_Game_Popn,
	&g_Game_Lights,
	&g_Game_Kickbox,
};

GameManager::GameManager()
{
	// Register with Lua.
	{
		Lua *L = LUA->Get();
		lua_pushstring( L, "GAMEMAN" );
		this->PushSelf( L );
		lua_settable( L, LUA_GLOBALSINDEX );
		LUA->Release( L );
	}
}

GameManager::~GameManager()
{
	// Unregister with Lua.
	LUA->UnsetGlobal( "GAMEMAN" );
}


void GameManager::GetStylesForGame( const Game *pGame, vector<const Style*>& aStylesAddTo, bool editor )
{
	for( int s=0; pGame->m_apStyles[s]; ++s ) 
	{
		const Style *style = pGame->m_apStyles[s];
		if( !editor && !style->m_bUsedForGameplay )	
			continue;
		if( editor && !style->m_bUsedForEdit )	
			continue;

		aStylesAddTo.push_back( style );
	}
}

const Game *GameManager::GetGameForStyle( const Style *pStyle )
{
	for( size_t g=0; g<ARRAYLEN(g_Games); ++g )
	{
		const Game *pGame = g_Games[g];
		for( int s=0; pGame->m_apStyles[s]; ++s ) 
		{
			if( pGame->m_apStyles[s] == pStyle )
				return pGame;
		}
	}
	FAIL_M(pStyle->m_szName);
}

const Style* GameManager::GetEditorStyleForStepsType( StepsType st )
{
	for( size_t g=0; g<ARRAYLEN(g_Games); ++g )
	{
		const Game *pGame = g_Games[g];
		for( int s=0; pGame->m_apStyles[s]; ++s ) 
		{
			const Style *style = pGame->m_apStyles[s];
			if( style->m_StepsType == st && style->m_bUsedForEdit )
				return style;
		}
	}

	ASSERT_M(0, ssprintf("The current game cannot use this Style with the editor!"));
	return nullptr;
}


void GameManager::GetStepsTypesForGame( const Game *pGame, vector<StepsType>& aStepsTypeAddTo )
{
	for( int i=0; pGame->m_apStyles[i]; ++i )
	{
		StepsType st = pGame->m_apStyles[i]->m_StepsType;
		ASSERT(st < NUM_StepsType);
		
		// Some Styles use the same StepsType (e.g. single and versus) so check
		// that we aren't doubling up.
		bool found = false;
		for( unsigned j=0; j < aStepsTypeAddTo.size(); j++ )
			if( st == aStepsTypeAddTo[j] ) { found = true; break; }
		if(found) continue;
			
		aStepsTypeAddTo.push_back( st );
	}
}

void GameManager::GetDemonstrationStylesForGame( const Game *pGame, vector<const Style*> &vpStylesOut )
{
	vpStylesOut.clear();

	for( int s=0; pGame->m_apStyles[s]; ++s ) 
	{
		const Style *style = pGame->m_apStyles[s];
		if( style->m_bUsedForDemonstration )
			vpStylesOut.push_back( style );
	}
	
	ASSERT( vpStylesOut.size()>0 );	// this Game is missing a Style that can be used with the demonstration
}

const Style* GameManager::GetHowToPlayStyleForGame( const Game *pGame )
{
	for( int s=0; pGame->m_apStyles[s]; ++s ) 
	{
		const Style *style = pGame->m_apStyles[s];
		if( style->m_bUsedForHowToPlay )
			return style;
	}

	FAIL_M(ssprintf("Game has no Style that can be used with HowToPlay: %s", pGame->m_szName));
}

void GameManager::GetCompatibleStyles( const Game *pGame, int iNumPlayers, vector<const Style*> &vpStylesOut )
{
	FOREACH_ENUM( StyleType, styleType )
	{
		int iNumPlayersRequired;
		switch( styleType )
		{
		DEFAULT_FAIL( styleType );
		case StyleType_OnePlayerOneSide:
		case StyleType_OnePlayerTwoSides:
			iNumPlayersRequired = 1;
			break;
		case StyleType_TwoPlayersTwoSides:
		case StyleType_TwoPlayersSharedSides:
			iNumPlayersRequired = 2;
			break;
		}

		if( iNumPlayers != iNumPlayersRequired )
			continue;

		for( int s=0; pGame->m_apStyles[s]; ++s ) 
		{
			const Style *style = pGame->m_apStyles[s];
			if( style->m_StyleType != styleType )
				continue;
			if( !style->m_bUsedForGameplay )
				continue;

			vpStylesOut.push_back( style );
		}
	}
}

const Style *GameManager::GetFirstCompatibleStyle( const Game *pGame, int iNumPlayers, StepsType st )
{
	vector<const Style*> vpStyles;
	GetCompatibleStyles( pGame, iNumPlayers, vpStyles );
	for (Style const *s : vpStyles)
	{
		if( s->m_StepsType == st )
		{
			return s;
		}
	}
	return nullptr;
}


void GameManager::GetEnabledGames( vector<const Game*>& aGamesOut )
{
	for( size_t g=0; g<ARRAYLEN(g_Games); ++g )
	{
		const Game *pGame = g_Games[g];
		if( IsGameEnabled( pGame ) )
			aGamesOut.push_back( pGame );
	}
}

const Game* GameManager::GetDefaultGame()
{
	const Game *pDefault = nullptr;
	if( pDefault == nullptr )
	{
		for( size_t i=0; pDefault == nullptr && i < ARRAYLEN(g_Games); ++i )
		{
			if( IsGameEnabled(g_Games[i]) )
				pDefault = g_Games[i];
		}

		if( pDefault == nullptr )
			RageException::Throw( "No NoteSkins found" );
	}
	
	return pDefault;
}

int GameManager::GetIndexFromGame( const Game* pGame )
{
	for( size_t g=0; g<ARRAYLEN(g_Games); ++g )
	{
		if( g_Games[g] == pGame )
			return g;
	}
	FAIL_M(ssprintf("Game not found: %s", pGame->m_szName));
}

const Game* GameManager::GetGameFromIndex( int index )
{
	ASSERT( index >= 0 );
	ASSERT( index < (int) ARRAYLEN(g_Games) );
	return g_Games[index];
}

bool GameManager::IsGameEnabled( const Game *pGame )
{
	return NOTESKIN->DoNoteSkinsExistForGame( pGame );
}

const StepsTypeInfo &GameManager::GetStepsTypeInfo( StepsType st )
{
	ASSERT( ARRAYLEN(g_StepsTypeInfos) == NUM_StepsType );
	ASSERT_M( st < NUM_StepsType, ssprintf("StepsType %d < NUM_StepsType (%d)", st, NUM_StepsType) );
	return g_StepsTypeInfos[st];
}

StepsType GameManager::StringToStepsType( RString sStepsType )
{
	sStepsType.MakeLower();

	for( int i=0; i<NUM_StepsType; i++ )
		if( g_StepsTypeInfos[i].szName == sStepsType )
			return StepsType(i);

	return StepsType_Invalid;
}

RString GameManager::StyleToLocalizedString( const Style* style )
{
	RString s = style->m_szName;
	s = Capitalize( s );
	if( THEME->HasString( "Style", s ) )
		return THEME->GetString( "Style", s );
	else
		return s;
}

const Game* GameManager::StringToGame( RString sGame )
{
	for( size_t i=0; i<ARRAYLEN(g_Games); ++i )
		if( !sGame.CompareNoCase(g_Games[i]->m_szName) )
			return g_Games[i];

	return nullptr;
}


const Style* GameManager::GameAndStringToStyle( const Game *game, RString sStyle )
{
	for( int s=0; game->m_apStyles[s]; ++s ) 
	{
		const Style* style = game->m_apStyles[s];
		if( sStyle.CompareNoCase(style->m_szName) == 0 )
			return style;
	}

	return nullptr;
}

// lua start
#include "LuaBinding.h"

/** @brief Allow Lua to have access to the GameManager. */ 
class LunaGameManager: public Luna<GameManager>
{
public:
	static int StepsTypeToLocalizedString( T* p, lua_State *L )	{ lua_pushstring(L, p->GetStepsTypeInfo(Enum::Check<StepsType>(L, 1)).GetLocalizedString() ); return 1; }
	static int GetFirstStepsTypeForGame( T* p, lua_State *L )
	{
		Game *pGame = Luna<Game>::check( L, 1 );

		vector<StepsType> vstAddTo;
		p->GetStepsTypesForGame( pGame, vstAddTo );
		ASSERT( !vstAddTo.empty() );
		StepsType st = vstAddTo[0];
		LuaHelpers::Push( L, st ); 
		return 1;
	}
	static int IsGameEnabled( T* p, lua_State *L )
	{
		const Game *pGame = p->StringToGame(SArg(1));
		if(pGame)
			lua_pushboolean(L, p->IsGameEnabled( pGame ) );
		else
			lua_pushnil(L);

		return 1;
	}
	static int GetStylesForGame( T* p, lua_State *L )
	{
		RString game_name= SArg(1);
		const Game *pGame = p->StringToGame(game_name);
		if(!pGame)
		{
			luaL_error(L, "GetStylesForGame: Invalid Game: '%s'", game_name.c_str());
		}
		vector<Style*> aStyles;
		lua_createtable(L, 0, 0);
		for( int s=0; pGame->m_apStyles[s]; ++s ) 
		{
			Style *pStyle = const_cast<Style *>( pGame->m_apStyles[s] );
			pStyle->PushSelf(L);
			lua_rawseti(L, -2, s+1);
		}		
		return 1;
	}
	static int GetEnabledGames( T* p, lua_State *L )
	{
		vector<const Game*> aGames;
		p->GetEnabledGames( aGames );
		lua_createtable(L, aGames.size(), 0);
		for(size_t i= 0; i < aGames.size(); ++i)
		{
			lua_pushstring(L, aGames[i]->m_szName);
			lua_rawseti(L, -2, i+1);
		}
		return 1;	
	}
	
	static int SetGame( T* p, lua_State *L )
	{
		RString game_name= SArg(1);
		const Game *pGame = p->StringToGame(game_name);
		if(!pGame)
		{
			luaL_error(L, "SetGame: Invalid Game: '%s'", game_name.c_str());
		}
		RString theme;
		if( lua_gettop(L) >= 2 && !lua_isnil(L, 2) )
		{
			theme = SArg(2);
			if(!THEME->IsThemeSelectable(theme))
			{
				luaL_error(L, "SetGame: Invalid Theme: '%s'", theme.c_str());
			}
		}
		GameLoop::ChangeGame(game_name, theme);
		return 0;
	}
	
	LunaGameManager()
	{
		ADD_METHOD( StepsTypeToLocalizedString );
		ADD_METHOD( GetFirstStepsTypeForGame );
		ADD_METHOD( IsGameEnabled );
		ADD_METHOD( GetStylesForGame );
		ADD_METHOD( GetEnabledGames );
		ADD_METHOD( SetGame );
	};
};

LUA_REGISTER_CLASS( GameManager )
// lua end


/*
 * (c) 2001-2006 Chris Danford, Glenn Maynard
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
