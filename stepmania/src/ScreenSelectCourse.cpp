#include "stdafx.h"
/*
-----------------------------------------------------------------------------
 Class: ScreenSelectCourse

 Desc: See header.

 Copyright (c) 2001-2002 by the person(s) listed below.  All rights reserved.
	Chris Danford
-----------------------------------------------------------------------------
*/


#include "ScreenSelectCourse.h"
#include "ScreenManager.h"
#include "PrefsManager.h"
#include "SongManager.h"
#include "GameManager.h"
#include "RageMusic.h"
#include "ScreenTitleMenu.h"
#include "GameConstantsAndTypes.h"
#include "ThemeManager.h"
#include "ScreenGameplay.h"
#include "ScreenPrompt.h"
#include "ScreenPlayerOptions.h"
#include "RageLog.h"
#include "InputMapper.h"
#include "InputQueue.h"
#include "ScreenStage.h"
#include "AnnouncerManager.h"


const float COURSE_INFO_FRAME_X	= 160;
const float COURSE_INFO_FRAME_Y	= SCREEN_TOP+118;

const float COURSE_CONTENTS_FRAME_X	= 160;
const float COURSE_CONTENTS_FRAME_Y	= CENTER_Y+100;

const float WHEEL_X		= CENTER_X+160;
const float WHEEL_Y		= CENTER_Y+8;

const float TWEEN_TIME		= 0.5f;



const ScreenMessage SM_GoToPrevState		=	ScreenMessage(SM_User+1);
const ScreenMessage SM_GoToNextState		=	ScreenMessage(SM_User+2);
const ScreenMessage SM_ConfirmChange		=	ScreenMessage(SM_User+3);



ScreenSelectCourse::ScreenSelectCourse()
{
	LOG->WriteLine( "ScreenSelectCourse::ScreenSelectCourse()" );

	// for debugging
	if( GAMEMAN->m_CurStyle == STYLE_NONE )
		GAMEMAN->m_CurStyle = STYLE_DANCE_SINGLE;

	m_Menu.Load(
		THEME->GetPathTo(GRAPHIC_SELECT_COURSE_BACKGROUND), 
		THEME->GetPathTo(GRAPHIC_SELECT_COURSE_TOP_EDGE),
		ssprintf("%c or %c change course    then press START", 
		char(1), char(2), char(3), char(3), char(4), char(4), char(3), char(4), char(3), char(4) )
				);
	this->AddActor( &m_Menu );

	m_CourseInfoFrame.SetXY( COURSE_INFO_FRAME_X, COURSE_INFO_FRAME_Y );
	this->AddActor( &m_CourseInfoFrame );

	m_CourseContentsFrame.SetXY( COURSE_CONTENTS_FRAME_X, COURSE_CONTENTS_FRAME_Y );
	this->AddActor( &m_CourseContentsFrame );

	m_MusicWheel.SetXY( WHEEL_X, WHEEL_Y );
	this->AddActor( &m_MusicWheel );

	m_textHoldForOptions.Load( THEME->GetPathTo(FONT_STAGE) );
	m_textHoldForOptions.SetXY( CENTER_X, CENTER_Y );
	m_textHoldForOptions.SetText( "hold START for options" );
	m_textHoldForOptions.SetZoom( 1 );
	m_textHoldForOptions.SetZoomY( 0 );
	m_textHoldForOptions.SetDiffuseColor( D3DXCOLOR(1,1,1,0) );
	m_textHoldForOptions.SetZ( -2 );
	this->AddActor( &m_textHoldForOptions );


	m_soundSelect.Load( THEME->GetPathTo(SOUND_MENU_START) );
	m_soundChangeNotes.Load( THEME->GetPathTo(SOUND_SELECT_MUSIC_CHANGE_NOTES) );

	SOUND->PlayOnceStreamedFromDir( ANNOUNCER->GetPathTo(ANNOUNCER_SELECT_COURSE_INTRO) );


	AfterCourseChange();
	TweenOnScreen();
	m_Menu.TweenOnScreenFromMenu( SM_None );
}


ScreenSelectCourse::~ScreenSelectCourse()
{
	LOG->WriteLine( "ScreenSelectCourse::~ScreenSelectCourse()" );

}

void ScreenSelectCourse::DrawPrimitives()
{
	m_Menu.DrawBottomLayer();
	Screen::DrawPrimitives();
	m_Menu.DrawTopLayer();
}

void ScreenSelectCourse::TweenOnScreen()
{
	m_CourseInfoFrame.SetXY( COURSE_INFO_FRAME_X - 400, COURSE_INFO_FRAME_Y );
	m_CourseInfoFrame.BeginTweening( TWEEN_TIME, Actor::TWEEN_BIAS_END );
	m_CourseInfoFrame.SetTweenXY( COURSE_INFO_FRAME_X, COURSE_INFO_FRAME_Y );

	m_MusicWheel.TweenOnScreen();
}

