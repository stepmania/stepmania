#include "global.h"
/*
-----------------------------------------------------------------------------
 Class: ScreenMachineOptions

 Desc: See header.

 Copyright (c) 2001-2002 by the person(s) listed below.  All rights reserved.
	Chris Danford
-----------------------------------------------------------------------------
*/

#include "ScreenMachineOptions.h"
#include "RageTextureManager.h"
#include "RageUtil.h"
#include "RageSoundManager.h"
#include "ScreenManager.h"
#include "PrefsManager.h"
#include "GameConstantsAndTypes.h"
#include "StepMania.h"
#include "PrefsManager.h"
#include "RageLog.h"
#include "ThemeManager.h"


enum {
	MO_MENU_TIMER,
	MO_NUM_ARCADE_STAGES,
	MO_JUDGE_DIFFICULTY,
	MO_LIFE_DIFFICULTY,
	MO_FAIL,
	MO_SHOWSTATS,
	MO_COIN_MODE,
	MO_COINS_PER_CREDIT,
	MO_JOINT_PREMIUM,
	MO_SONG_OPTIONS,
	NUM_MACHINE_OPTIONS_LINES
};
/* Hmm.  Ignore JoyAxes and Back Delayed probably belong in "input options",
 * preferably alongside button configuration. */
OptionRowData g_MachineOptionsLines[NUM_MACHINE_OPTIONS_LINES] = {
	{ "Menu\nTimer",		2, {"OFF","ON"} },
	{ "Arcade\nStages",		8, {"1","2","3","4","5","6","7","UNLIMITED"} },
	{ "Judge\nDifficulty",	8, {"1","2","3","4","5","6","7","8"} },
	{ "Life\nDifficulty",	7, {"1","2","3","4","5","6","7"} },
	{ "Default\nFail Type",	3, {"ARCADE","END OF SONG","OFF"} },	
	{ "Show\nStats",		2, {"OFF","ON"} },
	{ "Coin\nMode",			3, {"HOME","PAY","FREE PLAY"} },
	{ "Coins Per\nCredit",	8, {"1","2","3","4","5","6","7","8"} },
	{ "Joint\nPremium",		2, {"OFF","ON"} },
	{ "Song\nOptions",		2, {"HIDE","SHOW"} },
};

ScreenMachineOptions::ScreenMachineOptions() :
	ScreenOptions(
		THEME->GetPathTo("BGAnimations","machine options"),
		THEME->GetPathTo("Graphics","machine options page"),
		THEME->GetPathTo("Graphics","machine options top edge")
		)
{
	LOG->Trace( "ScreenMachineOptions::ScreenMachineOptions()" );

	// fill g_InputOptionsLines with explanation text
	for( int i=0; i<NUM_MACHINE_OPTIONS_LINES; i++ )
	{
		CString sLineName = g_MachineOptionsLines[i].szTitle;
		sLineName.Replace("\n","");
		sLineName.Replace(" ","");
		strcpy( g_MachineOptionsLines[i].szExplanation, THEME->GetMetric("ScreenMachineOptions",sLineName) );
	}

	Init( 
		INPUTMODE_BOTH, 
		g_MachineOptionsLines, 
		NUM_MACHINE_OPTIONS_LINES,
		false );
	m_Menu.StopTimer();

	SOUNDMAN->PlayMusic( THEME->GetPathTo("Sounds","machine options music") );
}

