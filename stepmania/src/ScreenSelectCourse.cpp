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
#include "GameConstantsAndTypes.h"
#include "PrefsManager.h"
#include "RageLog.h"
#include "InputMapper.h"
#include "InputQueue.h"
#include "AnnouncerManager.h"
#include "GameState.h"
#include "RageMusic.h"


#define BANNER_FRAME_X		THEME->GetMetricF("ScreenSelectCourse","BannerFrameX")
#define BANNER_FRAME_Y		THEME->GetMetricF("ScreenSelectCourse","BannerFrameY")
#define BANNER_X			THEME->GetMetricF("ScreenSelectCourse","BannerX")
#define BANNER_Y			THEME->GetMetricF("ScreenSelectCourse","BannerY")
#define BANNER_WIDTH		THEME->GetMetricF("ScreenSelectCourse","BannerWidth")
#define BANNER_HEIGHT		THEME->GetMetricF("ScreenSelectCourse","BannerHeight")
#define STAGES_X			THEME->GetMetricF("ScreenSelectCourse","StagesX")
#define STAGES_Y			THEME->GetMetricF("ScreenSelectCourse","StagesY")
#define TIME_X				THEME->GetMetricF("ScreenSelectCourse","TimeX")
#define TIME_Y				THEME->GetMetricF("ScreenSelectCourse","TimeY")
#define CONTENTS_X			THEME->GetMetricF("ScreenSelectCourse","ContentsX")
#define CONTENTS_Y			THEME->GetMetricF("ScreenSelectCourse","ContentsY")
#define WHEEL_X				THEME->GetMetricF("ScreenSelectCourse","WheelX")
#define WHEEL_Y				THEME->GetMetricF("ScreenSelectCourse","WheelY")
#define SCORE_P1_X			THEME->GetMetricF("ScreenSelectCourse","ScoreP1X")
#define SCORE_P1_Y			THEME->GetMetricF("ScreenSelectCourse","ScoreP1Y")
#define SCORE_P2_X			THEME->GetMetricF("ScreenSelectCourse","ScoreP2X")
#define SCORE_P2_Y			THEME->GetMetricF("ScreenSelectCourse","ScoreP2Y")
#define HELP_TEXT			THEME->GetMetric("ScreenSelectCourse","HelpText")
#define TIMER_SECONDS		THEME->GetMetricI("ScreenSelectCourse","TimerSeconds")

float BEST_TIME_X( int p ) {
	switch( p ) {
		case PLAYER_1:	return SCORE_P1_X;
		case PLAYER_2:	return SCORE_P2_X;
		default:		ASSERT(0);	return 0;
	}
}
float BEST_TIME_Y( int p ) {
	switch( p ) {
		case PLAYER_1:	return SCORE_P1_Y;
		case PLAYER_2:	return SCORE_P2_Y;
		default:		ASSERT(0);	return 0;
	}
}

const float TWEEN_TIME		= 0.5f;

const ScreenMessage SM_GoToPrevState		=	ScreenMessage(SM_User+1);
const ScreenMessage SM_GoToNextState		=	ScreenMessage(SM_User+2);



