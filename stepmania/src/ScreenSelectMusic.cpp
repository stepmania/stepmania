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
#include "RageMusic.h"
#include "GameConstantsAndTypes.h"
#include "PrefsManager.h"
#include "RageLog.h"
#include "InputMapper.h"
#include "InputQueue.h"
#include "AnnouncerManager.h"
#include "InputMapper.h"
#include "GameState.h"
#include "CodeDetector.h"


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

const float TWEEN_TIME		= 0.5f;


const ScreenMessage SM_GoToPrevScreen		=	ScreenMessage(SM_User+1);
const ScreenMessage SM_GoToNextScreen		=	ScreenMessage(SM_User+2);



ScreenSelectMusic::ScreenSelectMusic()
{
	LOG->Trace( "ScreenSelectMusic::ScreenSelectMusic()" );


	CodeDetector::RefreshCacheItems();


	int p;

	m_Menu.Load(
		THEME->GetPathTo("Graphics","select music background"), 
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

	m_textStage.LoadFromFont( THEME->GetPathTo("Fonts","Header2") );
	m_textStage.TurnShadowOff();
	m_textStage.SetZoom( STAGE_ZOOM );
	m_textStage.SetXY( STAGE_X, STAGE_Y );
	m_textStage.SetText( GAMESTATE->GetStageText() );
	m_textStage.SetDiffuse( GAMESTATE->GetStageColor() );
	this->AddChild( &m_textStage );

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

		m_DifficultyIcon[p].SetXY( DIFFICULTY_ICON_X(p), DIFFICULTY_ICON_Y(p) );
		this->AddChild( &m_DifficultyIcon[p] );
	}

	m_GrooveRadar.SetXY( RADAR_X, RADAR_Y );
	this->AddChild( &m_GrooveRadar );

//	m_OptionIcons.SetXY( OPTION_ICONS_X, OPTION_ICONS_Y );
//	this->AddChild( &m_OptionIcons );


	m_textSongOptions.LoadFromFont( THEME->GetPathTo("Fonts","normal") );
	m_textSongOptions.SetXY( SONG_OPTIONS_X, SONG_OPTIONS_Y );
	m_textSongOptions.SetZoom( 0.5f );
	if( GAMESTATE->IsExtraStage() || GAMESTATE->IsExtraStage2() )
		m_textSongOptions.SetEffectCamelion( 2.5f, D3DXCOLOR(1,0,0,1), D3DXCOLOR(1,1,1,1) );	// blink red
	m_textSongOptions.SetDiffuse( D3DXCOLOR(1,1,1,1) );	// white
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
			m_textPlayerOptions[p].SetEffectCamelion( 2.5f, D3DXCOLOR(1,0,0,1), D3DXCOLOR(1,1,1,1) );	// blink red
		m_textPlayerOptions[p].SetDiffuse( D3DXCOLOR(1,1,1,1) );	// white
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
		m_HighScore[p].SetZoom( 0.6f );
		m_HighScore[p].SetDiffuse( PlayerToColor(p) );
		this->AddChild( &m_HighScore[p] );
	}	

	m_MusicSortDisplay.SetXY( SORT_ICON_X, SORT_ICON_Y );
	//m_MusicSortDisplay.SetEffectGlowing( 1.0f );
	m_MusicSortDisplay.Set( GAMESTATE->m_SongSortOrder );
	this->AddChild( &m_MusicSortDisplay );


	m_textHoldForOptions.LoadFromFont( THEME->GetPathTo("Fonts","select music hold") );
	m_textHoldForOptions.SetXY( CENTER_X, CENTER_Y );
	m_textHoldForOptions.SetText( "press START again for options" );
	m_textHoldForOptions.SetZoom( 1 );
	m_textHoldForOptions.SetZoomY( 0 );
	m_textHoldForOptions.SetDiffuse( D3DXCOLOR(1,1,1,0) );
	m_textHoldForOptions.SetZ( -2 );
	this->AddChild( &m_textHoldForOptions );


	m_soundSelect.Load( THEME->GetPathTo("Sounds","menu start") );
	m_soundChangeNotes.Load( THEME->GetPathTo("Sounds","select music change notes") );
	m_soundOptionsChange.Load( THEME->GetPathTo("Sounds","select music change options") );
	m_soundLocked.Load( THEME->GetPathTo("Sounds","select music wheel locked") );

	SOUND->PlayOnceStreamedFromDir( ANNOUNCER->GetPathTo("select music intro") );

	m_bMadeChoice = false;
	m_bGoToOptions = false;

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
	m_textStage.FadeOn( 0, "bounce left", TWEEN_TIME );
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
	int i;

	m_sprBannerFrame.FadeOff( 0, "bounce left", TWEEN_TIME*2 );
	m_Banner.FadeOff( 0, "bounce left", TWEEN_TIME*2 );
	m_BPMDisplay.FadeOff( 0, "bounce left", TWEEN_TIME*2 );
	m_textStage.FadeOff( 0, "bounce left", TWEEN_TIME*2 );
	m_sprCDTitle.FadeOff( 0, "bounce left", TWEEN_TIME*2 );

	for( int p=0; p<NUM_PLAYERS; p++ )
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

		m_FootMeter[p].FadeOff( 0, "foldy", TWEEN_TIME );
	}

	m_MusicSortDisplay.FadeOff( 0, "fade", TWEEN_TIME );

	CArray<Actor*,Actor*> apActorsInScore;
	for( p=0; p<NUM_PLAYERS; p++ )
	{
		apActorsInScore.Add( &m_sprHighScoreFrame[p] );
		apActorsInScore.Add( &m_HighScore[p] );
	}
	for( i=0; i<apActorsInScore.GetSize(); i++ )
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

	CArray<Actor*,Actor*> apActorsInScore;
	for( int p=0; p<NUM_PLAYERS; p++ )
	{
		apActorsInScore.Add( &m_sprHighScoreFrame[p] );
		apActorsInScore.Add( &m_HighScore[p] );
	}
	for( int i=0; i<apActorsInScore.GetSize(); i++ )
	{
		apActorsInScore[i]->StopTweening();

		float fOriginalX = apActorsInScore[i]->GetX();
		apActorsInScore[i]->BeginTweening( TWEEN_TIME, TWEEN_BIAS_END );		// tween off screen
		apActorsInScore[i]->SetTweenX( fOriginalX+400 );
		
		apActorsInScore[i]->BeginTweening( 0.5f );		// sleep

		apActorsInScore[i]->BeginTweening( 1, TWEEN_BIAS_BEGIN );		// tween back on screen
		apActorsInScore[i]->SetTweenX( fOriginalX );
	}
}

