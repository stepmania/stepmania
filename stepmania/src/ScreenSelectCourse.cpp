#include "global.h"
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
#include "GameConstantsAndTypes.h"
#include "RageLog.h"
#include "InputMapper.h"
#include "InputQueue.h"
#include "AnnouncerManager.h"
#include "GameState.h"
#include "RageSoundManager.h"
#include "CodeDetector.h"
#include "ThemeManager.h"


#define EXPLANATION_X		THEME->GetMetricF("ScreenSelectCourse","ExplanationX")
#define EXPLANATION_Y		THEME->GetMetricF("ScreenSelectCourse","ExplanationY")
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
#define SCORE_X( p )		THEME->GetMetricF("ScreenSelectCourse",ssprintf("ScoreP%dX",p+1))
#define SCORE_Y( i )		THEME->GetMetricF("ScreenSelectCourse",ssprintf("ScoreP%dY",i+1))
#define SCORE_ZOOM			THEME->GetMetricF("ScreenSelectCourse","ScoreZoom")
#define HELP_TEXT			THEME->GetMetric("ScreenSelectCourse","HelpText")
#define TIMER_SECONDS		THEME->GetMetricI("ScreenSelectCourse","TimerSeconds")


const float TWEEN_TIME		= 0.5f;

static const ScreenMessage	SM_AllowOptionsMenuRepeat	= ScreenMessage(SM_User+1);



ScreenSelectCourse::ScreenSelectCourse() : Screen("ScreenSelectCourse")
{
	LOG->Trace( "ScreenSelectCourse::ScreenSelectCourse()" );

	CodeDetector::RefreshCacheItems();
 
	SOUNDMAN->PlayMusic( THEME->GetPathToS("ScreenSelectCourse music") );

	m_bMadeChoice = false;
	m_bGoToOptions = false;
	m_bAllowOptionsMenuRepeat = false;
	
	m_Menu.Load( "ScreenSelectCourse" );
	this->AddChild( &m_Menu );

	m_sprExplanation.Load( THEME->GetPathToG("ScreenSelectCourse explanation") );
	m_sprExplanation.SetXY( EXPLANATION_X, EXPLANATION_Y );
	this->AddChild( &m_sprExplanation );

	m_Banner.SetXY( BANNER_X, BANNER_Y );
	this->AddChild( &m_Banner );

	m_sprBannerFrame.Load( THEME->GetPathToG("ScreenSelectCourse banner frame") );
	m_sprBannerFrame.SetXY( BANNER_FRAME_X, BANNER_FRAME_Y );
	this->AddChild( &m_sprBannerFrame );

	m_textNumSongs.LoadFromNumbers( THEME->GetPathToN("ScreenSelectCourse num songs") );
	m_textNumSongs.EnableShadow( false );
	m_textNumSongs.SetXY( STAGES_X, STAGES_Y );
	this->AddChild( &m_textNumSongs );

	m_textTime.LoadFromNumbers( THEME->GetPathToN("ScreenSelectCourse total time") );
	m_textTime.EnableShadow( false );
	m_textTime.SetXY( TIME_X, TIME_Y );
	this->AddChild( &m_textTime );

	m_CourseContentsFrame.SetXY( CONTENTS_X, CONTENTS_Y );
	this->AddChild( &m_CourseContentsFrame );

	m_MusicWheel.SetXY( WHEEL_X, WHEEL_Y );
	this->AddChild( &m_MusicWheel );

	for( int p=0; p<NUM_PLAYERS; p++ )
	{
		if( !GAMESTATE->IsHumanPlayer((PlayerNumber)p) )
			continue;	// skip

		m_sprHighScoreFrame[p].Load( THEME->GetPathToG("ScreenSelectCourse score frame") );
		m_sprHighScoreFrame[p].StopAnimating();
		m_sprHighScoreFrame[p].SetState( p );
		m_sprHighScoreFrame[p].SetXY( SCORE_X(p), SCORE_Y(p) );
		this->AddChild( &m_sprHighScoreFrame[p] );

		m_HighScore[p].SetXY( SCORE_X(p), SCORE_Y(p) );
		m_HighScore[p].SetZoom( SCORE_ZOOM );
		m_HighScore[p].SetDiffuse( PlayerToColor(p) );
		this->AddChild( &m_HighScore[p] );
	}	

	m_sprOptionsMessage.Load( THEME->GetPathToG("ScreenSelectCourse options message 1x2") );
	m_sprOptionsMessage.StopAnimating();
	m_sprOptionsMessage.SetXY( CENTER_X, CENTER_Y );
	m_sprOptionsMessage.SetZoomY( 0 );
	m_sprOptionsMessage.SetDiffuse( RageColor(1,1,1,0) );
//	this->AddChild( &m_sprOptionsMessage );	// draw and update manually


	m_soundSelect.Load( THEME->GetPathToS("Common start") );
	m_soundOptionsChange.Load( THEME->GetPathToS("ScreenSelectCourse options") );
	m_soundChangeNotes.Load( THEME->GetPathToS("ScreenSelectCourse difficulty") );


	SOUNDMAN->PlayOnceFromDir( ANNOUNCER->GetPathTo("select course intro") );

	SOUNDMAN->PlayMusic( THEME->GetPathToS("ScreenSelectCourse music") );

	UpdateOptionsDisplays();

	AfterCourseChange();
	TweenOnScreen();
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
	m_sprOptionsMessage.Draw();
}

