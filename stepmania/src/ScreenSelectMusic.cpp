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
#include "ScreenTitleMenu.h"
#include "GameConstantsAndTypes.h"
#include "PrefsManager.h"
#include "ScreenGameplay.h"
#include "ScreenPrompt.h"
#include "ScreenPlayerOptions.h"
#include "RageLog.h"
#include "InputMapper.h"
#include "InputQueue.h"
#include "ScreenStage.h"
#include "AnnouncerManager.h"
#include "InputMapper.h"
#include "GameState.h"


#define BANNER_FRAME_X		THEME->GetMetricF("SelectMusic","BannerFrameX")
#define BANNER_FRAME_Y		THEME->GetMetricF("SelectMusic","BannerFrameY")
#define BANNER_X			THEME->GetMetricF("SelectMusic","BannerX")
#define BANNER_Y			THEME->GetMetricF("SelectMusic","BannerY")
#define BANNER_WIDTH		THEME->GetMetricF("SelectMusic","BannerWidth")
#define BANNER_HEIGHT		THEME->GetMetricF("SelectMusic","BannerHeight")
#define BPM_X				THEME->GetMetricF("SelectMusic","BPMX")
#define BPM_Y				THEME->GetMetricF("SelectMusic","BPMY")
#define STAGE_X				THEME->GetMetricF("SelectMusic","StageX")
#define STAGE_Y				THEME->GetMetricF("SelectMusic","StageY")
#define CD_TITLE_X			THEME->GetMetricF("SelectMusic","CDTitleX")
#define CD_TITLE_Y			THEME->GetMetricF("SelectMusic","CDTitleY")
#define DIFFICULTY_X		THEME->GetMetricF("SelectMusic","DifficultyX")
#define DIFFICULTY_Y		THEME->GetMetricF("SelectMusic","DifficultyY")
#define ICON_P1_X			THEME->GetMetricF("SelectMusic","IconP1X")
#define ICON_P1_Y			THEME->GetMetricF("SelectMusic","IconP1Y")
#define ICON_P2_X			THEME->GetMetricF("SelectMusic","IconP2X")
#define ICON_P2_Y			THEME->GetMetricF("SelectMusic","IconP2Y")
#define RADAR_X				THEME->GetMetricF("SelectMusic","RadarX")
#define RADAR_Y				THEME->GetMetricF("SelectMusic","RadarY")
#define SORT_ICON_X			THEME->GetMetricF("SelectMusic","SortIconX")
#define SORT_ICON_Y			THEME->GetMetricF("SelectMusic","SortIconY")
#define SCORE_P1_X			THEME->GetMetricF("SelectMusic","ScoreP1X")
#define SCORE_P1_Y			THEME->GetMetricF("SelectMusic","ScoreP1Y")
#define SCORE_P2_X			THEME->GetMetricF("SelectMusic","ScoreP2X")
#define SCORE_P2_Y			THEME->GetMetricF("SelectMusic","ScoreP2Y")
#define METER_FRAME_X		THEME->GetMetricF("SelectMusic","MeterFrameX")
#define METER_FRAME_Y		THEME->GetMetricF("SelectMusic","MeterFrameY")
#define METER_P1_X			THEME->GetMetricF("SelectMusic","MeterP1X")
#define METER_P1_Y			THEME->GetMetricF("SelectMusic","MeterP1Y")
#define METER_P2_X			THEME->GetMetricF("SelectMusic","MeterP2X")
#define METER_P2_Y			THEME->GetMetricF("SelectMusic","MeterP2Y")
#define WHEEL_X				THEME->GetMetricF("SelectMusic","WheelX")
#define WHEEL_Y				THEME->GetMetricF("SelectMusic","WheelY")
#define PLAYER_OPTIONS_P1_X	THEME->GetMetricF("SelectMusic","PlayerOptionsP1X")
#define PLAYER_OPTIONS_P1_Y	THEME->GetMetricF("SelectMusic","PlayerOptionsP1Y")
#define PLAYER_OPTIONS_P2_X	THEME->GetMetricF("SelectMusic","PlayerOptionsP2X")
#define PLAYER_OPTIONS_P2_Y	THEME->GetMetricF("SelectMusic","PlayerOptionsP2Y")
#define SONG_OPTIONS_X		THEME->GetMetricF("SelectMusic","SongOptionsX")
#define SONG_OPTIONS_Y		THEME->GetMetricF("SelectMusic","SongOptionsY")
#define HELP_TEXT			THEME->GetMetric("SelectMusic","HelpText")
#define TIMER_SECONDS		THEME->GetMetricI("SelectMusic","TimerSeconds")

