#include "stdafx.h"
/*
-----------------------------------------------------------------------------
 Class: ScreenPlayerOptions

 Desc: Select a song.

 Copyright (c) 2001-2002 by the person(s) listed below.  All rights reserved.
	Chris Danford
-----------------------------------------------------------------------------
*/

#include "ScreenPlayerOptions.h"
#include "RageUtil.h"
#include "ScreenManager.h"
#include "RageLog.h"
#include "GameState.h"
#include "ThemeManager.h"
#include "AnnouncerManager.h"

#define SONGSEL_SCREEN				THEME->GetMetric("ScreenGameplay","SongSelectScreen")

enum {
	PO_SPEED = 0,
	PO_ACCEL,
	PO_EFFECT,
	PO_APPEAR,
	PO_TURN,
	PO_LITTLE,
	PO_SCROLL,
	PO_COLOR,
	PO_HOLD_NOTES,
	PO_DARK,
	NUM_PLAYER_OPTIONS_LINES
};
OptionRowData g_PlayerOptionsLines[NUM_PLAYER_OPTIONS_LINES] = {
	{ "Speed",			11, {"x0.25","x0.5","x0.75","x1","x1.5","x2","x3","x4","x5","x8", "x12"} },	
	{ "Acceler\n-ation",6, {"OFF","BOOST","LAND","WAVE","EXPAND","BOOMERANG"} },	
	{ "Effect",			7, {"OFF","DRUNK","DIZZY","SPACE","MINI","FLIP","TORNADO"} },	
	{ "Appear\n-ance",	5, {"VISIBLE","HIDDEN","SUDDEN","STEALTH","BLINK"} },	
	{ "Turn",			6, {"OFF","MIRROR","LEFT","RIGHT","SHUFFLE","SUPER SHUFFLE"} },	
	{ "Little",			2, {"OFF","ON"} },	
	{ "Scroll",			2, {"STANDARD","REVERSE"} },	
	{ "Color",			3, {"VIVID","NOTE","FLAT"} },	
	{ "Holds",			2, {"OFF","ON"} },	
	{ "Dark",			2, {"OFF","ON"} },	
};


ScreenPlayerOptions::ScreenPlayerOptions() :
	ScreenOptions(
		THEME->GetPathTo("BGAnimations","player options"),
		THEME->GetPathTo("Graphics","player options page"),
		THEME->GetPathTo("Graphics","player options top edge")
		)
{
	LOG->Trace( "ScreenPlayerOptions::ScreenPlayerOptions()" );
	
	Init( 
		INPUTMODE_PLAYERS, 
		g_PlayerOptionsLines, 
		NUM_PLAYER_OPTIONS_LINES,
		true );

	SOUNDMAN->PlayOnceFromDir( ANNOUNCER->GetPathTo("player options intro") );
}


void ScreenPlayerOptions::ImportOptions()
{
	for( int p=0; p<NUM_PLAYERS; p++ )
	{
		PlayerOptions &po = GAMESTATE->m_PlayerOptions[p];
		
		if(		 po.m_fScrollSpeed == 0.25f )	m_iSelectedOption[p][PO_SPEED] = 0;
		else if( po.m_fScrollSpeed == 0.5f )	m_iSelectedOption[p][PO_SPEED] = 1;
		else if( po.m_fScrollSpeed == 0.75f)	m_iSelectedOption[p][PO_SPEED] = 2;
		else if( po.m_fScrollSpeed == 1.0f )	m_iSelectedOption[p][PO_SPEED] = 3;
		else if( po.m_fScrollSpeed == 1.5f )	m_iSelectedOption[p][PO_SPEED] = 4;
		else if( po.m_fScrollSpeed == 2.0f )	m_iSelectedOption[p][PO_SPEED] = 5;
		else if( po.m_fScrollSpeed == 3.0f )	m_iSelectedOption[p][PO_SPEED] = 6;
		else if( po.m_fScrollSpeed == 4.0f )	m_iSelectedOption[p][PO_SPEED] = 7;
		else if( po.m_fScrollSpeed == 5.0f )	m_iSelectedOption[p][PO_SPEED] = 8;
		else if( po.m_fScrollSpeed == 8.0f )	m_iSelectedOption[p][PO_SPEED] = 9;
		else if( po.m_fScrollSpeed == 12.0f)	m_iSelectedOption[p][PO_SPEED] = 10;
		else										m_iSelectedOption[p][PO_SPEED] = 3;

		m_iSelectedOption[p][PO_ACCEL]		= po.m_AccelType;

		for( int i=0; i<PlayerOptions::NUM_EFFECT_TYPES; i++ ) 
		{
			if( po.m_bEffects[i] )
				m_iSelectedOption[p][PO_EFFECT] = i+1;
		}

		m_iSelectedOption[p][PO_APPEAR]		= po.m_AppearanceType;
		m_iSelectedOption[p][PO_TURN]		= po.m_TurnType;
		m_iSelectedOption[p][PO_LITTLE]		= po.m_bLittle ? 1 : 0;
		m_iSelectedOption[p][PO_SCROLL]		= po.m_bReverseScroll ? 1 : 0 ;
		m_iSelectedOption[p][PO_COLOR]		= po.m_ColorType;
		m_iSelectedOption[p][PO_HOLD_NOTES]	= po.m_bHoldNotes ? 1 : 0;
		m_iSelectedOption[p][PO_DARK]		= po.m_bDark ? 1 : 0;
	}
}