ScreenSelectCourse::ScreenSelectCourse()
{
	LOG->Trace( "ScreenSelectCourse::ScreenSelectCourse()" );
 
	MUSIC->LoadAndPlayIfNotAlready( THEME->GetPathTo("Sounds","select course music") );

	m_bMadeChoice = false;
	m_bGoToOptions = false;

	m_Menu.Load(
		THEME->GetPathTo("Graphics","select course background"), 
		THEME->GetPathTo("Graphics","select course top edge"),
		HELP_TEXT, false, true, TIMER_SECONDS 
		);
	this->AddSubActor( &m_Menu );

	m_Banner.SetXY( BANNER_X, BANNER_Y );
	m_Banner.SetCroppedSize( BANNER_WIDTH, BANNER_HEIGHT );
	this->AddSubActor( &m_Banner );

	m_sprBannerFrame.Load( THEME->GetPathTo("Graphics","select course info frame") );
	m_sprBannerFrame.SetXY( BANNER_FRAME_X, BANNER_FRAME_Y );
	this->AddSubActor( &m_sprBannerFrame );

	m_textNumStages.LoadFromFont( THEME->GetPathTo("Fonts","header1") );
	m_textNumStages.SetXY( STAGES_X, STAGES_Y );
	this->AddSubActor( &m_textNumStages );

	m_textTime.LoadFromFont( THEME->GetPathTo("Fonts","header1") );
	m_textTime.SetXY( TIME_X, TIME_Y );
	this->AddSubActor( &m_textTime );

	m_CourseContentsFrame.SetXY( CONTENTS_X, CONTENTS_Y );
	this->AddSubActor( &m_CourseContentsFrame );

	m_MusicWheel.SetXY( WHEEL_X, WHEEL_Y );
	this->AddSubActor( &m_MusicWheel );

	for( int p=0; p<NUM_PLAYERS; p++ )
	{
		if( !GAMESTATE->IsPlayerEnabled((PlayerNumber)p) )
			continue;	// skip

		m_sprHighScoreFrame[p].Load( THEME->GetPathTo("Graphics","select music score frame") );
		m_sprHighScoreFrame[p].StopAnimating();
		m_sprHighScoreFrame[p].SetState( p );
		m_sprHighScoreFrame[p].SetXY( BEST_TIME_X(p), BEST_TIME_Y(p) );
		this->AddSubActor( &m_sprHighScoreFrame[p] );

		m_HighScore[p].SetXY( BEST_TIME_X(p), BEST_TIME_Y(p) );
		m_HighScore[p].SetZoom( 0.6f );
		m_HighScore[p].SetDiffuseColor( PlayerToColor(p) );
		this->AddSubActor( &m_HighScore[p] );
	}	

	m_textHoldForOptions.LoadFromFont( THEME->GetPathTo("Fonts","Stage") );
	m_textHoldForOptions.SetXY( CENTER_X, CENTER_Y );
	m_textHoldForOptions.SetText( "press START again for options" );
	m_textHoldForOptions.SetZoom( 1 );
	m_textHoldForOptions.SetZoomY( 0 );
	m_textHoldForOptions.SetDiffuseColor( D3DXCOLOR(1,1,1,0) );
	this->AddSubActor( &m_textHoldForOptions );


	m_soundSelect.Load( THEME->GetPathTo("Sounds","menu start") );
	m_soundChangeNotes.Load( THEME->GetPathTo("Sounds","select music change notes") );

	SOUND->PlayOnceStreamedFromDir( ANNOUNCER->GetPathTo(ANNOUNCER_SELECT_COURSE_INTRO) );


	AfterCourseChange();
	TweenOnScreen();
	m_Menu.TweenOnScreenFromMenu( SM_None );
}


ScreenSelectCourse::~ScreenSelectCourse()
{
	LOG->Trace( "ScreenSelectCourse::~ScreenSelectCourse()" );

}

void ScreenSelectCourse::DrawPrimitives()
{
	m_Menu.DrawBottomLayer();
	Screen::DrawPrimitives();
	m_Menu.DrawTopLayer();
}

void ScreenSelectCourse::TweenOnScreen()
{
	Actor* pActorsInCourseInfoFrame[] = { &m_sprBannerFrame, &m_Banner, &m_textNumStages, &m_textTime };
	const int iNumActorsInGroupInfoFrame = sizeof(pActorsInCourseInfoFrame) / sizeof(Actor*);
	int i;
	for( i=0; i<iNumActorsInGroupInfoFrame; i++ )
	{
		float fOriginalX = pActorsInCourseInfoFrame[i]->GetX();
		pActorsInCourseInfoFrame[i]->SetX( fOriginalX-400 );
		pActorsInCourseInfoFrame[i]->BeginTweening( TWEEN_TIME, TWEEN_BOUNCE_END );
		pActorsInCourseInfoFrame[i]->SetTweenX( fOriginalX );
	}

	m_CourseContentsFrame.SetXY( CONTENTS_X - 400, CONTENTS_Y );
	m_CourseContentsFrame.BeginTweeningQueued( TWEEN_TIME, Actor::TWEEN_BIAS_END );
	m_CourseContentsFrame.SetTweenXY( CONTENTS_X, CONTENTS_Y );

	Actor* pActorsInScore[] = { &m_sprHighScoreFrame[0], &m_sprHighScoreFrame[1], &m_HighScore[0], &m_HighScore[1] };
	const int iNumActorsInScore = sizeof(pActorsInScore) / sizeof(Actor*);
	for( i=0; i<iNumActorsInScore; i++ )
	{
		float fOriginalX = pActorsInScore[i]->GetX();
		pActorsInScore[i]->SetX( fOriginalX+400 );
		pActorsInScore[i]->BeginTweening( TWEEN_TIME, TWEEN_BIAS_END );
		pActorsInScore[i]->SetTweenX( fOriginalX );
	}

	m_MusicWheel.TweenOnScreen();
}

