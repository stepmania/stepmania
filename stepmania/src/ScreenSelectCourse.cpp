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
#include "ProfileManager.h"
#include "SongManager.h"
#include "GameManager.h"
#include "GameConstantsAndTypes.h"
#include "RageLog.h"
#include "InputMapper.h"
#include "AnnouncerManager.h"
#include "GameState.h"
#include "RageSounds.h"
#include "CodeDetector.h"
#include "ThemeManager.h"
#include "Course.h"


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
#define WHEEL_X				THEME->GetMetricF("ScreenSelectCourse","WheelX")
#define WHEEL_Y				THEME->GetMetricF("ScreenSelectCourse","WheelY")
#define SCORE_X( p )		THEME->GetMetricF("ScreenSelectCourse",ssprintf("ScoreP%dX",p+1))
#define SCORE_Y( i )		THEME->GetMetricF("ScreenSelectCourse",ssprintf("ScoreP%dY",i+1))
#define SCORE_ZOOM			THEME->GetMetricF("ScreenSelectCourse","ScoreZoom")
#define HELP_TEXT			THEME->GetMetric("ScreenSelectCourse","HelpText")
#define TIMER_SECONDS		THEME->GetMetricI("ScreenSelectCourse","TimerSeconds")


const float TWEEN_TIME		= 0.5f;
const int NUM_SCORE_DIGITS  = 9;

static const ScreenMessage	SM_AllowOptionsMenuRepeat	= ScreenMessage(SM_User+1);



ScreenSelectCourse::ScreenSelectCourse( CString sClassName ) : ScreenWithMenuElements( sClassName )
{
	LOG->Trace( "ScreenSelectCourse::ScreenSelectCourse()" );

	/* Finish any previous stage.  It's OK to call this when we havn't played a stage yet. */
	GAMESTATE->FinishStage();

	SOUND->PlayMusic( THEME->GetPathToS("ScreenSelectCourse music") );

	m_MusicWheel.Load();

	m_bMadeChoice = false;
	m_bGoToOptions = false;
	m_bAllowOptionsMenuRepeat = false;
	
	m_sprExplanation.Load( THEME->GetPathToG("ScreenSelectCourse explanation") );
	m_sprExplanation.SetXY( EXPLANATION_X, EXPLANATION_Y );
	this->AddChild( &m_sprExplanation );

	m_Banner.SetXY( BANNER_X, BANNER_Y );
	m_Banner.ScaleToClipped( BANNER_WIDTH, BANNER_HEIGHT );
	this->AddChild( &m_Banner );

	m_sprBannerFrame.Load( THEME->GetPathToG("ScreenSelectCourse banner frame") );
	m_sprBannerFrame.SetXY( BANNER_FRAME_X, BANNER_FRAME_Y );
	this->AddChild( &m_sprBannerFrame );

	m_textNumSongs.LoadFromNumbers( THEME->GetPathToN("ScreenSelectCourse num songs") );
	m_textNumSongs.SetShadowLength( 0 );
	m_textNumSongs.SetXY( STAGES_X, STAGES_Y );
	this->AddChild( &m_textNumSongs );

	m_textTime.LoadFromNumbers( THEME->GetPathToN("ScreenSelectCourse total time") );
	m_textTime.SetShadowLength( 0 );
	m_textTime.SetXY( TIME_X, TIME_Y );
	this->AddChild( &m_textTime );

	m_CourseContentsFrame.SetName( "Contents" );
	SET_XY( m_CourseContentsFrame );
	this->AddChild( &m_CourseContentsFrame );

	m_MusicWheel.SetName( "MusicWheel", "Wheel" );
	m_MusicWheel.SetXY( WHEEL_X, WHEEL_Y );
	this->AddChild( &m_MusicWheel );

	for( int p=0; p<NUM_PLAYERS; p++ )
	{
		if( !GAMESTATE->IsHumanPlayer((PlayerNumber)p) )
			continue;	// skip

		m_sprHighScoreFrame[p].Load( THEME->GetPathToG(ssprintf("ScreenSelectCourse score frame p%d",p+1)) );
		m_sprHighScoreFrame[p].SetXY( SCORE_X(p), SCORE_Y(p) );
		this->AddChild( &m_sprHighScoreFrame[p] );

		m_HighScore[p].SetXY( SCORE_X(p), SCORE_Y(p) );
		m_HighScore[p].SetZoom( SCORE_ZOOM );
		m_HighScore[p].SetDiffuse( PlayerToColor(p) );
		this->AddChild( &m_HighScore[p] );
	}	

#ifdef _XBOX
	//shorten filenames for FATX
	m_sprOptionsMessage.Load( THEME->GetPathToG("ScreenSelectCourse opt message 1x2") );
#else
	m_sprOptionsMessage.Load( THEME->GetPathToG("ScreenSelectCourse options message 1x2") );
#endif

	m_sprOptionsMessage.StopAnimating();
	m_sprOptionsMessage.SetXY( CENTER_X, CENTER_Y );
	m_sprOptionsMessage.SetZoomY( 0 );
	m_sprOptionsMessage.SetDiffuse( RageColor(1,1,1,0) );
//	this->AddChild( &m_sprOptionsMessage );	// draw and update manually


	m_soundOptionsChange.Load( THEME->GetPathToS("ScreenSelectCourse options") );
	m_soundChangeNotes.Load( THEME->GetPathToS("ScreenSelectCourse difficulty") );


	SOUND->PlayOnceFromDir( ANNOUNCER->GetPathTo("select course intro") );

	SOUND->PlayMusic( THEME->GetPathToS("ScreenSelectCourse music") );

	UpdateOptionsDisplays();

	AfterCourseChange();
	TweenOnScreen();

	this->SortByZ();
}


