#include "stdafx.h"
/*
-----------------------------------------------------------------------------
 Class: ScreenSelectMusic

 Desc: See header.

 Copyright (c) 2001-2002 by the person(s) listed below.  All rights reserved.
	Chris Danford
-----------------------------------------------------------------------------
*/


#include "ScreenSelectMusic.h"
#include "ScreenManager.h"
#include "PrefsManager.h"
#include "SongManager.h"
#include "GameManager.h"
#include "RageSoundManager.h"
#include "GameConstantsAndTypes.h"
#include "PrefsManager.h"
#include "RageLog.h"
#include "InputMapper.h"
#include "InputQueue.h"
#include "AnnouncerManager.h"
#include "InputMapper.h"
#include "GameState.h"
#include "CodeDetector.h"
#include <math.h>
#include "GameConstantsAndTypes.h"
#include "ThemeManager.h"


#define BANNER_FRAME_X			THEME->GetMetricF("ScreenSelectMusic","BannerFrameX")
#define BANNER_FRAME_Y			THEME->GetMetricF("ScreenSelectMusic","BannerFrameY")
#define BANNER_X				THEME->GetMetricF("ScreenSelectMusic","BannerX")
#define BANNER_Y				THEME->GetMetricF("ScreenSelectMusic","BannerY")
#define BANNER_WIDTH			THEME->GetMetricF("ScreenSelectMusic","BannerWidth")
#define BANNER_HEIGHT			THEME->GetMetricF("ScreenSelectMusic","BannerHeight")
#define BPM_X					THEME->GetMetricF("ScreenSelectMusic","BPMX")
#define BPM_Y					THEME->GetMetricF("ScreenSelectMusic","BPMY")
#define BPM_ZOOM				THEME->GetMetricF("ScreenSelectMusic","BPMZoom")
#define STAGE_X					THEME->GetMetricF("ScreenSelectMusic","StageX")
#define STAGE_Y					THEME->GetMetricF("ScreenSelectMusic","StageY")
#define STAGE_ZOOM				THEME->GetMetricF("ScreenSelectMusic","StageZoom")
#define CD_TITLE_X				THEME->GetMetricF("ScreenSelectMusic","CDTitleX")
#define CD_TITLE_Y				THEME->GetMetricF("ScreenSelectMusic","CDTitleY")
#define DIFFICULTY_FRAME_X( p )	THEME->GetMetricF("ScreenSelectMusic",ssprintf("DifficultyFrameP%dX",p+1))
#define DIFFICULTY_FRAME_Y( p )	THEME->GetMetricF("ScreenSelectMusic",ssprintf("DifficultyFrameP%dY",p+1))
#define DIFFICULTY_ICON_X( p )	THEME->GetMetricF("ScreenSelectMusic",ssprintf("DifficultyIconP%dX",p+1))
#define DIFFICULTY_ICON_Y( p )	THEME->GetMetricF("ScreenSelectMusic",ssprintf("DifficultyIconP%dY",p+1))
#define RADAR_X					THEME->GetMetricF("ScreenSelectMusic","RadarX")
#define RADAR_Y					THEME->GetMetricF("ScreenSelectMusic","RadarY")
#define SORT_ICON_X				THEME->GetMetricF("ScreenSelectMusic","SortIconX")
#define SORT_ICON_Y				THEME->GetMetricF("ScreenSelectMusic","SortIconY")
#define SCORE_FRAME_X( p )		THEME->GetMetricF("ScreenSelectMusic",ssprintf("ScoreFrameP%dX",p+1))
#define SCORE_FRAME_Y( p )		THEME->GetMetricF("ScreenSelectMusic",ssprintf("ScoreFrameP%dY",p+1))
#define SCORE_X( p )			THEME->GetMetricF("ScreenSelectMusic",ssprintf("ScoreP%dX",p+1))
#define SCORE_Y( p )			THEME->GetMetricF("ScreenSelectMusic",ssprintf("ScoreP%dY",p+1))
#define SCORE_ZOOM				THEME->GetMetricF("ScreenSelectMusic","ScoreZoom")
#define METER_FRAME_X( p )		THEME->GetMetricF("ScreenSelectMusic",ssprintf("MeterFrameP%dX",p+1))
#define METER_FRAME_Y( p )		THEME->GetMetricF("ScreenSelectMusic",ssprintf("MeterFrameP%dY",p+1))
#define METER_X( p )			THEME->GetMetricF("ScreenSelectMusic",ssprintf("MeterP%dX",p+1))
#define METER_Y( p )			THEME->GetMetricF("ScreenSelectMusic",ssprintf("MeterP%dY",p+1))
#define WHEEL_X					THEME->GetMetricF("ScreenSelectMusic","WheelX")
#define WHEEL_Y					THEME->GetMetricF("ScreenSelectMusic","WheelY")
#define PLAYER_OPTIONS_X( p )	THEME->GetMetricF("ScreenSelectMusic",ssprintf("PlayerOptionsP%dX",p+1))
#define PLAYER_OPTIONS_Y( p )	THEME->GetMetricF("ScreenSelectMusic",ssprintf("PlayerOptionsP%dY",p+1))
#define SONG_OPTIONS_X			THEME->GetMetricF("ScreenSelectMusic","SongOptionsX")
#define SONG_OPTIONS_Y			THEME->GetMetricF("ScreenSelectMusic","SongOptionsY")
#define OPTION_ICONS_X( p )		THEME->GetMetricF("ScreenSelectMusic",ssprintf("OptionIconsP%dX",p+1))
#define OPTION_ICONS_Y( p )		THEME->GetMetricF("ScreenSelectMusic",ssprintf("OptionIconsP%dY",p+1))
#define HELP_TEXT				THEME->GetMetric("ScreenSelectMusic","HelpText")
#define TIMER_SECONDS			THEME->GetMetricI("ScreenSelectMusic","TimerSeconds")
#define SCORE_CONNECTED_TO_MUSIC_WHEEL	THEME->GetMetricB("ScreenSelectMusic","ScoreConnectedToMusicWheel")
#define SAMPLE_MUSIC_DELAY		THEME->GetMetricF("ScreenSelectMusic","SampleMusicDelay")

