#include "stdafx.h"
/*
-----------------------------------------------------------------------------
 Class: ScreenHighScores

 Desc: See header.

 Copyright (c) 2001-2002 by the person(s) listed below.  All rights reserved.
	Chris Danford
-----------------------------------------------------------------------------
*/

#include "ScreenHighScores.h"
#include "ThemeManager.h"


#define CATEGORY_X			THEME->GetMetricF("ScreenHighScores","CategoryX")
#define CATEGORY_Y			THEME->GetMetricF("ScreenHighScores","CategoryY")
#define LINE_SPACING_X		THEME->GetMetricF("ScreenHighScores","LineSpacingX")
#define LINE_SPACING_Y		THEME->GetMetricF("ScreenHighScores","LineSpacingY")
#define BULLETS_START_X		THEME->GetMetricF("ScreenHighScores","BulletsStartX")
#define BULLETS_START_Y		THEME->GetMetricF("ScreenHighScores","BulletsStartY")
#define NAMES_START_X		THEME->GetMetricF("ScreenHighScores","NamesStartX")
#define NAMES_START_Y		THEME->GetMetricF("ScreenHighScores","NamesStartY")
#define NAMES_ZOOM			THEME->GetMetricF("ScreenHighScores","NamesZoom")
#define NAMES_COLOR			THEME->GetMetricC("ScreenHighScores","NamesColor")
#define SCORES_START_X		THEME->GetMetricF("ScreenHighScores","ScoresStartX")
#define SCORES_START_Y		THEME->GetMetricF("ScreenHighScores","ScoresStartY")
#define SCORES_ZOOM			THEME->GetMetricF("ScreenHighScores","ScoresZoom")
#define SCORES_COLOR		THEME->GetMetricC("ScreenHighScores","ScoresColor")


ScreenHighScores::ScreenHighScores() : ScreenAttract("ScreenHighScores","high scores")
{
	m_textCategory.LoadFromFont( THEME->GetPathTo("Fonts","Header2") );
	m_textCategory.TurnShadowOff();
	m_textCategory.SetXY( CATEGORY_X, CATEGORY_Y );
	m_textCategory.SetText( "" );

	for( int i=0; i<NUM_HIGH_SCORE_LINES; i++ )
	{
		m_sprBullets[i].Load( THEME->GetPathTo("Graphics",("high scores bullets 1x5")) );
		m_sprBullets[i].SetXY( BULLETS_START_X+LINE_SPACING_X*i, BULLETS_START_Y+LINE_SPACING_Y*i );
		m_sprBullets[i].StopAnimating();
		m_sprBullets[i].SetState(i);
		this->AddChild( &m_sprBullets[i] );

		m_textNames[i].LoadFromFont( THEME->GetPathTo("Fonts","high scores") );
		m_textNames[i].TurnShadowOff();
		m_textNames[i].SetXY( NAMES_START_X+LINE_SPACING_X*i, NAMES_START_Y+LINE_SPACING_Y*i );
		m_textNames[i].SetText( ssprintf("NAME %d",i) );
		m_textNames[i].SetZoom( NAMES_ZOOM );
		m_textNames[i].SetDiffuse( NAMES_COLOR );
		this->AddChild( &m_textNames[i] );

		m_textScores[i].LoadFromFont( THEME->GetPathTo("Fonts","high scores") );
		m_textScores[i].TurnShadowOff();
		m_textScores[i].SetXY( SCORES_START_X+LINE_SPACING_X*i, SCORES_START_Y+LINE_SPACING_Y*i );
		m_textScores[i].SetText( ssprintf("SCORE %d",i) );
		m_textScores[i].SetZoom( SCORES_ZOOM );
		m_textScores[i].SetDiffuse( SCORES_COLOR );
		this->AddChild( &m_textScores[i] );
	}

	this->MoveToTail( &m_Fade );
}