#include "global.h"
/*
-----------------------------------------------------------------------------
 Class: ScreenRaveOptions

 Desc: See header.

 Copyright (c) 2001-2003 by the person(s) listed below.  All rights reserved.
	Chris Danford
	Chris Gomez
-----------------------------------------------------------------------------
*/

#include "ScreenRaveOptions.h"
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
#include "GameState.h"

#define PREV_SCREEN( play_mode )		THEME->GetMetric ("ScreenRaveOptions","PrevScreen"+Capitalize(PlayModeToString(play_mode)))
#define NEXT_SCREEN( play_mode )		THEME->GetMetric ("ScreenRaveOptions","NextScreen"+Capitalize(PlayModeToString(play_mode)))

enum {
	RO_HUMAN_SUPER,
	RO_CPU_SKILL,
	RO_CPU_SUPER,
	NUM_RAVE_OPTIONS_LINES
};

OptionRow g_RaveOptionsLines[NUM_RAVE_OPTIONS_LINES] = {
	OptionRow( "Human Super\nGrowth",	"25%","50%","75%","100%","125%","150%","175%","200%" ),
	OptionRow( "CPU\nSkill",			"-5","-4","-3","-2","-1","DEFAULT","+1","+2","+3","+4","+5" ),
	OptionRow( "CPU Super\nGrowth",		"25%","50%","75%","100%","125%","150%","175%","200%" )
};

PlayerNumber OPPOSITE_PLAYER[NUM_PLAYERS] = { PLAYER_2, PLAYER_1 };

ScreenRaveOptions::ScreenRaveOptions() :
	ScreenOptions("ScreenRaveOptions",false)
{
	LOG->Trace( "ScreenRaveOptions::ScreenRaveOptions()" );

	bool bComputerPlayersPresent = GAMESTATE->GetNumSidesJoined()==1;
	Init( 
		INPUTMODE_PLAYERS, 
		g_RaveOptionsLines, 
		bComputerPlayersPresent ? 3 : 1,
		false, false );

	SOUNDMAN->PlayMusic( THEME->GetPathToS("ScreenMachineOptions music") );
}

void ScreenRaveOptions::ImportOptions()
{
	for( int p=0; p<NUM_PLAYERS; p++ )
	{
		if( GAMESTATE->IsHumanPlayer(p) )
		{
			int iHumanSuperIndex = (int)SCALE( GAMESTATE->m_fSuperMeterGrowth[p], 0.25f, 2.f, 0.f, 7.f );
			CLAMP( iHumanSuperIndex, 0, 9 );
			m_iSelectedOption[p][RO_HUMAN_SUPER]	= iHumanSuperIndex;
		}
		else if( GAMESTATE->IsCpuPlayer(p) )
		{
			PlayerNumber pnHuman = OPPOSITE_PLAYER[pnHuman];

			m_iSelectedOption[pnHuman][RO_CPU_SKILL]		= GAMESTATE->m_iCpuSkill[p];
			CLAMP( m_iSelectedOption[pnHuman][RO_CPU_SKILL], 0, 10 );
			
			int iHumanSuperIndex = (int)SCALE( GAMESTATE->m_fSuperMeterGrowth[pnHuman], 0.25f, 2.f, 0.f, 7.f );
			CLAMP( iHumanSuperIndex, 0, 9 );
			m_iSelectedOption[pnHuman][RO_CPU_SUPER]	= iHumanSuperIndex;
		}
		else
			ASSERT(0);
	}
}

void ScreenRaveOptions::ExportOptions()
{
	for( int p=0; p<NUM_PLAYERS; p++ )
	{
		if( GAMESTATE->IsHumanPlayer(p) )
		{
			GAMESTATE->m_fSuperMeterGrowth[p]	= SCALE( m_iSelectedOption[p][RO_HUMAN_SUPER], 0.f, 7.f, 0.25f, 2.f );

			PlayerNumber pnCPU = OPPOSITE_PLAYER[p];
			GAMESTATE->m_iCpuSkill[pnCPU]			= m_iSelectedOption[p][RO_CPU_SKILL];
			GAMESTATE->m_fSuperMeterGrowth[pnCPU]	= SCALE( m_iSelectedOption[p][RO_CPU_SUPER], 0, 7, 0.25f, 2.f );
		}
	}
}

void ScreenRaveOptions::GoToPrevState()
{
	SCREENMAN->SetNewScreen( PREV_SCREEN(GAMESTATE->m_PlayMode) );
}

void ScreenRaveOptions::GoToNextState()
{
	SCREENMAN->SetNewScreen( NEXT_SCREEN(GAMESTATE->m_PlayMode) );
}

