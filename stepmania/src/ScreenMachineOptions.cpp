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
#include "RageUtil.h"
#include "RageSounds.h"
#include "ScreenManager.h"
#include "PrefsManager.h"
#include "GameConstantsAndTypes.h"
#include "StepMania.h"
#include "PrefsManager.h"
#include "RageLog.h"
#include "ThemeManager.h"
#include "PlayerOptions.h"
#include "SongOptions.h"


enum {
	MO_MENU_TIMER,
	MO_COIN_MODE,
	MO_NUM_ARCADE_STAGES,
	MO_JUDGE_DIFFICULTY,
	MO_LIFE_DIFFICULTY,
	MO_PROGRESSIVE_LIFEBAR,
	MO_PROG_STAGE_LIFEBAR,
	MO_PROG_NONSTOP_LIFEBAR,
	MO_FAIL,
	MO_SHOWSTATS,
	MO_COINS_PER_CREDIT,
	MO_JOINT_PREMIUM,
	MO_SHOW_SONG_OPTIONS,
	NUM_MACHINE_OPTIONS_LINES
};

OptionRow g_MachineOptionsLines[NUM_MACHINE_OPTIONS_LINES] = {
	OptionRow( "Menu\nTimer",			"OFF","ON" ),
	OptionRow( "Coin\nMode",			"HOME","PAY","FREE PLAY" ),
	OptionRow( "Songs Per\nPlay",		"1","2","3","4","5","6","7","UNLIMITED" ),
	OptionRow( "Judge\nDifficulty",		"1","2","3","4","5","6","7","8","JUSTICE" ),
	OptionRow( "Life\nDifficulty",		"1","2","3","4","5","6","7" ),
	OptionRow( "Progressive\nLifebar",	"OFF","1","2","3","4","5","6","7","8"),
	OptionRow( "Progressive\nStage Lifebar", "OFF","1","2","3","4"),
	OptionRow( "Progressive\nNonstop Lifebar",	"OFF","1","2","3","4"),
	OptionRow( "Default\nFail Type",	"ARCADE","END OF SONG","OFF" ),	
	OptionRow( "Show\nStats",			"OFF","ON" ),
	OptionRow( "Coins Per\nCredit",		"1","2","3","4","5","6","7","8" ),
	OptionRow( "Joint\nPremium",		"OFF","ON" ),
	OptionRow( "Song\nOptions",			"HIDE","SHOW","ASK" ),
};

ScreenMachineOptions::ScreenMachineOptions() :
	ScreenOptions("ScreenMachineOptions",false)
{
	LOG->Trace( "ScreenMachineOptions::ScreenMachineOptions()" );

	Init( 
		INPUTMODE_BOTH, 
		g_MachineOptionsLines, 
		NUM_MACHINE_OPTIONS_LINES,
		false, true );
	m_Menu.m_MenuTimer.Disable();

	SOUND->PlayMusic( THEME->GetPathToS("ScreenMachineOptions music") );
}