ScreenSelectCourse::~ScreenSelectCourse()
{
	LOG->Trace( "ScreenSelectCourse::~ScreenSelectCourse()" );

}

void ScreenSelectCourse::DrawPrimitives()
{
	Screen::DrawPrimitives();
	m_sprOptionsMessage.Draw();
}

void ScreenSelectCourse::Update( float fDelta )
{
	Screen::Update( fDelta );
	m_sprOptionsMessage.Update( fDelta );	
}

void ScreenSelectCourse::TweenOnScreen()
{
	m_sprExplanation.Command(	"Sleep,0.5;AddX,-640;BounceEnd,0.5;AddX,640" );
	m_sprBannerFrame.Command(	"AddX,-640;BounceEnd,0.5;AddX,640" );
	m_Banner.Command(			"AddX,-640;BounceEnd,0.5;AddX,640" );
	m_textNumSongs.Command(		"AddX,-640;BounceEnd,0.5;AddX,640" );
	m_textTime.Command(			"AddX,-640;BounceEnd,0.5;AddX,640" );
	m_CourseContentsFrame.Command( "ZoomY,0;BounceEnd,0.5;ZoomY,1" );
	for( int p=0; p<NUM_PLAYERS; p++ )
	{
		m_sprHighScoreFrame[p].Command( "AddX,640;BounceEnd,0.5;AddX,-640" );
		m_HighScore[p].Command( "AddX,640;BounceEnd,0.5;AddX,-640" );
	}
	m_MusicWheel.TweenOnScreen();
}