void ScreenSelectCourse::Update( float fDelta )
{
	Screen::Update( fDelta );
	m_sprOptionsMessage.Update( fDelta );	
}

void ScreenSelectCourse::TweenOnScreen()
{
	m_sprExplanation.FadeOn( 0.5f, "left bounce", TWEEN_TIME );
	m_sprBannerFrame.FadeOn( 0, "left bounce", TWEEN_TIME );
	m_Banner.FadeOn( 0, "left bounce", TWEEN_TIME );
	m_textNumSongs.FadeOn( 0, "left bounce", TWEEN_TIME );
	m_textTime.FadeOn( 0, "left bounce", TWEEN_TIME );
	m_CourseContentsFrame.FadeOn( 0, "foldy", TWEEN_TIME );
	for( int p=0; p<NUM_PLAYERS; p++ )
	{
		m_sprHighScoreFrame[p].FadeOn( 0, "right bounce", TWEEN_TIME );
		m_HighScore[p].FadeOn( 0, "right bounce", TWEEN_TIME );
	}
	m_MusicWheel.TweenOnScreen();
}

void ScreenSelectCourse::TweenOffScreen()
{
	m_sprExplanation.FadeOff( 0, "left bounce", TWEEN_TIME );
	m_sprBannerFrame.FadeOff( 0, "left bounce", TWEEN_TIME );
	m_Banner.FadeOff( 0, "left bounce", TWEEN_TIME );
	m_textNumSongs.FadeOff( 0, "left bounce", TWEEN_TIME );
	m_textTime.FadeOff( 0, "left bounce", TWEEN_TIME );
	m_CourseContentsFrame.FadeOff( 0, "foldy", TWEEN_TIME );
	for( int p=0; p<NUM_PLAYERS; p++ )
	{
		m_sprHighScoreFrame[p].FadeOff( 0, "right bounce", TWEEN_TIME );
		m_HighScore[p].FadeOff( 0, "right bounce", TWEEN_TIME );
	}
	m_MusicWheel.TweenOffScreen();
}