const float TWEEN_TIME		= 0.5f;

float ICON_X( int p ) {
	switch( p ) {
		case PLAYER_1:	return ICON_P1_X;
		case PLAYER_2:	return ICON_P2_X;
		default:		ASSERT(0);	return 0;
	}
}
float ICON_Y( int p ) {
	switch( p ) {
		case PLAYER_1:	return ICON_P1_Y;
		case PLAYER_2:	return ICON_P2_Y;
		default:		ASSERT(0);	return 0;
	}
}
float HIGH_SCORE_X( int p ) {
	switch( p ) {
		case PLAYER_1:	return SCORE_P1_X;
		case PLAYER_2:	return SCORE_P2_X;
		default:		ASSERT(0);	return 0;
	}
}
float HIGH_SCORE_Y( int p ) {
	switch( p ) {
		case PLAYER_1:	return SCORE_P1_Y;
		case PLAYER_2:	return SCORE_P2_Y;
		default:		ASSERT(0);	return 0;
	}
}
float METER_X( int p ) {
	switch( p ) {
		case PLAYER_1:	return METER_P1_X;
		case PLAYER_2:	return METER_P2_X;
		default:		ASSERT(0);	return 0;
	}
}
float METER_Y( int p ) {
	switch( p ) {
		case PLAYER_1:	return METER_P1_Y;
		case PLAYER_2:	return METER_P2_Y;
		default:		ASSERT(0);	return 0;
	}
}
#define PLAYER_OPTIONS_X(p) ( p==PLAYER_1 ? PLAYER_OPTIONS_P1_X : PLAYER_OPTIONS_P2_X )

float PLAYER_OPTIONS_Y( int p ) {
	switch( p ) {
		case PLAYER_1:	return PLAYER_OPTIONS_P1_Y;
		case PLAYER_2:	return PLAYER_OPTIONS_P2_Y;
		default:		ASSERT(0);	return 0;
	}
}

const ScreenMessage SM_GoToPrevState		=	ScreenMessage(SM_User+1);
const ScreenMessage SM_GoToNextState		=	ScreenMessage(SM_User+2);