const float TWEEN_TIME		= 0.5f;

const ScreenMessage	SM_AllowOptionsMenuRepeat	= ScreenMessage(SM_User+1);



ScreenSelectMusic::ScreenSelectMusic()
{
	LOG->Trace( "ScreenSelectMusic::ScreenSelectMusic()" );

	CodeDetector::RefreshCacheItems();

	int p;

	m_Menu.Load(
		THEME->GetPathTo("BGAnimations","select music"), 
		THEME->GetPathTo("Graphics","select music top edge"),
		HELP_TEXT, true, true, TIMER_SECONDS 
		);
	this->AddChild( &m_Menu );

	// these guys get loaded SetSong and TweenToSong
	m_Banner.SetXY( BANNER_X, BANNER_Y );
	m_Banner.SetCroppedSize( BANNER_WIDTH, BANNER_HEIGHT );
	this->AddChild( &m_Banner );

	m_sprBannerFrame.Load( THEME->GetPathTo("Graphics","select music banner frame") );
	m_sprBannerFrame.SetXY( BANNER_FRAME_X, BANNER_FRAME_Y );
	this->AddChild( &m_sprBannerFrame );

	m_BPMDisplay.SetXY( BPM_X, BPM_Y );
	m_BPMDisplay.SetZoom( BPM_ZOOM );
	this->AddChild( &m_BPMDisplay );

	m_StageDisplay.SetZoom( STAGE_ZOOM );
	m_StageDisplay.SetXY( STAGE_X, STAGE_Y );
	m_StageDisplay.Refresh();
	this->AddChild( &m_StageDisplay );

	m_sprCDTitle.Load( THEME->GetPathTo("Graphics","fallback cd title") );
	m_sprCDTitle.TurnShadowOff();
	m_sprCDTitle.SetXY( CD_TITLE_X, CD_TITLE_Y );
	this->AddChild( &m_sprCDTitle );

	for( p=0; p<NUM_PLAYERS; p++ )
	{
		if( !GAMESTATE->IsPlayerEnabled(p) )
			continue;	// skip

		m_sprDifficultyFrame[p].Load( THEME->GetPathTo("Graphics","select music difficulty frame 2x1") );
		m_sprDifficultyFrame[p].SetXY( DIFFICULTY_FRAME_X(p), DIFFICULTY_FRAME_Y(p) );
		m_sprDifficultyFrame[p].StopAnimating();
		m_sprDifficultyFrame[p].SetState( p );
		this->AddChild( &m_sprDifficultyFrame[p] );

		m_DifficultyIcon[p].Load( THEME->GetPathTo("graphics","select music difficulty icons 1x5") );
		m_DifficultyIcon[p].SetXY( DIFFICULTY_ICON_X(p), DIFFICULTY_ICON_Y(p) );
		this->AddChild( &m_DifficultyIcon[p] );

		m_AutoGenIcon[p].Load( THEME->GetPathTo("graphics","select music autogen icon") );
		m_AutoGenIcon[p].SetXY( DIFFICULTY_ICON_X(p), DIFFICULTY_ICON_Y(p) );
		this->AddChild( &m_AutoGenIcon[p] );
	}

	m_GrooveRadar.SetXY( RADAR_X, RADAR_Y );
	this->AddChild( &m_GrooveRadar );

//	m_OptionIcons.SetXY( OPTION_ICONS_X, OPTION_ICONS_Y );
//	this->AddChild( &m_OptionIcons );


	m_textSongOptions.LoadFromFont( THEME->GetPathTo("Fonts","normal") );
	m_textSongOptions.SetXY( SONG_OPTIONS_X, SONG_OPTIONS_Y );
	m_textSongOptions.SetZoom( 0.5f );
	if( GAMESTATE->IsExtraStage() || GAMESTATE->IsExtraStage2() )
		m_textSongOptions.SetEffectCamelion( 2.5f, RageColor(1,0,0,1), RageColor(1,1,1,1) );	// blink red
	m_textSongOptions.SetDiffuse( RageColor(1,1,1,1) );	// white
	this->AddChild( &m_textSongOptions );

/*	
	for( p=0; p<NUM_PLAYERS; p++ )
	{
		if( !GAMESTATE->IsPlayerEnabled(p) )
			continue;

		m_textPlayerOptions[p].LoadFromFont( THEME->GetPathTo("Fonts","normal") );
		m_textPlayerOptions[p].SetXY( PLAYER_OPTIONS_X(p), PLAYER_OPTIONS_Y(p) );
		m_textPlayerOptions[p].SetZoom( 0.5f );
		m_textPlayerOptions[p].SetHorizAlign( p==PLAYER_1 ? Actor::align_left : Actor::align_right );
		m_textPlayerOptions[p].SetVertAlign( Actor::align_middle );
		if( GAMESTATE->IsExtraStage() || GAMESTATE->IsExtraStage2() )
			m_textPlayerOptions[p].SetEffectCamelion( 2.5f, RageColor(1,0,0,1), RageColor(1,1,1,1) );	// blink red
		m_textPlayerOptions[p].SetDiffuse( RageColor(1,1,1,1) );	// white
		this->AddChild( &m_textPlayerOptions[p] );
	}
*/
	for( p=0; p<NUM_PLAYERS; p++ )
	{
		if( !GAMESTATE->IsPlayerEnabled(p) )
			continue;

		m_OptionIconRow[p].SetXY( OPTION_ICONS_X(p), OPTION_ICONS_Y(p) );
		m_OptionIconRow[p].Refresh( (PlayerNumber)p );
		this->AddChild( &m_OptionIconRow[p] );

		m_sprMeterFrame[p].Load( THEME->GetPathTo("Graphics","select music meter frame") );
		m_sprMeterFrame[p].SetXY( METER_FRAME_X(p), METER_FRAME_Y(p) );
		m_sprMeterFrame[p].StopAnimating();
		m_sprMeterFrame[p].SetState( p );
		this->AddChild( &m_sprMeterFrame[p] );

		m_FootMeter[p].SetXY( METER_X(p), METER_Y(p) );
		m_FootMeter[p].SetShadowLength( 2 );
		this->AddChild( &m_FootMeter[p] );
	}

	m_MusicWheel.SetXY( WHEEL_X, WHEEL_Y );
	this->AddChild( &m_MusicWheel );

	for( p=0; p<NUM_PLAYERS; p++ )
	{
		if( !GAMESTATE->IsPlayerEnabled((PlayerNumber)p) )
			continue;	// skip

		m_sprHighScoreFrame[p].Load( THEME->GetPathTo("Graphics","select music score frame 1x2") );
		m_sprHighScoreFrame[p].StopAnimating();
		m_sprHighScoreFrame[p].SetState( p );
		m_sprHighScoreFrame[p].SetXY( SCORE_X(p), SCORE_Y(p) );
		this->AddChild( &m_sprHighScoreFrame[p] );

		m_HighScore[p].SetXY( SCORE_X(p), SCORE_Y(p) );
		m_HighScore[p].SetZoom( SCORE_ZOOM );
		m_HighScore[p].SetDiffuse( PlayerToColor(p) );
		this->AddChild( &m_HighScore[p] );
	}	

	m_MusicSortDisplay.SetXY( SORT_ICON_X, SORT_ICON_Y );
	m_MusicSortDisplay.Set( GAMESTATE->m_SongSortOrder );
	this->AddChild( &m_MusicSortDisplay );


	m_sprOptionsMessage.Load( THEME->GetPathTo("Graphics","select music options message") );
	m_sprOptionsMessage.StopAnimating();
	m_sprOptionsMessage.SetXY( CENTER_X, CENTER_Y );
	m_sprOptionsMessage.SetZoom( 1 );
	m_sprOptionsMessage.SetDiffuse( RageColor(1,1,1,0) );
	this->AddChild( &m_sprOptionsMessage );


	m_soundSelect.Load( THEME->GetPathTo("Sounds","menu start") );
	m_soundChangeNotes.Load( THEME->GetPathTo("Sounds","select music notes") );
	m_soundOptionsChange.Load( THEME->GetPathTo("Sounds","select music options") );
	m_soundLocked.Load( THEME->GetPathTo("Sounds","select music locked") );

	SOUNDMAN->PlayOnceFromDir( ANNOUNCER->GetPathTo("select music intro") );

	m_bMadeChoice = false;
	m_bGoToOptions = false;
	m_fPlaySampleCountdown = 0;
	m_bAllowOptionsMenu = m_bAllowOptionsMenuRepeat = false;

	UpdateOptionsDisplays();

	AfterMusicChange();
	TweenOnScreen();
	m_Menu.TweenOnScreenFromMenu( SM_None );
}


