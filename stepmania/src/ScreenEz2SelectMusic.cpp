#include "stdafx.h"
/*
-----------------------------------------------------------------------------
 Class: ScreenEz2SelectMusic

 Desc: See header.

 Copyright (c) 2001-2002 by the person(s) listed below.  All rights reserved.
	Chris Danford
	Andrew Livy
-----------------------------------------------------------------------------
*/

/* OKAY!!!
Sod it!
I'll do this when you guys feel that it's OKAY to do it!!!

  - ANDY

*/


/*
#include "ScreenEz2SelectMusic.h"
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


#define BANNER_X				THEME->GetMetricF("ScreenEz2SelectMusic","BannerX")
#define BANNER_Y				THEME->GetMetricF("ScreenEz2SelectMusic","BannerY")
#define BANNER_WIDTH			THEME->GetMetricF("ScreenEz2SelectMusic","BannerWidth")
#define BANNER_HEIGHT			THEME->GetMetricF("ScreenEz2SelectMusic","BannerHeight")
#define STAGE_X					THEME->GetMetricF("ScreenEz2SelectMusic","StageX")
#define STAGE_Y					THEME->GetMetricF("ScreenEz2SelectMusic","StageY")
#define STAGE_ZOOM				THEME->GetMetricF("ScreenEz2SelectMusic","StageZoom")
#define SCORE_FRAME_X( p )		THEME->GetMetricF("ScreenEz2SelectMusic",ssprintf("ScoreFrameP%dX",p+1))
#define SCORE_FRAME_Y( p )		THEME->GetMetricF("ScreenEz2SelectMusic",ssprintf("ScoreFrameP%dY",p+1))
#define SCORE_X( p )			THEME->GetMetricF("ScreenEz2SelectMusic",ssprintf("ScoreP%dX",p+1))
#define SCORE_Y( p )			THEME->GetMetricF("ScreenEz2SelectMusic",ssprintf("ScoreP%dY",p+1))
#define METER_FRAME_X( p )		THEME->GetMetricF("ScreenEz2SelectMusic",ssprintf("MeterFrameP%dX",p+1))
#define METER_FRAME_Y( p )		THEME->GetMetricF("ScreenEz2SelectMusic",ssprintf("MeterFrameP%dY",p+1))
#define METER_X( p )			THEME->GetMetricF("ScreenEz2SelectMusic",ssprintf("MeterP%dX",p+1))
#define METER_Y( p )			THEME->GetMetricF("ScreenEz2SelectMusic",ssprintf("MeterP%dY",p+1))
#define WHEEL_X					THEME->GetMetricF("ScreenEz2SelectMusic","WheelX")
#define WHEEL_Y					THEME->GetMetricF("ScreenEz2SelectMusic","WheelY")
#define HELP_TEXT				THEME->GetMetric("ScreenEz2SelectMusic","HelpText")
#define TIMER_SECONDS			THEME->GetMetricI("ScreenEz2SelectMusic","TimerSeconds")
#define SCORE_CONNECTED_TO_MUSIC_WHEEL	THEME->GetMetricB("ScreenEz2SelectMusic","ScoreConnectedToMusicWheel")

const float TWEEN_TIME		= 0.5f;

const float SIDE_BANNER_ANGLE = 600.0f;
const float SIDE_BANNER_ZOOM = 340.0f;
const float SIDE_BANNER_SPACING = 150.0f;

const ScreenMessage SM_GoToPrevScreen		=	ScreenMessage(SM_User+1);
const ScreenMessage SM_GoToNextScreen		=	ScreenMessage(SM_User+2);



ScreenEz2SelectMusic::ScreenEz2SelectMusic()
{
	LOG->Trace( "ScreenEz2SelectMusic::ScreenEz2SelectMusic()" );


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

	// these guys get loaded SetSong and TweenToSong
	m_BannerNext.SetXY( BANNER_X+220, BANNER_Y );
	m_BannerNext.SetCroppedSize( BANNER_WIDTH, BANNER_HEIGHT );
//	this->AddChild( &m_BannerNext );

	// these guys get loaded SetSong and TweenToSong
	m_BannerPrevious.SetXY( BANNER_X-220, BANNER_Y );
	m_BannerPrevious.SetCroppedSize( BANNER_WIDTH, BANNER_HEIGHT );
//	this->AddChild( &m_BannerPrevious );

	m_MusicWheel.SetXY( WHEEL_X, WHEEL_Y );
	this->AddChild( &m_MusicWheel );

	for( p=0; p<NUM_PLAYERS; p++ )
	{
		if( !GAMESTATE->IsPlayerEnabled((PlayerNumber)p) )
			continue;	// skip

		m_sprHighScoreFrame[p].Load( THEME->GetPathTo("Graphics","select music score frame") );
		m_sprHighScoreFrame[p].StopAnimating();
		m_sprHighScoreFrame[p].SetState( p );
		m_sprHighScoreFrame[p].SetXY( SCORE_X(p), SCORE_Y(p) );
		this->AddChild( &m_sprHighScoreFrame[p] );

		m_HighScore[p].SetXY( SCORE_X(p), SCORE_Y(p) );
		m_HighScore[p].SetZoom( 0.6f );
		m_HighScore[p].SetDiffuse( PlayerToColor(p) );
		this->AddChild( &m_HighScore[p] );
	}	

   
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


ScreenEz2SelectMusic::~ScreenEz2SelectMusic()
{
	LOG->Trace( "ScreenEz2SelectMusic::~ScreenEz2SelectMusic()" );

}

void ScreenEz2SelectMusic::DrawPrimitives()
{

	m_Menu.DrawBottomLayer();
	Screen::DrawPrimitives();
	m_Menu.DrawTopLayer();

	D3DXMATRIX matOldView, matOldProj;

		// save old view and projection
		DISPLAY->GetViewTransform( &matOldView );
		DISPLAY->GetProjectionTransform( &matOldProj );

		// construct view and project matrix


		D3DXMATRIX matNewView;

			D3DXMatrixLookAtLH( 
				&matNewView, 
				&D3DXVECTOR3( CENTER_X+SIDE_BANNER_ANGLE, CENTER_Y, SIDE_BANNER_ZOOM ),
				&D3DXVECTOR3( CENTER_X-SIDE_BANNER_SPACING, CENTER_Y, 0.0f ), 
				&D3DXVECTOR3( 0.0f, -1.0f, 0.0f ) 
				);

		DISPLAY->SetViewTransform( &matNewView );

		D3DXMATRIX matNewProj;
		D3DXMatrixPerspectiveFovLH( &matNewProj, D3DX_PI/4.0f, SCREEN_WIDTH/(float)SCREEN_HEIGHT, 0.0f, 1000.0f );
		DISPLAY->SetProjectionTransform( &matNewProj );
//	}

	m_BannerNext.Draw();

			D3DXMatrixLookAtLH( 
				&matNewView, 
				&D3DXVECTOR3( CENTER_X-SIDE_BANNER_ANGLE, CENTER_Y, SIDE_BANNER_ZOOM ),
				&D3DXVECTOR3( CENTER_X+SIDE_BANNER_SPACING, CENTER_Y, 0.0f ), 
				&D3DXVECTOR3( 0.0f, -1.0f, 0.0f ) 
				);

		DISPLAY->SetViewTransform( &matNewView );

		D3DXMatrixPerspectiveFovLH( &matNewProj, D3DX_PI/4.0f, SCREEN_WIDTH/(float)SCREEN_HEIGHT, 0.0f, 1000.0f );
		DISPLAY->SetProjectionTransform( &matNewProj );

	m_BannerPrevious.Draw();

		// restore old view and projection
		DISPLAY->SetViewTransform( &matOldView );
		DISPLAY->SetProjectionTransform( &matOldProj );


}

void ScreenEz2SelectMusic::TweenOnScreen()
{
	int p;

	for( p=0; p<NUM_PLAYERS; p++ )
	{
		m_sprHighScoreFrame[p].FadeOn( 0, SCORE_CONNECTED_TO_MUSIC_WHEEL?"accelerate right":"accelerate left", TWEEN_TIME );
		m_HighScore[p].FadeOn( 0, SCORE_CONNECTED_TO_MUSIC_WHEEL?"accelerate right":"accelerate left", TWEEN_TIME );
	}

	m_MusicWheel.TweenOnScreen();
}

void ScreenEz2SelectMusic::TweenOffScreen()
{
	int i;

	m_sprBannerFrame.FadeOff( 0, "bounce left", TWEEN_TIME );
	m_Banner.FadeOff( 0, "bounce left", TWEEN_TIME );
	m_BPMDisplay.FadeOff( 0, "bounce left", TWEEN_TIME );
	m_textStage.FadeOff( 0, "bounce left", TWEEN_TIME );
	m_sprCDTitle.FadeOff( 0, "bounce left", TWEEN_TIME );

	for( int p=0; p<NUM_PLAYERS; p++ )
	{
		m_sprDifficultyFrame[p].FadeOff( 0, "fade", TWEEN_TIME );
		m_sprMeterFrame[p].FadeOff( 0, "fade", TWEEN_TIME );
	}


	m_GrooveRadar.TweenOffScreen();

	m_textSongOptions.FadeOff( 0, "fade", TWEEN_TIME );

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

void ScreenEz2SelectMusic::TweenScoreOnAndOffAfterChangeSort()
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

void ScreenEz2SelectMusic::Update( float fDeltaTime )
{
	Screen::Update( fDeltaTime );
}

void ScreenEz2SelectMusic::Input( const DeviceInput& DeviceI, const InputEventType type, const GameInput &GameI, const MenuInput &MenuI, const StyleInput &StyleI )
{
	LOG->Trace( "ScreenEz2SelectMusic::Input()" );
	
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
	if( CodeDetector::DetectAndAdjustOptions(GameI.controller) )
	{
		m_soundOptionsChange.Play();
		UpdateOptionsDisplays();
		return;
	}

	Screen::Input( DeviceI, type, GameI, MenuI, StyleI );	// default input handler
}


void ScreenEz2SelectMusic::EasierDifficulty( PlayerNumber pn )
{
	LOG->Trace( "ScreenEz2SelectMusic::EasierDifficulty( %d )", pn );

	if( !GAMESTATE->IsPlayerEnabled(pn) )
		return;
	if( m_arrayNotes.GetSize() == 0 )
		return;
	if( m_iSelection[pn] == 0 )
		return;

	m_iSelection[pn]--;
	// the user explicity switched difficulties.  Update the preferred difficulty
	GAMESTATE->m_PreferredDifficultyClass[pn] = m_arrayNotes[ m_iSelection[pn] ]->m_DifficultyClass;

	m_soundChangeNotes.Play();

	AfterNotesChange( pn );
}

void ScreenEz2SelectMusic::HarderDifficulty( PlayerNumber pn )
{
	LOG->Trace( "ScreenEz2SelectMusic::HarderDifficulty( %d )", pn );

	if( !GAMESTATE->IsPlayerEnabled(pn) )
		return;
	if( m_arrayNotes.GetSize() == 0 )
		return;
	if( m_iSelection[pn] == m_arrayNotes.GetSize()-1 )
		return;

	m_iSelection[pn]++;
	// the user explicity switched difficulties.  Update the preferred difficulty
	GAMESTATE->m_PreferredDifficultyClass[pn] = m_arrayNotes[ m_iSelection[pn] ]->m_DifficultyClass;

	m_soundChangeNotes.Play();

	AfterNotesChange( pn );
}


void ScreenEz2SelectMusic::HandleScreenMessage( const ScreenMessage SM )
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

void ScreenEz2SelectMusic::MenuLeft( PlayerNumber pn, const InputEventType type )
{
	if( type >= IET_SLOW_REPEAT  &&  INPUTMAPPER->IsButtonDown( MenuInput(pn, MENU_BUTTON_RIGHT) ) )
			return;		// ignore
	
	if( ! m_MusicWheel.WheelIsLocked() )
		MUSIC->Stop();

	m_MusicWheel.PrevMusic();
}


void ScreenEz2SelectMusic::MenuRight( PlayerNumber pn, const InputEventType type )
{
	if( type >= IET_SLOW_REPEAT  &&  INPUTMAPPER->IsButtonDown( MenuInput(pn, MENU_BUTTON_LEFT) ) )
		return;		// ignore

	if( ! m_MusicWheel.WheelIsLocked() )
		MUSIC->Stop();

	m_MusicWheel.NextMusic();
}

void ScreenEz2SelectMusic::MenuStart( PlayerNumber pn )
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
/*	}
	else	// if !bResult
	{
		// a song was selected
		switch( m_MusicWheel.GetSelectedType() )
		{
		case TYPE_SONG:
			{
				if( !m_MusicWheel.GetSelectedSong()->HasMusic() )
				{
					/* TODO: gray these out. 
					 *
					 * XXX: also, make sure they're not selected by roulette */
