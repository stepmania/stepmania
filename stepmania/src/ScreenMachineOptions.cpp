#include "stdafx.h"
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
#include "RageMusic.h"
#include "ScreenManager.h"
#include "PrefsManager.h"
#include "GameConstantsAndTypes.h"
#include "StepMania.h"
#include "PrefsManager.h"
#include "RageLog.h"



enum {
	MO_MENU_TIMER,
	MO_SHOW_DANGER,
	MO_NUM_ARCADE_STAGES,
	MO_JUDGE_DIFFICULTY,
	MO_LIFE_DIFFICULTY,
	MO_HIDDEN_SONGS,
	MO_SHOWSTATS,
	NUM_MACHINE_OPTIONS_LINES
};
/* Hmm.  Ignore JoyAxes and Back Delayed probably belong in "input options",
 * preferably alongside button configuration. */
OptionRowData g_MachineOptionsLines[NUM_MACHINE_OPTIONS_LINES] = {
	{ "Menu\nTimer",		2, {"OFF","ON"} },
	{ "Show\nDanger",		2, {"OFF","ON"} },
	{ "Arcade\nStages",		8, {"1","2","3","4","5","6","7","UNLIMITED"} },
	{ "Judge\nDifficulty",	8, {"1","2","3","4","5","6","7","8"} },
	{ "Life\nDifficulty",	7, {"1","2","3","4","5","6","7"} },
	{ "Hidden\nSongs",		2, {"OFF","ON"} },
	{ "Show\nStats",		2, {"OFF","ON"} },
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

	MUSIC->LoadAndPlayIfNotAlready( THEME->GetPathTo("Sounds","machine options music") );
}

void ScreenMachineOptions::ImportOptions()
{
	m_iSelectedOption[0][MO_MENU_TIMER]				= PREFSMAN->m_bMenuTimer ? 1:0;
	m_iSelectedOption[0][MO_SHOW_DANGER]			= PREFSMAN->m_bShowDanger ? 1:0;
	m_iSelectedOption[0][MO_NUM_ARCADE_STAGES]		= PREFSMAN->m_bEventMode ? 7 : PREFSMAN->m_iNumArcadeStages - 1;
	m_iSelectedOption[0][MO_HIDDEN_SONGS]			= PREFSMAN->m_bHiddenSongs ? 1:0;

	/* .02 difficulty is beyond our timing right now; even autoplay
	 * misses!  At least fix autoplay before enabling this, or we'll
	 * probably get lots of bug reports about it.  
	 *
	 * It's not *supposed* to be doable--it's supposed to be mostly
	 * impossible, and perhaps it *is* justice that even the CPU fails
	 * it.  :)
	 */
	if(      PREFSMAN->m_fJudgeWindowSeconds == 0.27f )	m_iSelectedOption[0][MO_JUDGE_DIFFICULTY] = 0;
	else if( PREFSMAN->m_fJudgeWindowSeconds == 0.24f )	m_iSelectedOption[0][MO_JUDGE_DIFFICULTY] = 1;
	else if( PREFSMAN->m_fJudgeWindowSeconds == 0.21f )	m_iSelectedOption[0][MO_JUDGE_DIFFICULTY] = 2;
	else if( PREFSMAN->m_fJudgeWindowSeconds == 0.18f )	m_iSelectedOption[0][MO_JUDGE_DIFFICULTY] = 3;
	else if( PREFSMAN->m_fJudgeWindowSeconds == 0.15f )	m_iSelectedOption[0][MO_JUDGE_DIFFICULTY] = 4;
	else if( PREFSMAN->m_fJudgeWindowSeconds == 0.12f )	m_iSelectedOption[0][MO_JUDGE_DIFFICULTY] = 5;
	else if( PREFSMAN->m_fJudgeWindowSeconds == 0.09f )	m_iSelectedOption[0][MO_JUDGE_DIFFICULTY] = 6;
	else if( PREFSMAN->m_fJudgeWindowSeconds == 0.06f )	m_iSelectedOption[0][MO_JUDGE_DIFFICULTY] = 7;
	else											m_iSelectedOption[0][MO_JUDGE_DIFFICULTY] = 3;

	if(      PREFSMAN->m_fLifeDifficultyScale == 1.60f )	m_iSelectedOption[0][MO_LIFE_DIFFICULTY] = 0;
	else if( PREFSMAN->m_fLifeDifficultyScale == 1.40f )	m_iSelectedOption[0][MO_LIFE_DIFFICULTY] = 1;
	else if( PREFSMAN->m_fLifeDifficultyScale == 1.20f )	m_iSelectedOption[0][MO_LIFE_DIFFICULTY] = 2;
	else if( PREFSMAN->m_fLifeDifficultyScale == 1.00f )	m_iSelectedOption[0][MO_LIFE_DIFFICULTY] = 3;
	else if( PREFSMAN->m_fLifeDifficultyScale == 0.80f )	m_iSelectedOption[0][MO_LIFE_DIFFICULTY] = 4;
	else if( PREFSMAN->m_fLifeDifficultyScale == 0.60f )	m_iSelectedOption[0][MO_LIFE_DIFFICULTY] = 5;
	else if( PREFSMAN->m_fLifeDifficultyScale == 0.40f )	m_iSelectedOption[0][MO_LIFE_DIFFICULTY] = 6;
	else													m_iSelectedOption[0][MO_LIFE_DIFFICULTY] = 3;

	m_iSelectedOption[0][MO_SHOWSTATS]				= PREFSMAN->m_bShowStats ? 1:0;
}

void ScreenMachineOptions::ExportOptions()
{
	PREFSMAN->m_bMenuTimer				= m_iSelectedOption[0][MO_MENU_TIMER] == 1;
	PREFSMAN->m_bShowDanger				= m_iSelectedOption[0][MO_SHOW_DANGER] == 1;
	PREFSMAN->m_bEventMode				= m_iSelectedOption[0][MO_NUM_ARCADE_STAGES] == 7;
	PREFSMAN->m_iNumArcadeStages		= m_iSelectedOption[0][MO_NUM_ARCADE_STAGES] + 1;
	PREFSMAN->m_bHiddenSongs			= m_iSelectedOption[0][MO_HIDDEN_SONGS]	== 1;

	switch( m_iSelectedOption[0][MO_JUDGE_DIFFICULTY] )
	{
	case 0:	PREFSMAN->m_fJudgeWindowSeconds = 0.27f;	break;
	case 1:	PREFSMAN->m_fJudgeWindowSeconds = 0.24f;	break;
	case 2:	PREFSMAN->m_fJudgeWindowSeconds = 0.21f;	break;
	case 3:	PREFSMAN->m_fJudgeWindowSeconds = 0.18f;	break;
	case 4:	PREFSMAN->m_fJudgeWindowSeconds = 0.15f;	break;
	case 5:	PREFSMAN->m_fJudgeWindowSeconds = 0.10f;	break;
	case 6:	PREFSMAN->m_fJudgeWindowSeconds = 0.07f;	break;
	case 7:	PREFSMAN->m_fJudgeWindowSeconds = 0.04f;	break;
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
	
	PREFSMAN->m_bShowStats	= m_iSelectedOption[0][MO_SHOWSTATS] == 1;
}

void ScreenMachineOptions::GoToPrevState()
{
	SCREENMAN->SetNewScreen( "ScreenTitleMenu" );
	PREFSMAN->SaveGlobalPrefsToDisk();
}

void ScreenMachineOptions::GoToNextState()
{
	GoToPrevState();
}