ScreenSelectMusic::~ScreenSelectMusic()
{
	LOG->Trace( "ScreenSelectMusic::~ScreenSelectMusic()" );

}

void ScreenSelectMusic::DrawPrimitives()
{
	m_Menu.DrawBottomLayer();
	Screen::DrawPrimitives();
	m_Menu.DrawTopLayer();
}

void ScreenSelectMusic::TweenOnScreen()
{
	int p;

	m_sprBannerFrame.FadeOn( 0, "bounce left", TWEEN_TIME );
	m_Banner.FadeOn( 0, "bounce left", TWEEN_TIME );
	m_BPMDisplay.FadeOn( 0, "bounce left", TWEEN_TIME );
	m_StageDisplay.FadeOn( 0, "bounce left", TWEEN_TIME );
	m_sprCDTitle.FadeOn( 0, "bounce left", TWEEN_TIME );

	for( p=0; p<NUM_PLAYERS; p++ )
	{
		m_sprDifficultyFrame[p].FadeOn( 0, "fade", TWEEN_TIME );
		m_sprMeterFrame[p].FadeOn( 0, "fade", TWEEN_TIME );
	}

	m_GrooveRadar.TweenOnScreen();
	
	m_textSongOptions.FadeOn( 0, "fade", TWEEN_TIME );
	
	for( p=0; p<NUM_PLAYERS; p++ )
	{		
		m_OptionIconRow[p].FadeOn( 0, "foldy", TWEEN_TIME );
//		fOriginalZoomY = m_textPlayerOptions[p].GetZoomY();
//		m_textPlayerOptions[p].BeginTweening( TWEEN_TIME );
//		m_textPlayerOptions[p].SetTweenZoomY( fOriginalZoomY );

		m_DifficultyIcon[p].FadeOn( 0, "foldy", TWEEN_TIME );
		m_AutoGenIcon[p].FadeOn( 0, "foldy", TWEEN_TIME );

		m_FootMeter[p].FadeOn( 0, "foldy", TWEEN_TIME );
	}

	m_MusicSortDisplay.FadeOn( 0, "fade", TWEEN_TIME );


	for( p=0; p<NUM_PLAYERS; p++ )
	{
		m_sprHighScoreFrame[p].FadeOn( 0, SCORE_CONNECTED_TO_MUSIC_WHEEL?"accelerate right":"accelerate left", TWEEN_TIME );
		m_HighScore[p].FadeOn( 0, SCORE_CONNECTED_TO_MUSIC_WHEEL?"accelerate right":"accelerate left", TWEEN_TIME );
	}

	m_MusicWheel.TweenOnScreen();
}