ScreenSelectMusic::ScreenSelectMusic()
{
	LOG->Trace( "ScreenSelectMusic::ScreenSelectMusic()" );

	// for debugging
	if( GAMESTATE->m_CurStyle == STYLE_NONE )
		GAMESTATE->m_CurStyle = STYLE_DANCE_SINGLE;

	int p;

	m_Menu.Load(
		THEME->GetPathTo("Graphics","select music background"), 
		THEME->GetPathTo("Graphics","select music top edge"),
		HELP_TEXT, false, true, TIMER_SECONDS 
		);
	this->AddSubActor( &m_Menu );

	// these guys get loaded SetSong and TweenToSong
	m_Banner.SetXY( BANNER_X, BANNER_Y );
	m_Banner.SetCroppedSize( BANNER_WIDTH, BANNER_HEIGHT );
	this->AddSubActor( &m_Banner );

	m_sprBannerFrame.Load( THEME->GetPathTo("Graphics","select music info frame") );
	m_sprBannerFrame.SetXY( BANNER_FRAME_X, BANNER_FRAME_Y );
	this->AddSubActor( &m_sprBannerFrame );

	m_BPMDisplay.SetXY( BPM_X, BPM_Y );
	m_BPMDisplay.SetZoomX( 1.0f );
	this->AddSubActor( &m_BPMDisplay );

	m_textStage.LoadFromFont( THEME->GetPathTo("Fonts","Header2") );
	m_textStage.TurnShadowOff();
	m_textStage.SetZoomX( 1.0f );
	m_textStage.SetXY( STAGE_X, STAGE_Y );
	m_textStage.SetText( GAMESTATE->GetStageText() );
	m_textStage.SetDiffuseColor( GAMESTATE->GetStageColor() );
	this->AddSubActor( &m_textStage );

	m_sprCDTitle.Load( THEME->GetPathTo("Graphics","fallback cd title") );
	m_sprCDTitle.TurnShadowOff();
	m_sprCDTitle.SetXY( CD_TITLE_X, CD_TITLE_Y );
	this->AddSubActor( &m_sprCDTitle );

	m_sprDifficultyFrame.Load( THEME->GetPathTo("Graphics","select music difficulty frame") );
	m_sprDifficultyFrame.SetXY( DIFFICULTY_X, DIFFICULTY_Y );
	this->AddSubActor( &m_sprDifficultyFrame );

	for( p=0; p<NUM_PLAYERS; p++ )
	{
		m_DifficultyIcon[p].SetXY( ICON_X(p), ICON_Y(p) );
		this->AddSubActor( &m_DifficultyIcon[p] );
	}

	m_GrooveRadar.SetXY( RADAR_X, RADAR_Y );
	this->AddSubActor( &m_GrooveRadar );

	m_textSongOptions.LoadFromFont( THEME->GetPathTo("Fonts","normal") );
	m_textSongOptions.SetXY( SONG_OPTIONS_X, SONG_OPTIONS_Y );
	m_textSongOptions.SetZoom( 0.5f );
	if( GAMESTATE->IsExtraStage() || GAMESTATE->IsExtraStage2() )
		m_textSongOptions.SetEffectCamelion( 2.5f, D3DXCOLOR(1,0,0,1), D3DXCOLOR(1,1,1,1) );	// blink red
	m_textSongOptions.SetDiffuseColor( D3DXCOLOR(1,1,1,1) );	// white
	this->AddSubActor( &m_textSongOptions );

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
		m_textPlayerOptions[p].SetDiffuseColor( D3DXCOLOR(1,1,1,1) );	// white
		this->AddSubActor( &m_textPlayerOptions[p] );
	}

	m_sprMeterFrame.Load( THEME->GetPathTo("Graphics","select music meter frame") );
	m_sprMeterFrame.SetXY( METER_FRAME_X, METER_FRAME_Y );
	this->AddSubActor( &m_sprMeterFrame );

	for( p=0; p<NUM_PLAYERS; p++ )
	{
		m_FootMeter[p].LoadFromFont( THEME->GetPathTo("Fonts","meter") );
		m_FootMeter[p].SetXY( METER_X(p), METER_Y(p) );
		m_FootMeter[p].SetShadowLength( 2 );
		this->AddSubActor( &m_FootMeter[p] );
	}

	m_MusicWheel.SetXY( WHEEL_X, WHEEL_Y );
	this->AddSubActor( &m_MusicWheel );

	for( p=0; p<NUM_PLAYERS; p++ )
	{
		if( !GAMESTATE->IsPlayerEnabled((PlayerNumber)p) )
			continue;	// skip

		m_sprHighScoreFrame[p].Load( THEME->GetPathTo("Graphics","select music score frame") );
		m_sprHighScoreFrame[p].StopAnimating();
		m_sprHighScoreFrame[p].SetState( p );
		m_sprHighScoreFrame[p].SetXY( HIGH_SCORE_X(p), HIGH_SCORE_Y(p) );
		this->AddSubActor( &m_sprHighScoreFrame[p] );

		m_HighScore[p].SetXY( HIGH_SCORE_X(p), HIGH_SCORE_Y(p) );
		m_HighScore[p].SetZoom( 0.6f );
		m_HighScore[p].SetDiffuseColor( PlayerToColor(p) );
		this->AddSubActor( &m_HighScore[p] );
	}	

	m_MusicSortDisplay.SetXY( SORT_ICON_X, SORT_ICON_Y );
	//m_MusicSortDisplay.SetEffectGlowing( 1.0f );
	m_MusicSortDisplay.Set( GAMESTATE->m_SongSortOrder );
	this->AddSubActor( &m_MusicSortDisplay );


	m_textHoldForOptions.LoadFromFont( THEME->GetPathTo("Fonts","stage") );
	m_textHoldForOptions.SetXY( CENTER_X, CENTER_Y );
	m_textHoldForOptions.SetText( "press START again for options" );
	m_textHoldForOptions.SetZoom( 1 );
	m_textHoldForOptions.SetZoomY( 0 );
	m_textHoldForOptions.SetDiffuseColor( D3DXCOLOR(1,1,1,0) );
	m_textHoldForOptions.SetZ( -2 );
	this->AddSubActor( &m_textHoldForOptions );


	m_soundSelect.Load( THEME->GetPathTo("Sounds","menu start") );
	m_soundChangeNotes.Load( THEME->GetPathTo("Sounds","select music change notes") );
	m_soundLocked.Load( THEME->GetPathTo("Sounds","select music wheel locked") );

	SOUND->PlayOnceStreamedFromDir( ANNOUNCER->GetPathTo(ANNOUNCER_SELECT_MUSIC_INTRO) );

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
	float fOriginalZoomY;

	Actor* pActorsInGroupInfoFrame[] = { &m_sprBannerFrame, &m_Banner, &m_BPMDisplay, &m_textStage, &m_sprCDTitle };
	const int iNumActorsInGroupInfoFrame = sizeof(pActorsInGroupInfoFrame) / sizeof(Actor*);
	int i;
	for( i=0; i<iNumActorsInGroupInfoFrame; i++ )
	{
		float fOriginalX = pActorsInGroupInfoFrame[i]->GetX();
		pActorsInGroupInfoFrame[i]->SetX( fOriginalX-400 );
		pActorsInGroupInfoFrame[i]->BeginTweening( TWEEN_TIME, TWEEN_BOUNCE_END );
		pActorsInGroupInfoFrame[i]->SetTweenX( fOriginalX );
	}

	fOriginalZoomY = m_sprDifficultyFrame.GetZoomY();
	m_sprDifficultyFrame.BeginTweening( TWEEN_TIME );
	m_sprDifficultyFrame.SetTweenZoomY( fOriginalZoomY );

	fOriginalZoomY = m_sprMeterFrame.GetZoomY();
	m_sprMeterFrame.BeginTweening( TWEEN_TIME );
	m_sprMeterFrame.SetTweenZoomY( fOriginalZoomY );

	m_GrooveRadar.TweenOnScreen();

	fOriginalZoomY = m_textSongOptions.GetZoomY();
	m_textSongOptions.BeginTweening( TWEEN_TIME );
	m_textSongOptions.SetTweenZoomY( fOriginalZoomY );

	for( int p=0; p<NUM_PLAYERS; p++ )
	{
		fOriginalZoomY = m_textPlayerOptions[p].GetZoomY();
		m_textPlayerOptions[p].BeginTweening( TWEEN_TIME );
		m_textPlayerOptions[p].SetTweenZoomY( fOriginalZoomY );

		fOriginalZoomY = m_DifficultyIcon[p].GetZoomY();
		m_DifficultyIcon[p].BeginTweening( TWEEN_TIME );
		m_DifficultyIcon[p].SetTweenZoomY( fOriginalZoomY );

		fOriginalZoomY = m_FootMeter[p].GetZoomY();
		m_FootMeter[p].BeginTweening( TWEEN_TIME );
		m_FootMeter[p].SetTweenZoomY( fOriginalZoomY );
	}

	m_MusicSortDisplay.SetDiffuseColor( D3DXCOLOR(1,1,1,0) );
	m_MusicSortDisplay.BeginTweening( TWEEN_TIME );
	m_MusicSortDisplay.SetTweenDiffuseColor( D3DXCOLOR(1,1,1,1) );

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