void ScreenSelectMusic::Update( float fDeltaTime )
{
	Screen::Update( fDeltaTime );

	float fNewRotation = m_sprCDTitle.GetRotationY()+D3DX_PI*fDeltaTime/2;
	fNewRotation = fmodf( fNewRotation, D3DX_PI*2 );
	m_sprCDTitle.SetRotationY( fNewRotation );
	if( fNewRotation > D3DX_PI/2  &&  fNewRotation <= D3DX_PI*3.0f/2 )
		m_sprCDTitle.SetDiffuse( D3DXCOLOR(0.2f,0.2f,0.2f,1) );
	else
		m_sprCDTitle.SetDiffuse( D3DXCOLOR(1,1,1,1) );
}

void ScreenSelectMusic::Input( const DeviceInput& DeviceI, const InputEventType type, const GameInput &GameI, const MenuInput &MenuI, const StyleInput &StyleI )
{
	LOG->Trace( "ScreenSelectMusic::Input()" );
	
	if( type == IET_RELEASE )	return;		// don't care

	if( m_Menu.IsClosing() )	return;		// ignore

	if( !GameI.IsValid() )		return;		// don't care

	if( m_bMadeChoice  &&  !m_bGoToOptions  &&  MenuI.IsValid()  &&  MenuI.button == MENU_BUTTON_START  &&  !GAMESTATE->IsExtraStage()  &&  !GAMESTATE->IsExtraStage2() )
	{
		m_bGoToOptions = true;
		m_textHoldForOptions.SetText( "Entering Options..." );
		SOUND->PlayOnceStreamed( THEME->GetPathTo("Sounds","menu start") );
		return;
	}
	
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
		if( GAMESTATE->IsExtraStage() || GAMESTATE->IsExtraStage2() )
			m_soundLocked.Play();
		else
			if( m_MusicWheel.NextSort() )
			{
				MUSIC->Stop();

				m_MusicSortDisplay.BeginTweening( 0.3f );
				m_MusicSortDisplay.SetTweenDiffuse( D3DXCOLOR(1,1,1,0) );

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
	if( m_arrayNotes[pn].GetSize() == 0 )
		return;
	if( m_iSelection[pn] == 0 )
		return;

	m_iSelection[pn]--;
	// the user explicity switched difficulties.  Update the preferred difficulty
	GAMESTATE->m_PreferredDifficultyClass[pn] = m_arrayNotes[pn][ m_iSelection[pn] ]->m_DifficultyClass;

	m_soundChangeNotes.Play();

	AfterNotesChange( pn );
}

void ScreenSelectMusic::HarderDifficulty( PlayerNumber pn )
{
	LOG->Trace( "ScreenSelectMusic::HarderDifficulty( %d )", pn );

	if( !GAMESTATE->IsPlayerEnabled(pn) )
		return;
	if( m_arrayNotes[pn].GetSize() == 0 )
		return;
	if( m_iSelection[pn] == m_arrayNotes[pn].GetSize()-1 )
		return;

	m_iSelection[pn]++;
	// the user explicity switched difficulties.  Update the preferred difficulty
	GAMESTATE->m_PreferredDifficultyClass[pn] = m_arrayNotes[pn][ m_iSelection[pn] ]->m_DifficultyClass;

	m_soundChangeNotes.Play();

	AfterNotesChange( pn );
}


void ScreenSelectMusic::HandleScreenMessage( const ScreenMessage SM )
{
	Screen::HandleScreenMessage( SM );

	switch( SM )
	{
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
		break;
	case SM_GoToNextScreen:
		if( m_bGoToOptions )
		{
			SCREENMAN->SetNewScreen( "ScreenPlayerOptions" );
		}
		else
		{
			MUSIC->Stop();
			SCREENMAN->SetNewScreen( "ScreenStage" );
		}
		break;
	case SM_PlaySongSample:
		PlayMusicSample();
		break;
	case SM_SongChanged:
		AfterMusicChange();
		break;
	case SM_SortOrderChanged:
		SortOrderChanged();
		break;
	}
}

void ScreenSelectMusic::MenuLeft( PlayerNumber pn, const InputEventType type )
{
	if( type >= IET_SLOW_REPEAT  &&  INPUTMAPPER->IsButtonDown( MenuInput(pn, MENU_BUTTON_RIGHT) ) )
			return;		// ignore
	
	if( ! m_MusicWheel.WheelIsLocked() )
		MUSIC->Stop();

	m_MusicWheel.PrevMusic();
}


void ScreenSelectMusic::MenuRight( PlayerNumber pn, const InputEventType type )
{
	if( type >= IET_SLOW_REPEAT  &&  INPUTMAPPER->IsButtonDown( MenuInput(pn, MENU_BUTTON_LEFT) ) )
		return;		// ignore

	if( ! m_MusicWheel.WheelIsLocked() )
		MUSIC->Stop();

	m_MusicWheel.NextMusic();
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
				MUSIC->Stop();

				m_MusicSortDisplay.BeginTweening( 0.3f );
				m_MusicSortDisplay.SetTweenDiffuse( D3DXCOLOR(1,1,1,0) );

				TweenScoreOnAndOffAfterChangeSort();
			}
		}
		return;
	}


	// this needs to check whether valid Notes are selected!
	bool bResult = m_MusicWheel.Select();

	if( !bResult )
	{
	/* why do this? breaks tabs and roulette -glenn */
//		if( pn != PLAYER_INVALID )
//			this->SendScreenMessage( SM_MenuTimer, 1 );	// re-throw a timer message
		return;
	}

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
			if( GAMESTATE->m_pCurNotes[p]  &&  GAMESTATE->m_pCurNotes[p]->m_iMeter >= 9 )
				bIsHard = true;
		}

		if( bIsNew )
			SOUND->PlayOnceStreamedFromDir( ANNOUNCER->GetPathTo("select music comment new") );
		else if( bIsHard )
			SOUND->PlayOnceStreamedFromDir( ANNOUNCER->GetPathTo("select music comment hard") );
		else
			SOUND->PlayOnceStreamedFromDir( ANNOUNCER->GetPathTo("select music comment general") );


		TweenOffScreen();

		m_bMadeChoice = true;

		m_soundSelect.Play();

		if( !GAMESTATE->IsExtraStage()  &&  !GAMESTATE->IsExtraStage2() )
		{
			// show "hold START for options"
			m_textHoldForOptions.SetDiffuse( D3DXCOLOR(1,1,1,0) );
			m_textHoldForOptions.BeginTweening( 0.25f );	// fade in
			m_textHoldForOptions.SetTweenZoomY( 1 );
			m_textHoldForOptions.SetTweenDiffuse( D3DXCOLOR(1,1,1,1) );
			m_textHoldForOptions.BeginTweening( 2.0f );	// sleep
			m_textHoldForOptions.BeginTweening( 0.25f );	// fade out
			m_textHoldForOptions.SetTweenDiffuse( D3DXCOLOR(1,1,1,0) );
			m_textHoldForOptions.SetTweenZoomY( 0 );
		}

		m_Menu.TweenOffScreenToBlack( SM_None, false );

		m_Menu.StopTimer();

		this->SendScreenMessage( SM_GoToNextScreen, 2.5f );
		break;
	}
	case TYPE_SECTION:
		break;
	case TYPE_ROULETTE:
		break;
	}
}