void ScreenSelectMusic::TweenOffScreen()
{
	m_sprBannerFrame.FadeOff( 0, "bounce left", TWEEN_TIME*2 );
	m_Banner.FadeOff( 0, "bounce left", TWEEN_TIME*2 );
	m_BPMDisplay.FadeOff( 0, "bounce left", TWEEN_TIME*2 );
	m_StageDisplay.FadeOff( 0, "bounce left", TWEEN_TIME*2 );
	m_sprCDTitle.FadeOff( 0, "bounce left", TWEEN_TIME*2 );

	int p;
	for( p=0; p<NUM_PLAYERS; p++ )
	{
		m_sprDifficultyFrame[p].FadeOff( 0, "fade", TWEEN_TIME );
		m_sprMeterFrame[p].FadeOff( 0, "fade", TWEEN_TIME );
	}


	m_GrooveRadar.TweenOffScreen();

	m_textSongOptions.FadeOff( 0, "foldy", TWEEN_TIME );

	for( p=0; p<NUM_PLAYERS; p++ )
	{
		m_OptionIconRow[p].FadeOff( 0, "foldy", TWEEN_TIME );
//		m_textPlayerOptions[p].FadeOff( 0, "fade", TWEEN_TIME );

		m_DifficultyIcon[p].FadeOff( 0, "foldy", TWEEN_TIME );
		m_AutoGenIcon[p].FadeOff( 0, "foldy", TWEEN_TIME );

		m_FootMeter[p].FadeOff( 0, "foldy", TWEEN_TIME );
	}

	m_MusicSortDisplay.FadeOff( 0, "fade", TWEEN_TIME );

	vector<Actor*> apActorsInScore;
	for( p=0; p<NUM_PLAYERS; p++ )
	{
		apActorsInScore.push_back( &m_sprHighScoreFrame[p] );
		apActorsInScore.push_back( &m_HighScore[p] );
	}
	for( unsigned i=0; i<apActorsInScore.size(); i++ )
	{
		apActorsInScore[i]->BeginTweening( TWEEN_TIME, TWEEN_BIAS_END );
		apActorsInScore[i]->SetTweenX( SCORE_CONNECTED_TO_MUSIC_WHEEL ? apActorsInScore[i]->GetX()+400 : apActorsInScore[i]->GetX()-400 );
	}

	m_MusicWheel.TweenOffScreen();
}

void ScreenSelectMusic::TweenScoreOnAndOffAfterChangeSort()
{
	if( !SCORE_CONNECTED_TO_MUSIC_WHEEL )
		return;	// do nothing

	/* XXX metric this with MusicWheel::TweenOnScreen */
	float factor = 0.25f;

	vector<Actor*> apActorsInScore;
	for( int p=0; p<NUM_PLAYERS; p++ )
	{
		apActorsInScore.push_back( &m_sprHighScoreFrame[p] );
		apActorsInScore.push_back( &m_HighScore[p] );
	}
	for( unsigned i=0; i<apActorsInScore.size(); i++ )
	{
		/* Grab the tween destination.  (If we're tweening, this is where
		 * it'll end up; otherwise it's the static position.) */
		Actor::TweenState original = apActorsInScore[i]->GetDestTweenState();

		apActorsInScore[i]->StopTweening();

		float fOriginalX = apActorsInScore[i]->GetX();
		apActorsInScore[i]->BeginTweening( factor*TWEEN_TIME, TWEEN_BIAS_END );		// tween off screen
		apActorsInScore[i]->SetTweenX( fOriginalX+400 );
		
		apActorsInScore[i]->BeginTweening( factor*0.5f );		// sleep

		/* Go back to where we were (or to where we were going.) */
		apActorsInScore[i]->BeginTweening( factor*1, TWEEN_BIAS_BEGIN );		// tween back on screen
		apActorsInScore[i]->SetTweenState(original);
	}
}