void ScreenSelectCourse::Input( const DeviceInput& DeviceI, InputEventType type, const GameInput &GameI, const MenuInput &MenuI, const StyleInput &StyleI )
{
	LOG->Trace( "ScreenSelectCourse::Input()" );

	if( DeviceI.device == DEVICE_KEYBOARD && DeviceI.button == SDLK_F9 )
	{
		if( type != IET_FIRST_PRESS ) return;
		PREFSMAN->m_bShowNative ^= 1;
		m_MusicWheel.RebuildMusicWheelItems();
		Course* pCourse = m_MusicWheel.GetSelectedCourse();
		if(pCourse)
			m_CourseContentsFrame.SetFromCourse( pCourse );
		return;
	}

	if( MenuI.button == MENU_BUTTON_RIGHT || MenuI.button == MENU_BUTTON_LEFT )
	{
		if( !MenuI.IsValid() ) return;
		if( !GAMESTATE->IsHumanPlayer(MenuI.player) ) return;

		int dir = 0;
		if(INPUTMAPPER->IsButtonDown( MenuInput(MenuI.player, MENU_BUTTON_RIGHT) ) )
			dir++;
		if(INPUTMAPPER->IsButtonDown( MenuInput(MenuI.player, MENU_BUTTON_LEFT) ) )
			dir--;
		
		m_MusicWheel.Move(dir);
		return;
	}

	if( type == IET_RELEASE )	return;		// don't care

	if( !GameI.IsValid() )		return;		// don't care

	if( m_bMadeChoice  &&  MenuI.IsValid()  &&  MenuI.button == MENU_BUTTON_START  &&  !GAMESTATE->IsExtraStage()  &&  !GAMESTATE->IsExtraStage2() )
	{
		if(m_bGoToOptions) return; /* got it already */

		if( !m_bAllowOptionsMenuRepeat &&
			(type == IET_SLOW_REPEAT || type == IET_FAST_REPEAT ))
			return; /* not allowed yet */

		m_bGoToOptions = true;
		m_sprOptionsMessage.SetState( 1 );
		m_soundSelect.Play();
		return;
	}
	
	if( m_Menu.IsTransitioning() )	return;		// ignore

	if( m_bMadeChoice )
		return;

	if( CodeDetector::DetectAndAdjustMusicOptions(GameI.controller) )
	{
		m_soundOptionsChange.Play();
		UpdateOptionsDisplays();
		return;
	}

	if( CodeDetector::EnteredEasierDifficulty(GameI.controller) &&
		GAMESTATE->m_bDifficultCourses)
	{
		m_soundChangeNotes.Play();
		GAMESTATE->m_bDifficultCourses = false;
		SCREENMAN->PostMessageToTopScreen(SM_SongChanged,0);
	}

	if( CodeDetector::EnteredHarderDifficulty(GameI.controller) &&
		!GAMESTATE->m_bDifficultCourses)
	{
		m_soundChangeNotes.Play();
		GAMESTATE->m_bDifficultCourses = true;
		SCREENMAN->PostMessageToTopScreen(SM_SongChanged,0);
	}


	Screen::Input( DeviceI, type, GameI, MenuI, StyleI );	// default input handler
}

/* Adjust game options.  These settings may be overridden again later by the
 * SongOptions menu. */
void ScreenSelectCourse::AdjustOptions()
{
	if(GAMESTATE->m_pCurCourse->m_iLives != -1)
	{
		/* oni */
		GAMESTATE->m_SongOptions.m_LifeType = SongOptions::LIFE_BATTERY;
		GAMESTATE->m_SongOptions.m_iBatteryLives = GAMESTATE->m_pCurCourse->m_iLives;
	}
}

void ScreenSelectCourse::HandleScreenMessage( const ScreenMessage SM )
{
	Screen::HandleScreenMessage( SM );

	switch( SM )
	{
	case SM_AllowOptionsMenuRepeat:
		m_bAllowOptionsMenuRepeat = true;
		break;
	case SM_MenuTimer:
		MenuStart(PLAYER_1);
		break;
	case SM_GoToPrevScreen:
		SCREENMAN->SetNewScreen( "ScreenTitleMenu" );
		break;
	case SM_GoToNextScreen:

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
	case SM_SongChanged:
		AfterCourseChange();
		break;
	}
}