void ScreenSelectCourse::TweenOffScreen()
{
	Actor* pActorsInCourseInfoFrame[] = { &m_sprBannerFrame, &m_Banner, &m_textNumStages, &m_textTime };
	const int iNumActorsInGroupInfoFrame = sizeof(pActorsInCourseInfoFrame) / sizeof(Actor*);
	int i;
	for( i=0; i<iNumActorsInGroupInfoFrame; i++ )
	{
		pActorsInCourseInfoFrame[i]->BeginTweeningQueued( TWEEN_TIME, TWEEN_BOUNCE_BEGIN );
		pActorsInCourseInfoFrame[i]->SetTweenX( pActorsInCourseInfoFrame[i]->GetX()-400 );
	}

	m_CourseContentsFrame.BeginTweeningQueued( TWEEN_TIME, Actor::TWEEN_BOUNCE_BEGIN );
	m_CourseContentsFrame.SetTweenXY( CONTENTS_X - 400, CONTENTS_Y );

	Actor* pActorsInScore[] = { &m_sprHighScoreFrame[0], &m_sprHighScoreFrame[1], &m_HighScore[0], &m_HighScore[1] };
	const int iNumActorsInScore = sizeof(pActorsInScore) / sizeof(Actor*);
	for( i=0; i<iNumActorsInScore; i++ )
	{
		pActorsInScore[i]->BeginTweening( TWEEN_TIME, TWEEN_BIAS_BEGIN );
		pActorsInScore[i]->SetTweenX( pActorsInScore[i]->GetX()+400 );
	}

	m_MusicWheel.TweenOffScreen();
}


void ScreenSelectCourse::Input( const DeviceInput& DeviceI, const InputEventType type, const GameInput &GameI, const MenuInput &MenuI, const StyleInput &StyleI )
{
	LOG->Trace( "ScreenSelectCourse::Input()" );
	if(type == IET_RELEASE) return; // don't care

	if( m_Menu.IsClosing() )
		return;		// ignore

	if( MenuI.player == PLAYER_INVALID )
		return;

	if( m_bMadeChoice && !m_bGoToOptions && MenuI.button == MENU_BUTTON_START )
	{
		m_bGoToOptions = true;
		m_textHoldForOptions.SetText( "Entering Options..." );
		SOUND->PlayOnceStreamed( THEME->GetPathTo("Sounds","menu start") );
		return;
	}

	if( m_bMadeChoice )
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
		SCREENMAN->SetNewScreen( "ScreenTitleMenu" );
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

		if( bIsHoldingNext || m_bGoToOptions )
			SCREENMAN->SetNewScreen( "ScreenPlayerOptions" );
		else
			SCREENMAN->SetNewScreen( "ScreenStage" );

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
	m_MusicWheel.Select();

	switch( m_MusicWheel.GetSelectedType() )
	{
	case TYPE_COURSE:
		SOUND->PlayOnceStreamedFromDir( ANNOUNCER->GetPathTo(ANNOUNCER_SELECT_MUSIC_COMMENT_GENERAL) );
	
		TweenOffScreen();

		m_soundSelect.PlayRandom();

		m_bMadeChoice = true;

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

		Course* pCourse = m_MusicWheel.GetSelectedCourse();
		GAMESTATE->m_pCurCourse = pCourse;
		for( int p=0; p<NUM_PLAYERS; p++ )
			pCourse->GetPlayerOptions( &GAMESTATE->m_PlayerOptions[p] );
		pCourse->GetSongOptions( &GAMESTATE->m_SongOptions );

		m_Menu.StopTimer();

		this->SendScreenMessage( SM_GoToNextState, 2.5f );
		
		break;
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

			m_textNumStages.SetText( ssprintf("%d", pCourse->m_iStages) );
			float fTotalSeconds = 0;
			for( int i=0; i<pCourse->m_iStages; i++ )
				fTotalSeconds += pCourse->m_apSongs[i]->m_fMusicLengthSeconds;
			m_textTime.SetText( SecondsToTime(fTotalSeconds) );

			m_Banner.SetFromCourse( pCourse );

			m_CourseContentsFrame.SetFromCourse( pCourse );
		}
		break;
	case TYPE_SECTION:	// if we get here, there are no courses
		break;
	default:
		ASSERT(0);
	}
}