void ScreenSelectMusic::Update( float fDeltaTime )
{
	Screen::Update( fDeltaTime );

	if( m_fPlaySampleCountdown > 0 )
	{
		m_fPlaySampleCountdown -= fDeltaTime;
		/* Make sure we don't start the sample when rouletting is
		 * spinning down. */
		if( m_fPlaySampleCountdown <= 0 && !m_MusicWheel.IsRouletting() )
			this->PlayMusicSample();
	}

	float fNewRotation = m_sprCDTitle.GetRotationY()+PI*fDeltaTime/2;
	fNewRotation = fmodf( fNewRotation, PI*2 );
	m_sprCDTitle.SetRotationY( fNewRotation );
	if( fNewRotation > PI/2  &&  fNewRotation <= PI*3.0f/2 )
		m_sprCDTitle.SetDiffuse( RageColor(0.2f,0.2f,0.2f,1) );
	else
		m_sprCDTitle.SetDiffuse( RageColor(1,1,1,1) );
}

void ScreenSelectMusic::Input( const DeviceInput& DeviceI, InputEventType type, const GameInput &GameI, const MenuInput &MenuI, const StyleInput &StyleI )
{
	LOG->Trace( "ScreenSelectMusic::Input()" );
	
	if( MenuI.button == MENU_BUTTON_RIGHT || MenuI.button == MENU_BUTTON_LEFT )
	{
		if( !MenuI.IsValid() ) return;
		if( !GAMESTATE->IsPlayerEnabled(MenuI.player) ) return;

		/* If we're rouletting, hands off. */
		if(m_MusicWheel.IsRouletting())
			return;

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
		if(!m_bAllowOptionsMenu) return; /* not allowed */

		if( !m_bAllowOptionsMenuRepeat &&
			(type == IET_SLOW_REPEAT || type == IET_FAST_REPEAT ))
			return; /* not allowed yet */
		
		m_bGoToOptions = true;
		m_sprOptionsMessage.SetState( 1 );
		SOUNDMAN->PlayOnce( THEME->GetPathTo("Sounds","menu start") );
		return;
	}
	
	if( m_Menu.IsClosing() )	return;		// ignore

	if( m_bMadeChoice )
		return;

	PlayerNumber pn = GAMESTATE->GetCurrentStyleDef()->ControllerToPlayerNumber( GameI.controller );

	if( CodeDetector::EnteredEasierDifficulty(GameI.controller) )
	{
		if( GAMESTATE->IsExtraStage() || GAMESTATE->IsExtraStage2() )
			m_soundLocked.Play();
		else
			EasierDifficulty( pn );
		return;
	}
	if( CodeDetector::EnteredHarderDifficulty(GameI.controller) )
	{
		if( GAMESTATE->IsExtraStage() || GAMESTATE->IsExtraStage2() )
			m_soundLocked.Play();
		else
			HarderDifficulty( pn );
		return;
	}
	if( CodeDetector::EnteredNextSort(GameI.controller) )
	{
		if( ( GAMESTATE->IsExtraStage() && !PREFSMAN->m_bPickExtraStage ) || GAMESTATE->IsExtraStage2() )
			m_soundLocked.Play();
		else
			if( m_MusicWheel.NextSort() )
			{
				SOUNDMAN->StopMusic();

//				m_MusicSortDisplay.FadeOff( 0, "fade", TWEEN_TIME );

				TweenScoreOnAndOffAfterChangeSort();
			}
		return;
	}
	if( !GAMESTATE->IsExtraStage() && !GAMESTATE->IsExtraStage2() && CodeDetector::DetectAndAdjustOptions(GameI.controller) )
	{
		m_soundOptionsChange.Play();
		UpdateOptionsDisplays();
		return;
	}

	Screen::Input( DeviceI, type, GameI, MenuI, StyleI );	// default input handler
}


void ScreenSelectMusic::EasierDifficulty( PlayerNumber pn )
{
	LOG->Trace( "ScreenSelectMusic::EasierDifficulty( %d )", pn );

	if( !GAMESTATE->IsPlayerEnabled(pn) )
		return;
	if( m_arrayNotes[pn].empty() )
		return;
	if( m_iSelection[pn] == 0 )
		return;

	m_iSelection[pn]--;
	// the user explicity switched difficulties.  Update the preferred difficulty
	GAMESTATE->m_PreferredDifficulty[pn] = m_arrayNotes[pn][ m_iSelection[pn] ]->GetDifficulty();

	m_soundChangeNotes.Play();

	AfterNotesChange( pn );
}

