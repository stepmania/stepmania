#include "global.h"
/*
-----------------------------------------------------------------------------
 Class: CourseEntryDisplay

 Desc: See header.

 Copyright (c) 2001-2002 by the person(s) listed below.  All rights reserved.
	Chris Danford
-----------------------------------------------------------------------------
*/

#include "CourseEntryDisplay.h"
#include "RageUtil.h"
#include "GameConstantsAndTypes.h"
#include "PrefsManager.h"
#include "RageLog.h"
#include "PrefsManager.h"
#include "Course.h"
#include "SongManager.h"
#include <math.h>
#include "ThemeManager.h"
#include "Steps.h"
#include "GameState.h"
#include "StyleDef.h"


#define TEXT_BANNER_X			THEME->GetMetricF("CourseEntryDisplay","TextBannerX")
#define TEXT_BANNER_Y			THEME->GetMetricF("CourseEntryDisplay","TextBannerY")
#define TEXT_BANNER_ZOOM		THEME->GetMetricF("CourseEntryDisplay","TextBannerZoom")
#define NUMBER_X				THEME->GetMetricF("CourseEntryDisplay","NumberX")
#define NUMBER_Y				THEME->GetMetricF("CourseEntryDisplay","NumberY")
#define NUMBER_ZOOM				THEME->GetMetricF("CourseEntryDisplay","NumberZoom")
#define FOOT_X					THEME->GetMetricF("CourseEntryDisplay","FootX")
#define FOOT_Y					THEME->GetMetricF("CourseEntryDisplay","FootY")
#define FOOT_ZOOM				THEME->GetMetricF("CourseEntryDisplay","FootZoom")
#define DIFFICULTY_X			THEME->GetMetricF("CourseEntryDisplay","DifficultyX")
#define DIFFICULTY_Y			THEME->GetMetricF("CourseEntryDisplay","DifficultyY")
#define DIFFICULTY_ZOOM			THEME->GetMetricF("CourseEntryDisplay","DifficultyZoom")
#define MODIFIERS_X				THEME->GetMetricF("CourseEntryDisplay","ModifiersX")
#define MODIFIERS_Y				THEME->GetMetricF("CourseEntryDisplay","ModifiersY")
#define MODIFIERS_ZOOM			THEME->GetMetricF("CourseEntryDisplay","ModifiersZoom")
#define MODIFIERS_HORIZ_ALIGN	THEME->GetMetricI("CourseEntryDisplay","ModifiersHorizAlign")


CourseEntryDisplay::CourseEntryDisplay()
{
	m_sprFrame.Load( THEME->GetPathToG("CourseEntryDisplay bar") );
	this->AddChild( &m_sprFrame );

	m_textNumber.LoadFromFont( THEME->GetPathToF("CourseEntryDisplay number") );
	m_textNumber.SetXY( NUMBER_X, NUMBER_Y );
	m_textNumber.EnableShadow( false );
	this->AddChild( &m_textNumber );

	m_TextBanner.SetXY( TEXT_BANNER_X, TEXT_BANNER_Y );
	this->AddChild( &m_TextBanner );

	m_textFoot.LoadFromTextureAndChars( THEME->GetPathToG("CourseEntryDisplay difficulty 2x1"),"10" );
	m_textFoot.SetXY( FOOT_X, FOOT_Y );
	m_textFoot.EnableShadow( false );
	this->AddChild( &m_textFoot );

	m_textDifficultyNumber.LoadFromFont( THEME->GetPathToF("Common normal") );
	m_textDifficultyNumber.SetXY( DIFFICULTY_X, DIFFICULTY_Y );
	m_textDifficultyNumber.SetZoom( DIFFICULTY_ZOOM );
	m_textDifficultyNumber.EnableShadow( false );
	this->AddChild( &m_textDifficultyNumber );

	m_textModifiers.LoadFromFont( THEME->GetPathToF("Common normal") );
	m_textModifiers.SetXY( MODIFIERS_X, MODIFIERS_Y );
	m_textModifiers.SetZoom( MODIFIERS_ZOOM );
	m_textModifiers.SetHorizAlign( (Actor::HorizAlign)MODIFIERS_HORIZ_ALIGN );
	m_textModifiers.EnableShadow( false );
	this->AddChild( &m_textModifiers );
}


void CourseEntryDisplay::LoadFromSongAndNotes( int iNum, Song* pSong, Steps* pNotes, CString sModifiers )
{
	m_textNumber.SetText( ssprintf("%d", iNum) );

	RageColor colorGroup = SONGMAN->GetSongColor( pSong );
	RageColor colorNotes = SONGMAN->GetDifficultyColor( pNotes->GetDifficulty() );

	m_TextBanner.LoadFromSong( pSong );
	m_TextBanner.SetDiffuse( colorGroup );

	m_textFoot.SetText( "1" );
	m_textFoot.SetDiffuse( colorNotes );

	m_textDifficultyNumber.SetText( ssprintf("%d", pNotes->GetMeter()) );
	m_textDifficultyNumber.SetDiffuse( colorNotes );

	m_textModifiers.SetText( sModifiers );
}

void CourseEntryDisplay::LoadFromDifficulty( int iNum, Difficulty dc, CString sModifiers )
{
	m_textNumber.SetText( ssprintf("%d", iNum) );

	RageColor colorGroup = RageColor(1,1,1,1);	// TODO: What should this be?
	RageColor colorNotes = SONGMAN->GetDifficultyColor( dc );

	m_TextBanner.LoadFromString( "??????????", "??????????", "", "", "", "" );
	m_TextBanner.SetDiffuse( colorGroup );

	m_textFoot.SetText( "1" );
	m_textFoot.SetDiffuse( colorNotes );

	m_textDifficultyNumber.SetText( "?" );
	m_textDifficultyNumber.SetDiffuse( colorNotes );

	m_textModifiers.SetText( sModifiers );
}

void CourseEntryDisplay::LoadFromMeterRange( int iNum, int iLow, int iHigh, CString sModifiers )
{
	m_textNumber.SetText( ssprintf("%d", iNum) );

	RageColor colorGroup = RageColor(1,1,1,1);	// TODO: What should this be?
	RageColor colorNotes = RageColor(1,1,1,1);

	m_TextBanner.LoadFromString( "??????????", "??????????", "", "", "", "" );
	m_TextBanner.SetDiffuse( colorGroup );

	m_textFoot.SetText( "1" );
	m_textFoot.SetDiffuse( colorNotes );

	m_textDifficultyNumber.SetText( ssprintf(iLow==iHigh?"%d":"%d-%d", iLow, iHigh) );
	m_textDifficultyNumber.SetDiffuse( colorNotes );

	m_textModifiers.SetText( sModifiers );
}

