#include "stdafx.h"
/*
-----------------------------------------------------------------------------
 File: ScreenPlayerOptions.h

 Desc: Select a song.

 Copyright (c) 2001-2002 by the person(s) listed below.  All rights reserved.
-----------------------------------------------------------------------------
*/

#include "ScreenPlayerOptions.h"
#include <assert.h>
#include "RageTextureManager.h"
#include "RageUtil.h"
#include "RageMusic.h"
#include "ScreenManager.h"
#include "ScreenGameplay.h"
#include "ScreenSongOptions.h"
#include "GameConstantsAndTypes.h"
#include "ThemeManager.h"
#include "ScreenSelectMusic.h"
#include "RageLog.h"
#include "GameManager.h"


enum {
	PO_SPEED = 0,
	PO_EFFECT,
	PO_APPEAR,
	PO_TURN,
	PO_LITTLE,
	PO_SCROLL,
	PO_COLOR,
	PO_FREEZES,
	PO_DRAIN,
	PO_SKIN,
	NUM_PLAYER_OPTIONS_LINES
};
OptionLineData g_PlayerOptionsLines[NUM_PLAYER_OPTIONS_LINES] = {
	{ "Speed", 6, {"x1","x1.5","x2","x3","x5","x8"} },	
	{ "Effect", 6, {"OFF","BOOST","WAVE", "DRUNK", "DIZZY", "SPACE"} },	
	{ "Appear", 4, {"VISIBLE","HIDDEN","SUDDEN","STEALTH"} },	
	{ "Turn", 5, {"OFF","MIRROR","LEFT","RIGHT","SHUFFLE"} },	
	{ "Little", 2, {"OFF","ON"} },	
	{ "Scroll", 2, {"STANDARD","REVERSE"} },	
	{ "Color", 4, {"ARCADE","NOTE","FLAT","PLAIN"} },	
	{ "Freezes", 2, {"OFF","ON"} },	
	{ "Drain", 3, {"NORMAL", "NO-RECOVER", "SUDDEN-DEATH"} },
	{ "Skin", 0, {""} },	// fill this in on ImportOptions();
};



ScreenPlayerOptions::ScreenPlayerOptions() :
	ScreenOptions(
		THEME->GetPathTo(GRAPHIC_PLAYER_OPTIONS_BACKGROUND),
		THEME->GetPathTo(GRAPHIC_PLAYER_OPTIONS_TOP_EDGE)
		)
{
	LOG->WriteLine( "ScreenPlayerOptions::ScreenPlayerOptions()" );
	
	Init( 
		INPUTMODE_2PLAYERS, 
		g_PlayerOptionsLines, 
		NUM_PLAYER_OPTIONS_LINES
		);
}


void ScreenPlayerOptions::ImportOptions()
{
	// fill in skin names
	CStringArray arraySkinNames;
	GAME->GetSkinNames( arraySkinNames );

	m_OptionLineData[PO_SKIN].iNumOptions	=	arraySkinNames.GetSize(); 
	
	for( int i=0; i<arraySkinNames.GetSize(); i++ )
		strcpy( m_OptionLineData[PO_SKIN].szOptionsText[i], arraySkinNames[i] ); 


	for( int p=0; p<NUM_PLAYERS; p++ )
	{
		PlayerOptions &po = PREFS->m_PlayerOptions[p];
		
		if(		 po.m_fArrowScrollSpeed == 1.0f )	m_iSelectedOption[p][PO_SPEED] = 0;
		else if( po.m_fArrowScrollSpeed == 1.5f )	m_iSelectedOption[p][PO_SPEED] = 1;
		else if( po.m_fArrowScrollSpeed == 2.0f )	m_iSelectedOption[p][PO_SPEED] = 2;
		else if( po.m_fArrowScrollSpeed == 3.0f )	m_iSelectedOption[p][PO_SPEED] = 3;
		else if( po.m_fArrowScrollSpeed == 5.0f )	m_iSelectedOption[p][PO_SPEED] = 4;
		else if( po.m_fArrowScrollSpeed == 8.0f )	m_iSelectedOption[p][PO_SPEED] = 5;
		else										m_iSelectedOption[p][PO_SPEED] = 0;

		m_iSelectedOption[p][PO_EFFECT]		= po.m_EffectType;
		m_iSelectedOption[p][PO_APPEAR]		= po.m_AppearanceType;
		m_iSelectedOption[p][PO_TURN]		= po.m_TurnType;
		m_iSelectedOption[p][PO_LITTLE]		= po.m_bLittle ? 1 : 0;
		m_iSelectedOption[p][PO_SCROLL]		= po.m_bReverseScroll ? 1 : 0 ;
		m_iSelectedOption[p][PO_COLOR]		= po.m_ColorType;
		m_iSelectedOption[p][PO_FREEZES]	= po.m_bAllowFreezeArrows ? 1 : 0;
		m_iSelectedOption[p][PO_DRAIN]		= po.m_DrainType;

		// highlight currently selected skin
		for( int s=0; i<m_OptionLineData[PO_SKIN].iNumOptions; s++ )	// foreach skin
			if( m_OptionLineData[PO_SKIN].szOptionsText[s] == GAME->m_sCurrentSkin[p] )
			{
				m_iSelectedOption[p][PO_DRAIN] = s;
				break;
			}
	}
}