void ScreenMachineOptions::ImportOptions()
{
	m_iSelectedOption[0][MO_MENU_TIMER]				= PREFSMAN->m_bMenuTimer ? 1:0;
	m_iSelectedOption[0][MO_NUM_ARCADE_STAGES]		= PREFSMAN->m_bEventMode ? 7 : PREFSMAN->m_iNumArcadeStages - 1;

	/* .02 difficulty is beyond our timing right now; even autoplay
	 * misses!  At least fix autoplay before enabling this, or we'll
	 * probably get lots of bug reports about it.  
	 *
	 * It's not *supposed* to be doable--it's supposed to be mostly
	 * impossible, and perhaps it *is* justice that even the CPU fails
	 * it.  :)
	 */
	if(      PREFSMAN->m_fJudgeWindowScale == 1.50f )	m_iSelectedOption[0][MO_JUDGE_DIFFICULTY] = 0;
	else if( PREFSMAN->m_fJudgeWindowScale == 1.33f )	m_iSelectedOption[0][MO_JUDGE_DIFFICULTY] = 1;
	else if( PREFSMAN->m_fJudgeWindowScale == 1.16f )	m_iSelectedOption[0][MO_JUDGE_DIFFICULTY] = 2;
	else if( PREFSMAN->m_fJudgeWindowScale == 1.00f )	m_iSelectedOption[0][MO_JUDGE_DIFFICULTY] = 3;
	else if( PREFSMAN->m_fJudgeWindowScale == 0.84f )	m_iSelectedOption[0][MO_JUDGE_DIFFICULTY] = 4;
	else if( PREFSMAN->m_fJudgeWindowScale == 0.66f )	m_iSelectedOption[0][MO_JUDGE_DIFFICULTY] = 5;
	else if( PREFSMAN->m_fJudgeWindowScale == 0.50f )	m_iSelectedOption[0][MO_JUDGE_DIFFICULTY] = 6;
	else if( PREFSMAN->m_fJudgeWindowScale == 0.33f )	m_iSelectedOption[0][MO_JUDGE_DIFFICULTY] = 7;
	else												m_iSelectedOption[0][MO_JUDGE_DIFFICULTY] = 3;

	if(      PREFSMAN->m_fLifeDifficultyScale == 1.60f )	m_iSelectedOption[0][MO_LIFE_DIFFICULTY] = 0;
	else if( PREFSMAN->m_fLifeDifficultyScale == 1.40f )	m_iSelectedOption[0][MO_LIFE_DIFFICULTY] = 1;
	else if( PREFSMAN->m_fLifeDifficultyScale == 1.20f )	m_iSelectedOption[0][MO_LIFE_DIFFICULTY] = 2;
	else if( PREFSMAN->m_fLifeDifficultyScale == 1.00f )	m_iSelectedOption[0][MO_LIFE_DIFFICULTY] = 3;
	else if( PREFSMAN->m_fLifeDifficultyScale == 0.80f )	m_iSelectedOption[0][MO_LIFE_DIFFICULTY] = 4;
	else if( PREFSMAN->m_fLifeDifficultyScale == 0.60f )	m_iSelectedOption[0][MO_LIFE_DIFFICULTY] = 5;
	else if( PREFSMAN->m_fLifeDifficultyScale == 0.40f )	m_iSelectedOption[0][MO_LIFE_DIFFICULTY] = 6;
	else													m_iSelectedOption[0][MO_LIFE_DIFFICULTY] = 3;

	m_iSelectedOption[0][MO_FAIL]					= PREFSMAN->m_DefaultFailType;
	m_iSelectedOption[0][MO_SHOWSTATS]				= PREFSMAN->m_bShowStats ? 1:0;
	m_iSelectedOption[0][MO_COIN_MODE]				= PREFSMAN->m_CoinMode;
	m_iSelectedOption[0][MO_COINS_PER_CREDIT]		= PREFSMAN->m_iCoinsPerCredit - 1;
	m_iSelectedOption[0][MO_JOINT_PREMIUM]			= PREFSMAN->m_bJointPremium ? 1:0;
}

void ScreenMachineOptions::ExportOptions()
{
	PREFSMAN->m_bMenuTimer				= m_iSelectedOption[0][MO_MENU_TIMER] == 1;
	PREFSMAN->m_bEventMode				= m_iSelectedOption[0][MO_NUM_ARCADE_STAGES] == 7;
	PREFSMAN->m_iNumArcadeStages		= m_iSelectedOption[0][MO_NUM_ARCADE_STAGES] + 1;

	switch( m_iSelectedOption[0][MO_JUDGE_DIFFICULTY] )
	{
	case 0:	PREFSMAN->m_fJudgeWindowScale = 1.50f;	break;
	case 1:	PREFSMAN->m_fJudgeWindowScale = 1.33f;	break;
	case 2:	PREFSMAN->m_fJudgeWindowScale = 1.16f;	break;
	case 3:	PREFSMAN->m_fJudgeWindowScale = 1.00f;	break;
	case 4:	PREFSMAN->m_fJudgeWindowScale = 0.84f;	break;
	case 5:	PREFSMAN->m_fJudgeWindowScale = 0.66f;	break;
	case 6:	PREFSMAN->m_fJudgeWindowScale = 0.50f;	break;
	case 7:	PREFSMAN->m_fJudgeWindowScale = 0.33f;	break;
	default:	ASSERT(0);
	}

	switch( m_iSelectedOption[0][MO_LIFE_DIFFICULTY] )
	{
	case 0:	PREFSMAN->m_fLifeDifficultyScale = 1.60f;	break;
	case 1:	PREFSMAN->m_fLifeDifficultyScale = 1.40f;	break;
	case 2:	PREFSMAN->m_fLifeDifficultyScale = 1.20f;	break;
	case 3:	PREFSMAN->m_fLifeDifficultyScale = 1.00f;	break;
	case 4:	PREFSMAN->m_fLifeDifficultyScale = 0.80f;	break;
	case 5:	PREFSMAN->m_fLifeDifficultyScale = 0.60f;	break;
	case 6:	PREFSMAN->m_fLifeDifficultyScale = 0.40f;	break;
	default:	ASSERT(0);
	}
	
	(int&)PREFSMAN->m_DefaultFailType	= m_iSelectedOption[0][MO_FAIL];
	PREFSMAN->m_bShowStats				= m_iSelectedOption[0][MO_SHOWSTATS] == 1;
	(int&)PREFSMAN->m_CoinMode			= m_iSelectedOption[0][MO_COIN_MODE];
	PREFSMAN->m_iCoinsPerCredit			= m_iSelectedOption[0][MO_COINS_PER_CREDIT] + 1;
	PREFSMAN->m_bJointPremium			= m_iSelectedOption[0][MO_JOINT_PREMIUM] == 1;
}

void ScreenMachineOptions::GoToPrevState()
{
	SCREENMAN->SetNewScreen( "ScreenOptionsMenu" );
}

void ScreenMachineOptions::GoToNextState()
{
	PREFSMAN->SaveGlobalPrefsToDisk();
	GoToPrevState();
}