void ScreenSelectMusic::MenuBack( PlayerNumber pn )
{
	MUSIC->Stop();

	m_Menu.TweenOffScreenToBlack( SM_GoToPrevScreen, true );
}

void ScreenSelectMusic::AfterNotesChange( PlayerNumber pn )
{
	if( !GAMESTATE->IsPlayerEnabled(pn) )
		return;
	
	m_iSelection[pn] = clamp( m_iSelection[pn], 0, m_arrayNotes[pn].GetSize()-1 );	// bounds clamping

	Notes* pNotes = m_arrayNotes[pn].GetSize()>0 ? m_arrayNotes[pn][m_iSelection[pn]] : NULL;

	GAMESTATE->m_pCurNotes[pn] = pNotes;

//	m_BPMDisplay.SetZoomY( 0 );
//	m_BPMDisplay.BeginTweening( 0.2f );
//	m_BPMDisplay.SetTweenZoomY( 1.2f );

	Notes* m_pNotes = GAMESTATE->m_pCurNotes[pn];
	
	if( m_pNotes )
		m_HighScore[pn].SetScore( (float)m_pNotes->m_iTopScore );

	m_DifficultyIcon[pn].SetFromNotes( pNotes );
	m_FootMeter[pn].SetFromNotes( pNotes );
	m_GrooveRadar.SetFromNotes( pn, pNotes );
	m_MusicWheel.NotesChanged( pn );
}