void ScreenPlayerOptions::ExportOptions()
{
	for( int p=0; p<NUM_PLAYERS; p++ )
	{
		PlayerOptions &po = PREFS->m_PlayerOptions[p];

		switch( m_iSelectedOption[p][PO_SPEED] )
		{
		case 0:	po.m_fArrowScrollSpeed = 1.0f;	break;
		case 1:	po.m_fArrowScrollSpeed = 1.5f;	break;
		case 2:	po.m_fArrowScrollSpeed = 2.0f;	break;
		case 3:	po.m_fArrowScrollSpeed = 3.0f;	break;
		case 4:	po.m_fArrowScrollSpeed = 5.0f;	break;
		case 5:	po.m_fArrowScrollSpeed = 8.0f;	break;
		}


		po.m_EffectType			= (PlayerOptions::EffectType)m_iSelectedOption[p][PO_EFFECT];
		po.m_AppearanceType		= (PlayerOptions::AppearanceType)m_iSelectedOption[p][PO_APPEAR];
		po.m_TurnType			= (PlayerOptions::TurnType)m_iSelectedOption[p][PO_TURN];
		po.m_bLittle			= m_iSelectedOption[p][PO_LITTLE] == 1;
		po.m_bReverseScroll		= (m_iSelectedOption[p][PO_SCROLL] == 1);
		po.m_ColorType			= (PlayerOptions::ColorType)m_iSelectedOption[p][PO_COLOR];
		po.m_bAllowFreezeArrows = (m_iSelectedOption[p][PO_FREEZES] == 1);
		po.m_DrainType			= (PlayerOptions::DrainType)m_iSelectedOption[p][PO_DRAIN];

		switch(po.m_DrainType) {
			case po.DRAIN_NORMAL:
				po.m_fInitialLifePercentage = 0.5f;
				po.m_fLifeAdjustments[po.LIFE_PERFECT] =	 0.010f;
				po.m_fLifeAdjustments[po.LIFE_GREAT]	 =	 0.005f;
				po.m_fLifeAdjustments[po.LIFE_GOOD]	 =	 0.000f;
				po.m_fLifeAdjustments[po.LIFE_BOO]	 =	-0.015f;
				po.m_fLifeAdjustments[po.LIFE_MISS]	 =	-0.030f;
				break;

			case po.DRAIN_NO_RECOVER:
				po.m_fInitialLifePercentage = 1.0f;
				po.m_fLifeAdjustments[po.LIFE_PERFECT] =	 0.000f;
				po.m_fLifeAdjustments[po.LIFE_GREAT]	=	 0.000f;
				po.m_fLifeAdjustments[po.LIFE_GOOD]	=	 0.000f;
				po.m_fLifeAdjustments[po.LIFE_BOO]		=	-0.015f;
				po.m_fLifeAdjustments[po.LIFE_MISS]	=	-0.030f;
				break;

			case po.DRAIN_SUDDEN_DEATH:
				po.m_fInitialLifePercentage = 1.0f;
				po.m_fLifeAdjustments[po.LIFE_PERFECT] =	 0.000f;
				po.m_fLifeAdjustments[po.LIFE_GREAT]	=	 0.000f;
				po.m_fLifeAdjustments[po.LIFE_GOOD]	=	-1.000f;
				po.m_fLifeAdjustments[po.LIFE_BOO]		=	-1.000f;
				po.m_fLifeAdjustments[po.LIFE_MISS]	=	-1.000f;
				break;

			default:
				LOG->WriteLine("Unknown Life Meter Drain Setting: %d", m_iSelectedOption[p][PO_DRAIN]);
		}

		
		for( int s=0; s<m_OptionLineData[PO_SKIN].iNumOptions; s++ )	// foreach skin
		if( m_OptionLineData[PO_SKIN].szOptionsText[s] == GAME->m_sCurrentSkin[p] )
		{
			m_iSelectedOption[p][PO_DRAIN] = s;
			break;
		}
	}
}

void ScreenPlayerOptions::GoToPrevState()
{
	SCREENMAN->SetNewScreen( new ScreenSelectMusic );
}

void ScreenPlayerOptions::GoToNextState()
{
	SCREENMAN->SetNewScreen( new ScreenSongOptions );
}