void ScreenPlayerOptions::ExportOptions()
{
	for( int p=0; p<NUM_PLAYERS; p++ )
	{
		PlayerOptions &po = GAMESTATE->m_PlayerOptions[p];

		switch( m_iSelectedOption[p][PO_SPEED] )
		{
		case 0:	po.m_fScrollSpeed = 0.25f;	break;
		case 1:	po.m_fScrollSpeed = 0.5f;	break;
		case 2:	po.m_fScrollSpeed = 0.75f;	break;
		case 3:	po.m_fScrollSpeed = 1.0f;	break;
		case 4:	po.m_fScrollSpeed = 1.5f;	break;
		case 5:	po.m_fScrollSpeed = 2.0f;	break;
		case 6:	po.m_fScrollSpeed = 3.0f;	break;
		case 7:	po.m_fScrollSpeed = 4.0f;	break;
		case 8:	po.m_fScrollSpeed = 5.0f;	break;
		case 9:	po.m_fScrollSpeed = 8.0f;	break;
		case 10: po.m_fScrollSpeed = 12.0f;	break;
		}

		po.m_AccelType	= (PlayerOptions::AccelType)m_iSelectedOption[p][PO_ACCEL];

		ZERO( po.m_bEffects );
		if( m_iSelectedOption[p][PO_EFFECT] > 0 )
			po.m_bEffects[ m_iSelectedOption[p][PO_EFFECT]-1 ] = true;

		po.m_AppearanceType	= (PlayerOptions::AppearanceType)m_iSelectedOption[p][PO_APPEAR];
		po.m_TurnType		= (PlayerOptions::TurnType)m_iSelectedOption[p][PO_TURN];
		po.m_bLittle		= m_iSelectedOption[p][PO_LITTLE] == 1;
		po.m_bReverseScroll	= (m_iSelectedOption[p][PO_SCROLL] == 1);
		po.m_ColorType		= (PlayerOptions::ColorType)m_iSelectedOption[p][PO_COLOR];
		po.m_bHoldNotes		= (m_iSelectedOption[p][PO_HOLD_NOTES] == 1);
		po.m_bDark			= (m_iSelectedOption[p][PO_DARK] == 1);
	}
}

void ScreenPlayerOptions::GoToPrevState()
{
	if( GAMESTATE->m_PlayMode == PLAY_MODE_NONSTOP  ||
		GAMESTATE->m_PlayMode == PLAY_MODE_ONI  ||
		GAMESTATE->m_PlayMode == PLAY_MODE_ENDLESS)
		SCREENMAN->SetNewScreen( "ScreenSelectCourse" );
	else
		SCREENMAN->SetNewScreen( SONGSEL_SCREEN );
}

void ScreenPlayerOptions::GoToNextState()
{
	if( PREFSMAN->m_bShowSongOptions == true )
		SCREENMAN->SetNewScreen( "ScreenSongOptions" );
	else
		SCREENMAN->SetNewScreen( "ScreenStage" );
}