void ScreenSelectMusic::HarderDifficulty( PlayerNumber pn )
{
	LOG->Trace( "ScreenSelectMusic::HarderDifficulty( %d )", pn );

	if( !GAMESTATE->IsPlayerEnabled(pn) )
		return;
	if( m_arrayNotes[pn].empty() )
		return;
	if( m_iSelection[pn] == int(m_arrayNotes[pn].size()-1) )
		return;

	m_iSelection[pn]++;
	// the user explicity switched difficulties.  Update the preferred difficulty
	GAMESTATE->m_PreferredDifficulty[pn] = m_arrayNotes[pn][ m_iSelection[pn] ]->GetDifficulty();

	m_soundChangeNotes.Play();

	AfterNotesChange( pn );
}

void ScreenSelectMusic::AdjustOptions()
{
	/* Find the easiest difficulty notes selected by either player. */
	Difficulty dc = DIFFICULTY_INVALID;
	for( int p=0; p<NUM_PLAYERS; p++ )
	{
		if( !GAMESTATE->IsPlayerEnabled(p) )
			continue;	// skip

		dc = min(dc, GAMESTATE->m_pCurNotes[p]->GetDifficulty());
	}

	/* In event mode, switch to fail-end-of-stage if either chose beginner or easy. */
	if(PREFSMAN->m_bEventMode && dc <= DIFFICULTY_EASY)
		GAMESTATE->m_SongOptions.m_FailType = SongOptions::FailType::FAIL_END_OF_SONG;

	/* Otherwise, if either chose beginner's steps, and this is the first stage,
	 * turn off failure completely (always give a second shot). */
	else if(dc == DIFFICULTY_BEGINNER)
	{
		/* Beginners get a freebie and then end-of-song. */
		if(GAMESTATE->m_iCurrentStageIndex == 0)
			GAMESTATE->m_SongOptions.m_FailType = SongOptions::FailType::FAIL_OFF;
		else if(!GAMESTATE->IsExtraStage() && !GAMESTATE->IsExtraStage2())
			GAMESTATE->m_SongOptions.m_FailType = SongOptions::FailType::FAIL_END_OF_SONG;
	}
	/* Easy is always end-of-song. */
	else if(dc == DIFFICULTY_EASY && !GAMESTATE->IsExtraStage() && !GAMESTATE->IsExtraStage2())
		GAMESTATE->m_SongOptions.m_FailType = SongOptions::FailType::FAIL_END_OF_SONG;
}

void ScreenSelectMusic::HandleScreenMessage( const ScreenMessage SM )
{
	Screen::HandleScreenMessage( SM );

	switch( SM )
	{
	case SM_AllowOptionsMenuRepeat:
		m_bAllowOptionsMenuRepeat = true;
		break;
	case SM_MenuTimer:
		if( m_MusicWheel.IsRouletting() )
		{
			MenuStart(PLAYER_INVALID);
			m_Menu.SetTimer( 15 );
		}
		else if( m_MusicWheel.GetSelectedType() != TYPE_SONG )
		{
			m_MusicWheel.StartRoulette();
			m_Menu.SetTimer( 15 );
		}
		else
		{
			MenuStart(PLAYER_INVALID);
		}
		break;
	case SM_GoToPrevScreen:
		SCREENMAN->SetNewScreen( "ScreenTitleMenu" );
		/* We may have stray SM_SongChanged messages from the music wheel.  We can't
		 * handle them anymore, since the title menu (and attract screens) reset
		 * the game state, so just discard them. */
		ClearMessageQueue();
		break;
	case SM_GoToNextScreen:
		if( m_bGoToOptions )
		{
			SCREENMAN->SetNewScreen( "ScreenPlayerOptions" );
		}
		else
		{
			SOUNDMAN->StopMusic();
			SCREENMAN->SetNewScreen( "ScreenStage" );
		}
		break;
	case SM_SongChanged:
		AfterMusicChange();
		break;
	case SM_SortOrderChanged:
		SortOrderChanged();
		break;
	}
}