void ScreenSelectMusic::TweenOffScreen()
{
	Actor* pActorsInGroupInfoFrame[] = { &m_sprBannerFrame, &m_Banner, &m_BPMDisplay, &m_textStage, &m_sprCDTitle };
	const int iNumActorsInGroupInfoFrame = sizeof(pActorsInGroupInfoFrame) / sizeof(Actor*);
	int i;
	for( i=0; i<iNumActorsInGroupInfoFrame; i++ )
	{
		pActorsInGroupInfoFrame[i]->BeginTweeningQueued( TWEEN_TIME, TWEEN_BOUNCE_BEGIN );
		pActorsInGroupInfoFrame[i]->SetTweenX( pActorsInGroupInfoFrame[i]->GetX()-400 );
	}

	m_sprDifficultyFrame.BeginTweening( TWEEN_TIME );
	m_sprDifficultyFrame.SetTweenZoomY( 0 );

	m_sprMeterFrame.BeginTweening( TWEEN_TIME );
	m_sprMeterFrame.SetTweenZoomY( 0 );

	m_GrooveRadar.TweenOffScreen();

	m_textSongOptions.BeginTweening( TWEEN_TIME );
	m_textSongOptions.SetTweenZoomY( 0 );

	for( int p=0; p<NUM_PLAYERS; p++ )
	{
		m_textPlayerOptions[p].BeginTweening( TWEEN_TIME );
		m_textPlayerOptions[p].SetTweenZoomY( 0 );

		m_DifficultyIcon[p].BeginTweening( TWEEN_TIME );
		m_DifficultyIcon[p].SetTweenZoomY( 0 );

		m_FootMeter[p].BeginTweening( TWEEN_TIME );
		m_FootMeter[p].SetTweenZoomY( 0 );
	}

	m_MusicSortDisplay.SetEffectNone();
	m_MusicSortDisplay.BeginTweening( TWEEN_TIME );
	m_MusicSortDisplay.SetTweenDiffuseColor( D3DXCOLOR(1,1,1,0) );

	Actor* pActorsInScore[] = { &m_sprHighScoreFrame[0], &m_sprHighScoreFrame[1], &m_HighScore[0], &m_HighScore[1] };
	const int iNumActorsInScore = sizeof(pActorsInScore) / sizeof(Actor*);
	for( i=0; i<iNumActorsInScore; i++ )
	{
		pActorsInScore[i]->BeginTweening( TWEEN_TIME, TWEEN_BIAS_BEGIN );
		pActorsInScore[i]->SetTweenX( pActorsInScore[i]->GetX()+400 );
	}

	m_MusicWheel.TweenOffScreen();
}