void ScreenSelectCourse::MenuStart( PlayerNumber pn )
{
	// this needs to check whether valid Notes are selected!
	m_MusicWheel.Select();

	switch( m_MusicWheel.GetSelectedType() )
	{
	case TYPE_COURSE:
		SOUNDMAN->PlayOnceFromDir( ANNOUNCER->GetPathTo("select course comment general") );
	
		TweenOffScreen();

		m_soundSelect.Play();

		m_bMadeChoice = true;

		float fShowSeconds = m_Menu.m_Out.GetLengthSeconds();

		// show "hold START for options"
		m_sprOptionsMessage.SetDiffuse( RageColor(1,1,1,0) );
		m_sprOptionsMessage.BeginTweening( 0.15f );	// fade in
		m_sprOptionsMessage.SetZoomY( 1 );
		m_sprOptionsMessage.SetDiffuse( RageColor(1,1,1,1) );
		m_sprOptionsMessage.BeginTweening( fShowSeconds-0.35f );	// sleep
		m_sprOptionsMessage.BeginTweening( 0.15f );	// fade out
		m_sprOptionsMessage.SetDiffuse( RageColor(1,1,1,0) );
		m_sprOptionsMessage.SetZoomY( 0 );

		/* Don't accept a held START for a little while, so it's not
		 * hit accidentally.  Accept an initial START right away, though,
		 * so we don't ignore deliberate fast presses (which would be
		 * annoying). */
		this->PostScreenMessage( SM_AllowOptionsMenuRepeat, 0.5f );


		m_Menu.StartTransitioning( SM_GoToNextScreen );

		Course* pCourse = m_MusicWheel.GetSelectedCourse();
		GAMESTATE->m_pCurCourse = pCourse;
		// apply #LIVES
		if( pCourse->m_iLives != -1 )
		{
			GAMESTATE->m_SongOptions.m_LifeType = SongOptions::LIFE_BATTERY;
			GAMESTATE->m_SongOptions.m_iBatteryLives = pCourse->m_iLives;
		}

		//
		// Do NOT apply the first song's modifiers before going to the options screen.
		// The user should not be able to override modifiers specifically designated
		// by the course.
		//
		//Song* pSong;
		//Notes* pNotes;
		//CString sModifiers;
		//pCourse->GetFirstStageInfo( pSong, pNotes, sModifiers, GAMESTATE->GetCurrentStyleDef()->m_NotesType );
		//for( int p=0; p<NUM_PLAYERS; p++ )
		//	GAMESTATE->m_PlayerOptions[p].FromString( sModifiers );
		//GAMESTATE->m_SongOptions.FromString( sModifiers );

		AdjustOptions();

		break;
	}
}


void ScreenSelectCourse::MenuBack( PlayerNumber pn )
{
	SOUNDMAN->StopMusic();

	m_Menu.Back( SM_GoToPrevScreen );
}


void ScreenSelectCourse::AfterCourseChange()
{
	switch( m_MusicWheel.GetSelectedType() )
	{
	case TYPE_COURSE:
		{
			Course* pCourse = m_MusicWheel.GetSelectedCourse();

			m_textNumSongs.SetText( ssprintf("%d", pCourse->GetEstimatedNumStages()) );
			float fTotalSeconds;
			if( pCourse->GetTotalSeconds(fTotalSeconds) )
				m_textTime.SetText( SecondsToTime(fTotalSeconds) );
			else
				m_textTime.SetText( "xx:xx:xx" );	// The numbers format doesn't have a '?'.  Is there a better solution?

			m_Banner.LoadFromCourse( pCourse );
			m_Banner.ScaleToClipped( BANNER_WIDTH, BANNER_HEIGHT );

			m_CourseContentsFrame.SetFromCourse( pCourse );
			m_CourseContentsFrame.TweenInAfterChangedCourse();
		}
		break;
	case TYPE_SECTION:	// if we get here, there are no courses
		m_Banner.LoadFromGroup( "" );
		break;
	default:
		ASSERT(0);
	}
}

void ScreenSelectCourse::UpdateOptionsDisplays()
{
	for( int p=0; p<NUM_PLAYERS; p++ )
	{
		if( GAMESTATE->IsHumanPlayer(p) )
		{
			CString s = GAMESTATE->m_PlayerOptions[p].GetString();
			s.Replace( ", ", "\n" );
//			m_textPlayerOptions[p].SetText( s );
		}
	}

	CString s = GAMESTATE->m_SongOptions.GetString();
	s.Replace( ", ", "\n" );
//	m_textSongOptions.SetText( s );
}
