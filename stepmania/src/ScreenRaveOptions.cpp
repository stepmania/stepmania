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

#define PREV_SCREEN			THEME->GetMetric ("ScreenRaveOptions","PrevScreen")
#define NEXT_SCREEN			THEME->GetMetric ("ScreenRaveOptions","NextScreen")

enum {
	RO_SUPER_GROWTH,
	RO_CPU_SKILL,
	NUM_RAVE_OPTIONS_LINES
};

OptionRow g_RaveOptionsLines[NUM_RAVE_OPTIONS_LINES] = {
	OptionRow( "Super\nGrowth",		false, "25%","50%","75%","100%","125%","150%","175%","200%" ),
	OptionRow( "CPU\nSkill",		true, "-5","-4","-3","-2","-1","DEFAULT","+1","+2","+3","+4","+5" )
};

ScreenRaveOptions::ScreenRaveOptions( CString sClassName ): ScreenOptions( sClassName )
{
	LOG->Trace( "ScreenRaveOptions::ScreenRaveOptions()" );

	Init( 
		INPUTMODE_TOGETHER, 
		g_RaveOptionsLines, 
		GAMESTATE->AnyPlayersAreCpu()? 2 : 1 );
}

void ScreenRaveOptions::ImportOptions()
{
	for( int p=0; p<NUM_PLAYERS; p++ )
	{
		int iSuperGrowthIndex = (int)SCALE( GAMESTATE->m_fSuperMeterGrowthScale[p], 0.25f, 2.f, 0.f, 7.f );
		CLAMP( iSuperGrowthIndex, 0, 9 );
		m_iSelectedOption[p][RO_SUPER_GROWTH] = iSuperGrowthIndex;
	}

	m_iSelectedOption[0][RO_CPU_SKILL] = GAMESTATE->m_iCpuSkill[0];
	CLAMP( m_iSelectedOption[0][RO_CPU_SKILL], 0, 10 );
}

void ScreenRaveOptions::ExportOptions()
{
	int p;
	for( p=0; p<NUM_PLAYERS; p++ )
		GAMESTATE->m_fSuperMeterGrowthScale[p]	= SCALE( m_iSelectedOption[p][RO_SUPER_GROWTH], 0.f, 7.f, 0.25f, 2.f );
	for( p=0; p<NUM_PLAYERS; p++ )
		GAMESTATE->m_iCpuSkill[p]	= m_iSelectedOption[0][RO_CPU_SKILL];
}

void ScreenRaveOptions::GoToPrevState()
{
	SCREENMAN->SetNewScreen( PREV_SCREEN );
}

void ScreenRaveOptions::GoToNextState()
{
	SCREENMAN->SetNewScreen( NEXT_SCREEN );
}