void ScreenSelectMusic::Update( float fDeltaTime )
{
	Screen::Update( fDeltaTime );

	float fNewRotation = m_sprCDTitle.GetRotationY()+D3DX_PI*fDeltaTime/2;
	fNewRotation = fmodf( fNewRotation, D3DX_PI*2 );
	m_sprCDTitle.SetRotationY( fNewRotation );
	if( fNewRotation > D3DX_PI/2  &&  fNewRotation <= D3DX_PI*3.0f/2 )
		m_sprCDTitle.SetDiffuseColor( D3DXCOLOR(0.2f,0.2f,0.2f,1) );
	else
		m_sprCDTitle.SetDiffuseColor( D3DXCOLOR(1,1,1,1) );
}

const GameButton DANCE_EASIER_DIFFICULTY_PATTERN[] = { DANCE_BUTTON_UP, DANCE_BUTTON_UP };
const int DANCE_EASIER_DIFFICULTY_PATTERN_SIZE = sizeof(DANCE_EASIER_DIFFICULTY_PATTERN) / sizeof(GameButton);

const GameButton DANCE_HARDER_DIFFICULTY_PATTERN[] = { DANCE_BUTTON_DOWN, DANCE_BUTTON_DOWN };
const int DANCE_HARDER_DIFFICULTY_PATTERN_SIZE = sizeof(DANCE_HARDER_DIFFICULTY_PATTERN) / sizeof(GameButton);

const MenuButton MENU_EASIER_DIFFICULTY_PATTERN[] = { MENU_BUTTON_UP, MENU_BUTTON_UP };
const int MENU_EASIER_DIFFICULTY_PATTERN_SIZE = sizeof(MENU_EASIER_DIFFICULTY_PATTERN) / sizeof(MenuButton);

const MenuButton MENU_HARDER_DIFFICULTY_PATTERN[] = { MENU_BUTTON_DOWN, MENU_BUTTON_DOWN };
const int MENU_HARDER_DIFFICULTY_PATTERN_SIZE = sizeof(MENU_HARDER_DIFFICULTY_PATTERN) / sizeof(MenuButton);

const MenuButton MENU_NEXT_SORT_PATTERN[] = { MENU_BUTTON_UP, MENU_BUTTON_DOWN, MENU_BUTTON_UP, MENU_BUTTON_DOWN };
const int MENU_NEXT_SORT_PATTERN_SIZE = sizeof(MENU_NEXT_SORT_PATTERN) / sizeof(MenuButton);