void ScreenSelectMusic::MenuStart( PlayerNumber pn )
{
	if( pn != PLAYER_INVALID  &&
		INPUTMAPPER->IsButtonDown( MenuInput(pn, MENU_BUTTON_LEFT) )  &&
		INPUTMAPPER->IsButtonDown( MenuInput(pn, MENU_BUTTON_RIGHT) ) )
	{
		if( GAMESTATE->IsExtraStage() || GAMESTATE->IsExtraStage2() )
			m_soundLocked.Play();
		else
		{
			if( m_MusicWheel.NextSort() )
			{
				SOUNDMAN->StopMusic();

//				m_MusicSortDisplay.FadeOff( 0, "fade", TWEEN_TIME );

				TweenScoreOnAndOffAfterChangeSort();
			}
		}
		return;
	}


	// this needs to check whether valid Notes are selected!
	bool bResult = m_MusicWheel.Select();

	/* If false, we don't have a selection just yet. */
	if( !bResult )
		return;

	// a song was selected
	switch( m_MusicWheel.GetSelectedType() )
	{
	case TYPE_SONG: {
		if( !m_MusicWheel.GetSelectedSong()->HasMusic() )
		{
			/* TODO: gray these out. 
				*
				* XXX: also, make sure they're not selected by roulette */
			SCREENMAN->Prompt( SM_None, "ERROR:\n \nThis song does not have a music file\n and cannot be played." );
			return;
		}

		bool bIsNew = m_MusicWheel.GetSelectedSong()->IsNew();
		bool bIsHard = false;
		for( int p=0; p<NUM_PLAYERS; p++ )
		{
			if( !GAMESTATE->IsPlayerEnabled( (PlayerNumber)p ) )
				continue;	// skip
			if( GAMESTATE->m_pCurNotes[p]  &&  GAMESTATE->m_pCurNotes[p]->GetMeter() >= 10 )
				bIsHard = true;
		}

		if( bIsNew )
			SOUNDMAN->PlayOnceFromDir( ANNOUNCER->GetPathTo("select music comment new") );
		else if( bIsHard )
			SOUNDMAN->PlayOnceFromDir( ANNOUNCER->GetPathTo("select music comment hard") );
		else
			SOUNDMAN->PlayOnceFromDir( ANNOUNCER->GetPathTo("select music comment general") );


		TweenOffScreen();

		m_bMadeChoice = true;

		m_soundSelect.Play();

		if( !GAMESTATE->IsExtraStage()  &&  !GAMESTATE->IsExtraStage2() )
		{
			// show "hold START for options"
			m_sprOptionsMessage.SetDiffuse( RageColor(1,1,1,0) );
			m_sprOptionsMessage.BeginTweening( 0.25f );	// fade in
			m_sprOptionsMessage.SetTweenZoomY( 1 );
			m_sprOptionsMessage.SetTweenDiffuse( RageColor(1,1,1,1) );
			m_sprOptionsMessage.BeginTweening( 2.0f );	// sleep
			m_sprOptionsMessage.BeginTweening( 0.25f );	// fade out
			m_sprOptionsMessage.SetTweenDiffuse( RageColor(1,1,1,0) );
			m_sprOptionsMessage.SetTweenZoomY( 0 );

			m_bAllowOptionsMenu = true;
			/* Don't accept a held START for a little while, so it's not
			 * hit accidentally.  Accept an initial START right away, though,
			 * so we don't ignore deliberate fast presses (which would be
			 * annoying). */
			this->SendScreenMessage( SM_AllowOptionsMenuRepeat, 0.75f );
		}

		m_Menu.TweenOffScreenToBlack( SM_None, false );

		m_Menu.StopTimer();
		AdjustOptions();

		this->SendScreenMessage( SM_GoToNextScreen, 2.5f );
		break;
	}
	case TYPE_SECTION:
		break;
	case TYPE_ROULETTE:
		break;
	}

	if( GAMESTATE->IsExtraStage() && PREFSMAN->m_bPickExtraStage )
	{
		/* Check if user selected the real extra stage. */
		Song* pSong;
		Notes* pNotes;
		PlayerOptions po;
		SongOptions so;
		SONGMAN->GetExtraStageInfo( false, GAMESTATE->m_pCurSong->m_sGroupName, GAMESTATE->GetCurrentStyleDef(), pSong, pNotes, po, so );
		ASSERT(pSong);
		
		/* Enable 2nd extra stage if user chose the correct song */
		if( m_MusicWheel.GetSelectedSong() == pSong )
			GAMESTATE->m_bAllow2ndExtraStage = true;
		else
			GAMESTATE->m_bAllow2ndExtraStage = false;
	}
}


void ScreenSelectMusic::MenuBack( PlayerNumber pn )
{
	SOUNDMAN->StopMusic();

	m_Menu.TweenOffScreenToBlack( SM_GoToPrevScreen, true );
}

void ScreenSelectMusic::AfterNotesChange( PlayerNumber pn )
{
	if( !GAMESTATE->IsPlayerEnabled(pn) )
		return;
	
	m_iSelection[pn] = clamp( m_iSelection[pn], 0, int(m_arrayNotes[pn].size()-1) );	// bounds clamping

	Notes* pNotes = m_arrayNotes[pn].empty()? NULL: m_arrayNotes[pn][m_iSelection[pn]];

	GAMESTATE->m_pCurNotes[pn] = pNotes;

//	m_BPMDisplay.SetZoomY( 0 );
//	m_BPMDisplay.BeginTweening( 0.2f );
//	m_BPMDisplay.SetTweenZoomY( 1.2f );

	Notes* m_pNotes = GAMESTATE->m_pCurNotes[pn];
	
	if( m_pNotes && SONGMAN->IsUsingMemoryCard(pn) )
		m_HighScore[pn].SetScore( m_pNotes->m_MemCardScores[pn].fScore );

	m_DifficultyIcon[pn].SetFromNotes( pn, pNotes );
	if( pNotes && pNotes->IsAutogen() )
	{
		m_AutoGenIcon[pn].SetEffectCamelion();
	}
	else
	{
		m_AutoGenIcon[pn].SetEffectNone();
		m_AutoGenIcon[pn].SetDiffuse( RageColor(1,1,1,0) );
	}
	m_FootMeter[pn].SetFromNotes( pNotes );
	m_GrooveRadar.SetFromNotes( pn, pNotes );
	m_MusicWheel.NotesChanged( pn );
}