void ScreenSelectCourse::TweenOffScreen()
{
	m_CourseInfoFrame.BeginTweening( TWEEN_TIME, Actor::TWEEN_BOUNCE_END );
	m_CourseInfoFrame.SetTweenXY( COURSE_INFO_FRAME_X - 400, COURSE_INFO_FRAME_Y );

	m_MusicWheel.TweenOffScreen();
}


void ScreenSelectCourse::Input( const DeviceInput& DeviceI, const InputEventType type, const GameInput &GameI, const MenuInput &MenuI, const StyleInput &StyleI )
{
	LOG->WriteLine( "ScreenSelectCourse::Input()" );

	if( m_Menu.IsClosing() )
		return;		// ignore

	if( MenuI.player == PLAYER_INVALID )
		return;

	Screen::Input( DeviceI, type, GameI, MenuI, StyleI );	// default input handler
}


void ScreenSelectCourse::HandleScreenMessage( const ScreenMessage SM )
{
	Screen::HandleScreenMessage( SM );

	switch( SM )
	{
	case SM_MenuTimer:
		MenuStart(PLAYER_1);
		break;
	case SM_GoToPrevState:
		SCREENMAN->SetNewScreen( new ScreenTitleMenu );
		break;
	case SM_GoToNextState:

		// find out if the Next button is being held down on any of the pads
		bool bIsHoldingNext;
		bIsHoldingNext = false;
		int player;
		for( player=0; player<NUM_PLAYERS; player++ )
		{
			MenuInput mi( (PlayerNumber)player, MENU_BUTTON_START );
			if( INPUTMAPPER->IsButtonDown( mi ) )
				bIsHoldingNext = true;
		}

		if( bIsHoldingNext )
		{
			SCREENMAN->SetNewScreen( new ScreenPlayerOptions );
		}
		else
		{
			MUSIC->Stop();

			SCREENMAN->SetNewScreen( new ScreenStage );
		}
		break;
	}
}

void ScreenSelectCourse::MenuLeft( const PlayerNumber p, const InputEventType type )
{
	m_MusicWheel.PrevMusic();
	
	AfterCourseChange();
}


void ScreenSelectCourse::MenuRight( const PlayerNumber p, const InputEventType type )
{
	m_MusicWheel.NextMusic();

	AfterCourseChange();
}

void ScreenSelectCourse::MenuStart( const PlayerNumber p )
{
	// this needs to check whether valid Notes are selected!
	bool bChoseACourse = m_MusicWheel.Select();


	if( bChoseACourse )
	{
		switch( m_MusicWheel.GetSelectedType() )
		{
		case TYPE_COURSE:
			SONGMAN->m_pCurCourse = m_MusicWheel.GetSelectedCourse();

			SOUND->PlayOnceStreamedFromDir( ANNOUNCER->GetPathTo(ANNOUNCER_SELECT_MUSIC_COMMENT_GENERAL) );

			TweenOffScreen();

			m_soundSelect.PlayRandom();

			// show "hold START for options"
			m_textHoldForOptions.SetDiffuseColor( D3DXCOLOR(1,1,1,0) );
			m_textHoldForOptions.BeginTweeningQueued( 0.25f );	// fade in
			m_textHoldForOptions.SetTweenZoomY( 1 );
			m_textHoldForOptions.SetTweenDiffuseColor( D3DXCOLOR(1,1,1,1) );
			m_textHoldForOptions.BeginTweeningQueued( 2.0f );	// sleep
			m_textHoldForOptions.BeginTweeningQueued( 0.25f );	// fade out
			m_textHoldForOptions.SetTweenDiffuseColor( D3DXCOLOR(1,1,1,0) );
			m_textHoldForOptions.SetTweenZoomY( 0 );

			m_Menu.TweenOffScreenToBlack( SM_None, false );

			this->SendScreenMessage( SM_GoToNextState, 2.5f );
			
			break;
		}
	}
}


void ScreenSelectCourse::MenuBack( const PlayerNumber p )
{
	MUSIC->Stop();

	m_Menu.TweenOffScreenToBlack( SM_GoToPrevState, true );
}


void ScreenSelectCourse::AfterCourseChange()
{
	switch( m_MusicWheel.GetSelectedType() )
	{
	case TYPE_COURSE:
		{
			Course* pCourse = m_MusicWheel.GetSelectedCourse();
			m_CourseInfoFrame.SetFromCourse( pCourse );
			m_CourseContentsFrame.SetFromCourse( pCourse );
		}
		break;
	default:
		ASSERT(0);
	}
}