void ScreenSelectMusic::Input( const DeviceInput& DeviceI, const InputEventType type, const GameInput &GameI, const MenuInput &MenuI, const StyleInput &StyleI )
{
	LOG->Trace( "ScreenSelectMusic::Input()" );
	if(type == IET_RELEASE) return; // don't care

	if( MenuI.player == PLAYER_INVALID )
		return;

	if( m_Menu.IsClosing() )
		return;		// ignore

	if( m_bMadeChoice  &&  !m_bGoToOptions  &&  MenuI.button == MENU_BUTTON_START  &&  !GAMESTATE->IsExtraStage()  &&  !GAMESTATE->IsExtraStage2() )
	{
		m_bGoToOptions = true;
		m_textHoldForOptions.SetText( "Entering Options..." );
		SOUND->PlayOnceStreamed( THEME->GetPathTo("Sounds","menu start") );
		return;
	}
	
	if( m_bMadeChoice )
		return;

	switch( GAMESTATE->m_CurGame )
	{
	case GAME_DANCE:
		if( INPUTQUEUE->MatchesPattern(GameI.controller, DANCE_EASIER_DIFFICULTY_PATTERN, DANCE_EASIER_DIFFICULTY_PATTERN_SIZE) )
		{
			if( GAMESTATE->IsExtraStage() || GAMESTATE->IsExtraStage2() )
				m_soundLocked.Play();
			else
				EasierDifficulty( MenuI.player );
			return;
		}
		if( INPUTQUEUE->MatchesPattern(GameI.controller, DANCE_HARDER_DIFFICULTY_PATTERN, DANCE_HARDER_DIFFICULTY_PATTERN_SIZE) )
		{
			if( GAMESTATE->IsExtraStage() || GAMESTATE->IsExtraStage2() )
				m_soundLocked.Play();
			else
				HarderDifficulty( MenuI.player );
			return;
		}
		break;
	}

	if( INPUTQUEUE->MatchesPattern(GameI.controller, MENU_EASIER_DIFFICULTY_PATTERN, MENU_EASIER_DIFFICULTY_PATTERN_SIZE) )
	{
		if( GAMESTATE->IsExtraStage() || GAMESTATE->IsExtraStage2() )
			m_soundLocked.Play();
		else
			EasierDifficulty( MenuI.player );
		return;
	}
	if( INPUTQUEUE->MatchesPattern(GameI.controller, MENU_HARDER_DIFFICULTY_PATTERN, MENU_HARDER_DIFFICULTY_PATTERN_SIZE) )
	{
		if( GAMESTATE->IsExtraStage() || GAMESTATE->IsExtraStage2() )
			m_soundLocked.Play();
		else
			HarderDifficulty( MenuI.player );
		return;
	}
	if( INPUTQUEUE->MatchesPattern(GameI.controller, MENU_NEXT_SORT_PATTERN, MENU_NEXT_SORT_PATTERN_SIZE) )
	{
		if( GAMESTATE->IsExtraStage() || GAMESTATE->IsExtraStage2() )
			m_soundLocked.Play();
		else
			if( m_MusicWheel.NextSort() )
			{
				MUSIC->Stop();
				// tween music sort off screen
				//m_MusicSortDisplay.SetEffectNone();
				m_MusicSortDisplay.BeginTweening( 0.3f );
				m_MusicSortDisplay.SetTweenDiffuseColor( D3DXCOLOR(1,1,1,0) );
			}
		return;
	}

	Screen::Input( DeviceI, type, GameI, MenuI, StyleI );	// default input handler
}


void ScreenSelectMusic::EasierDifficulty( const PlayerNumber p )
{
	LOG->Trace( "ScreenSelectMusic::EasierDifficulty( %d )", p );

	if( !GAMESTATE->IsPlayerEnabled(p) )
		return;
	if( m_arrayNotes.GetSize() == 0 )
		return;
	if( m_iSelection[p] == 0 )
		return;

	m_iSelection[p]--;
	// the user explicity switched difficulties.  Update the preferred difficulty
	GAMESTATE->m_PreferredDifficultyClass[p] = m_arrayNotes[ m_iSelection[p] ]->m_DifficultyClass;

	m_soundChangeNotes.Play();

	AfterNotesChange( p );
}