void ScreenSelectMusic::AfterMusicChange()
{
	m_Menu.StallTimer();

	Song* pSong = m_MusicWheel.GetSelectedSong();
	GAMESTATE->m_pCurSong = pSong;

	int pn;
	for( pn = 0; pn < NUM_PLAYERS; ++pn)
		m_arrayNotes[pn].RemoveAll();

	switch( m_MusicWheel.GetSelectedType() )
	{
	case TYPE_SECTION:
		{	
			CString sGroup = m_MusicWheel.GetSelectedSection();
			for( int p=0; p<NUM_PLAYERS; p++ )
			{
				m_iSelection[p] = -1;
			}

			m_Banner.SetFromGroup( sGroup );	// if this isn't a group, it'll default to the fallback banner
			m_BPMDisplay.SetBPMRange( 0, 0 );
			m_sprCDTitle.UnloadTexture();
		}
		break;
	case TYPE_SONG:
		{
			for( int pn = 0; pn < NUM_PLAYERS; ++pn) {
				pSong->GetNotesThatMatch( GAMESTATE->GetCurrentStyleDef(), pn, m_arrayNotes[pn] );
				SortNotesArrayByDifficulty( m_arrayNotes[pn] );
			}

			m_Banner.SetFromSong( pSong );

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
				for( int i=0; i<m_arrayNotes[p].GetSize(); i++ )
					if( m_arrayNotes[p][i]->m_DifficultyClass == GAMESTATE->m_PreferredDifficultyClass[p] )
						m_iSelection[p] = i;

				m_iSelection[p] = clamp( m_iSelection[p], 0, m_arrayNotes[p].GetSize() ) ;
			}
		}
		break;
	case TYPE_ROULETTE:
		m_Banner.SetRoulette();
		m_BPMDisplay.SetBPMRange( 0, 0 );
		m_sprCDTitle.UnloadTexture();
		break;
	}

	for( int p=0; p<NUM_PLAYERS; p++ )
	{
		AfterNotesChange( (PlayerNumber)p );
	}
}


void ScreenSelectMusic::PlayMusicSample()
{
	//LOG->Trace( "ScreenSelectSong::PlaySONGample()" );

	Song* pSong = m_MusicWheel.GetSelectedSong();
	if( pSong )
	{
		MUSIC->Stop();
		MUSIC->Load( pSong->GetMusicPath() );
		MUSIC->Play( true, pSong->m_fMusicSampleStartSeconds, pSong->m_fMusicSampleLengthSeconds );
	}
	else
		MUSIC->LoadAndPlayIfNotAlready( THEME->GetPathTo("Sounds","select music music") );
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
	m_MusicSortDisplay.SetState( GAMESTATE->m_SongSortOrder );

	// tween music sort on screen
//	m_MusicSortDisplay.SetEffectGlowing();
	m_MusicSortDisplay.BeginTweening( 0.3f );
	m_MusicSortDisplay.SetTweenDiffuse( D3DXCOLOR(1,1,1,1) );		
}

