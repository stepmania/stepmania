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
#include "RageUtil.h"
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
	RO_P1_SUPER_GROWTH,
	RO_P2_SUPER_GROWTH,
	RO_CPU_SKILL,
	NUM_RAVE_OPTIONS_LINES
};

OptionRow g_RaveOptionsLines[NUM_RAVE_OPTIONS_LINES] = {
	OptionRow( "P1 Super\nGrowth",	"25%","50%","75%","100%","125%","150%","175%","200%" ),
	OptionRow( "P2 Super\nGrowth",	"25%","50%","75%","100%","125%","150%","175%","200%" ),
	OptionRow( "CPU\nSkill",		"-5","-4","-3","-2","-1","DEFAULT","+1","+2","+3","+4","+5" )
};

PlayerNumber OPPOSITE_PLAYER[NUM_PLAYERS] = { PLAYER_2, PLAYER_1 };

ScreenRaveOptions::ScreenRaveOptions() :
	ScreenOptions("ScreenRaveOptions",true)
{
	LOG->Trace( "ScreenRaveOptions::ScreenRaveOptions()" );

	bool bComputerPlayersPresent = GAMESTATE->GetNumSidesJoined()==1;
	Init( 
		INPUTMODE_BOTH, 
		g_RaveOptionsLines, 
		bComputerPlayersPresent ? 3 : 1,
		false, false );
}

void ScreenRaveOptions::ImportOptions()
{
	for( int p=0; p<NUM_PLAYERS; p++ )
	{
		int iSuperGrowthIndex = (int)SCALE( GAMESTATE->m_fSuperMeterGrowthScale[p], 0.25f, 2.f, 0.f, 7.f );
		CLAMP( iSuperGrowthIndex, 0, 9 );
		m_iSelectedOption[0][RO_P1_SUPER_GROWTH+p] = iSuperGrowthIndex;
	}

	m_iSelectedOption[0][RO_CPU_SKILL] = GAMESTATE->m_iCpuSkill[0];
	CLAMP( m_iSelectedOption[0][RO_CPU_SKILL], 0, 10 );
}

void ScreenRaveOptions::ExportOptions()
{
	int p;
	for( p=0; p<NUM_PLAYERS; p++ )
		GAMESTATE->m_fSuperMeterGrowthScale[p]	= SCALE( m_iSelectedOption[0][RO_P1_SUPER_GROWTH+p], 0.f, 7.f, 0.25f, 2.f );
	for( p=0; p<NUM_PLAYERS; p++ )
		GAMESTATE->m_iCpuSkill[p]	= m_iSelectedOption[0][RO_CPU_SKILL];
}

void ScreenRaveOptions::GoToPrevState()
{
	SCREENMAN->SetNewScreen( PREV_SCREEN(GAMESTATE->m_PlayMode) );
}

void ScreenRaveOptions::GoToNextState()
{
	SCREENMAN->SetNewScreen( NEXT_SCREEN(GAMESTATE->m_PlayMode) );
}