void ScreenMachineOptions::ImportOptions()
{
	m_iSelectedOption[0][MO_COIN_MODE]				= PREFSMAN->m_iCoinMode;
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
	else if( PREFSMAN->m_fJudgeWindowScale == 0.20f )	m_iSelectedOption[0][MO_JUDGE_DIFFICULTY] = 8;
	else												m_iSelectedOption[0][MO_JUDGE_DIFFICULTY] = 3;

	if(      PREFSMAN->m_fLifeDifficultyScale == 1.60f )	m_iSelectedOption[0][MO_LIFE_DIFFICULTY] = 0;
	else if( PREFSMAN->m_fLifeDifficultyScale == 1.40f )	m_iSelectedOption[0][MO_LIFE_DIFFICULTY] = 1;
	else if( PREFSMAN->m_fLifeDifficultyScale == 1.20f )	m_iSelectedOption[0][MO_LIFE_DIFFICULTY] = 2;
	else if( PREFSMAN->m_fLifeDifficultyScale == 1.00f )	m_iSelectedOption[0][MO_LIFE_DIFFICULTY] = 3;
	else if( PREFSMAN->m_fLifeDifficultyScale == 0.80f )	m_iSelectedOption[0][MO_LIFE_DIFFICULTY] = 4;
	else if( PREFSMAN->m_fLifeDifficultyScale == 0.60f )	m_iSelectedOption[0][MO_LIFE_DIFFICULTY] = 5;
	else if( PREFSMAN->m_fLifeDifficultyScale == 0.40f )	m_iSelectedOption[0][MO_LIFE_DIFFICULTY] = 6;
	else													m_iSelectedOption[0][MO_LIFE_DIFFICULTY] = 3;

	SongOptions so;
	so.FromString( PREFSMAN->m_sDefaultModifiers );
	m_iSelectedOption[0][MO_PROGRESSIVE_LIFEBAR]	= PREFSMAN->m_iProgressiveLifebar;
	m_iSelectedOption[0][MO_PROG_NONSTOP_LIFEBAR]	= PREFSMAN->m_iProgressiveNonstopLifebar;
	m_iSelectedOption[0][MO_PROG_STAGE_LIFEBAR]		= PREFSMAN->m_iProgressiveStageLifebar;
	m_iSelectedOption[0][MO_FAIL]					= so.m_FailType;
	m_iSelectedOption[0][MO_SHOWSTATS]				= PREFSMAN->m_bShowStats ? 1:0;
	m_iSelectedOption[0][MO_COINS_PER_CREDIT]		= PREFSMAN->m_iCoinsPerCredit - 1;
	m_iSelectedOption[0][MO_JOINT_PREMIUM]			= PREFSMAN->m_bJointPremium ? 1:0;
	switch(PREFSMAN->m_ShowSongOptions)
	{
	case PrefsManager::YES: m_iSelectedOption[0][MO_SHOW_SONG_OPTIONS] = 1; break;
	case PrefsManager::NO: m_iSelectedOption[0][MO_SHOW_SONG_OPTIONS] = 0; break;
	case PrefsManager::ASK: m_iSelectedOption[0][MO_SHOW_SONG_OPTIONS] = 2; break;
	default:	ASSERT(0);
	}
}

void ScreenMachineOptions::ExportOptions()
{
	PREFSMAN->m_iCoinMode					= m_iSelectedOption[0][MO_COIN_MODE];
	PREFSMAN->m_bMenuTimer					= m_iSelectedOption[0][MO_MENU_TIMER] == 1;
	PREFSMAN->m_iNumArcadeStages			= m_iSelectedOption[0][MO_NUM_ARCADE_STAGES] + 1;
	PREFSMAN->m_bEventMode					= m_iSelectedOption[0][MO_NUM_ARCADE_STAGES] == 7;
	PREFSMAN->m_iProgressiveNonstopLifebar	= m_iSelectedOption[0][MO_PROG_NONSTOP_LIFEBAR];
	PREFSMAN->m_iProgressiveStageLifebar	= m_iSelectedOption[0][MO_PROG_STAGE_LIFEBAR];

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
	case 8:	PREFSMAN->m_fJudgeWindowScale = 0.20f;	break;
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

	PREFSMAN->m_iProgressiveLifebar = m_iSelectedOption[0][MO_PROGRESSIVE_LIFEBAR];

	CString sModifiers = PREFSMAN->m_sDefaultModifiers;
	PlayerOptions po;
	po.FromString( sModifiers );
	SongOptions so;
	so.FromString( sModifiers );
	switch( m_iSelectedOption[0][MO_FAIL] )
	{
	case 0:	so.m_FailType = SongOptions::FAIL_ARCADE;		break;
	case 1:	so.m_FailType = SongOptions::FAIL_END_OF_SONG;	break;
	case 2:	so.m_FailType = SongOptions::FAIL_OFF;			break;
	default:
		ASSERT(0);
	}
	CStringArray as;
	if( po.GetString() != "" )
		as.push_back( po.GetString() );
	if( so.GetString() != "" )
		as.push_back( so.GetString() );
	PREFSMAN->m_sDefaultModifiers		= join(", ",as);
	PREFSMAN->m_bShowStats				= m_iSelectedOption[0][MO_SHOWSTATS] == 1;
	PREFSMAN->m_iCoinsPerCredit			= m_iSelectedOption[0][MO_COINS_PER_CREDIT] + 1;
	PREFSMAN->m_bJointPremium			= m_iSelectedOption[0][MO_JOINT_PREMIUM] == 1;

	switch(m_iSelectedOption[0][MO_SHOW_SONG_OPTIONS])
	{
	case 0: PREFSMAN->m_ShowSongOptions = PrefsManager::NO; break;
	case 1: PREFSMAN->m_ShowSongOptions = PrefsManager::YES; break;
	case 2: PREFSMAN->m_ShowSongOptions = PrefsManager::ASK; break;
	default:	ASSERT(0);
	}
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