void ScreenSelectCourse::TweenOffScreen()
{
	m_sprExplanation.Command( "BounceBegin,0.5;AddX,-640" );
	m_sprBannerFrame.Command( "BounceBegin,0.5;AddX,-640" );
	m_Banner.Command( "BounceBegin,0.5;AddX,-640" );
	m_textNumSongs.Command( "BounceBegin,0.5;AddX,-640" );
	m_textTime.Command( "BounceBegin,0.5;AddX,-640" );
	m_CourseContentsFrame.Command( "Linear,0.5;ZoomY,0" );
	for( int p=0; p<NUM_PLAYERS; p++ )
	{
		m_sprHighScoreFrame[p].Command( "BounceBegin,0.5;AddX,640" );
		m_HighScore[p].Command( "BounceBegin,0.5;AddX,640" );
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

	/* XXX: What's the difference between this and StyleI.player? */
	PlayerNumber pn = GAMESTATE->GetCurrentStyleDef()->ControllerToPlayerNumber( GameI.controller );
	if( !GAMESTATE->IsHumanPlayer(pn) )
		return;

	if( m_bMadeChoice  &&  MenuI.IsValid()  &&  MenuI.button == MENU_BUTTON_START  &&  !GAMESTATE->IsExtraStage()  &&  !GAMESTATE->IsExtraStage2() )
	{
		if(m_bGoToOptions) return; /* got it already */

		if( !m_bAllowOptionsMenuRepeat &&
			(type == IET_SLOW_REPEAT || type == IET_FAST_REPEAT ))
			return; /* not allowed yet */

		m_bGoToOptions = true;
		m_sprOptionsMessage.SetState( 1 );
		SCREENMAN->PlayStartSound();
		return;
	}
	
	if( IsTransitioning() )	return;		// ignore

	if( m_bMadeChoice )
		return;

	if( CodeDetector::DetectAndAdjustMusicOptions(GameI.controller) )
	{
		m_soundOptionsChange.Play();
		UpdateOptionsDisplays();
		return;
	}

	if( CodeDetector::EnteredEasierDifficulty(GameI.controller) )
	{
		if( GAMESTATE->ChangeCourseDifficulty( pn, -1 ) )
		{
			m_soundChangeNotes.Play();
			SCREENMAN->PostMessageToTopScreen(SM_SongChanged,0);
		}
	}

	if( CodeDetector::EnteredHarderDifficulty(GameI.controller) )
	{
		if( GAMESTATE->ChangeCourseDifficulty( pn, +1 ) )
		{
			m_soundChangeNotes.Play();
			SCREENMAN->PostMessageToTopScreen(SM_SongChanged,0);
		}
	}


	Screen::Input( DeviceI, type, GameI, MenuI, StyleI );	// default input handler
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
	// this needs to check whether valid Steps are selected!
	m_MusicWheel.Select();

	switch( m_MusicWheel.GetSelectedType() )
	{
	case TYPE_COURSE:
		SOUND->PlayOnceFromDir( ANNOUNCER->GetPathTo("select course comment general") );
	
		TweenOffScreen();

		SCREENMAN->PlayStartSound();

		m_bMadeChoice = true;

		float fShowSeconds = m_Out.GetLengthSeconds();

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


		StartTransitioning( SM_GoToNextScreen );

		Course* pCourse = m_MusicWheel.GetSelectedCourse();
		GAMESTATE->m_pCurCourse = pCourse;

		// Apply number of lives without turning on LIFE_BATTERY.
		// Don't turn on LIFE_BATTERY because it will override
		// the user's choice if they Back out of gameplay or are in 
		// event mode.
		if( pCourse->m_iLives != -1 )
			GAMESTATE->m_SongOptions.m_iBatteryLives = GAMESTATE->m_pCurCourse->m_iLives;

		break;
	}
}


void ScreenSelectCourse::MenuBack( PlayerNumber pn )
{
	SOUND->StopMusic();

	Back( SM_GoToPrevScreen );
}


void ScreenSelectCourse::AfterCourseChange()
{
	switch( m_MusicWheel.GetSelectedType() )
	{
	case TYPE_COURSE:
		{
			Course* pCourse = m_MusicWheel.GetSelectedCourse();
			const StepsType &st = GAMESTATE->GetCurrentStyleDef()->m_StepsType;

			m_textNumSongs.SetText( ssprintf("%d", pCourse->GetEstimatedNumStages()) );
			float fTotalSeconds;
			if( pCourse->GetTotalSeconds(fTotalSeconds) )
				m_textTime.SetText( SecondsToMMSSMsMs(fTotalSeconds) );
			else
				m_textTime.SetText( "xx:xx.xx" );	// The numbers format doesn't have a '?'.  Is there a better solution?

			m_Banner.LoadFromCourse( pCourse );

			m_CourseContentsFrame.SetFromCourse( pCourse );
			m_CourseContentsFrame.TweenInAfterChangedCourse();

			ASSERT(pCourse);
			for( int p=0; p<NUM_PLAYERS; p++ )
			{
				CourseDifficulty cd = GAMESTATE->m_PreferredCourseDifficulty[p];

				Profile* pProfile;
				if( PROFILEMAN->IsUsingProfile( (PlayerNumber)p ) )
					pProfile = PROFILEMAN->GetProfile((PlayerNumber)p);
				else
					pProfile = PROFILEMAN->GetMachineProfile();


				/* Courses are scored by survive time, dance points, 
				 * percent, and normal score.  Every last mother will
				 * have an opinion on which should be used here for
				 * each of oni, endless, nonstop --
				 * should this choice be an option or a metric? */
				const HighScoreList& hsl = pProfile->GetCourseHighScoreList( pCourse, st, cd );
				if ( pCourse->IsOni() || pCourse->IsEndless() )
				{
					/* use survive time */
					float fSurviveSeconds = hsl.GetTopScore().fSurviveSeconds;
					CString s = SecondsToMMSSMsMs(fSurviveSeconds);

					/* dim the inital unsignificant digits */
					/*XXX we'd like to have a dimmed ':' and '.', but 
					 *    BitmapText only supports dimmed '0' (which
					 *    is used for the ' ' char) */
					unsigned i = 0;
					for ( ; i<s.length(); i++ ) {
						switch (s[i]) {
							case ':':
							case '.':
							case '0': s[i] = ' '; break;
							default: i = s.length();
						}
					}

					m_HighScore[p].SetText( s );
				}
				else /* pCourse->IsNonStop() */
				{
					/* use score */
					int iScore = hsl.GetTopScore().iScore;
					m_HighScore[p].SetText( ssprintf("%*i", NUM_SCORE_DIGITS, iScore) );
				}

			}

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