void ScreenSelectMusic::AfterMusicChange()
{
	m_Menu.StallTimer();

	Song* pSong = m_MusicWheel.GetSelectedSong();
	GAMESTATE->m_pCurSong = pSong;

	m_StageDisplay.Refresh();

	int pn;
	for( pn = 0; pn < NUM_PLAYERS; ++pn)
		m_arrayNotes[pn].clear();

	/* If we're rouletting, and we're moving fast, don't touch the banner. */
	bool no_banner_change = false;
	if(m_MusicWheel.IsMoving()) 
	{
		/* We're moving fast.  Don't change banners if we're rouletting, or if
		 * we've been told to never change banners when moving fast.
		 *
		 * XXX: When we're not changing banners and not rouletting, show some
		 * kind of "moving fast" fallback banner.  (When rouletting, just keep
		 * showing the roulette banner.) */
		if(m_MusicWheel.IsRouletting() ||
			(m_MusicWheel.IsMoving() && !PREFSMAN->m_bChangeBannersWhenFast))
			no_banner_change = true;
	}

	switch( m_MusicWheel.GetSelectedType() )
	{
	case TYPE_SECTION:
		{	
			CString sGroup = m_MusicWheel.GetSelectedSection();
			for( int p=0; p<NUM_PLAYERS; p++ )
				m_iSelection[p] = -1;

			if(!no_banner_change)
				m_Banner.LoadFromGroup( sGroup );	// if this isn't a group, it'll default to the fallback banner
			m_BPMDisplay.SetBPMRange( 0, 0 );
			m_sprCDTitle.UnloadTexture();
		}
		break;
	case TYPE_SONG:
		{
			SOUNDMAN->StopMusic();
			m_fPlaySampleCountdown = SAMPLE_MUSIC_DELAY;

			for( int pn = 0; pn < NUM_PLAYERS; ++pn) {
				pSong->GetNotes( m_arrayNotes[pn], GAMESTATE->GetCurrentStyleDef()->m_NotesType );
				SortNotesArrayByDifficulty( m_arrayNotes[pn] );
			}

			if(!no_banner_change)
				m_Banner.LoadFromSong( pSong );

			float fMinBPM, fMaxBPM;
			pSong->GetMinMaxBPM( fMinBPM, fMaxBPM );
			m_BPMDisplay.SetBPMRange( fMinBPM, fMaxBPM );

			if( pSong->HasCDTitle() )
				m_sprCDTitle.Load( pSong->GetCDTitlePath() );
			else
				m_sprCDTitle.Load( THEME->GetPathTo("Graphics","fallback cd title") );
			for( int p=0; p<NUM_PLAYERS; p++ )
			{
				if( !GAMESTATE->IsPlayerEnabled( PlayerNumber(p) ) )
					continue;
				for( unsigned i=0; i<m_arrayNotes[p].size(); i++ )
				{
					if( m_arrayNotes[p][i]->GetDifficulty() == GAMESTATE->m_PreferredDifficulty[p] )
					{
						m_iSelection[p] = i;
						break;
					}
				}

				m_iSelection[p] = clamp( m_iSelection[p], 0, int(m_arrayNotes[p].size()) ) ;
			}
		}
		break;
	case TYPE_ROULETTE:
		if(!no_banner_change)
			m_Banner.LoadRoulette();
		m_BPMDisplay.SetBPMRange( 0, 0 );
		m_sprCDTitle.UnloadTexture();
		break;
	}

	for( int p=0; p<NUM_PLAYERS; p++ )
	{
		AfterNotesChange( (PlayerNumber)p );
	}

	/* Make sure we never start the sample when moving fast. */
	if(m_MusicWheel.IsMoving())
		m_fPlaySampleCountdown = 0;
}

void ScreenSelectMusic::PlayMusicSample()
{
	//LOG->Trace( "ScreenSelectSong::PlayMusicSample()" );

	Song* pSong = m_MusicWheel.GetSelectedSong();
	if( pSong  &&  pSong->HasMusic() )
	{
		SOUNDMAN->PlayMusic(pSong->GetMusicPath(), true,
			pSong->m_fMusicSampleStartSeconds,
			pSong->m_fMusicSampleLengthSeconds,
			1.5f); /* fade out for 1.5 seconds */
	}
//	else
//		SOUNDMAN->PlayMusic( THEME->GetPathTo("Sounds","select music music") );
}

void ScreenSelectMusic::UpdateOptionsDisplays()
{
//	m_OptionIcons.Load( GAMESTATE->m_PlayerOptions, &GAMESTATE->m_SongOptions );

//	m_PlayerOptionIcons.Refresh();

	for( int p=0; p<NUM_PLAYERS; p++ )
	{
		m_OptionIconRow[p].Refresh( (PlayerNumber)p  );

		if( GAMESTATE->IsPlayerEnabled(p) )
		{
			CString s = GAMESTATE->m_PlayerOptions[p].GetString();
			s.Replace( ", ", "\n" );
//			m_textPlayerOptions[p].SetText( s );
		}
	}

	CString s = GAMESTATE->m_SongOptions.GetString();
	s.Replace( ", ", "\n" );
	m_textSongOptions.SetText( s );
}

void ScreenSelectMusic::SortOrderChanged()
{
	m_MusicSortDisplay.Set( GAMESTATE->m_SongSortOrder );

	// tween music sort on screen
//	m_MusicSortDisplay.FadeOn( 0, "fade", TWEEN_TIME );
}