/*					SCREENMAN->Prompt( SM_None, "ERROR:\n \nThis song does not have a music file\n and cannot be played." );
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
			}
			break;
		case TYPE_SECTION:
			
			break;
		case TYPE_ROULETTE:

			break;
		}
	}

}


void ScreenEz2SelectMusic::MenuBack( PlayerNumber pn )
{
	MUSIC->Stop();

	m_Menu.TweenOffScreenToBlack( SM_GoToPrevScreen, true );
}

void ScreenEz2SelectMusic::AfterNotesChange( PlayerNumber pn )
{
	if( !GAMESTATE->IsPlayerEnabled(pn) )
		return;
	
	m_iSelection[pn] = clamp( m_iSelection[pn], 0, m_arrayNotes.GetSize()-1 );	// bounds clamping

	Notes* pNotes = m_arrayNotes.GetSize()>0 ? m_arrayNotes[m_iSelection[pn]] : NULL;

	GAMESTATE->m_pCurNotes[pn] = pNotes;

//	m_BPMDisplay.SetZoomY( 0 );
//	m_BPMDisplay.BeginTweening( 0.2f );
//	m_BPMDisplay.SetTweenZoomY( 1.2f );

	DifficultyClass dc = GAMESTATE->m_PreferredDifficultyClass[pn];
	Song* pSong = GAMESTATE->m_pCurSong;
	Notes* m_pNotes = GAMESTATE->m_pCurNotes[pn];
	
	if( m_pNotes )
		m_HighScore[pn].SetScore( (float)m_pNotes->m_iTopScore );

	m_DifficultyIcon[pn].SetFromNotes( pNotes );
	m_FootMeter[pn].SetFromNotes( pNotes );
	m_GrooveRadar.SetFromNotes( pn, pNotes );
	m_MusicWheel.NotesChanged( pn );
}

void ScreenEz2SelectMusic::AfterMusicChange()
{
	m_Menu.StallTimer();

	Song* pSong = m_MusicWheel.GetSelectedSong();

	GAMESTATE->m_pCurSong = pSong;

	m_arrayNotes.clear();

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
			m_BannerNext.SetFromGroup( sGroup );	// if this isn't a group, it'll default to the fallback banner
			m_BannerPrevious.SetFromGroup( sGroup );	// if this isn't a group, it'll default to the fallback banner
			m_BPMDisplay.SetBPMRange( 0, 0 );
			m_sprCDTitle.UnloadTexture();
		}
		break;
	case TYPE_SONG:
		{
			pSong->GetNotesThatMatch( GAMESTATE->GetCurrentStyleDef(), PLAYER_1, m_arrayNotes );
			SortNotesArrayByDifficulty( m_arrayNotes );

			m_Banner.SetFromSong( pSong );
			m_BannerNext.SetFromSong( pSong );
			m_BannerPrevious.SetFromSong( pSong );

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
		m_BannerNext.SetRoulette();
		m_BannerPrevious.SetRoulette();
		m_BPMDisplay.SetBPMRange( 0, 0 );
		m_sprCDTitle.UnloadTexture();
		break;
	}

	for( int p=0; p<NUM_PLAYERS; p++ )
	{
		AfterNotesChange( (PlayerNumber)p );
	}
}


void ScreenEz2SelectMusic::PlayMusicSample()
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

void ScreenEz2SelectMusic::UpdateOptionsDisplays()
{
//	m_OptionIcons.Load( GAMESTATE->m_PlayerOptions, &GAMESTATE->m_SongOptions );

//	m_PlayerOptionIcons.Refresh();

//	for( int p=0; p<NUM_PLAYERS; p++ )
//	{
//		m_OptionIconRow[p].Refresh( (PlayerNumber)p  );
//
//		if( GAMESTATE->IsPlayerEnabled(p) )
//		{
//			CString s = GAMESTATE->m_PlayerOptions[p].GetString();
//			s.Replace( ", ", "\n" );
//			m_textPlayerOptions[p].SetText( s );
//		}
//	}

//	CString s = GAMESTATE->m_SongOptions.GetString();
//	s.Replace( ", ", "\n" );
//	m_textSongOptions.SetText( s );
}

void ScreenEz2SelectMusic::SortOrderChanged()
{
	m_MusicSortDisplay.SetState( GAMESTATE->m_SongSortOrder );

	// tween music sort on screen
//	m_MusicSortDisplay.SetEffectGlowing();
	m_MusicSortDisplay.BeginTweening( 0.3f );
	m_MusicSortDisplay.SetTweenDiffuse( D3DXCOLOR(1,1,1,1) );		
}

*/