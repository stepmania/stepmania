#include "stdafx.h"
/*
-----------------------------------------------------------------------------
 Class: ScreenNameEntry

 Desc: See header.

 Copyright (c) 2001-2002 by the person(s) listed below.  All rights reserved.
	Chris Danford
-----------------------------------------------------------------------------
*/

#include "ScreenNameEntry.h"
#include "SongManager.h"
#include "ScreenManager.h"
#include "GameConstantsAndTypes.h"
#include "RageUtil.h"
#include "PrefsManager.h"
#include "GameManager.h"
#include "RageLog.h"
#include "GameState.h"
#include "RageSoundManager.h"
#include "ThemeManager.h"
#include "ScreenHighScores.h"


//
// Defines specific to ScreenNameEntry
//
#define EXPLANATION_X				THEME->GetMetricF("ScreenNameEntry","ExplanationX")
#define EXPLANATION_Y				THEME->GetMetricF("ScreenNameEntry","ExplanationY")
#define EXPLANATION_TEXT			THEME->GetMetric("ScreenNameEntry","ExplanationText")
#define HELP_TEXT					THEME->GetMetric("ScreenNameEntry","HelpText")

const ScreenMessage SM_GoToPrevScreen		=	ScreenMessage(SM_User+1);
const ScreenMessage SM_GoToNextScreen		=	ScreenMessage(SM_User+2);


ScreenNameEntry::ScreenNameEntry()
{
	LOG->Trace( "ScreenNameEntry::ScreenNameEntry()" );

	m_Background;

	for( int p=0; p<NUM_PLAYERS; p++ )
	{
		if( !GAMESTATE->IsPlayerEnabled(p) )
			continue;	// skip

		m_iHighScoreIndex[p] = -1;

		// is this a new high score?
			


		m_GrayArrowRow[p];
		m_textSelectedChars[p];
		m_textScrollingChars[p];
		m_textCategory[p];
	}

	m_Timer;

	m_Fade;
}


ScreenNameEntry::~ScreenNameEntry()
{
	LOG->Trace( "ScreenNameEntry::~ScreenNameEntry()" );
}

void ScreenNameEntry::DrawPrimitives()
{
	Screen::DrawPrimitives();
}

void ScreenNameEntry::Input( const DeviceInput& DeviceI, const InputEventType type, const GameInput &GameI, const MenuInput &MenuI, const StyleInput &StyleI )
{
	LOG->Trace( "ScreenNameEntry::Input()" );

	Screen::Input( DeviceI, type, GameI, MenuI, StyleI );
}

void ScreenNameEntry::HandleScreenMessage( const ScreenMessage SM )
{
	switch( SM )
	{
	case SM_GoToNextScreen:
		SCREENMAN->SetNewScreen( "ScreenMusicScroll" );
		break;
	}
}