void ScreenSelectMusic::HarderDifficulty( const PlayerNumber p )
{
	LOG->Trace( "ScreenSelectMusic::HarderDifficulty( %d )", p );

	if( !GAMESTATE->IsPlayerEnabled(p) )
		return;
	if( m_arrayNotes.GetSize() == 0 )
		return;
	if( m_iSelection[p] == m_arrayNotes.GetSize()-1 )
		return;

	m_iSelection[p]++;
	// the user explicity switched difficulties.  Update the preferred difficulty
	GAMESTATE->m_PreferredDifficultyClass[p] = m_arrayNotes[ m_iSelection[p] ]->m_DifficultyClass;

	m_soundChangeNotes.Play();

	AfterNotesChange( p );
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
	case SM_GoToPrevState:
		SCREENMAN->SetNewScreen( new ScreenTitleMenu );
		break;
	case SM_GoToNextState:
		if( m_bGoToOptions )
		{
			SCREENMAN->SetNewScreen( new ScreenPlayerOptions );
		}
		else
		{
			MUSIC->Stop();
			SCREENMAN->SetNewScreen( new ScreenStage );
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

void ScreenSelectMusic::MenuLeft( const PlayerNumber p, const InputEventType type )
{
	if( type >= IET_SLOW_REPEAT  &&  INPUTMAPPER->IsButtonDown( MenuInput(p, MENU_BUTTON_RIGHT) ) )
			return;		// ignore
	
	if( ! m_MusicWheel.WheelIsLocked() )
		MUSIC->Stop();

	m_MusicWheel.PrevMusic();
}


void ScreenSelectMusic::MenuRight( const PlayerNumber p, const InputEventType type )
{
	if( type >= IET_SLOW_REPEAT  &&  INPUTMAPPER->IsButtonDown( MenuInput(p, MENU_BUTTON_LEFT) ) )
		return;		// ignore

	if( ! m_MusicWheel.WheelIsLocked() )
		MUSIC->Stop();

	m_MusicWheel.NextMusic();
}

void ScreenSelectMusic::MenuStart( const PlayerNumber p )
{
	if( p != PLAYER_INVALID  &&
		INPUTMAPPER->IsButtonDown( MenuInput(p, MENU_BUTTON_LEFT) )  &&
		INPUTMAPPER->IsButtonDown( MenuInput(p, MENU_BUTTON_RIGHT) ) )
	{
		if( GAMESTATE->IsExtraStage() || GAMESTATE->IsExtraStage2() )
			m_soundLocked.Play();
		else
		{
			if( m_MusicWheel.NextSort() )
			{
				MUSIC->Stop();

				// tween music sort off screen
				//m_MusicSortDisplay.SetEffectNone();
				m_MusicSortDisplay.BeginTweening( 0.3f );
				m_MusicSortDisplay.SetTweenDiffuseColor( D3DXCOLOR(1,1,1,0) );
			}
		}
		return;
	}


	// this needs to check whether valid Notes are selected!
	bool bResult = m_MusicWheel.Select();

	if( !bResult )
	{
		if( p != PLAYER_INVALID )
			this->SendScreenMessage( SM_MenuTimer, 1 );	// re-throw a timer message
	}
	else	// if !bResult
	{
		// a song was selected
		switch( m_MusicWheel.GetSelectedType() )
		{
		case TYPE_SONG:
			{
				if( !m_MusicWheel.GetSelectedSong()->HasMusic() )
				{
					SCREENMAN->AddScreenToTop( new ScreenPrompt( SM_None, "ERROR:\n \nThis song does not have a music file\n and cannot be played.", PROMPT_OK) );
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
					SOUND->PlayOnceStreamedFromDir( ANNOUNCER->GetPathTo(ANNOUNCER_SELECT_MUSIC_COMMENT_NEW) );
				else if( bIsHard )
					SOUND->PlayOnceStreamedFromDir( ANNOUNCER->GetPathTo(ANNOUNCER_SELECT_MUSIC_COMMENT_HARD) );
				else
					SOUND->PlayOnceStreamedFromDir( ANNOUNCER->GetPathTo(ANNOUNCER_SELECT_MUSIC_COMMENT_GENERAL) );


				TweenOffScreen();

				m_bMadeChoice = true;

				m_soundSelect.Play();

				if( !GAMESTATE->IsExtraStage()  &&  !GAMESTATE->IsExtraStage2() )
				{
					// show "hold START for options"
					m_textHoldForOptions.SetDiffuseColor( D3DXCOLOR(1,1,1,0) );
					m_textHoldForOptions.BeginTweeningQueued( 0.25f );	// fade in
					m_textHoldForOptions.SetTweenZoomY( 1 );
					m_textHoldForOptions.SetTweenDiffuseColor( D3DXCOLOR(1,1,1,1) );
					m_textHoldForOptions.BeginTweeningQueued( 2.0f );	// sleep
					m_textHoldForOptions.BeginTweeningQueued( 0.25f );	// fade out
					m_textHoldForOptions.SetTweenDiffuseColor( D3DXCOLOR(1,1,1,0) );
					m_textHoldForOptions.SetTweenZoomY( 0 );
				}

				m_Menu.TweenOffScreenToBlack( SM_None, false );

				m_Menu.StopTimer();

				this->SendScreenMessage( SM_GoToNextState, 2.5f );
			}
			break;
		case TYPE_SECTION:
			
			break;
		case TYPE_ROULETTE:

			break;
		}
	}

}


void ScreenSelectMusic::MenuBack( const PlayerNumber p )
{
	MUSIC->Stop();

	m_Menu.TweenOffScreenToBlack( SM_GoToPrevState, true );
}

void ScreenSelectMusic::AfterNotesChange( const PlayerNumber p )
{
	if( !GAMESTATE->IsPlayerEnabled(p) )
		return;
	
	m_iSelection[p] = clamp( m_iSelection[p], 0, m_arrayNotes.GetSize()-1 );	// bounds clamping

	Notes* pNotes = m_arrayNotes.GetSize()>0 ? m_arrayNotes[m_iSelection[p]] : NULL;

	GAMESTATE->m_pCurNotes[p] = pNotes;

//	m_BPMDisplay.SetZoomY( 0 );
//	m_BPMDisplay.BeginTweening( 0.2f );
//	m_BPMDisplay.SetTweenZoomY( 1.2f );

	DifficultyClass dc = GAMESTATE->m_PreferredDifficultyClass[p];
	Song* pSong = GAMESTATE->m_pCurSong;
	Notes* m_pNotes = GAMESTATE->m_pCurNotes[p];
	
	if( m_pNotes )
		m_HighScore[p].SetScore( (float)m_pNotes->m_iTopScore );

	m_DifficultyIcon[p].SetFromNotes( pNotes );
	m_FootMeter[p].SetFromNotes( pNotes );
	m_GrooveRadar.SetFromNotes( p, pNotes );
	m_MusicWheel.NotesChanged( p );
}

void ScreenSelectMusic::AfterMusicChange()
{
	m_Menu.StallTimer();

	Song* pSong = m_MusicWheel.GetSelectedSong();
	GAMESTATE->m_pCurSong = pSong;

	m_arrayNotes.RemoveAll();

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
			pSong->GetNotesThatMatch( GAMESTATE->GetCurrentStyleDef()->m_NotesType, m_arrayNotes );
			SortNotesArrayByDifficulty( m_arrayNotes );

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
				for( int i=0; i<m_arrayNotes.GetSize(); i++ )
					if( m_arrayNotes[i]->m_DifficultyClass == GAMESTATE->m_PreferredDifficultyClass[p] )
						m_iSelection[p] = i;

				m_iSelection[p] = clamp( m_iSelection[p], 0, m_arrayNotes.GetSize() ) ;
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

	MUSIC->Stop();

	Song* pSong = m_MusicWheel.GetSelectedSong();
	if( pSong == NULL )
		return;

	CString sSongToPlay = pSong->GetMusicPath();
 
	if( pSong->HasMusic() )
	{
		MUSIC->Load( sSongToPlay );
		MUSIC->Play( true, pSong->m_fMusicSampleStartSeconds, pSong->m_fMusicSampleLengthSeconds );
	}
}

void ScreenSelectMusic::UpdateOptionsDisplays()
{
	for( int p=0; p<NUM_PLAYERS; p++ )
	{
		if( GAMESTATE->IsPlayerEnabled(p) )
		{
			CString s = GAMESTATE->m_PlayerOptions[p].GetString();
			s.Replace( ", ", "\n" );
			m_textPlayerOptions[p].SetText( s );
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
	m_MusicSortDisplay.SetTweenDiffuseColor( D3DXCOLOR(1,1,1,1) );		
}

